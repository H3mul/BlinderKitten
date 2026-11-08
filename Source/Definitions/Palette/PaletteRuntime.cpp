#include "JuceHeader.h"
#include "PaletteRuntime.h"
#include "Palette.h"
#include "PaletteManager.h"
#include "PaletteRemap.h"
#include "../Cuelist/Cuelist.h"
#include "../Cue/Cue.h"
#include "../Command/Command.h"
#include "../Command/CommandValue.h"
#include "../Carousel/Carousel.h"
#include "../Mapper/Mapper.h"
#include "../Programmer/Programmer.h"
#include "../../Brain.h"
#include "../../BKEngine.h"
#include "../../UserInputManager.h"

PaletteRuntime& PaletteRuntime::get()
{
	static PaletteRuntime* inst = new PaletteRuntime(); // never freed: a Timer must not be destroyed during static teardown
	return *inst;
}

void PaletteRuntime::stateChanged(Palette* p)
{
	int cur = p->currentState->intValue();
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) {
		p->baseState = cur;
		p->layers.clear();
		p->baseFrom = 0; p->baseT = 1; p->fading = false;
		return;
	}
	if (p->manualWrite) return; // the dominant state written after a layer change

	// external change: acts on the base, the layers stay on top
	double secs = p->getFadeSeconds();
	int prev = p->getBase();
	if (secs > 0 && prev != cur) {
		// a change mid-fade pops to the previous target (prev) and fades from there
		p->baseFrom = prev; p->baseState = cur; p->baseT = 0;
		p->fading = true;
		p->fadeStart = Time::getMillisecondCounterHiRes();
		p->fadeLen = secs * 1000.0;
	}
	else {
		p->baseFrom = 0; p->baseState = cur; p->baseT = 1; p->fading = false;
	}
	if (!p->layers.isEmpty()) { // currentState stays the dominant state
		p->manualWrite = true;
		p->currentState->setValue(jlimit(1, jmax(1, p->states.items.size()), p->getDominantState()));
		p->manualWrite = false;
	}

	BKEngine* e = dynamic_cast<BKEngine*>(Engine::mainEngine);
	if (e != nullptr && e->defaultPaletteId->intValue() == p->registeredId) Brain::getInstance()->defaultValuesNeedRefresh = true;

	get().consumers.remove(p->registeredId); // rescan: the refs may have changed since the last change
	get().markPending(p);
}

int PaletteRuntime::newOwnerId()
{
	return ++get().nextOwner;
}

// After any layer change: dominant state into currentState (no timed fade), then refresh.
void PaletteRuntime::layersChanged(Palette* p)
{
	int target = jlimit(1, jmax(1, p->states.items.size()), p->getDominantState());
	p->manualWrite = true;
	p->currentState->setValue(target);
	p->manualWrite = false;
	markPending(p);
}

void PaletteRuntime::setLayer(Palette* p, int ownerId, int targetState, float weight, int baseIfFirst)
{
	float w = jlimit(0.f, 1.f, weight);
	if (w <= 0) { clearLayer(p, ownerId); return; }
	PaletteRuntime& r = get();
	int n = jmax(1, p->states.items.size());
	if (p->baseState == 0) p->baseState = p->currentState->intValue();
	if (p->layers.isEmpty() && baseIfFirst > 0) {
		p->baseState = jlimit(1, n, baseIfFirst);
		p->baseFrom = 0; p->baseT = 1; p->fading = false;
	}
	int st = jlimit(1, n, targetState);
	for (auto& l : p->layers) {
		if (l.ownerId != ownerId) continue;
		l.state = st; l.weight = w;
		r.layersChanged(p);
		return;
	}
	p->layers.add({ ownerId, st, w, ++r.nextLayerSeq });
	r.layersChanged(p);
}

void PaletteRuntime::clearLayer(Palette* p, int ownerId)
{
	for (int i = p->layers.size(); --i >= 0;) {
		if (p->layers.getReference(i).ownerId == ownerId) {
			p->layers.remove(i);
			get().layersChanged(p);
			return;
		}
	}
}

void PaletteRuntime::clearOwner(int ownerId)
{
	if (PaletteManager::getInstanceWithoutCreating() == nullptr) return;
	for (auto* p : PaletteManager::getInstance()->items) clearLayer(p, ownerId);
}

void PaletteRuntime::markPending(Palette* p)
{
	pending.addIfNotAlreadyThere(p->registeredId);
	if (flushPending.exchange(true)) return;
	MessageManager::callAsync([this]() { flushPending = false; flush(); });
}

void PaletteRuntime::flush()
{
	Array<int> ids = pending;
	pending.clear();
	for (int id : ids) {
		Palette* p = Brain::getInstance()->getPaletteById(id);
		if (p == nullptr) continue;
		refresh(p);
		if (p->fading) fading.addIfNotAlreadyThere(id);
	}
	if (!fading.isEmpty() && !isTimerRunning()) startTimerHz(30);
}

void PaletteRuntime::timerCallback()
{
	double now = Time::getMillisecondCounterHiRes();
	Array<int> ids = fading;
	for (int id : ids) {
		Palette* p = Brain::getInstance()->getPaletteById(id);
		if (p == nullptr || !p->fading) { fading.removeFirstMatchingValue(id); continue; }
		double t = (now - p->fadeStart) / jmax(1.0, p->fadeLen);
		if (t >= 1) {
			p->baseT = 1; p->baseFrom = 0; p->fading = false;
			fading.removeFirstMatchingValue(id);
		}
		else {
			p->baseT = (float)t;
		}
		refresh(p);
	}
	if (fading.isEmpty()) stopTimer();
}

std::shared_ptr<PaletteRuntime::Consumers> PaletteRuntime::getConsumers(Palette* p)
{
	double now = Time::getMillisecondCounterHiRes();
	if (consumers.contains(p->registeredId)) {
		std::shared_ptr<Consumers> c = consumers[p->registeredId];
		if (p->fading || now - c->scanned < 1000) return c;
	}
	return scan(p);
}

// One pass over every command value, once per change (not per fade tick).
std::shared_ptr<PaletteRuntime::Consumers> PaletteRuntime::scan(Palette* p)
{
	Brain* b = Brain::getInstance();
	auto c = std::make_shared<Consumers>();
	c->scanned = Time::getMillisecondCounterHiRes();
	c->paletteIds.add(p->registeredId);

	for (auto it = b->cuelists.begin(); it != b->cuelists.end(); it.next()) {
		Cuelist* cl = it.getValue();
		if (cl == nullptr) continue;
		for (auto* cue : cl->cues.items) {
			for (auto* r : cue->paletteRemap.items) {
				if (r->toId->intValue() == p->registeredId) c->paletteIds.addIfNotAlreadyThere(r->fromId->intValue());
			}
		}
	}
	// a cue that includes another cue's palette refs is a consumer too, whichever cuelist owns the refs
	for (auto it = b->cuelists.begin(); it != b->cuelists.end(); it.next()) {
		Cuelist* cl = it.getValue();
		if (cl == nullptr) continue;
		for (auto* cue : cl->cues.items) {
			if (cue->refsPalettes(c->paletteIds)) { c->cuelists.addIfNotAlreadyThere(cl); break; }
		}
	}

	b->usingLibraries.enter();
	for (auto* cv : b->allCommandValues) {
		if (cv == nullptr || cv->presetOrValue->getValueData() != "palette") continue;
		if (!c->paletteIds.contains(cv->paletteId->intValue())) continue;
		for (ControllableContainer* o = cv->parentContainer.get(); o != nullptr; o = o->parentContainer.get()) {
			if (auto* cl = dynamic_cast<Cuelist*>(o)) { c->cuelists.addIfNotAlreadyThere(cl); break; }
			if (auto* ca = dynamic_cast<Carousel*>(o)) { c->carousels.addIfNotAlreadyThere(ca); break; }
			if (auto* ma = dynamic_cast<Mapper*>(o)) { c->mappers.addIfNotAlreadyThere(ma); break; }
		}
	}
	b->usingLibraries.exit();

	consumers.set(p->registeredId, c);
	return c;
}

void PaletteRuntime::refresh(Palette* p)
{
	Brain* b = Brain::getInstance();
	std::shared_ptr<Consumers> c = getConsumers(p);
	for (auto* cl : c->cuelists) {
		if (b->cuelists.containsValue(cl) && cl->isCuelistOn->boolValue()) cl->refreshPaletteRefs(c->paletteIds);
	}
	for (auto* ca : c->carousels) {
		if (b->carousels.containsValue(ca) && ca->isOn) ca->computeData();
	}
	for (auto* ma : c->mappers) {
		if (b->mappers.containsValue(ma) && ma->isOn) ma->computeData();
	}
	Programmer* prog = UserInputManager::getInstance()->getProgrammer(false);
	if (prog != nullptr) b->pleaseUpdate(prog);
}

void PaletteRuntime::presetRenumbered(int from, int to)
{
	for (auto* p : PaletteManager::getInstance()->items) {
		for (auto* s : p->states.items) {
			for (auto* slot : s->slots) {
				if (slot->intValue() == from) slot->setValue(to);
			}
		}
	}
	presetsChanged();
}

void PaletteRuntime::presetsChanged()
{
	if (PaletteManager::getInstanceWithoutCreating() == nullptr) return;
	for (auto* p : PaletteManager::getInstance()->items) p->applySlotLabels();
}

int PaletteRuntime::defaultPresetId()
{
	BKEngine* e = dynamic_cast<BKEngine*>(Engine::mainEngine);
	if (e == nullptr) return 0;
	int pid = e->defaultPaletteId->intValue();
	if (pid > 0) return Palette::resolveSlot(pid, e->defaultPaletteSlot->intValue(), nullptr);
	return e->defaultPresetId->intValue();
}

int PaletteRuntime::beginHold()
{
	return ++get().nextHold;
}

void PaletteRuntime::holdPalette(int holdId, Palette* p, int appliedState)
{
	PaletteRuntime& r = get();
	r.holds.add({ holdId, p->registeredId, appliedState, p->getBase() });
	p->setStateIndex(appliedState);
}

void PaletteRuntime::releaseHold(int holdId)
{
	PaletteRuntime& r = get();
	Array<int> pids;
	for (auto& h : r.holds) if (h.holdId == holdId) pids.addIfNotAlreadyThere(h.paletteId);

	for (int pid : pids) {
		int idx = -1, top = -1;
		for (int i = 0; i < r.holds.size(); i++) {
			if (r.holds[i].paletteId != pid) continue;
			top = i;
			if (idx < 0 && r.holds[i].holdId == holdId) idx = i;
		}
		if (idx < 0) continue;
		Hold h = r.holds[idx];
		r.holds.remove(idx);

		if (idx != top) { // a later hold sits above: it inherits this hold's prior state
			for (int j = idx; j < r.holds.size(); j++) {
				if (r.holds[j].paletteId == pid) { r.holds.getReference(j).prior = h.prior; break; }
			}
			continue;
		}
		Palette* p = Brain::getInstance()->getPaletteById(pid); // may have been deleted during the hold
		if (p != nullptr && p->getBase() == h.applied) p->setStateIndex(h.prior);
	}
}

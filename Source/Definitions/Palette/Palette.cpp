#include "JuceHeader.h"
#include "Palette.h"
#include "PaletteManager.h"
#include "PaletteRemap.h"
#include "PaletteRuntime.h"
#include "../TimingPreset/TimingPreset.h"
#include "../Cue/Cue.h"
#include "../../Brain.h"

PaletteState::PaletteState(var params) :
	BaseItem(params.getProperty("name", "State"))
{
	itemDataType = "PaletteState";
	for (int i = 0; i < maxSlots; i++) {
		slots.add(addIntParameter("Slot " + String(i + 1), "Preset id of this slot in this state (0 = none)", 0, 0));
	}
}

PaletteState::~PaletteState()
{
}

void PaletteState::applySlotLabels(int count, const StringArray& names)
{
	// Only descriptions and visibility change: renaming a parameter would change its saved control address.
	for (int i = 0; i < slots.size(); i++) {
		String label = "Slot " + String(i + 1);
		if (i < names.size() && names[i].isNotEmpty()) label += " (" + names[i] + ")";
		slots[i]->description = "Preset id of " + label + " in this state (0 = none)";
		if (slots[i]->intValue() > 0 && Brain::getInstance()->getPresetById(slots[i]->intValue()) == nullptr) slots[i]->description += " - MISSING PRESET";
		slots[i]->hideInEditor = i >= count;
	}
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

void PaletteState::onContainerParameterChangedInternal(Parameter* p)
{
	// keep the "missing preset" note of an edited slot current
	if (dynamic_cast<IntParameter*>(p) == nullptr || !slots.contains((IntParameter*)p)) return;
	IntParameter* sp = (IntParameter*)p;
	sp->description = sp->description.upToFirstOccurrenceOf(" - MISSING PRESET", false, false);
	if (sp->intValue() > 0 && Brain::getInstance()->getPresetById(sp->intValue()) == nullptr) sp->description += " - MISSING PRESET";
}

PaletteStateManager::PaletteStateManager() :
	BaseManager("States")
{
	itemDataType = "PaletteState";
	selectItemWhenCreated = false;
}

PaletteStateManager::~PaletteStateManager()
{
}

void PaletteStateManager::addItemInternal(PaletteState* s, var data)
{
	Palette* p = dynamic_cast<Palette*>(parentContainer.get());
	if (p != nullptr) {
		s->applySlotLabels(p->slotCount->intValue(), p->getSlotNameList());
	}
}

Palette::Palette(var params) :
	BaseItem(params.getProperty("name", "Palette")),
	objectType(params.getProperty("type", "Palette").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = false;

	itemDataType = "Palette";

	id = addIntParameter("ID", "ID of this palette", 1, 1);
	userName = addStringParameter("Name", "Name of this palette", "New palette");
	updateName();

	slotCount = addIntParameter("Slots", "Number of slots (roles) of this palette", 2, 1, PaletteState::maxSlots);
	slotNames = addStringParameter("Slot names", "Comma separated slot labels, e.g. base,accent. The slot index is its role.", "base,accent");
	currentState = addIntParameter("Current state", "Position of the active state (1 = first)", 1, 1);
	fadeTime = addFloatParameter("Fade time", "Seconds to fade between states, 0 = snap. Ignored while a timing preset is set and found.", 0, 0);
	timingPresetId = addIntParameter("Timing preset", "ID of a timing preset whose fade time is used (0 = none). Share one preset to control several palettes.", 0, 0);

	states.selectItemWhenCreated = false;
	addChildControllableContainer(&states);

	Brain::getInstance()->registerPalette(this, id->getValue());
}

Palette::~Palette()
{
	Brain::getInstance()->unregisterPalette(this);
}

void Palette::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == userName || p == id) {
		updateName();
	}
	if (p == id) {
		Brain::getInstance()->registerPalette(this, id->getValue(), true);
	}
	if (p == slotCount || p == slotNames) {
		applySlotLabels();
	}
	if (p == currentState) {
		PaletteRuntime::stateChanged(this);
	}
}

void Palette::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	// currentState already refreshes everything through paletteChanged()
	if (c != currentState) Brain::getInstance()->liveEditChanged();
}

void Palette::afterLoadJSONDataInternal()
{
	BaseItem::afterLoadJSONDataInternal();
	baseState = currentState->intValue();
	applySlotLabels();
}

void Palette::updateName()
{
	String n = userName->getValue();
	if (parentContainer != nullptr) {
		PaletteManager* m = dynamic_cast<PaletteManager*>(parentContainer.get());
		if (m != nullptr) m->reorderItems();
	}
	setNiceName(String((int)id->getValue()) + " - " + n);
}

void Palette::applySlotLabels()
{
	StringArray names = getSlotNameList();
	for (auto* s : states.items) {
		s->applySlotLabels(slotCount->intValue(), names);
	}
}

StringArray Palette::getSlotNameList() const
{
	StringArray names;
	names.addTokens(slotNames->stringValue(), ",", "");
	names.trim();
	return names;
}

bool Palette::hasSlot(int slot) const
{
	return slot >= 1 && slot <= slotCount->intValue();
}

int Palette::getPresetId(int slot)
{
	return getPresetIdForState(currentState->intValue(), slot);
}

int Palette::getPresetIdForState(int state, int slot)
{
	if (!hasSlot(slot) || states.items.size() == 0) return 0;
	int idx = jlimit(1, states.items.size(), state);
	PaletteState* s = states.items[idx - 1];
	return s != nullptr ? s->slots[slot - 1]->intValue() : 0;
}

float Palette::getFadeSeconds()
{
	int tid = timingPresetId->intValue();
	if (tid > 0) {
		TimingPreset* t = Brain::getInstance()->getTimingPresetById(tid);
		if (t != nullptr) return t->fadeFrom->floatValue();
	}
	return fadeTime->floatValue();
}

void Palette::setStateIndex(int oneBased)
{
	int n = jmax(1, states.items.size());
	int v = jlimit(1, n, oneBased);
	if (v == currentState->intValue() && v != getBase()) PaletteRuntime::stateChanged(this); // layers dominate: no value change event, the base still moves
	else currentState->setValue(v);
}

Array<std::pair<int, float>> Palette::getBlendStack()
{
	Array<std::pair<int, float>> out;
	float maxW = 0, sumW = 0;
	for (auto& l : layers) { maxW = jmax(maxW, l.weight); sumW += l.weight; }
	float kb = 1.f - maxW, total = kb + sumW;
	auto add = [&out](int state, float share) {
		if (share <= 0) return;
		for (auto& e : out) if (e.first == state) { e.second += share; return; }
		out.add({ state, share });
	};
	int b = getBase();
	if (baseFrom > 0 && baseT < 1.f) { add(baseFrom, kb * (1.f - baseT) / total); add(b, kb * baseT / total); }
	else add(b, kb / total);
	for (auto& l : layers) add(l.state, l.weight / total);
	return out;
}

int Palette::getDominantState()
{
	int b = getBase();
	if (layers.isEmpty()) return b;
	Array<std::pair<int, float>> stack;
	float maxW = 0;
	for (auto& l : layers) maxW = jmax(maxW, l.weight);
	auto add = [&stack](int state, float share) {
		for (auto& e : stack) if (e.first == state) { e.second += share; return; }
		stack.add({ state, share });
	};
	add(b, 1.f - maxW); // the base counts as its target state while it fades
	for (auto& l : layers) add(l.state, l.weight);
	int best = 0; float bestShare = -1; int bestRank = -1;
	for (auto& e : stack) {
		int rank = 0; // 0 = base only, else 1 + newest layer seq on this state
		for (auto& l : layers) if (l.state == e.first) rank = jmax(rank, 1 + l.seq);
		bool better = e.second > bestShare + 1e-5f
			|| (std::abs(e.second - bestShare) <= 1e-5f && (rank > bestRank || (rank == bestRank && e.first < best)));
		if (better) { best = e.first; bestShare = e.second; bestRank = rank; }
	}
	return best;
}

int Palette::findStateIndex(const String& name)
{
	for (int i = 0; i < states.items.size(); i++) {
		if (states.items[i]->niceName == name) return i + 1;
	}
	return 0;
}

Palette* Palette::resolve(int paletteId, int slot, Cue* cue)
{
	Brain* b = Brain::getInstance();
	if (cue != nullptr) {
		for (auto* r : cue->paletteRemap.items) {
			if (r->fromId->intValue() != paletteId) continue;
			Palette* target = b->getPaletteById(r->toId->intValue());
			if (target != nullptr && target->hasSlot(slot)) return target;
			break; // target missing or too short: fall through to the original palette
		}
	}
	return b->getPaletteById(paletteId);
}

int Palette::resolveSlot(int paletteId, int slot, Cue* cue)
{
	Palette* p = resolve(paletteId, slot, cue);
	return p != nullptr ? p->getPresetId(slot) : 0;
}

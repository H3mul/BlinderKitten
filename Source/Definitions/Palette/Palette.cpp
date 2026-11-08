#include "JuceHeader.h"
#include "Palette.h"
#include "PaletteManager.h"
#include "PaletteRemap.h"
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
		slots[i]->hideInEditor = i >= count;
	}
	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
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
		Brain::getInstance()->paletteChanged();
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
	if (!hasSlot(slot) || states.items.size() == 0) return 0;
	int idx = jlimit(1, states.items.size(), currentState->intValue());
	PaletteState* s = states.items[idx - 1];
	return s != nullptr ? s->slots[slot - 1]->intValue() : 0;
}

void Palette::setStateIndex(int oneBased)
{
	int n = jmax(1, states.items.size());
	currentState->setValue(jlimit(1, n, oneBased));
}

int Palette::findStateIndex(const String& name)
{
	for (int i = 0; i < states.items.size(); i++) {
		if (states.items[i]->niceName == name) return i + 1;
	}
	return 0;
}

int Palette::resolveSlot(int paletteId, int slot, Cue* cue)
{
	Brain* b = Brain::getInstance();
	if (cue != nullptr) {
		for (auto* r : cue->paletteRemap.items) {
			if (r->fromId->intValue() != paletteId) continue;
			Palette* target = b->getPaletteById(r->toId->intValue());
			if (target != nullptr && target->hasSlot(slot)) return target->getPresetId(slot);
			break; // target missing or too short: fall through to the original palette
		}
	}
	Palette* p = b->getPaletteById(paletteId);
	return p != nullptr ? p->getPresetId(slot) : 0;
}

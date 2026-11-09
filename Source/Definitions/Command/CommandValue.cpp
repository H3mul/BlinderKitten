/*
  ==============================================================================

    CommandTypeChannel.cpp
    Created: 8 Nov 2021 7:28:28pm
    Author:  No

  ==============================================================================
*/

#include "CommandValue.h"
#include "../ChannelFamily/ChannelFamilyManager.h"
#include "Brain.h"
#include "../Preset/Preset.h"
#include "../Palette/Palette.h"

CommandValue::CommandValue(var params) :
    BaseItem(params.getProperty("name", "Value")),
    objectType(params.getProperty("type", "CommandValue").toString()),
    objectData(params)

{
    presetOrValue = addEnumParameter("Value type", "What kind of value do you want to apply ?");
    presetOrValue-> addOption("Raw Values", "value");
    presetOrValue->addOption("Preset", "preset");
    presetOrValue->addOption("Release", "release");
    presetOrValue->addOption("Palette", "palette");
    presetOrValue->addOption("Palette shift", "paletteshift");

    channelType = addTargetParameter("Channel type", "Type of Channel", ChannelFamilyManager::getInstance());
    channelType->maxDefaultSearchLevel = 2;
    channelType->targetType = TargetParameter::CONTAINER;
    channelType->typesFilter.add("ChannelType");

    // release = addBoolParameter("Release value", "release tracked values", false);
    valueFrom = addFloatParameter("Value", "Value of the first element", 0, 0, 1);
    presetIdFrom = addIntParameter("ID From", "ID of the preset for the first element", 0, 0);

    thru = addBoolParameter("Thru", "Do you want to apply to multiple elements ?", false);

    valueTo = addFloatParameter("Value To", "Value of the last element", 0, 0, 1);
    presetIdTo = addIntParameter("ID To", "ID of the preset for the last element", 0, 0);
    paletteId = addIntParameter("Palette", "ID of the palette", 0, 0);
    slotFrom = addIntParameter("Slot From", "Slot of the palette for the first element", 1, 1, PaletteState::maxSlots);
    slotTo = addIntParameter("Slot To", "Slot of the palette for the last element", 2, 1, PaletteState::maxSlots);

    paletteIdTo = addIntParameter("Palette To", "Last palette id of the range the shift applies to", 0, 0);
    stateName = addStringParameter("State", "Name of the palette state to play, for the palette slot each fixture already has from its cue. Palettes without that state stay as they are.", "");

    symmetry = addBoolParameter("Symmetry", "Apply value with symmetry ?", false);
    randomize = addBoolParameter("Randomize", "Randomize values", false);

    HTPOverride = addBoolParameter("HTP Override", "If checked, HTP channels will be computed as LTP.", false);

    stepSize = addFloatParameter("Step size", "Size of the step",1,0);
    stepSize-> hideInEditor = true;

    Brain::getInstance()->allCommandValues.add(this);

    updateDisplay();
};

CommandValue::~CommandValue()
{
    Brain::getInstance()->allCommandValues.removeAllInstancesOf(this);

};

void CommandValue::updateDisplay() 
{
    bool val = presetOrValue->getValue() == "value";
    bool prst = presetOrValue->getValue() == "preset";
    bool pal = presetOrValue->getValue() == "palette";
    bool th = thru->getValue();
    bool rel = presetOrValue->getValue() == "release";

    channelType->hideInEditor = !(val || rel);
    valueFrom->hideInEditor = !val;
    valueTo->hideInEditor = !(th && val);

    presetIdFrom->hideInEditor = !(prst);
    presetIdTo->hideInEditor = !(th && prst);
    bool shift = presetOrValue->getValue() == "paletteshift";
    paletteId->hideInEditor = !(pal || shift);
    paletteIdTo->hideInEditor = !(th && shift);
    slotFrom->hideInEditor = !pal;
    slotTo->hideInEditor = !(th && pal);
    stateName->hideInEditor = !shift;

    symmetry->hideInEditor = !th || randomize->boolValue() || shift;
    randomize->hideInEditor = !th || shift;

    thru->hideInEditor = rel;
    HTPOverride->hideInEditor = rel || shift;

    stepSize->hideInEditor = !shouldShowStepSize;

    queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

void CommandValue::onContainerParameterChangedInternal(Parameter* p) {
    if (p == thru || p == presetOrValue) {
        updateDisplay();
    }
    if (p == presetOrValue) {
        if (presetOrValue->getValue() == "value") {
            channelType->typesFilter.removeString("ChannelFamily");
        }
        if (presetOrValue->getValue() == "release") { 
            channelType->typesFilter.add("ChannelFamily");
        }
    }
}

void CommandValue::afterLoadJSONDataInternal() {
    BaseItem::afterLoadJSONDataInternal();
    updateDisplay();
}


bool CommandValue::usesPreset() {
    String v = presetOrValue->getValue().toString();
    return v == "preset" || v == "palette";
}

void CommandValue::resolvePalette(Cue* cue, Preset*& from, Preset*& to) {
    Brain* b = Brain::getInstance();
    int pid = paletteId->intValue();
    from = b->getPresetById(Palette::resolveSlot(pid, slotFrom->intValue(), cue), true);
    to = b->getPresetById(Palette::resolveSlot(pid, slotTo->intValue(), cue), true);
    if (from != nullptr) from->computeValues();
    if (to != nullptr) to->computeValues();

    auto buildStack = [&](Palette* p, int slot, PresetStack& out) {
        out.clear();
        if (p == nullptr) return;
        auto stack = p->getBlendStack();
        // one entry is the plain current state, unless the base fade is at its very start (only the old state has a share)
        if (stack.size() < 2 && (stack.isEmpty() || stack[0].first == p->currentState->intValue())) return;
        for (auto& e : stack) {
            Preset* pr = b->getPresetById(p->getPresetIdForState(e.first, slot), true);
            if (pr != nullptr) pr->computeValues();
            out.add({ pr, e.second });
        }
    };
    buildStack(Palette::resolve(pid, slotFrom->intValue(), cue), slotFrom->intValue(), stackFrom);
    buildStack(Palette::resolve(pid, slotTo->intValue(), cue), slotTo->intValue(), stackTo);
}

// Per channel, share-weighted average over the states that have it. A channel appears when those shares reach half;
// a release (-1) counts as a state: released when release shares are at least half of the shares that have the channel, else ignored.
static void mixPaletteValues(HashMap<ChannelType*, float>& out, const CommandValue::PresetStack& stack, SubFixture* sf) {
    Array<std::shared_ptr<HashMap<ChannelType*, float>>> pvs;
    Array<ChannelType*> keys;
    for (auto& e : stack) {
        auto pv = e.first != nullptr ? e.first->getSubFixtureValues(sf) : nullptr;
        pvs.add(pv);
        if (pv != nullptr) for (auto it = pv->begin(); it != pv->end(); it.next()) keys.addIfNotAlreadyThere(it.getKey());
    }
    out.clear();
    const float eps = 1e-4f;
    for (auto* k : keys) {
        float have = 0, rel = 0, sum = 0;
        for (int i = 0; i < stack.size(); i++) {
            if (pvs[i] == nullptr || !pvs[i]->contains(k)) continue;
            float v = pvs[i]->getReference(k), sh = stack[i].second;
            have += sh;
            if (v < 0) rel += sh; else sum += sh * v;
        }
        if (have < 0.5f - eps) continue;
        if (2 * rel >= have - eps) out.set(k, -1.f);
        else out.set(k, sum / (have - rel));
    }
}

void CommandValue::applyPaletteBlend(SubFixture* sf, HashMap<ChannelType*, float>& valuesFrom, HashMap<ChannelType*, float>& valuesTo) {
    if (!stackFrom.isEmpty()) mixPaletteValues(valuesFrom, stackFrom, sf);
    if (!stackTo.isEmpty()) mixPaletteValues(valuesTo, stackTo, sf);
}

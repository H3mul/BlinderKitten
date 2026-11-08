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
    paletteId->hideInEditor = !pal;
    slotFrom->hideInEditor = !pal;
    slotTo->hideInEditor = !(th && pal);

    symmetry->hideInEditor = !th || randomize->boolValue();
    randomize->hideInEditor = !th;

    thru->hideInEditor = rel;
    HTPOverride->hideInEditor = rel;

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

    blendPrevFrom = blendPrevTo = nullptr;
    blendAmount = 1.f;
    Palette* pf = Palette::resolve(pid, slotFrom->intValue(), cue);
    Palette* pt = Palette::resolve(pid, slotTo->intValue(), cue);
    if (pf != nullptr && pf->isBlending()) {
        blendPrevFrom = b->getPresetById(pf->getPresetIdForState(pf->fromState, slotFrom->intValue()), true);
        blendAmount = pf->blend;
    }
    if (pt != nullptr && pt->isBlending()) {
        blendPrevTo = b->getPresetById(pt->getPresetIdForState(pt->fromState, slotTo->intValue()), true);
        blendAmount = pt->blend;
    }
    if (blendPrevFrom != nullptr) blendPrevFrom->computeValues();
    if (blendPrevTo != nullptr) blendPrevTo->computeValues();
}

// Per channel: interpolate previous to current by blendAmount; a channel in only one state, or a release (-1), snaps at the midpoint.
static void mixPaletteValues(HashMap<ChannelType*, float>& cur, Preset* prev, SubFixture* sf, float blend) {
    std::shared_ptr<HashMap<ChannelType*, float>> pv = prev != nullptr ? prev->getSubFixtureValues(sf) : nullptr;
    Array<ChannelType*> drop;
    Array<std::pair<ChannelType*, float>> set;
    for (auto it = cur.begin(); it != cur.end(); it.next()) {
        ChannelType* k = it.getKey();
        float c = it.getValue();
        if (pv != nullptr && pv->contains(k)) {
            float a = pv->getReference(k);
            set.add({ k, (a < 0 || c < 0) ? (blend < 0.5f ? a : c) : a + (c - a) * blend });
        }
        else if (blend < 0.5f) {
            drop.add(k);
        }
    }
    if (pv != nullptr && blend < 0.5f) {
        for (auto it = pv->begin(); it != pv->end(); it.next()) {
            if (!cur.contains(it.getKey())) set.add({ it.getKey(), it.getValue() });
        }
    }
    for (auto* k : drop) cur.remove(k);
    for (auto& s : set) cur.set(s.first, s.second);
}

void CommandValue::applyPaletteBlend(SubFixture* sf, HashMap<ChannelType*, float>& valuesFrom, HashMap<ChannelType*, float>& valuesTo) {
    if (blendAmount >= 1.f) return;
    if (blendPrevFrom != nullptr) mixPaletteValues(valuesFrom, blendPrevFrom, sf, blendAmount);
    if (blendPrevTo != nullptr) mixPaletteValues(valuesTo, blendPrevTo, sf, blendAmount);
}

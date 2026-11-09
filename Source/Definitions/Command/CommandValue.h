/*
  ==============================================================================

    ChannelType.h
    Created: 7 Nov 2021 7:40:48pm
    Author:  No

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class Cue;
class Preset;
class SubFixture;
class ChannelType;

class CommandValue:
    public BaseItem,
    public Inspectable::InspectableListener
{
    public:
    CommandValue(var params = var());
    ~CommandValue();

    String objectType;
    var objectData;

    EnumParameter* presetOrValue;

    TargetParameter* channelType;

    BoolParameter* release;

    IntParameter* presetIdFrom;
    FloatParameter* valueFrom;

    BoolParameter* thru;

    IntParameter* presetIdTo;
    IntParameter* paletteId;
    IntParameter* slotFrom;
    IntParameter* slotTo;
    StringParameter* stateName; // palette ref: named state override (empty = live)
    FloatParameter* valueTo;
    BoolParameter* symmetry;
    BoolParameter* randomize;

    BoolParameter* HTPOverride;

    bool shouldShowStepSize = false;
    FloatParameter* stepSize;

    void updateDisplay();

    // true for the Preset and Palette value types
    bool usesPreset();
    // Palette value type: resolve slotFrom/slotTo to presets (cue = owning cue for its remap, may be null)
    void resolvePalette(Cue* cue, Preset*& from, Preset*& to);
    // While a palette blends (layers or a timed fade): replace the values with the share-weighted mix of the stack (no-op otherwise). Call after the preset values are filled.
    void applyPaletteBlend(SubFixture* sf, HashMap<ChannelType*, float>& valuesFrom, HashMap<ChannelType*, float>& valuesTo);
    typedef Array<std::pair<Preset*, float>> PresetStack; // preset (may be null) and share, shares sum to 1
    PresetStack stackFrom, stackTo;                        // empty = no blend

    void afterLoadJSONDataInternal();
    void onContainerParameterChangedInternal(Parameter* p);

};
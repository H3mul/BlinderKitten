/*
  ==============================================================================

    Palette.h

    A palette holds named slots (base, accent...) and named states. Each state
    holds one preset id per slot. Commands reference a palette slot instead of
    a preset id, and the palette's current state picks the preset.

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class Cue;
class Palette;

class PaletteState :
    public BaseItem
{
public:
    PaletteState(var params = var());
    ~PaletteState();

    static const int maxSlots = 8;
    Array<IntParameter*> slots;

    void applySlotLabels(int count, const StringArray& names);
    void onContainerParameterChangedInternal(Parameter* p) override;

    String getTypeString() const override { return "PaletteState"; }
    static PaletteState* create(var params) { return new PaletteState(params); }
};

class PaletteStateManager :
    public BaseManager<PaletteState>
{
public:
    PaletteStateManager();
    ~PaletteStateManager();

    void addItemInternal(PaletteState* s, var data) override;
};

class Palette :
    public BaseItem
{
public:
    Palette(var params = var());
    virtual ~Palette();

    String objectType;
    var objectData;

    IntParameter* id;
    int registeredId = 0;
    StringParameter* userName;

    IntParameter* slotCount;
    StringParameter* slotNames;   // comma separated labels, index = slot - 1
    IntParameter* currentState;   // 1-based
    PaletteStateManager states;

    // v3 base + blend layers. Runtime only (not saved, plain members so the live-edit feedback hook never sees them).
    FloatParameter* fadeTime;     // seconds, 0 = snap
    IntParameter* timingPresetId; // 0 = none; a found timing preset's fade wins over fadeTime
    struct Layer { int ownerId; int state; float weight; int seq; };
    Array<Layer> layers;          // fader layers on top of the base
    int baseState = 0;            // 0 = not set yet, follows currentState
    int baseFrom = 0;             // timed fade of the base: state fading out, 0 = no fade
    float baseT = 1.f;            // 0 = baseFrom, 1 = baseState
    bool fading = false;
    bool manualWrite = false;     // currentState written as the dominant state (not an external change)
    double fadeStart = 0, fadeLen = 0;
    float getFadeSeconds();
    int getBase() { return baseState > 0 ? baseState : currentState->intValue(); }
    // Spring mix: (state, share) with the shares summing to 1, aggregated per state.
    Array<std::pair<int, float>> getBlendStack();
    // State with the largest share (base counts as its target state); ties: layer over base, newest layer, lower state.
    int getDominantState();
    bool isBlending() { return !layers.isEmpty() || (baseFrom > 0 && baseT < 1.f); }
    int getPresetIdForState(int state, int slot);

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;
    void afterLoadJSONDataInternal() override;
    void updateName();
    void applySlotLabels();

    StringArray getSlotNameList() const;
    bool hasSlot(int slot) const;               // slot is 1-based
    int getPresetId(int slot);                  // preset id of the current state, 0 if none
    void setStateIndex(int oneBased);           // external change: acts on the base, clamps to the existing states
    int findStateIndex(const String& name);     // 1-based, 0 if no state has this name

    // Palette a ref resolves to, applying the cue's remap with fall-through (nullptr if none).
    static Palette* resolve(int paletteId, int slot, Cue* cue);
    // Preset id for a palette ref (current state), applying the cue's remap with fall-through.
    static int resolveSlot(int paletteId, int slot, Cue* cue);

    String getTypeString() const override { return objectType; }
    static Palette* create(var params) { return new Palette(params); }
};

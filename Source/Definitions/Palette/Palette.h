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

    void onContainerParameterChangedInternal(Parameter* p) override;
    void afterLoadJSONDataInternal() override;
    void updateName();
    void applySlotLabels();

    StringArray getSlotNameList() const;
    bool hasSlot(int slot) const;               // slot is 1-based
    int getPresetId(int slot);                  // preset id of the current state, 0 if none
    void setStateIndex(int oneBased);           // clamps to the existing states
    int findStateIndex(const String& name);     // 1-based, 0 if no state has this name

    // Preset id for a palette ref, applying the cue's remap with fall-through.
    static int resolveSlot(int paletteId, int slot, Cue* cue);

    String getTypeString() const override { return objectType; }
    static Palette* create(var params) { return new Palette(params); }
};

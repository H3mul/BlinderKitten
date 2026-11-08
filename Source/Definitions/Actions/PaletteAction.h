#pragma once
#include "JuceHeader.h"
#include "../../Common/Action/Action.h"

class PaletteAction :
    public Action
{
public:
    PaletteAction(var params = var());
    ~PaletteAction();

    enum ActionType { PAL_SET, PAL_NEXT, PAL_PREVIOUS, PAL_SHIFT_ALL, PAL_SHIFT_ALL_HOLD, PAL_BLEND, PAL_BLEND_ALL };
    ActionType actionType;

    IntParameter* paletteId = nullptr;
    IntParameter* stateIndex = nullptr;
    StringParameter* stateName = nullptr;

    IntParameter* stateA = nullptr;   // blend: state at fader 0
    IntParameter* stateB = nullptr;   // blend: state at fader 1

    // blend faders own one layer per palette under this id (dropped with the action); not saved
    int ownerId = 0;

    // momentary shift: holds on PaletteRuntime's per-palette stacks, released on value 0 (not saved)
    int holdId = 0;

    void triggerInternal() override;
    void setValueInternal(var value, String origin, int incrementIndex, bool isRelative) override;

    void shiftAll();
    void blendAll(float value);

    static PaletteAction* create(var params) { return new PaletteAction(params); }
};

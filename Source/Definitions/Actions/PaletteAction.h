#pragma once
#include "JuceHeader.h"
#include "../../Common/Action/Action.h"

class PaletteAction :
    public Action
{
public:
    PaletteAction(var params = var());
    ~PaletteAction();

    enum ActionType { PAL_SET, PAL_NEXT, PAL_PREVIOUS, PAL_SHIFT_ALL, PAL_SHIFT_ALL_HOLD };
    ActionType actionType;

    IntParameter* paletteId = nullptr;
    IntParameter* stateIndex = nullptr;
    StringParameter* stateName = nullptr;

    // momentary shift: states recorded on press, restored on release (not saved)
    Array<int> snapshotIds;
    Array<int> snapshotStates;

    void triggerInternal() override;
    void setValueInternal(var value, String origin, int incrementIndex, bool isRelative) override;

    void shiftAll();
    void restoreSnapshot();

    static PaletteAction* create(var params) { return new PaletteAction(params); }
};

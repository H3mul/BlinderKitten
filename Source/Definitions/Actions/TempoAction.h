/*
  ==============================================================================

    TempoAction.h
    Actions that drive a Tempo from virtual buttons, faders and cue tasks

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../../Common/Action/Action.h"

class TempoAction :
    public Action
{
public:
    TempoAction(var params = var());
    ~TempoAction();

    enum ActionType { TEMPO_TAP, TEMPO_SET, TEMPO_SETAT, TEMPO_SCALEAT, TEMPO_DOUBLE, TEMPO_HALF, TEMPO_RESETSCALE, TEMPO_DOWNBEAT, TEMPO_DOUBLE_HOLD, TEMPO_HALF_HOLD, TEMPO_SCALEAT_HOLD };
    ActionType actionType;
    IntParameter* targetId;
    FloatParameter* fromValue;
    FloatParameter* toValue;
    FloatParameter* atValue;
    bool held = false; // momentary press in progress, so a stray release doesn't reset the scale

    void triggerInternal() override;
    void setValueInternal(var value, String origin, int incrementIndex, bool isRelative) override;

    var getValue() override;

    static TempoAction* create(var params) { return new TempoAction(params); }
};

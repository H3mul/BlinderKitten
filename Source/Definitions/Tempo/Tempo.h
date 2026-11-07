/*
  ==============================================================================

    Tempo.h
    Shared BPM that bound effects and carousels follow

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class Tempo :
    public BaseItem
{
public:
    Tempo(var params = var());
    virtual ~Tempo();

    String objectType;
    var objectData;

    IntParameter* id;
    int registeredId = 0;
    StringParameter* userName;
    FloatParameter* bpm;
    FloatParameter* scale;
    FloatParameter* effectiveBpm;
    BoolParameter* resyncOnEveryTap;
    Trigger* tapBtn;
    Trigger* downbeatBtn;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void updateName();
    void updateEffectiveBpm();
    void triggerTriggered(Trigger* t) override;
    void tap();
    void downbeat();

    double lastTap = 0;
    Array<double> tapHistory;

    String getTypeString() const override { return objectType; }
    static Tempo* create(var params) { return new Tempo(params); }
};

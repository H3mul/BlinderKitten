/*
  ==============================================================================

    InputFilter.h
    One row of a virtual fader's input filter stack (bk-input-filters):
    damping, ease / lerp, log scale, curve preset.

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Multiplicator/MultiplicatorLinkManager.h"

class InputFilter :
    public BaseItem
{
public:
    InputFilter(var params = var());
    virtual ~InputFilter();

    enum Type { DAMPING, EASE, LOG, CURVE };

    EnumParameter* type;
    EnumParameter* dampMode;   // exponential / constant rate
    EnumParameter* easeShape;
    EnumParameter* easeDir;
    EnumParameter* logShape;
    FloatParameter* time;      // seconds
    FloatParameter* timeDown;  // seconds, 0 = same as time (damping only)
    FloatParameter* amount;    // log strength
    IntParameter* presetId;    // curve preset
    MultiplicatorLinkManager timeMult; // scales Time / Time down (damping, ease)

    // dt in seconds; the first call after reset() takes the input as the current output.
    float process(float in, double dt);
    bool isSettled() const;
    void reset() { initialised = false; }

    void onContainerParameterChangedInternal(Parameter* p) override;
    void updateDisplay();

    String getTypeString() const override { return "InputFilter"; }
    static InputFilter* create(var params) { return new InputFilter(params); }

private:
    bool initialised = false;
    float y = 0;       // current output (damping, ease)
    float lastIn = 0;
    float easeStart = 0;
    double easeT = 0;

    float ease(float p) const;
    float effTime(bool down = false);
};

class InputFilterManager :
    public BaseManager<InputFilter>
{
public:
    InputFilterManager(String name = "Input filters");
    ~InputFilterManager() {}
};

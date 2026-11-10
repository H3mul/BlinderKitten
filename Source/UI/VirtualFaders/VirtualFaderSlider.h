/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Common/CommonIncludes.h"
#include "Definitions/InputFilter/InputFilter.h"

class VirtualFaderCol;

class VirtualFaderSlider :
    public BaseItem,
    public Timer
{
public:
    VirtualFaderSlider(var params = var());
    virtual ~VirtualFaderSlider();

    String objectType;
    var objectData;

    bool isFader = false;
    int validIncrementIndex = 0;

    EnumParameter * targetType;
    IntParameter * targetId;
    StringParameter* customText;
    EnumParameter * cuelistAction;
    EnumParameter * effectAction;
    EnumParameter* carouselAction;
    EnumParameter* mapperAction;
    EnumParameter* trackerAction;
    FloatParameter* maxSpeed;

    ActionManager actionManager;
    InputFilterManager filters;

    // input filter stack (bk-input-filters)
    bool filtersActive();
    float getDisplayValue();
    void timerCallback() override;

    void onContainerParameterChangedInternal(Parameter* p);
    void updateName();
    void updateDisplay();
    float getTargetValue();

    void moved(float value, String origin, int incrementIndex, bool isRelative);
    void applyValue(float value, String origin, bool isRelative);
    void runFilters(double dt, bool apply = true);
    void feedback(var value, String origin);

    void released();
    String getBtnText(String columnType);

    String getTypeString() const override { return objectType; }
    bool isAllowedToMove(String origin, int incrementIndex, float newValue);

    float rawInput = 0;
    float lastWritten = -1;
    String lastOrigin;
    bool filtersPrimed = false;
    bool filtersRunning = false;
    double lastTick = 0;

    VirtualFaderCol* parentColumn = nullptr;
    bool checkParentColumn();

    static VirtualFaderSlider* create(var params) { return new VirtualFaderSlider(params); }
};

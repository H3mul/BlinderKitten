/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================

*/

#pragma once
#include "JuceHeader.h"
#include "../Command/CommandSelectionManager.h"
#include "CarouselRow.h"
#include "Definitions/Multiplicator/MultiplicatorLinkManager.h"
class CarouselStep;
class SubFixtureChannel;

class Carousel :
    public BaseItem
{
public:
    Carousel(var params = var());
    virtual ~Carousel();

    String objectType;
    var objectData;

    IntParameter* id;
    IntParameter* layerId;
    int registeredId = 0;
    StringParameter* userName;
    void onContainerParameterChangedInternal(Parameter* p);
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c);
    void updateName();

    BoolParameter* isCarouselOn;
    bool isOn = false;
    bool isFlashing = false;
    bool isSwopping = false;

    Trigger* startBtn;
    Trigger* stopBtn;

    BoolParameter* autoStartAndStop;
    FloatParameter* speed;
    FloatParameter* sizeValue;
    FloatParameter* flashValue;
    String currentSizeController = "";
    String nextSizeController = "";
    float lastSize = 0;
    BaseManager<CarouselRow> rows;

    FloatParameter* fadeInTime;
    FloatParameter* fadeOutTime;
    double TSStartFadeIn = 0;
    double TSStartFadeOut = 0;
    double TSEndFadeIn = 0;
    double TSEndFadeOut = 0;

    IntParameter* soloPool;

    FloatParameter* currentPosition;
    double TSLastUpdate;
    double totalElapsed;

    HashMap<SubFixtureChannel*, std::shared_ptr<Array<CarouselRow*>>> chanToCarouselRow;
    CriticalSection isComputing;

    double lastTapTempo;
    Array<double> tapTempoHistory;
    Trigger* tapTempoBtn;
    IntParameter* beatPerCycle;

    MultiplicatorLinkManager speedMult;
    MultiplicatorLinkManager sizeMult;
    float currentSizeMult = 1;

    String getTypeString() const override { return objectType; }

    void triggerTriggered(Trigger* t);
    void update(double now);
    void computeData();
    bool computed = false;
    bool computing = false;
    void pleaseComputeIfRunning();
    void userStart(bool useFadeIn = true);
    void start(bool useFadeIn = true);
    void stop();
    void kill();
    float applyToChannel(SubFixtureChannel* fc, float currentVal, double now);
    void tapTempo();

    void flash(bool on, bool swop = false);
    bool userPressedGo = false;

    static Carousel* create(var params) { return new Carousel(params); }
    
};

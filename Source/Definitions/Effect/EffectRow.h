/*
  ==============================================================================

    ChannelType.h
    Created: 7 Nov 2021 7:40:48pm
    Author:  No

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Command/CommandSelectionManager.h"
#include "EffectParam.h"
using namespace siv;

class EffectRow:
    public BaseItem
{
    public:
    EffectRow(var params = var());
    ~EffectRow();

    String objectType;
    var objectData;

    ControllableContainer curveContainer;
    BaseManager<EffectParam> paramContainer;

    EnumParameter* curvePresetOrValue;
    Automation curve;
    FloatParameter* curveOrigin;
    IntParameter* presetId;
    StringParameter* paletteState; // non-empty: palette row, drives blend layers toward this state instead of channels
    int paletteOwnerId = 0;
    float lastPaletteWeight = -1;
    void clearPaletteLayers();
    FloatParameter* chaserFade;
    IntParameter* chaserBuddying;
    FloatParameter* speed;
    EnumParameter* direction;

    CommandSelectionManager selection;

    HashMap<SubFixture*, double> computedPositions;
    CriticalSection isComputing;

    PerlinNoise perlinNoise;

    double maxOffset = 0;

    Effect* parentEffect = nullptr;
    void checkParentEffect();

    void computeData();

    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*);
    void updateDisplay();


};
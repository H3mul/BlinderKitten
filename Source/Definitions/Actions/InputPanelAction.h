/*
  ==============================================================================

    CuelistAction.h
    Created: 3 Feb 2022 10:15:35pm
    Author:  No

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include"../../Common/Action/Action.h"

class InputPanelAction :
    public Action
{
public:
    InputPanelAction(var params);
    ~InputPanelAction();

    enum ActionType { IP_PRESS, IP_GM, IP_KILLCL, IP_OFFCL, IP_STOPFX, IP_STOPCAR, IP_RANDOMSEED, IP_SELECTWINDOW, IP_SAVE, IP_LOADALLCUELISTS, IP_BLACKOUTTEMP, IP_BLACKOUTTOGGLE, IP_UPDATE, IP_REPLACE, IP_LOADCONTENT};
    ActionType actionType;
    EnumParameter* targetButton;
    IntParameter* randomSeed;
    IntParameter* targetEncoder;
    EnumParameter* targetWindow;

    void triggerInternal() override;
    void setValueInternal(var value, String origin, int incrementIndex, bool isRelative) override;

    var getValue();

    static InputPanelAction* create(var params) { return new InputPanelAction(params); }

};
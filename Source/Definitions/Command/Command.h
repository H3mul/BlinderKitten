/*
  ==============================================================================
 
    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "CommandValue.h"
#include "CommandTiming.h"
#include "CommandValueManager.h"
#include "CommandSelectionManager.h"
#include "MoveInBlackManager.h"

class CommandSelectionManager;

class CommandTiming;
class ChannelValue;
class SubFixtureChannel;
class Cuelist;
class Cue;
class Palette;
class CommandSelection;
class ChannelType;

class Command:
    public BaseItem
{
public:
    Command(var params = var());
    virtual ~Command();

    String objectType;
    var objectData;

    BoolParameter* doNotTrack;
    MoveInBlackManager moveInBlack;
    FloatParameter* moveInBlackDelay;
    BoolParameter* useValuesAsPath;

    CommandSelectionManager selection;
    CommandValueManager values;
    CommandTiming timing;

    HashMap<SubFixtureChannel*, std::shared_ptr<ChannelValue>> computedValues;
    HashMap<SubFixtureChannel*, CommandValue*> channelToCommandValue;
    CriticalSection isComputing;

    struct ShiftTable { CommandValue* cv; Palette* palette; int state; double built; HashMap<SubFixtureChannel*, float> values; };
    OwnedArray<ShiftTable> shiftTables; // short-lived cache (250 ms) of shifted values
    CriticalSection shiftLock;
    ShiftTable* buildShiftTable(CommandValue* cv, Palette* p, int state);
    bool toDelete = false;

    String getTypeString() const override { return objectType; }
    void parameterValueChanged(Parameter* p);
    void afterLoadJSONDataInternal();
    void updateDisplay();

    double maxTiming;
    void computeValues();
    void computeValues(Cuelist* cl, Cue* c, Programmer* p = nullptr);

    // Palette shift (carousel): the value this command would give fc if the palette it refs for fc were in the state named stateName,
    // using this command's own slots and thru. -1 = no result (not a palette value, palette outside [idLo, idHi] (0 = all), no such state, or computing).
    float shiftValueFor(SubFixtureChannel* fc, const String& stateName, int idLo, int idHi);
    Cue* lastCue = nullptr; // callingCue of the last compute (not owned), for the palette remap

    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) override;
    void triggerTriggered(Trigger* t);
    static Command* create(var params) { return new Command(params); }

    StringArray getCommandAsTexts(bool useNames = false);
    StringArray getCommandSelectionAsTexts();
    String getUserName(String type, int id);
    //Trigger* viewCommandBtn;
    Trigger* explodeSelectionBtn;
    String formatValue(float f);

    bool userCanPressPlusOrMinus = true;
    bool userCanPressSelectionType = true;
    bool userCanPressSubSelection = false;
    bool userCanPressValueType = false;
    bool userCanPressTimingType = false;
    bool userCanPressThru = false;
    bool userCanPressNumber = false;
    bool userCanPressValue = false;
    bool userCanPressSym = false;
    bool userCanHaveAnotherCommand = false;
    void userCantPress();
    void userPress(String s);

    CommandSelection* currentUserSelection = nullptr;
    CommandValue* currentUserValue = nullptr;
    Parameter* currentUserTarget = nullptr;
    BoolParameter* currentUserThru = nullptr;

    float getChannelValue(ChannelType* t, bool thru = false);
    String lastTarget = "";

    void explodeSelection(bool takeOutputValue = false);
    void cleanUnused();

};


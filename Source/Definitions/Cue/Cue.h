/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Command/Command.h"
#include "Task.h"
#include "Command/MoveInBlackManager.h"
#include "CueTimecode.h"
#include "CueTimecodeManager.h"
#include "Definitions/Palette/PaletteRemap.h"

class Cuelist;

// One extra source cue for "Use cue commands": applied after the main source, in list order, before the cue's own commands.
class CueReuse :
    public BaseItem
{
public:
    CueReuse(var params = var());
    ~CueReuse() {}

    TargetParameter* cue;
    BoolParameter* useHostTiming;

    void onContainerParameterChangedInternal(Parameter* p) override;
    String getTypeString() const override { return "CueReuse"; }
    static CueReuse* create(var params) { return new CueReuse(params); }
};

class Cue:
    public BaseItem,
    public ChangeBroadcaster
{
public:
    Cue(var params = var());
    virtual ~Cue();

    String objectType;
    var objectData;

    Cuelist* parentCuelist = nullptr;

    BaseManager<Command> commands;
    BaseManager<Task> tasks;
    BaseManager<Task> tasksOffCue;
    FloatParameter* id;

    StringParameter* cueText;
    StringParameter* goText;
    StringParameter* chapterText;

    EnumParameter* autoFollow = nullptr;
    FloatParameter* autoFollowTiming;
    FloatParameter* autoFollowCountDown;
    BoolParameter* canBeRandomlyCalled;
    BoolParameter* loadWindowBreakLine;
    double TSAutoFollowStart;
    double TSAutoFollowEnd;

    BoolParameter* releaseCurrentTracking;
    TargetParameter* reuseCue;

    ControllableContainer actionsContainer;
    Trigger* goBtn;
    Trigger* loadBtn;
    Trigger* replaceBtn;
    Trigger* mergeBtn;
    Trigger* createAfterBtn;
    Trigger* createBeforeBtn;
    Trigger* cleanUnusedCommandsBtn;
    Trigger* regroupCommandsBtn;
    Trigger* takeSelectionBtn;

    MoveInBlackManager moveInBlack;
    FloatParameter* moveInBlackDelay;

    bool isComputing = false;

    HashMap<SubFixtureChannel*, std::shared_ptr<ChannelValue>> computedValues;
    HashMap<SubFixtureChannel*, Command*> channelToCommand;
    CriticalSection csComputing;

    Array<Command*> commandHistory;

    void triggerTriggered(Trigger* t) override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c);
    void controllableStateChanged(Controllable* c) override;

    void computeValues();
    // cuelistOverride: cuelist whose timing the commands use instead of this cue's own cuelist (host timing)
    void computeValues(Array<Cue*> history, Cue* callingCue, Cuelist* cuelistOverride = nullptr);
    void update(double now);
    void go();
    void go(float forcedDelay, float forcedFade);
    void off();
    void off(float forcedDelay, float forcedFade);
    void endTransition();

    void cleanUnused();
    void regroupCommands();
    void takeSelection(Programmer* p);

    void loadContent(Programmer *p);
    void loadContentWithTracking(Programmer* p);
    void replaceContent(Programmer* p);
    void mergeContent(Programmer* p);

    void writeTimeStamp();

    Array<Task*> getTasks();
    Array<Task*> getTasks(Array<Cue*> history);

    Array<Task*> getOffTasks();
    Array<Task*> getOffTasks(Array<Cue*> history);

    void runTasks(float forcedDelay = -1, float forcedFade = -1);
    void runOffTasks(float forcedDelay =-1, float forcedFade = -1);

    String getCommandsText(bool useName);
    String getCommandsText(bool useName, Array<Cue*> history);

    String getTypeString() const override { return objectType; }
    static Cue* create(var params) { return new Cue(params); }
    double maxTiming = 0;
    //void updateChannels();

    FloatParameter* htpInDelay;
    FloatParameter* htpOutDelay;
    FloatParameter* ltpDelay;
    FloatParameter* htpInFade;
    FloatParameter* htpOutFade;
    FloatParameter* ltpFade;

    ControllableContainer timingContainer;
    StringParameter* lastTriggeredTS;

    void onContainerNiceNameChanged() override;

    void checkId();
    void checkParentCuelist();

    void timecodeUpdated();

    CueTimecodeManager timecode;

    // palette ref remap for this cue's commands (A -> B)
    BaseManager<PaletteRemap> paletteRemap;

    // more source cues after `reuseCue`; later sources win per channel, own commands win over all
    BaseManager<CueReuse> moreReuse;
    Array<Cue*> getReuseSources(Array<bool>* hostTiming = nullptr);
    // true if this cue, or a cue it includes, has a palette ref to one of these palette ids
    // this cue, then the cues it includes (depth first, cycle-safe): their palette remaps apply to this cue, first match wins
    Array<Cue*> getRemapCues(Array<Cue*> history = Array<Cue*>());
    bool refsPalettes(const Array<int>& paletteIds, Array<Cue*> history = Array<Cue*>());

};


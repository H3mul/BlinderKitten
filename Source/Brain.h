/*
  ==============================================================================

    Brain.h
    Created: 14 Nov 2021 10:42:56pm
    Author:  No

  ==============================================================================
*/
#pragma once

#include "JuceHeader.h"
#include "Common/SizeOwners.h"

class SubFixture;
class SubFixtureChannel;
class FixtureType;
class Fixture;
class Group;
class Command;
class CommandSelection;
class CommandValue;
class Cue;
class Cuelist;
class Preset;
class Programmer;
class CurvePreset;
class TimingPreset;
class Palette;
class Tempo;
class BKPathPreset;
class Effect;
class Stamp;
class Carousel;
class Mapper;
class RunningTask;
class Task;
class Layout;
class Tracker;
class SelectionMaster;
class Bundle;
class DMXArtNetDevice;
class SubFixturePhysicalChannel;


class Brain :
    public Thread
{
public:
    juce_DeclareSingleton(Brain, true);
    Brain();
    ~Brain();
    void clear();
    void clearUpdates();

    juce::ThreadPool pool;

    HashMap<int, SubFixture*>subFixtures;
    HashMap<int, Group*>groups;
    HashMap<int, Fixture*>fixtures;
    HashMap<int, Preset*>presets;
    HashMap<int, Cuelist*>cuelists;
    HashMap<int, Programmer*>programmers;
    HashMap<int, CurvePreset*>curvePresets;
    HashMap<int, TimingPreset*>timingPresets;
    HashMap<int, Palette*>palettes;
    HashMap<int, Tempo*>tempos;
    HashMap<int, BKPathPreset*>bkPathPresets;
    HashMap<int, Effect*>effects;
    HashMap<int, Stamp*>stamps;
    HashMap<int, Carousel*>carousels;
    HashMap<int, Mapper*>mappers;
    HashMap<int, Tracker*>trackers;
    HashMap<int, SelectionMaster*>selectionMasters;
    HashMap<int, Layout*>layouts;
    HashMap<int, Bundle*>bundles;

    Array<Cuelist*> cuelistPoolUpdating;
    Array<Cuelist*> cuelistPoolWaiting;
    Array<SubFixtureChannel*> subFixtureChannelPoolUpdating;
    Array<SubFixtureChannel*> subFixtureChannelPoolWaiting;
    Array<Cue*> cuePoolUpdating;
    Array<Cue*> cuePoolWaiting;
    Array<Programmer*> programmerPoolUpdating;
    Array<Programmer*> programmerPoolWaiting;
    Array<Effect*> effectPoolUpdating;
    Array<Effect*> effectPoolWaiting;
    Array<Stamp*> stampPoolUpdating;
    Array<Stamp*> stampPoolWaiting;
    Array<Carousel*> carouselPoolUpdating;
    Array<Carousel*> carouselPoolWaiting;
    Array<Mapper*> mapperPoolUpdating;
    Array<Mapper*> mapperPoolWaiting;
    Array<Tracker*> trackerPoolUpdating;
    Array<Tracker*> trackerPoolWaiting;
    Array<SelectionMaster*> selectionMasterPoolUpdating;
    Array<SelectionMaster*> selectionMasterPoolWaiting;

    Array<SubFixtureChannel*> swoppableChannels;
    Array<SubFixtureChannel*> grandMasterChannels;
    Array<Cuelist*> swoppedCuelists;
    Array<Effect*> swoppedEffects;
    Array<Stamp*> swoppedStamps;
    Array<Carousel*> swoppedCarousels;

    Array<Command*> allCommands;
    Array<CommandSelection*> allCommandSelections;
    Array<CommandValue*> allCommandValues;
    Array<SubFixture*> allSubfixtures;
    Array<SubFixtureChannel*> allSubfixtureChannels;
    Array<SubFixtureChannel*> allVirtualSubfixtureChannels;
    Array<SubFixtureChannel*> allDMXSubfixtureChannels;

    Array<SubFixturePhysicalChannel*> dirtyPhysicalChannels;

    Array<DMXArtNetDevice*> syncedArtnetDevices;

    Random mainRandom;

    double now;
    bool loadingIsRunning = false;
    bool isClearing = false;

    bool reconstructVirtuals = false;
    bool virtualFadersNeedUpdate = false;
    bool virtualFaderButtonsNeedUpdate = false;
    bool virtualButtonsNeedUpdate = false;
    bool encoderValuesNeedRefresh = false;
    bool cuelistGridNeedRefresh = false;
    bool carouselGridNeedRefresh = false;
    bool effectGridNeedRefresh = false;
    bool mapperGridNeedRefresh = false;
    bool layoutViewerNeedRefresh = false;
    bool layoutViewerNeedRepaint = false;
    bool defaultValuesNeedRefresh = false;

    void run() override;
    void brainLoop();
    bool skipLoop = false;

    double TSBundles = 0;
    CriticalSection usingLibraries;
    CriticalSection usingPools;
    CriticalSection usingPhysicalChannels;
    CriticalSection usingTasksCollection;
    void registerSubFixture(SubFixture* f, int id);
    void unregisterSubFixture(SubFixture* f);
    void registerFixture(Fixture* f, int id);
    void unregisterFixture(Fixture* f);
    void registerGroup(Group* f, int id);
    void unregisterGroup(Group* f);
    void registerPreset(Preset* p, int id);
    void unregisterPreset(Preset* p);
    void registerCuelist(Cuelist* c, int id, bool swap = false);
    void unregisterCuelist(Cuelist* c);
    void registerProgrammer(Programmer* p, int id, bool swap = false);
    void unregisterProgrammer(Programmer* p);
    void registerCurvePreset(CurvePreset* p, int id, bool swap = false);
    void unregisterCurvePreset(CurvePreset* p);
    void registerTimingPreset(TimingPreset* p, int id, bool swap = false);
    void unregisterTimingPreset(TimingPreset* p);
    void registerPalette(Palette* p, int id, bool swap = false);
    void unregisterPalette(Palette* p);
    void registerTempo(Tempo* p, int id, bool swap = false);
    void unregisterTempo(Tempo* p);
    void registerBKPathPreset(BKPathPreset* p, int id, bool swap = false);
    void unregisterBKPathPreset(BKPathPreset* p);
    void registerEffect(Effect* p, int id, bool swap = false);
    void unregisterEffect(Effect* p);
    void registerStamp(Stamp* p, int id, bool swap = false);
    void unregisterStamp(Stamp* p);
    void registerCarousel(Carousel* p, int id, bool swap = false);
    void unregisterCarousel(Carousel* p);
    void registerMapper(Mapper* p, int id, bool swap = false);
    void unregisterMapper(Mapper* p);
    void registerLayout(Layout* p, int id, bool swap = false);
    void unregisterLayout(Layout* p);
    void registerTracker(Tracker* p, int id, bool swap = false);
    void unregisterTracker(Tracker* p);
    void registerSelectionMaster(SelectionMaster* p, int id, bool swap = false);
    void unregisterSelectionMaster(SelectionMaster* p);
    void registerBundle(Bundle* p, int id, bool swap = false);
    void unregisterBundle(Bundle* p);

    // cue pairing: effects, carousels, mappers and trackers scaled by the cuelist that started them
    float ownerFactor(const SizeOwners& o);
    bool hasRunningOwner(const SizeOwners& o);
    void noteTaskStart(const String& type, int id, int cuelistId);
    void ownedLevelChanged(int cuelistId);
    void stopOwned(const String& type, int id, int cuelistId, float forcedFade);
    void releaseOwner(int cuelistId);

    void pleaseUpdate(Cuelist* c);
    void pleaseUpdate(SubFixtureChannel* f);
    void pleaseUpdate(Cue* c);
    void pleaseUpdate(Programmer* p);
    void pleaseUpdate(Effect* f);
    void pleaseUpdate(Stamp* s);
    void pleaseUpdate(Carousel* c);
    void pleaseUpdate(Mapper* c);
    void pleaseUpdate(Tracker* c);
    void pleaseUpdate(SelectionMaster* c);

    void setPhysicaChannelDirty(SubFixturePhysicalChannel* sfpc);

    void updateAllChannels();

    void grandMasterChanged();

    // Effect master: global multiplier on effect size (transient, never saved)
    float effectMaster = 1;
    String effectMasterController = "";
    void setEffectMaster(float value, bool all, int fromId, int toId);

    //void updateChannel(SubFixtureChannel* fc);

    SubFixture* getSubFixtureById(int id);
    Fixture* getFixtureById(int id, bool followIfAnother = false);
    Group* getGroupById(int id, bool followIfAnother = false);
    Preset* getPresetById(int id, bool followIfAnother = false);
    Cuelist* getCuelistById(int id);
    Programmer* getProgrammerById(int id);
    CurvePreset* getCurvePresetById(int id);
    TimingPreset* getTimingPresetById(int id, bool followIfAnother = false);
    Palette* getPaletteById(int id);
    // A palette changed state: running cuelists, carousels, mappers and the programmer re-resolve (coalesced, message thread).
    void paletteChanged();
    void refreshPaletteUsers(bool onlyLive = false);
    // Something a cue plays was edited: cuelists with liveEdit on (and carousels/mappers while any is armed) re-resolve, coalesced.
    void liveEditChanged();
    std::atomic<bool> liveRefreshPending{ false };
    std::atomic<bool> paletteRefreshPending{ false };
    Tempo* getTempoById(int id);
    BKPathPreset* getBKPathPresetById(int id, bool followIfAnother = false);
    Effect* getEffectById(int id);
    Stamp* getStampById(int id);
    Carousel* getCarouselById(int id);
    Mapper* getMapperById(int id);
    Layout* getLayoutById(int id);
    Tracker* getTrackerById(int id);
    SelectionMaster* getSelectionMasterById(int id);
    Bundle* getBundleById(int id);

    void swoppedCuelist(Cuelist* c);
    void swoppedEffect(Effect* c);
    void swoppedStamp(Stamp* c);
    void swoppedCarousel(Carousel* c);
    void unswoppedCuelist(Cuelist* c);
    void unswoppedEffect(Effect* c);
    void unswoppedStamp(Stamp* c);
    void unswoppedCarousel(Carousel* c);
    bool isSwopping = false;

    static float symPosition(int index, int nElements);

    OwnedArray<RunningTask> runningTasks;
    int currentTaskId = 0;
    int newTaskId();
    void startTask(Task* t, double startTime, int cuelistId, float forcedDelay = -1, float forceFade = -1);
    void stopTasks(int cuelistId, int taskId);

    void goAllLoadedCuelists();
    void killAllCuelists();
    void offAllCuelists();
    void stopAllEffects();
    void stopAllStamps();
    void stopAllCarousels();

    void resetRandomSeed(int seed);

    void showWindow(String name);

    void loadRunningCuelistsInProgrammer();
    void loadRunningCuelistsInProgrammer(Programmer* p);
    
    void getSoloPoolItems(int poolId, Array<Cuelist*>& cuelists, Array<Effect*>& effects, Array<Carousel*> &carousels, Array<Stamp*> &stamps);

    void soloPoolCuelistStarted(int poolId, Cuelist* c);
    void soloPoolEffectStarted(int poolId, Effect* c);
    void soloPoolStampStarted(int poolId, Stamp* c);
    void soloPoolCarouselStarted(int poolId, Carousel* c);
    void soloPoolCheck(int poolId, String excludeType, int excludeId);
    void soloPoolRandom(int poolId);
    void soloPoolStart(int poolId, float forcedFade = -1);
    void soloPoolStop(int poolId, float forcedFade = -1);

    HashMap<int, String> lastSoloPoolItemType;
    HashMap<int, int> lastSoloPoolItemId;

    void soloPoolTapTempo(int poolId);
    void soloPoolSetSize(int poolId, float val, bool size, bool HTP, bool LTP, bool flash);
    void soloPoolSetSpeed(int poolId, float val, bool tapTempoDivide = false);
    void soloPoolSetSizeRel(int poolId, float val, bool size, bool HTP, bool LTP, bool flash);
    void soloPoolSetSpeedRel(int poolId, float val, bool tapTempoDivide = false);
    void soloPoolSpeedMult(int poolId, float mult);

    void soloPoolSetFadeIn(int poolId, float val);
    void soloPoolSetFadeOut(int poolId, float val);

    void soloPoolFlash(int poolId, bool on, bool timing, bool swop = false);



    void replaceFixtureIdEverywhere(int from, int to);
    void replaceGroupIdEverywhere(int from, int to);
    void replacePresetIdEverywhere(int from, int to);



};

/*
  ==============================================================================

    Brain.cpp
    Created: 14 Nov 2021 10:42:56pm
    Author:  No

  ==============================================================================
*/


#include "Brain.h"
#include "Definitions/ChannelValue.h"
#include "UI/VirtualFaders/VirtualFaderColGrid.h"
#include "UI/VirtualButtons/VirtualButtonGrid.h"
#include "UI/Encoders.h"
#include "UserInputManager.h"
#include "UI/GridView/CuelistGridView.h"
#include "UI/GridView/EffectGridView.h"
#include "UI/GridView/CarouselGridView.h"
#include "UI/ConductorInfos.h"

juce_ImplementSingleton(Brain);

Brain::Brain() :
    Thread("BKBrain") 
{
    startThread(juce::Thread::Priority::highest);
}

Brain :: ~Brain() {
    clear();
    signalThreadShouldExit();
    waitForThreadToExit(100);
    clearSingletonInstance();
}

void Brain::clear()
{
    isClearing = true;
    ScopedLock lock(usingCollections);
    subFixtures.clear();
    groups.clear();
    fixtures.clear();
    presets.clear();
    cuelists.clear();
    programmers.clear();
    curvePresets.clear();
    timingPresets.clear();
    effects.clear();
    carousels.clear();
    mappers.clear();
    cuelistPoolUpdating.clear();
    cuelistPoolWaiting.clear();
    subFixtureChannelPoolUpdating.clear();
    subFixtureChannelPoolWaiting.clear();
    cuePoolUpdating.clear();
    cuePoolWaiting.clear();
    programmerPoolUpdating.clear();
    programmerPoolWaiting.clear();
    effectPoolUpdating.clear();
    effectPoolWaiting.clear();
    carouselPoolUpdating.clear();
    carouselPoolWaiting.clear();
    mapperPoolUpdating.clear();
    mapperPoolWaiting.clear();
    swoppableChannels.clear();
    isClearing = false;
}

void Brain::clearUpdates()
{
    ScopedLock lock(usingCollections);
    cuelistPoolUpdating.clear();
    cuelistPoolWaiting.clear();
    subFixtureChannelPoolUpdating.clear();
    subFixtureChannelPoolWaiting.clear();
    cuePoolUpdating.clear();
    cuePoolWaiting.clear();
    programmerPoolUpdating.clear();
    programmerPoolWaiting.clear();
    effectPoolUpdating.clear();
    effectPoolWaiting.clear();
    carouselPoolUpdating.clear();
    carouselPoolWaiting.clear();
    mapperPoolUpdating.clear();
    mapperPoolWaiting.clear();
}

void Brain::run() {
    while(!threadShouldExit()) {
        //double begin = Time::getMillisecondCounterHiRes();
        brainLoop();
        //double end = Time::getMillisecondCounterHiRes();
        //double delta = end-begin;

        wait(10);
    }
}

void Brain::brainLoop() {
    if (skipLoop) {
        return;
    }


    if (pleaseClearProgrammer) {
        pleaseClearProgrammer = false;
        if (Programmer* p = getProgrammerById(1)) {
            MessageManager::callAsync([this,p]() {p->clearCurrent(); });
        }
    }


    now = Time::getMillisecondCounterHiRes();
    if (cuePoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < cuePoolWaiting.size(); i++) {
            cuePoolUpdating.add(cuePoolWaiting[i]);
        }
        cuePoolWaiting.clear();
    }
    for (int i = 0; i < cuePoolUpdating.size(); i++) {
        cuePoolUpdating[i]->update(now);
    }
    cuePoolUpdating.clear();

    if (cuelistPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < cuelistPoolWaiting.size(); i++) {
            cuelistPoolUpdating.add(cuelistPoolWaiting[i]);
        }
        cuelistPoolWaiting.clear();
    }
    for (int i = 0; i < cuelistPoolUpdating.size(); i++) {
        cuelistPoolUpdating[i]->update();
    }
    cuelistPoolUpdating.clear();

    if (effectPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < effectPoolWaiting.size(); i++) {
            effectPoolUpdating.add(effectPoolWaiting[i]);
        }
        effectPoolWaiting.clear();
    }
    for (int i = 0; i < effectPoolUpdating.size(); i++) {
        effectPoolUpdating[i]->update(now);
    }
    effectPoolUpdating.clear();

    if (carouselPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < carouselPoolWaiting.size(); i++) {
            carouselPoolUpdating.add(carouselPoolWaiting[i]);
        }
        carouselPoolWaiting.clear();
    }
    for (int i = 0; i < carouselPoolUpdating.size(); i++) {
        carouselPoolUpdating[i]->update(now);
    }
    carouselPoolUpdating.clear();

    if (mapperPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < mapperPoolWaiting.size(); i++) {
            mapperPoolUpdating.add(mapperPoolWaiting[i]);
        }
        mapperPoolWaiting.clear();
    }
    for (int i = 0; i < mapperPoolUpdating.size(); i++) {
        mapperPoolUpdating[i]->update(now);
    }
    mapperPoolUpdating.clear();

    if (trackerPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < trackerPoolWaiting.size(); i++) {
            trackerPoolUpdating.add(trackerPoolWaiting[i]);
        }
        trackerPoolWaiting.clear();
    }
    for (int i = 0; i < trackerPoolUpdating.size(); i++) {
        trackerPoolUpdating[i]->update(now);
    }
    trackerPoolUpdating.clear();

    if (programmerPoolWaiting.size() > 0) {
        ScopedLock lock(usingCollections);
        for (int i = 0; i < programmerPoolWaiting.size(); i++) {
            programmerPoolUpdating.add(programmerPoolWaiting[i]);
        }
        programmerPoolWaiting.clear();
    }
    for (int i = 0; i < programmerPoolUpdating.size(); i++) {
        if (!programmerPoolUpdating[i]->isDeleted) {
            programmerPoolUpdating[i]->update(now);
        }
    }
    programmerPoolUpdating.clear();

    Command* currentCommand = nullptr;
    Array<SubFixtureChannel* > modifiedSF;

    for (SubFixtureChannel* sfc : allSubfixtureChannels) {
        if (sfc != nullptr && !sfc->isDeleted && sfc->isDirty) {
            modifiedSF.add(sfc);
            sfc->isDirty = false;
            sfc->updateVal(now);
        }
    }

    if (modifiedSF.size() > 0) {
        if (!loadingIsRunning && UserInputManager::getInstance()->currentProgrammer != nullptr) {
            UserInputManager::getInstance()->currentProgrammer->computing.enter();
            if (UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
                for (int i = 0; i < modifiedSF.size() && !encoderValuesNeedRefresh; i++) {
                    SubFixtureChannel* sfc = modifiedSF[i];
                    encoderValuesNeedRefresh = encoderValuesNeedRefresh || UserInputManager::getInstance()->currentProgrammer->currentUserCommand->selection.computedSelectedSubFixtures.contains(sfc->parentSubFixture);
                }
            }
            UserInputManager::getInstance()->currentProgrammer->computing.exit();
        }

    }

    if (runningTasks.size()>0) 
    {
        for (int i = 0; i < runningTasks.size(); i++) {
            runningTasks[i]->update(now);
        }
        ScopedLock lock(usingTasksCollection);
        for (int i = runningTasks.size() - 1; i >= 0; i--) {
            if (runningTasks[i]->isEnded) {
                runningTasks.remove(i);
            }
        }
    }

    if (reconstructVirtuals) {
        reconstructVirtuals = false;
        MessageManager::callAsync([this]() {
            VirtualFaderColGrid::getInstance()->fillCells();
            VirtualButtonGrid::getInstance()->fillCells();
            });
    }

    if (virtualFadersNeedUpdate) {
        virtualFadersNeedUpdate = false;
        MessageManager::callAsync([this]() {VirtualFaderColGrid::getInstance()->updateSlidersValues(); });
    }

    if (virtualFaderButtonsNeedUpdate) {
        virtualFaderButtonsNeedUpdate = false;
        MessageManager::callAsync([this]() {VirtualFaderColGrid::getInstance()->updateButtons(); });
    }

    if (virtualButtonsNeedUpdate) {
        virtualButtonsNeedUpdate = false;
        MessageManager::callAsync([this]() {VirtualButtonGrid::getInstance()->updateButtons(); });
    }

    if (encoderValuesNeedRefresh) {
        encoderValuesNeedRefresh = false;
        MessageManager::callAsync([this]() {Encoders::getInstance()->updateEncodersValues(); });
    }

    if (cuelistGridNeedRefresh) {
        cuelistGridNeedRefresh = false;
        MessageManager::callAsync([this]() {CuelistGridView::getInstance()->updateButtons(); });
    }

    if (carouselGridNeedRefresh) {
        carouselGridNeedRefresh = false;
        MessageManager::callAsync([this]() {CarouselGridView::getInstance()->updateButtons(); });
    }

    if (effectGridNeedRefresh) {
        effectGridNeedRefresh = false;
        MessageManager::callAsync([this]() {EffectGridView::getInstance()->updateButtons(); });
    }

    if (defaultValuesNeedRefresh) {
        defaultValuesNeedRefresh = false;
        int presetId = dynamic_cast<BKEngine*>(Engine::mainEngine)->defaultPresetId->intValue();
        Preset* def = getPresetById(presetId);
        if (def != nullptr) def->computeValues();

        for (SubFixture* sf : allSubfixtures) {
            float presetVal = -1;
            std::shared_ptr < HashMap<ChannelType*, float>> presetValues = def != nullptr ? def->getSubFixtureValues(sf) : nullptr;
            for (SubFixtureChannel* sfc : sf->channelsContainer) {
                float presetValue = presetValues != nullptr && presetValues->contains(sfc->channelType) ? presetValues->getReference(sfc->channelType) : -1;
                if (presetValue != sfc->defaultPresetValue) {
                    sfc->defaultPresetValue = presetValue;
                    sfc->isDirty = true;
                }
            }
        }
    }
    //double delta = Time::getMillisecondCounterHiRes() - now;
    //LOG(delta);

}

void Brain::registerSubFixture(SubFixture* f, int id) {
    int askedId = id;
    if (subFixtures.getReference(id) == f) { return; }
    if (subFixtures.containsValue(f)) {
        subFixtures.removeValue(f);
    }
    bool idIsOk = false;
    while (!idIsOk) {
        if (subFixtures.contains(id) && subFixtures.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    subFixtures.set(id, f);
    f->subId =id;
    if (id != askedId) {
    }
}

void Brain::unregisterSubFixture(SubFixture* f) {
    if (subFixtures.containsValue(f)) {
        subFixtures.removeValue(f);
    }
}

void Brain::registerFixture(Fixture* p, int id, bool swap) {
    int askedId = id;
    if (fixtures.getReference(id) == p) { return; }
    if (fixtures.containsValue(p)) {
        fixtures.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (fixtures.contains(id) && fixtures.getReference(id) != nullptr) {
            Fixture* presentItem = fixtures.getReference(id);
            unregisterFixture(p);
            registerFixture(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (fixtures.contains(id) && fixtures.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    fixtures.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}

void Brain::unregisterFixture(Fixture* d) {
    if (fixtures.containsValue(d)) {
        fixtures.removeValue(d);
    }
}

void Brain::registerGroup(Group* p, int id, bool swap) {
    int askedId = id;
    if (groups.getReference(id) == p) { return; }
    if (groups.containsValue(p)) {
        groups.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (groups.contains(id) && groups.getReference(id) != nullptr) {
            Group* presentItem = groups.getReference(id);
            unregisterGroup(p);
            registerGroup(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (groups.contains(id) && groups.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    groups.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}

void Brain::unregisterGroup(Group* g) {
    if (groups.containsValue(g)) {
        groups.removeValue(g);
    }
}

void Brain::registerPreset(Preset* p, int id, bool swap) {
    int askedId = id;
    if (presets.getReference(id) == p) { return; }
    if (presets.containsValue(p)) {
        presets.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (presets.contains(id) && presets.getReference(id) != nullptr ){
            Preset* presentItem = presets.getReference(id);
            unregisterPreset(p);
            registerPreset(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (presets.contains(id) && presets.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    presets.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}

void Brain::unregisterPreset(Preset* p) {
    if (presets.containsValue(p)) {
        presets.removeValue(p);
    }
}

void Brain::registerCuelist(Cuelist* p, int id, bool swap) {
    int conductorId = dynamic_cast<BKEngine*>(Engine::mainEngine)->conductorCuelistId->intValue();
    bool relinkConductor = false;

    int askedId = id;
    //LOG("register Cuelist");
    if (cuelists.getReference(id) == p) { return; }

    if (cuelists.contains(conductorId) && cuelists.getReference(conductorId) == p) {
        relinkConductor = true;
    }

    if (cuelists.containsValue(p)) {
        cuelists.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (cuelists.contains(id) && cuelists.getReference(id) != nullptr) {
            Cuelist* presentItem = cuelists.getReference(id);
            unregisterCuelist(p);
            registerCuelist(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (cuelists.contains(id) && cuelists.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    cuelists.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }

    if (id == dynamic_cast<BKEngine*>(Engine::mainEngine)->conductorCuelistId->intValue()) {
        ConductorInfos::getInstance()->linkFadeSlider();
    }

    if (conductorId == id || relinkConductor) {
        ConductorInfos::getInstance()->linkFadeSlider();
    }
}

void Brain::unregisterCuelist(Cuelist* c) {
    if (cuelists.containsValue(c)) {
        cuelists.removeValue(c);
    }
    reconstructVirtuals = true;
}

void Brain::registerProgrammer(Programmer* p, int id, bool swap) {
    int askedId = id;
    if (programmers.getReference(id) == p) { return; }
    if (programmers.containsValue(p)) {
        programmers.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (programmers.contains(id) && programmers.getReference(id) != nullptr) {
            Programmer* presentItem = programmers.getReference(id);
            unregisterProgrammer(p);
            registerProgrammer(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (programmers.contains(id) && programmers.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    programmers.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterProgrammer(Programmer* c) {
    if (programmers.containsValue(c)) {
        programmers.removeValue(c);
    }
}

void Brain::registerCurvePreset(CurvePreset* p, int id, bool swap) {
    int askedId = id;
    if (curvePresets.getReference(id) == p) { return; }
    if (curvePresets.containsValue(p)) {
        curvePresets.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (curvePresets.contains(id) && curvePresets.getReference(id) != nullptr) {
            CurvePreset* presentItem = curvePresets.getReference(id);
            unregisterCurvePreset(p);
            registerCurvePreset(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (curvePresets.contains(id) && curvePresets.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    curvePresets.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterCurvePreset(CurvePreset* c) {
    if (curvePresets.containsValue(c)) {
        curvePresets.removeValue(c);
    }
}

void Brain::registerTimingPreset(TimingPreset* p, int id, bool swap) {
    int askedId = id;
    if (timingPresets.getReference(id) == p) { return; }
    if (timingPresets.containsValue(p)) {
        timingPresets.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (timingPresets.contains(id) && timingPresets.getReference(id) != nullptr) {
            TimingPreset* presentItem = timingPresets.getReference(id);
            unregisterTimingPreset(p);
            registerTimingPreset(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (timingPresets.contains(id) && timingPresets.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    timingPresets.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterTimingPreset(TimingPreset* c) {
    if (timingPresets.containsValue(c)) {
        timingPresets.removeValue(c);
    }
}

void Brain::registerEffect(Effect* p, int id, bool swap) {
    int askedId = id;
    if (effects.getReference(id) == p) { return; }
    if (effects.containsValue(p)) {
        effects.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (effects.contains(id) && effects.getReference(id) != nullptr) {
            Effect* presentItem = effects.getReference(id);
            unregisterEffect(p);
            registerEffect(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (effects.contains(id) && effects.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    effects.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterEffect(Effect* c) {
    if (effects.containsValue(c)) {
        effects.removeValue(c);
    }
    reconstructVirtuals = true;
}

void Brain::registerCarousel(Carousel* p, int id, bool swap) {
    int askedId = id;
    if (carousels.getReference(id) == p) { return; }
    if (carousels.containsValue(p)) {
        carousels.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (carousels.contains(id) && carousels.getReference(id) != nullptr) {
            Carousel* presentItem = carousels.getReference(id);
            unregisterCarousel(p);
            registerCarousel(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (carousels.contains(id) && carousels.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    carousels.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterCarousel(Carousel* c) {
    if (carousels.containsValue(c)) {
        carousels.removeValue(c);
    }
    reconstructVirtuals = true;
}

void Brain::registerMapper(Mapper* p, int id, bool swap) {
    int askedId = id;
    if (mappers.getReference(id) == p) { return; }
    if (mappers.containsValue(p)) {
        mappers.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (mappers.contains(id) && mappers.getReference(id) != nullptr) {
            Mapper* presentItem = mappers.getReference(id);
            unregisterMapper(p);
            registerMapper(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (mappers.contains(id) && mappers.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    mappers.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterMapper(Mapper* c) {
    if (mappers.containsValue(c)) {
        mappers.removeValue(c);
    }
    reconstructVirtuals = true;
}

void Brain::registerLayout(Layout* p, int id, bool swap) {
    int askedId = id;
    if (layouts.getReference(id) == p) { return; }
    if (layouts.containsValue(p)) {
        layouts.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (layouts.contains(id) && layouts.getReference(id) != nullptr) {
            Layout* presentItem = layouts.getReference(id);
            unregisterLayout(p);
            registerLayout(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (layouts.contains(id) && layouts.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    layouts.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterLayout(Layout* c) {
    if (layouts.containsValue(c)) {
        layouts.removeValue(c);
    }
}

void Brain::registerTracker(Tracker* p, int id, bool swap) {
    int askedId = id;
    if (trackers.getReference(id) == p) { return; }
    if (trackers.containsValue(p)) {
        trackers.removeValue(p);
    }
    bool idIsOk = false;
    if (swap && p->registeredId != 0) {
        if (trackers.contains(id) && trackers.getReference(id) != nullptr) {
            Tracker* presentItem = trackers.getReference(id);
            unregisterTracker(p);
            registerTracker(presentItem, p->registeredId, false);
        }
    }
    while (!idIsOk) {
        if (trackers.contains(id) && trackers.getReference(id) != nullptr) {
            id++;
        }
        else {
            idIsOk = true;
        }
    }
    trackers.set(id, p);
    p->id->setValue(id);
    p->registeredId = id;
    if (id != askedId) {
    }
}


void Brain::unregisterTracker(Tracker* c) {
    if (trackers.containsValue(c)) {
        trackers.removeValue(c);
    }
}




void Brain::pleaseUpdate(Cuelist* c) {
    if (c == nullptr) {return;}
    ScopedLock lock(usingCollections);
    if (!cuelistPoolWaiting.contains(c)) {
        cuelistPoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(SubFixtureChannel* f) {
    if (f == nullptr || f->isDeleted || f->objectType != "SubFixtureChannel") { return; }
    if (f == nullptr) { return; };
    f->isDirty = true;
    return;
}

void Brain::pleaseUpdate(Cue* c) {
    if (c == nullptr || c->objectType != "Cue") { return; }
    ScopedLock lock(usingCollections);
    if (!cuePoolWaiting.contains(c)) {
        cuePoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(Programmer* c) {
    if (c == nullptr || c->objectType != "Programmer") { return; }
    ScopedLock lock(usingCollections);
    if (!programmerPoolWaiting.contains(c)) {
        programmerPoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(Effect* f) {
    if (f == nullptr || f->objectType != "Effect") { return; }
    ScopedLock lock(usingCollections);
    if (!effectPoolWaiting.contains(f)) {
        effectPoolWaiting.add(f);
    }
}

void Brain::pleaseUpdate(Carousel* f) {
    if (f == nullptr || f->objectType != "Carousel") { return; }
    ScopedLock lock(usingCollections);
    if (!carouselPoolWaiting.contains(f)) {
        carouselPoolWaiting.add(f);
    }
}

void Brain::pleaseUpdate(Mapper* f) {
    if (f == nullptr || f->objectType != "Mapper") { return; }
    ScopedLock lock(usingCollections);
    if (!mapperPoolWaiting.contains(f)) {
        mapperPoolWaiting.add(f);
    }
}

void Brain::pleaseUpdate(Tracker* f) {
    if (f == nullptr || f->objectType != "Tracker") { return; }
    ScopedLock lock(usingCollections);
    if (!trackerPoolWaiting.contains(f)) {
        trackerPoolWaiting.add(f);
    }
}

void Brain::grandMasterChanged()
{
    virtualFadersNeedUpdate = true;
    for (int i = 0; i < grandMasterChannels.size(); i++) {
        pleaseUpdate(grandMasterChannels[i]);
    }
}

// utils

float Brain::symPosition(int index, int nElements) {
    float position = 0;
    if (nElements == 0) { return 0; }
    if (index < float(nElements / 2)) {
        position = float(index) / float((nElements - 1) / 2);
    }
    else {
        position = float(nElements - index - 1) / float((nElements - 1) / 2);
    }
    return position;
}

int Brain::newTaskId()
{
    currentTaskId++;
    return currentTaskId;
}

void Brain::startTask(Task* t, double startTime, int cuelistId, float forcedDelay, float forcedFade)
{
    ScopedLock lock(usingTasksCollection);

    String actionType = "";
    String targetType = t->targetType->getValue();
    Array<int> targetIds;

    if (t->targetThru->getValue()) {
        int tId = t -> targetId -> getValue();
        int tIdTo = t -> targetIdTo -> getValue();
        int from = jmin(tId, tIdTo);
        int to = jmax(tId, tIdTo);
        for (int i = from; i <= to; i++) {
            targetIds.add(i);
        }
    }
    else {
        targetIds.add(t->targetId->getValue());
    }
    for (int i = 0; i < targetIds.size(); i++) {
        bool valid = false;
        double startValue = 0;
        double endValue = 1;
        int targetId = targetIds[i];
        if (targetType == "cuelist") {
            actionType = t->cuelistAction->getValue();
            Cuelist* target = getCuelistById(targetId);
            if (target != nullptr) {
                valid = true;
                if (actionType == "htplevel") {
                    startValue = target->HTPLevel->getValue();
                    endValue = t->targetValue->getValue();
                } else if (actionType == "ltplevel") {
                    startValue = target->LTPLevel->getValue();
                    endValue = t->targetValue->getValue();
                } else if (actionType == "flashLevel") {
                    startValue = target->FlashLevel->getValue();
                    endValue = t->targetValue->getValue();
                } else if (actionType == "speed") {
                    startValue = target->chaserSpeed->getValue();
                    endValue = t->targetValue->getValue();
                }
            }
        }
        else if (targetType == "effect") {
            actionType = t->effectAction->getValue();
            Effect* target = getEffectById(targetId);
            if (target != nullptr) {
                valid = true;
                if (actionType == "size") {
                    startValue = target->sizeValue->getValue();
                    endValue = t->targetValue->getValue();
                }
                else if (actionType == "speed") {
                    startValue = target->speed->getValue();
                    endValue = t->targetValue->getValue();
                }
            }
        }
        else if (targetType == "carousel") {
            actionType = t->carouselAction->getValue();
            Carousel* target = getCarouselById(targetId);
            if (target != nullptr) {
                valid = true;
                if (actionType == "size") {
                    startValue = target->sizeValue->getValue();
                    endValue = t->targetValue->getValue();
                }
                else if (actionType == "speed") {
                    startValue = target->speed->getValue();
                    endValue = t->targetValue->getValue();
                }
            }
        }
        else if (targetType == "mapper") {
            actionType = t->mapperAction->getValue();
            Mapper* target = getMapperById(targetId);
            if (target != nullptr) {
                valid = true;
                if (actionType == "size") {
                    startValue = target->sizeValue->getValue();
                    endValue = t->targetValue->getValue();
                }
            }
        }
        else if (targetType == "action") {
            actionType = t->mapperAction->getValue();
            valid = true;
        }

        if (valid) {
            RunningTask* rt = runningTasks.add(new RunningTask());
            rt->parentTask = t;
            rt->id = newTaskId();
            rt->cuelistId = cuelistId;
            rt->actionType = actionType;
            rt->targetType = targetType;
            rt->targetId = targetId;

            rt->delay = forcedDelay < 0 ? (double)t->delay->getValue() * 1000 : forcedDelay;
            rt->fade = forcedFade < 0 ? (double)t->fade->getValue() * 1000 : forcedFade;

            rt->TSInit = startTime;
            rt->TSStart = startTime + rt->delay;
            rt->TSEnd = startTime + rt->delay + rt->fade;

            rt->startValue = startValue;
            rt->endValue = endValue;
            rt->isEnded = false;
            //LOG(rt->endValue);
        }
    }

}

void Brain::stopTasks(int cuelistId, int taskId)
{
    usingTasksCollection.enter();
    for (int i = runningTasks.size()-1; i >= 0; i--) {
        if (runningTasks[i]->cuelistId == cuelistId && (taskId == -1 || taskId > runningTasks[i]->id)) {
            runningTasks.remove(i);
        }
    }
    usingTasksCollection.exit();
}

SubFixture* Brain::getSubFixtureById(int id) {
    if (subFixtures.contains(id)) {
        return subFixtures.getReference(id);
    }
    else {
        return nullptr;
    }
}

Fixture* Brain::getFixtureById(int id) {
    if (fixtures.contains(id)) {
        return fixtures.getReference(id);
    }
    else {
        return nullptr;
    }
}

Group* Brain::getGroupById(int id) {
    if (groups.contains(id)) {
        return groups.getReference(id);
    }
    else {
        return nullptr;
    }
}

Preset* Brain::getPresetById(int id) {
    if (presets.contains(id)) {
        return presets.getReference(id);
    }
    else {
        return nullptr;
    }
}

Cuelist* Brain::getCuelistById(int id) {
    if (cuelists.contains(id)) {
        return cuelists.getReference(id);
    }
    else {
        return nullptr;
    }
}

Programmer* Brain::getProgrammerById(int id) {
    if (programmers.contains(id)) {
        return programmers.getReference(id);
    }
    else {
        return nullptr;
    }
}

CurvePreset* Brain::getCurvePresetById(int id) {
    if (curvePresets.contains(id)) {
        return curvePresets.getReference(id);
    }
    else {
        return nullptr;
    }
}

TimingPreset* Brain::getTimingPresetById(int id) {
    if (timingPresets.contains(id)) {
        return timingPresets.getReference(id);
    }
    else {
        return nullptr;
    }
}

Effect* Brain::getEffectById(int id) {
    if (effects.contains(id)) {
        return effects.getReference(id);
    }
    else {
        return nullptr;
    }
}

Carousel* Brain::getCarouselById(int id) {
    if (carousels.contains(id)) {
        return carousels.getReference(id);
    }
    else {
        return nullptr;
    }
}

Mapper* Brain::getMapperById(int id) {
    if (mappers.contains(id)) {
        return mappers.getReference(id);
    }
    else {
        return nullptr;
    }
}

Layout* Brain::getLayoutById(int id) {
    if (layouts.contains(id)) {
        return layouts.getReference(id);
    }
    else {
        return nullptr;
    }
}

Tracker* Brain::getTrackerById(int id) {
    if (trackers.contains(id)) {
        return trackers.getReference(id);
    }
    else {
        return nullptr;
    }
}


void Brain::swoppedCuelist(Cuelist* c) {
    usingCollections.enter();
    if (!swoppedCuelists.contains(c)) {
        swoppedCuelists.add(c);
    }
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();
    isSwopping = true;
}

void Brain::swoppedEffect(Effect* c) {
    usingCollections.enter();
    if (!swoppedEffects.contains(c)) {
        swoppedEffects.add(c);
    }
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();
    isSwopping = true;
}

void Brain::swoppedCarousel(Carousel* c) {
    usingCollections.enter();
    if (!swoppedCarousels.contains(c)) {
        swoppedCarousels.add(c);
    }
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();
    isSwopping = true;
}


void Brain::unswoppedCuelist(Cuelist* c) {
    usingCollections.enter();
    swoppedCuelists.removeAllInstancesOf(c);
    isSwopping = swoppedCuelists.size() + swoppedEffects.size() + swoppedCarousels.size() > 0;
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();

}

void Brain::unswoppedEffect(Effect* c) {
    usingCollections.enter();
    swoppedEffects.removeAllInstancesOf(c);
    isSwopping = swoppedCuelists.size() + swoppedEffects.size() + swoppedCarousels.size() > 0;
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();

}

void Brain::unswoppedCarousel(Carousel* c) {
    usingCollections.enter();
    swoppedCarousels.removeAllInstancesOf(c);
    isSwopping = swoppedCuelists.size() + swoppedEffects.size() + swoppedCarousels.size() > 0;
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    usingCollections.exit();

}


void Brain::updateAllChannels() {
    for (auto it = fixtures.begin(); it != fixtures.end(); it.next()) {
        Fixture* fixt = it.getValue();
        for (auto it2 = fixt->subFixtures.begin(); it2 != fixt->subFixtures.end(); it2.next()) {
            SubFixture* subFixt = it2.getValue();
            for (auto it3 = subFixt->channelsMap.begin(); it3 != subFixt->channelsMap.end(); it3.next()) {
                pleaseUpdate(it3.getValue());
            }
        }
    }
}

void Brain::goAllLoadedCuelists() {
    for (auto it = cuelists.begin(); it != cuelists.end(); it.next()) {
        Cuelist* c = it.getValue();
        Cue* cueB = dynamic_cast<Cue*>(c->nextCue->targetContainer.get());
        if (cueB != nullptr) {
            c->go();
        }
        else if ((int)c->nextCueId->getValue() > 0) {
            c->go();
        }
    }
}

void Brain::killAllCuelists()
{
    for (auto it = cuelists.begin(); it != cuelists.end(); it.next()) {
        Cuelist* c = it.getValue();
        c->kill();
    }
}

void Brain::offAllCuelists()
{
    for (auto it = cuelists.begin(); it != cuelists.end(); it.next()) {
        Cuelist* c = it.getValue();
        c->off();
    }
}

void Brain::stopAllEffects()
{
    for (auto it = effects.begin(); it != effects.end(); it.next()) {
        Effect* f = it.getValue();
        f->stop();
    }
}

void Brain::stopAllCarousels()
{
    for (auto it = carousels.begin(); it != carousels.end(); it.next()) {
        Carousel* f = it.getValue();
        f->stop();
    }
}

void Brain::resetRandomSeed(int seed)
{
    srand(seed);
}

void Brain::showWindow(String name)
{
    bool done = false;
    for (int i = 0; i < ShapeShifterManager::getInstance()->openedPanels.size(); i++) {
        auto panel = ShapeShifterManager::getInstance()->openedPanels[i];
        for (int j = 0; j < panel->header.tabs.size(); j++) {
            if (panel->header.tabs[j]->content->contentName == name) {
                panel->setCurrentContent(name);
                done = true;
            }
        }
    }
    if (!done) {
        for (int i = 0; i < ShapeShifterFactory::getInstance()->defs.size() && !done; i++) {
            String contentName = ShapeShifterFactory::getInstance()->defs[i]->contentName;
            if (name == contentName) {
                ShapeShifterManager::getInstance()->showContent(contentName);
                done = true;
            }

        }
    }
}

void Brain::loadRunningCuelistsInProgrammer()
{
    loadRunningCuelistsInProgrammer(UserInputManager::getInstance()->getProgrammer(true));
}

void Brain::loadRunningCuelistsInProgrammer(Programmer* p)
{

    //for (Fixture* f : fixtures) 
    //{
    //    for (SubFixture* sf : f->subFixtures)
    //    {
    //        for (SubFixtureChannel* sfc : sf->channelsMap)
    //        {
    //            if (sfc->activeCommand != nullptr)
    //            {
    //                activeCommands.addIfNotAlreadyThere(sfc->activeCommand);
    //            }
    //        }
    //    }
    //}

    //if (activeCommands.size() == 0) return;
    MessageManager::callAsync([this]()
        {

        Array<Command*> activeCommands;
        HashMap<Command*, double> htpLevel;
        HashMap<Command*, double> ltpLevel;

        for (Cuelist* c : cuelists) 
        {
            if (c->isCuelistOn->boolValue()) 
            {
                c->isComputing.enter();
                for (auto it = c->activeValues.begin(); it != c->activeValues.end(); it.next()) 
                {
                    SubFixtureChannel* sfc = it.getKey();
                    Command* com = sfc->activeCommand;
                    if (com != nullptr && activeCommands.indexOf(com) == -1)
                    {
                        activeCommands.addIfNotAlreadyThere(com);
                        htpLevel.set(com, c->HTPLevel->floatValue());
                        ltpLevel.set(com, c->LTPLevel->floatValue());
                    }
                }
                c->isComputing.exit();
            }
        }

        Programmer* p = UserInputManager::getInstance()->getProgrammer(true);
        p->currentUserCommand = nullptr;
        p->commands.clear();
        for (Command* cmd : activeCommands)
        {
            if (cmd != nullptr) 
            {
                Command* newCmd = p->commands.addItemFromData(cmd->getJSONData(), false);
                for (CommandValue* cv : newCmd->values.items) 
                {
                    String type = cv->presetOrValue->getValueData();
                    if (cv->presetOrValue->getValueData() == "value") {
                        ChannelType* ct = dynamic_cast<ChannelType*>(cv->channelType->targetContainer.get());
                        if (ct != nullptr) {
                            String prio = ct->priority->getValueData();
                            float lvl = 1.0;
                            if (prio == "HTP") {
                                lvl = htpLevel.contains(cmd) ? htpLevel.getReference(cmd) : 1.0;
                            }
                            else {
                                lvl = ltpLevel.contains(cmd) ? ltpLevel.getReference(cmd) : 1.0;
                            }
                            cv->valueFrom->setValue(cv->valueFrom->floatValue() * lvl);
                            cv->valueTo->setValue(cv->valueTo->floatValue() * lvl);
                        }
                    }
                }
            }
        }
        p->selectCommand(activeCommands[0]);
        UserInputManager::getInstance()->programmerCommandStructureChanged(p);
    });
    
}

void Brain::soloPoolCuelistStarted(int poolId, Cuelist* c)
{
    soloPoolCheck(poolId, "Cuelist", c->id->intValue());
}

void Brain::soloPoolEffectStarted(int poolId, Effect* c)
{
    soloPoolCheck(poolId, "Effect", c->id->intValue());
}

void Brain::soloPoolCarouselStarted(int poolId, Carousel* c)
{
    soloPoolCheck(poolId, "Carousel", c->id->intValue());
}

void Brain::soloPoolCheck(int poolId, String excludeType, int excludeId)
{
    if (poolId == 0) return; 

    Array<Cuelist*> offCuelists;
    Array<Effect*> offEffects;
    Array<Carousel*> offCarousels;

    usingCollections.enter();
    for (auto it = cuelists.begin(); it != cuelists.end(); it.next()) {
        Cuelist* c = it.getValue();
        if (c->soloPool->intValue() == poolId && c->isCuelistOn->boolValue()) {
            if (excludeType != "Cuelist" || excludeId != c->id->intValue()) {
                offCuelists.add(c);
            }
        }
    }
    for (auto it = effects.begin(); it != effects.end(); it.next()) {
        Effect* c = it.getValue();
        if (c->soloPool->intValue() == poolId && c->isEffectOn->boolValue()) {
            if (excludeType != "Effect" || excludeId != c->id->intValue()) {
                offEffects.add(c);
            }
        }
    }
    for (auto it = carousels.begin(); it != carousels.end(); it.next()) {
        Carousel* c = it.getValue();
        if (c->soloPool->intValue() == poolId && c->isCarouselOn->boolValue()) {
            if (excludeType != "Carousel" || excludeId != c->id->intValue()) {
                offCarousels.add(c);
            }
        }
    }

    usingCollections.exit();
    for (Cuelist* c : offCuelists) c->off();
    for (Effect* c : offEffects) c->stop();
    for (Carousel* c : offCarousels) c->stop();

}

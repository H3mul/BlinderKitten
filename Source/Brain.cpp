/*
  ==============================================================================

    Brain.cpp
    Created: 14 Nov 2021 10:42:56pm
    Author:  No

  ==============================================================================
*/


#include "Brain.h"
#include "Definitions/ChannelValue.h"
juce_ImplementSingleton(Brain);

Brain::Brain() :
    Thread("BKBrain") 
{
    startThread();
}

Brain :: ~Brain() {
    clear();
    signalThreadShouldExit();
    waitForThreadToExit(100);
    clearSingletonInstance();
}

void Brain::clear()
{
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
}

void Brain::clearUpdates()
{
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

}

void Brain::run() {
    while(!threadShouldExit()) {
        try {
            now = Time::getMillisecondCounterHiRes();
            updateCuesIsRunning = true;
            cuePoolUpdating.addArray(cuePoolWaiting);
            cuePoolWaiting.clear();
            updateCuesIsRunning = false;
            for (int i = 0; i < cuePoolUpdating.size(); i++) {
                cuePoolUpdating.getReference(i)->update(now);
            }
            cuePoolUpdating.clear();

            updateCuelistsIsRunning = true;
            if (cuelistPoolWaiting.size() > 0) {
                cuelistPoolUpdating.addArray(cuelistPoolWaiting);
                cuelistPoolWaiting.clear();
            }
            updateCuelistsIsRunning = false;
            for (int i = 0; i < cuelistPoolUpdating.size(); i++) {
                cuelistPoolUpdating.getReference(i)->update();
            }
            cuelistPoolUpdating.clear();

            updateEffectsIsRunning = true;
            effectPoolUpdating.addArray(effectPoolWaiting);
            effectPoolWaiting.clear();
            updateEffectsIsRunning = false;
            for (int i = 0; i < effectPoolUpdating.size(); i++) {
                effectPoolUpdating.getReference(i)->update(now);
            }
            effectPoolUpdating.clear();

            updateCarouselsIsRunning = true;
            carouselPoolUpdating.addArray(carouselPoolWaiting);
            carouselPoolWaiting.clear();
            updateCarouselsIsRunning = false;
            for (int i = 0; i < carouselPoolUpdating.size(); i++) {
                carouselPoolUpdating.getReference(i)->update(now);
            }
            carouselPoolUpdating.clear();

            updateProgrammersIsRunning = true;
            programmerPoolUpdating.addArray(programmerPoolWaiting);
            programmerPoolWaiting.clear();
            updateProgrammersIsRunning = false;
            for (int i = 0; i < programmerPoolUpdating.size(); i++) {
                programmerPoolUpdating.getReference(i)->update(now);
            }
            programmerPoolUpdating.clear();

            updateChannelsIsRunning = true;
            subFixtureChannelPoolUpdating.addArray(subFixtureChannelPoolWaiting);
            subFixtureChannelPoolWaiting.clear();
            updateChannelsIsRunning = false;
            for (int i = 0; i < subFixtureChannelPoolUpdating.size(); i++) {
                // bug ici
                if (subFixtureChannelPoolUpdating.getReference(i) != nullptr && !subFixtureChannelPoolUpdating.getReference(i)->isDeleted) {
                    subFixtureChannelPoolUpdating.getReference(i)->updateVal(now);
                }
            }
            subFixtureChannelPoolUpdating.clear();

            //double delta = Time::getMillisecondCounterHiRes() - now;
            //LOG(delta);
        }
        catch (...) {
            LOG("maow");
            // ...
        }
        wait(10);
    }
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
    int askedId = id;
    LOG("register Cuelist");
    if (cuelists.getReference(id) == p) { return; }
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
}

void Brain::unregisterCuelist(Cuelist* c) {
    if (cuelists.containsValue(c)) {
        cuelists.removeValue(c);
    }
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
}

void Brain::pleaseUpdate(Cuelist* c) {
    if (c == nullptr) {return;}
    while (updateCuelistsIsRunning) {wait(5); }
    if (!cuelistPoolWaiting.contains(c)) {
        cuelistPoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(SubFixtureChannel* f) {
    if (f == nullptr || f->objectType != "SubFixtureChannel") { return; }
    while (updateChannelsIsRunning) { wait(5); }
    if (f == nullptr) { return; };
    //bug ici
    if (!subFixtureChannelPoolWaiting.contains(f)) {
        subFixtureChannelPoolWaiting.add(f);
    }
}

void Brain::pleaseUpdate(Cue* c) {
    if (c == nullptr || c->objectType != "Cue") { return; }
    while (updateCuesIsRunning) { wait(5); }
    if (!cuePoolWaiting.contains(c)) {
        cuePoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(Programmer* c) {
    if (c == nullptr || c->objectType != "Programmer") { return; }
    while (updateProgrammersIsRunning) { wait(5); }
    if (!programmerPoolWaiting.contains(c)) {
        programmerPoolWaiting.add(c);
    }
}

void Brain::pleaseUpdate(Effect* f) {
    if (f == nullptr || f->objectType != "Effect") { return; }
    while (updateEffectsIsRunning) { wait(5); }
    if (!effectPoolWaiting.contains(f)) {
        effectPoolWaiting.add(f);
    }
}

void Brain::pleaseUpdate(Carousel* f) {
    if (f == nullptr || f->objectType != "Carousel") { return; }
    while (updateCarouselsIsRunning) { wait(5); }
    if (!carouselPoolWaiting.contains(f)) {
        carouselPoolWaiting.add(f);
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


void Brain::swoppedCuelist(Cuelist* c) {
    if (!swoppedCuelists.contains(c)) {
        swoppedCuelists.add(c);
    }
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
    isSwopping = true;
}

void Brain::unswoppedCuelist(Cuelist* c) {
    
    while (swoppedCuelists.indexOf(c) >= 0) {
        swoppedCuelists.removeAllInstancesOf(c);
    }

    isSwopping = swoppedCuelists.size() > 0;
    for (int i = 0; i < swoppableChannels.size(); i++) {
        pleaseUpdate(swoppableChannels.getReference(i));
    }
}

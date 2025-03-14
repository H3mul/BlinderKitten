/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "Cue.h"
//#include "../SubFixture/SubFixture.h"
class SubFixture;

class CueManager :
    public BaseManager<Cue>
{
public:
    CueManager();
    ~CueManager();

    Cuelist* parentCuelist = nullptr;

    void askForMoveBefore(BaseItem*) override;
    void askForMoveAfter(BaseItem*) override;
    void addItemInternal(Cue* c, var data);
    void askForDuplicateItem(BaseItem* item);
    void correctCueIds();
    void setItemIndex(Cue* item, int newIndex, bool addToUndo = true);
    double calcId(double prev, double next);

    void removeItemInternal(Cue*) override;

};
/*
  ==============================================================================

    TempoManager.h

  ==============================================================================
*/

#pragma once
#include "Tempo.h"

class TempoManager :
    public BaseManager<Tempo>
{
public:
    juce_DeclareSingleton(TempoManager, true);

    TempoManager();
    ~TempoManager();

    void addItemInternal(Tempo* o, var data) override;
    void removeItemInternal(Tempo* o) override;
};

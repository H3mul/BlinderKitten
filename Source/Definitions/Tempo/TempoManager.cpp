/*
  ==============================================================================

    TempoManager.cpp

  ==============================================================================
*/

#include "Tempo.h"
#include "TempoManager.h"

juce_ImplementSingleton(TempoManager);

TempoManager::TempoManager() :
    BaseManager("Tempo")
{
    itemDataType = "Tempo";
    selectItemWhenCreated = true;
}

TempoManager::~TempoManager()
{
}

void TempoManager::addItemInternal(Tempo* o, var data)
{
}

void TempoManager::removeItemInternal(Tempo* o)
{
}

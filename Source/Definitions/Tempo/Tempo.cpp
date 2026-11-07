/*
  ==============================================================================

    Tempo.cpp
    Shared BPM that bound effects and carousels follow

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Tempo.h"
#include "../../Brain.h"

Tempo::Tempo(var params) :
	BaseItem(params.getProperty("name", "Tempo")),
	objectType(params.getProperty("type", "Tempo").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = false;

	itemDataType = "Tempo";

	id = addIntParameter("ID", "ID of this tempo", 1, 1);
	userName = addStringParameter("Name", "Name of this tempo", "New tempo");
	bpm = addFloatParameter("BPM", "Master tempo in beats per minute", 120, 1);
	scale = addFloatParameter("Scale", "Runtime multiplier on bpm: 2 is double time, 0.5 is half time", 1, 0.0625f);
	effectiveBpm = addFloatParameter("Effective BPM", "bpm x scale, the rate bound effects and carousels follow", 120);
	effectiveBpm->isControllableFeedbackOnly = true;
	effectiveBpm->isSavable = false;
	resyncOnEveryTap = addBoolParameter("Resync on every tap", "Every tap also restarts the phase of bound effects, not only the first tap after a pause", false);
	tapBtn = addTrigger("Tap", "Hit me at least twice to set the bpm");
	downbeatBtn = addTrigger("Downbeat", "Restart the phase of all effects and carousels bound to this tempo");

	updateName();
	updateEffectiveBpm();
	Brain::getInstance()->registerTempo(this, id->getValue());
}

Tempo::~Tempo()
{
	Brain::getInstance()->unregisterTempo(this);
}

void Tempo::onContainerParameterChangedInternal(Parameter* p)
{
	BaseItem::onContainerParameterChangedInternal(p);
	if (p == id) {
		Brain::getInstance()->registerTempo(this, id->getValue(), true);
	}
	if (p == userName || p == id) {
		updateName();
	}
	if (p == bpm || p == scale) {
		updateEffectiveBpm();
		Brain::getInstance()->virtualFadersNeedUpdate = true;
	}
}

void Tempo::updateName()
{
	setNiceName(String(id->intValue()) + " - " + userName->stringValue());
}

void Tempo::updateEffectiveBpm()
{
	effectiveBpm->setValue(bpm->floatValue() * scale->floatValue());
}

/*
  ==============================================================================

    Tempo.cpp
    Shared BPM that bound effects and carousels follow

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Tempo.h"
#include "../../Brain.h"
#include "../../Brain.h"
#include "../Effect/Effect.h"
#include "../Carousel/Carousel.h"
#include "BKEngine.h"

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

void Tempo::triggerTriggered(Trigger* t)
{
	if (t == tapBtn) tap();
	else if (t == downbeatBtn) downbeat();
}

void Tempo::tap()
{
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTap;
	lastTap = now;
	if (delta >= 3000) {
		// first tap after a pause is the song start
		tapHistory.clear();
		downbeat();
		return;
	}
	// ponytail: taps closer than 1 ms (a ranged tap over several bound items) are ignored, avoiding divide by zero
	if (delta < 1) return;

	BKEngine* e = dynamic_cast<BKEngine*>(BKEngine::mainEngine);
	int historySize = jmax(1, e->tapTempoHistory->intValue());
	tapHistory.add(delta);
	while (tapHistory.size() > historySize) tapHistory.remove(0);
	double sum = 0;
	for (int i = 0; i < tapHistory.size(); i++) sum += tapHistory[i];
	bpm->setValue(60000. / (sum / tapHistory.size()));
	if (resyncOnEveryTap->boolValue()) downbeat();
}

void Tempo::downbeat()
{
	int myId = id->intValue();
	for (Effect* e : Brain::getInstance()->effects) {
		if (e->tempoId->intValue() == myId) e->totalElapsed = 0;
	}
	for (Carousel* c : Brain::getInstance()->carousels) {
		if (c->tempoId->intValue() == myId) c->totalElapsed = 0;
	}
}

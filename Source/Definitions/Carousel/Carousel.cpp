/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Carousel.h"
#include "../Command/CommandSelectionManager.h"
#include "../SubFixture/SubFixtureChannel.h"
#include "../../Brain.h"
#include "CarouselManager.h"
#include "../CurvePreset/CurvePreset.h"
#include "../ChannelValue.h"
#include "UI/GridView/CarouselGridView.h"

Carousel::Carousel(var params) :
	BaseItem(params.getProperty("name", "Carousel")),
	objectType(params.getProperty("type", "Carousel").toString()),
	objectData(params),
	rows("Rows"),
	speedMult("Speed multiplicators"),
	sizeMult("Size multiplicators")
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = false;

	itemDataType = "Carousel";

	id = addIntParameter("ID", "ID of this Carousel", 1, 1);
	userName = addStringParameter("Name", "Name of this Carousel", "New Carousel");
	layerId = addIntParameter("Layer", "Higher layer, higer priority", 1, 1);
	updateName();

	isCarouselOn = addBoolParameter("is ON", "Enable or disable this Carousel",false);
	isCarouselOn->isControllableFeedbackOnly;
	isCarouselOn->setEnabled(false);
	isCarouselOn->isSavable = false;
	isOn = false;

	startBtn = addTrigger("Start", "Start this Carousel");
	stopBtn = addTrigger("Stop", "Stop this Carousel");

	currentPosition = addFloatParameter("Current Position", "Actual position of the Carousel",0,0,1);
	currentPosition->isControllableFeedbackOnly = true;
	currentPosition->isSavable = false;

	autoStartAndStop = addBoolParameter("Auto Start / Stop", "Start and stop the Carousel when size is modified", true);
	sizeValue = addFloatParameter("Size", "Master of this Carousel", 1, 0, 1);
	flashValue = addFloatParameter("Flash", "Flash master of this Carousel", 1, 0, 1);
	speed = addFloatParameter("Speed", "Speed of this Carousel in cycles/minutes", 5, 0);

	beatPerCycle = addIntParameter("Beat by cycles", "Number of tap tempo beats by cycle", 1, 1);
	tapTempoBtn = addTrigger("Tap tempo", "");

	soloPool = addIntParameter("Solo pool", "If greater than zero, only one element can be activated at a time with this number", 0,0);

	addChildControllableContainer(&speedMult);
	addChildControllableContainer(&sizeMult);
	rows.selectItemWhenCreated = false;
	addChildControllableContainer(&rows);

	Brain::getInstance()->registerCarousel(this, id->getValue());

	if (params.isVoid()) {
		rows.addItem();
	}
	if (!Brain::getInstance()->loadingIsRunning) {
		CarouselGridView::getInstance()->updateCells();
	}

}

Carousel::~Carousel()
{
	stop();
	rows.clear();
	isOn = false;
	Brain::getInstance()->unregisterCarousel(this);
	Brain::getInstance()->usingCollections.enter();
	Brain::getInstance()->carouselPoolWaiting.removeAllInstancesOf(this);
	Brain::getInstance()->carouselPoolUpdating.removeAllInstancesOf(this);
	Brain::getInstance()->usingCollections.exit();
	for (auto it = chanToCarouselRow.begin(); it != chanToCarouselRow.end(); it.next()) {
		SubFixtureChannel* sfc = it.getKey();
		sfc->carouselOutOfStack(this);
		Brain::getInstance()->pleaseUpdate(sfc);
	}
	for (int i = 0; i < rows.items.size(); i++) {
		rows.items[i]->parentCarousel = nullptr;
		for (int j = 0; j < rows.items[i]->paramContainer.items.size(); j++) {
			rows.items[i]->paramContainer.items[j]->parentCarousel = nullptr;
		}
	}
	CarouselGridView::getInstance()->updateCells();
}

void Carousel::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) {
	pleaseComputeIfRunning();
}


void Carousel::onContainerParameterChangedInternal(Parameter* p) {
	if (p == id) {
		Brain::getInstance()->registerCarousel(this, id->getValue(), true);
	}
	if (p == isCarouselOn) {
		Brain::getInstance()->virtualButtonsNeedUpdate = true;
		Brain::getInstance()->virtualFaderButtonsNeedUpdate = true;
		Brain::getInstance()->carouselGridNeedRefresh = true;
	}
	if (p == userName || p == id) {
		updateName();
		CarouselGridView::getInstance()->updateCells();
	}
	if (p == sizeValue) {
		if (autoStartAndStop->getValue()) {
			if (isOn && (float)sizeValue->getValue() == 0) {
				stop();
			}
			else if(!isOn && (float)sizeValue->getValue() > 0 && lastSize == 0) {
				userStart();
			}
		}
		lastSize = p->getValue();
		currentSizeController = nextSizeController;
		nextSizeController = "";
		Brain::getInstance()->virtualFadersNeedUpdate = true;
	}
	if (p == speed) {
		Brain::getInstance()->virtualFadersNeedUpdate = true;
	}
}

void Carousel::triggerTriggered(Trigger* t) {
	if (t == startBtn) {
		userStart();
	}
	else if (t == stopBtn) {
		stop();
	}
	else if (t == tapTempoBtn) {
		tapTempo();
	}
	else {}
}

void Carousel::userStart() {
	userPressedGo = true;
	start();
}

void Carousel::start() {
	TSLastUpdate = Time::getMillisecondCounterHiRes();
	isOn = true;
	isCarouselOn->setValue(true);
	totalElapsed = 0;
	computeData();
	if (soloPool->intValue() > 0) Brain::getInstance()->soloPoolCarouselStarted(soloPool->intValue(), this);
}

void Carousel::stop() {
	isOn = false;
	userPressedGo = false;
	isCarouselOn->setValue(false);
	for (auto it = chanToCarouselRow.begin(); it != chanToCarouselRow.end(); it.next()) {
		if (it.getKey() != nullptr) {
			it.getKey()->carouselOutOfStack(this);
		}
	}
}

void Carousel::update(double now) {
	if (computed == false) {
		computeData();
	}
	if (isOn) {
		Brain::getInstance()->pleaseUpdate(this);
		currentSizeMult = sizeMult.getValue();
		double deltaTime = now - TSLastUpdate;
		TSLastUpdate = now;
		double currentSpeed = speed->getValue();
		float speedMultVal = speedMult.getValue();
		currentSpeed *= speedMultVal;
		if (speed != 0) {
			double duration = 60000. / currentSpeed;
			double delta = deltaTime / duration;
			totalElapsed += delta;
			currentPosition->setValue(fmodf(totalElapsed, 1.0));
		}
	}
	else {
		currentPosition->setValue(0);
	}
}

void Carousel::pleaseComputeIfRunning() {
	if (isOn) {
		computed = false;
		Brain::getInstance()->pleaseUpdate(this);
	}
}

void Carousel::computeData() {
	if (computing) { 
		return; 
	}
	computed = true;
	computing = true;
	isComputing.enter();
	chanToCarouselRow.clear();
	for (int i = 0; i < rows.items.size(); i++) {
		rows.items[i]->computeData();
	}
	computing = false;
	isComputing.exit();
	if (isOn) {
		for (auto it = chanToCarouselRow.begin(); it != chanToCarouselRow.end(); it.next()) {
			if (it.getKey() != nullptr) {
				it.getKey()->carouselOnTopOfStack(this);
				Brain::getInstance()->pleaseUpdate(it.getKey());
			}
		}
		Brain::getInstance()->pleaseUpdate(this);
	}
}

float Carousel::applyToChannel(SubFixtureChannel* fc, float currentVal, double now) {
	if (!chanToCarouselRow.contains(fc)) {return currentVal; }
	if (isOn) {Brain::getInstance()->pleaseUpdate(fc); }
	isComputing.enter();
	float calcValue = currentVal;
	bool htpOver = false;
	std::shared_ptr<Array<CarouselRow*>> activeRows = chanToCarouselRow.getReference(fc);
	for (int rId = 0; rId < activeRows->size(); rId++) {
		CarouselRow * r = activeRows->getReference(rId);
		r->checkParentCarousel();
		double offset = totalElapsed;
		offset += r->subFixtureChannelOffsets.getReference(fc);
		while (offset < 0) {
			offset += 1;
		}
		offset = fmodf(offset, 1);
		
		CarouselStep* toApply = nullptr;
		
		for (int stepId = 0; stepId < r->paramContainer.items.size(); stepId++) {
			CarouselStep* step = r->paramContainer.items[stepId];
			step->checkParentCarousel();
			if (step->relativeStartPosition <= offset) {
				toApply = step;
			}	
		}

		if (toApply == nullptr) {
//			LOG("miaou !");
//			LOG(offset);
			return currentVal;
		}
		std::shared_ptr<ChannelValue> cVal = toApply->computedValues.getReference(fc);
		if (cVal != nullptr) {
			float fadeValue = 1;
			if (cVal->htpOverride) {
				htpOver = true;
			}
			float fadeWidth = toApply->fadeRatio->getValue();
			float pos = (offset-toApply->relativeStartPosition) / toApply->relativeDuration;
			if (pos < fadeWidth) {
				fadeValue = pos/fadeWidth;
				fadeValue = toApply->curve.getValueAtPosition(fadeValue);
			}

			float start = cVal->startValue;
			float end = cVal->endValue;
			start = start == -1 ? currentVal : start;
			end = end == -1 ? currentVal : end;
			calcValue = jmap(fadeValue, start, end);
		}


	}

	float s = sizeValue->getValue();
	s *= currentSizeMult;
	if (isFlashing) { s = flashValue->floatValue(); }
	if (s>1) {s = 1;}
	if (fc->isHTP && !htpOver) {
		calcValue *= s;
		currentVal = jmax(calcValue, currentVal);
	}
	else {
		currentVal = jmap(s, currentVal, calcValue);
	}

	isComputing.exit();

	return currentVal;
}


void Carousel::updateName() {
	String n = userName->getValue();
	if (parentContainer != nullptr) {
		dynamic_cast<CarouselManager*>(parentContainer.get())->reorderItems();
	}
	setNiceName(String((int)id->getValue()) + " - " + n);
}

void Carousel::tapTempo() {
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTapTempo;
	lastTapTempo = now;
	if (delta < 3000) {
		delta = delta * (int)beatPerCycle->getValue();
		double cpm = 60000. / delta;
		speed->setValue(cpm);
	}
}


void Carousel::flash(bool on, bool swop)
{
	if (on) {
		if (!isOn) {
			start();
		}
		isFlashing = true;
		if (swop) {
			isSwopping = true;
			Brain::getInstance()->swoppedCarousel(this);
		}
	}
	else {
		isFlashing = false;
		if (isSwopping) {
			isSwopping = false;
			Brain::getInstance()->unswoppedCarousel(this);
		}
		if (!userPressedGo) {
			stop();
		}
	}
}

/*
  ==============================================================================

    FixtureTypeChannel.cpp
    Created: 8 Nov 2021 7:28:28pm
    Author:  No

  ==============================================================================
*/

#include "CarouselStep.h"
#include "../ChannelFamily/ChannelFamilyManager.h"
#include "Carousel.h"
#include "../ChannelValue.h"
#include "../SubFixture/SubFixtureChannel.h"
#include "../Preset/Preset.h"
#include "../../Brain.h"
#include "../Palette/Palette.h"
#include "../Palette/PaletteManager.h"

CarouselStep::CarouselStep(var params) :
    BaseItem(params.getProperty("name", "CarouselStep")),
    objectType(params.getProperty("type", "CarouselStep").toString()),
    objectData(params),
	values("CommandValue")
{
    saveAndLoadRecursiveData = true;

    stepDuration = addFloatParameter("Step duration", "Pause time on the value, unit is relative to other", 1, 0);
    fadeRatio = addFloatParameter("Fade", "Transition fade", 1, 0, 1);

    curve.saveAndLoadRecursiveData = true;
    curve.editorIsCollapsed = true;
    curve.allowKeysOutside = false;
    curve.isSelectable = false;
    curve.length->setValue(1);
    curve.addKey(0, 0, false);
    curve.items[0]->easingType->setValueWithData(Easing::LINEAR);
    curve.addKey(1, 1, false);
    curve.selectItemWhenCreated = false;
    curve.editorCanBeCollapsed = true;
    curve.editorIsCollapsed = false;

    addChildControllableContainer(&curve);
    addChildControllableContainer(&values);

	if (params.isVoid()) {
		values.addItem();
	}

    updateDisplay();
};

CarouselStep::~CarouselStep()
{
	if (parentCarousel != nullptr) {
		parentCarousel->computeData();
	}
};


void CarouselStep::checkParentCarousel()
{
	if (parentCarousel == nullptr && 
		parentContainer != nullptr && 
		parentContainer->parentContainer != nullptr &&
		parentContainer->parentContainer->parentContainer != nullptr &&
		parentContainer->parentContainer->parentContainer->parentContainer != nullptr
		) {
		parentCarousel = dynamic_cast<Carousel*>(parentContainer->parentContainer.get());
	}
}

void CarouselStep::onContainerParameterChangedInternal(Parameter* c) {
}

void CarouselStep::updateDisplay() {
    queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}


// Palette shift: the values are resolved per channel when the carousel applies (Carousel::applyToChannel); here only the channels are declared,
// those set by the named state's presets in every palette of the range that has the state.
void CarouselStep::computeShift(CommandValue* cv, Array<SubFixture*>& SubFixtures) {
	String state = cv->stateName->stringValue();
	if (state.isEmpty()) return;
	int lo = cv->paletteId->intValue();
	int hi = jmax(lo, cv->thru->boolValue() ? cv->paletteIdTo->intValue() : lo);
	Brain* b = Brain::getInstance();
	for (auto* p : PaletteManager::getInstance()->items) {
		if (lo > 0 && (p->registeredId < lo || p->registeredId > hi)) continue;
		int st = p->findStateIndex(state);
		if (st <= 0) continue;
		for (int slot = 1; slot <= p->slotCount->intValue(); slot++) {
			Preset* pr = b->getPresetById(p->getPresetIdForState(st, slot), true);
			if (pr == nullptr) continue;
			pr->computeValues();
			for (SubFixture* sf : SubFixtures) {
				auto vals = pr->getSubFixtureValues(sf);
				if (vals == nullptr) continue;
				for (auto it = vals->begin(); it != vals->end(); it.next()) {
					if (!sf->channelsMap.contains(it.getKey())) continue;
					SubFixtureChannel* fchan = sf->channelsMap.getReference(it.getKey());
					if (!computedValues.contains(fchan)) computedValues.set(fchan, std::make_shared<ChannelValue>());
					auto cvv = computedValues.getReference(fchan);
					cvv->values.set(1, -1);
					cvv->paletteShift = true;
					cvv->shiftState = state;
					cvv->shiftLo = lo;
					cvv->shiftHi = hi;
				}
			}
		}
	}
}

void CarouselStep::computeValues(Array<SubFixture*> &SubFixtures) {
	double now = Time::getMillisecondCounterHiRes();
	//LOG("compute carouselStep");
	isComputing.enter();
	computedValues.clear();
	if (!enabled->boolValue()) {
		isComputing.exit();
		return;
	}
	Array<CommandValue*> commandValues = values.getItemsWithType<CommandValue>();

	for (int commandIndex = 0; commandIndex < commandValues.size(); commandIndex++) {
		CommandValue* cv = commandValues[commandIndex];
		if (!cv -> enabled->boolValue()) continue;
		if (cv->presetOrValue->getValueData() == "paletteshift") { computeShift(cv, SubFixtures); continue; }
		bool symValues = cv->symmetry->getValue();
		Preset* pFrom = nullptr;
		Preset* pTo = nullptr;
		if (cv->presetOrValue->getValue() == "palette") {
			cv->resolvePalette(nullptr, pFrom, pTo);
		}
		else if (cv->presetOrValue->getValue() == "preset") {
			pFrom = Brain::getInstance()->getPresetById(cv->presetIdFrom->getValue(), true);
			pTo = Brain::getInstance()->getPresetById(cv->presetIdTo->getValue(), true);
			if (pFrom != nullptr) {
				pFrom->computeValues();
			}
			if (pTo != nullptr) {
				pTo->computeValues();
			}
		}
		//LOG("sub step 1 " << Time::getMillisecondCounterHiRes() - now);
		ChannelType* rawChan = dynamic_cast<ChannelType*>(cv->channelType->targetContainer.get());
		ChannelFamily* rawFam = dynamic_cast<ChannelFamily*>(cv->channelType->targetContainer.get());
		for (int indexFixt = 0; indexFixt < SubFixtures.size(); indexFixt++) {

			if (indexFixt <2) {
				//LOG("sub step 2 " << Time::getMillisecondCounterHiRes() - now);
			}
			std::shared_ptr < HashMap<ChannelType*, float>> valuesFrom = std::make_shared<HashMap<ChannelType*, float>>();
			std::shared_ptr < HashMap<ChannelType*, float>> valuesTo = std::make_shared<HashMap<ChannelType*, float>>();
			String test = cv->presetOrValue->getValue();
			if (cv->usesPreset()) {

				std::shared_ptr < HashMap<ChannelType*, float>> tempValuesFrom = pFrom != nullptr ? pFrom->getSubFixtureValues(SubFixtures[indexFixt]) : nullptr;
				std::shared_ptr < HashMap<ChannelType*, float>> tempValuesTo = pTo != nullptr ? pTo->getSubFixtureValues(SubFixtures[indexFixt]) : nullptr;
				if (tempValuesFrom != nullptr) {
					for (auto it = tempValuesFrom->begin(); it != tempValuesFrom->end(); it.next()) {
						valuesFrom->set(it.getKey(), it.getValue());
					}
					//tempValuesFrom->~HashMap();
				}
				if (tempValuesTo != nullptr) {
					for (auto it = tempValuesTo->begin(); it != tempValuesTo->end(); it.next()) {
						valuesTo->set(it.getKey(), it.getValue());
					}
					//tempValuesTo->~HashMap();
				}

			}

			cv->applyPaletteBlend(SubFixtures[indexFixt], *valuesFrom, *valuesTo);
			if (cv->presetOrValue->getValue() == "value") {
				valuesFrom->set(rawChan, cv->valueFrom->getValue());
				valuesTo->set(rawChan, cv->valueTo->getValue());
			}
			else if (cv->presetOrValue->stringValue() == "release") {
				for (SubFixtureChannel* sfc : SubFixtures[indexFixt]->channelsContainer) {
					if (sfc->channelType == rawChan && rawChan != nullptr) {
						valuesFrom->set(rawChan, -1);
						valuesTo->set(rawChan, -1);
					}
					else if (sfc->channelType->parentFamily == rawFam && rawFam != nullptr) {
						valuesFrom->set(sfc->channelType, -1);
						valuesTo->set(sfc->channelType, -1);
					}
					else if (rawChan == nullptr && rawFam == nullptr) {
						valuesFrom->set(sfc->channelType, -1);
						valuesTo->set(sfc->channelType, -1);
					}
				}


			}


			if (indexFixt <2) {
				//LOG("sub step 3 " << Time::getMillisecondCounterHiRes() - now);
			}
			for (auto it = valuesFrom->begin(); it != valuesFrom->end(); it.next()) {
				SubFixtureChannel* fchan = SubFixtures[indexFixt]->channelsMap.contains(it.getKey()) ? SubFixtures[indexFixt]->channelsMap.getReference(it.getKey()) : nullptr;

				float valueFrom = it.getValue();
				float valueTo = valueFrom;
				if (valuesTo->contains(it.getKey())) {
					valueTo = valuesTo->getReference(it.getKey());
				}

				if (fchan != nullptr) {
					if (!computedValues.contains(fchan)) {
						computedValues.set(fchan, std::make_shared<ChannelValue>());
					}
					std::shared_ptr<ChannelValue> finalValue = computedValues.getReference(fchan);
					float val = valueFrom;
					if (cv->thru->getValue() && SubFixtures.size() > 1) {
						float position = float(indexFixt) / float(SubFixtures.size() - 1);
						if (symValues) { position = Brain::symPosition(indexFixt, SubFixtures.size()); }
						val = jmap(position, val, valueTo);
					}
					finalValue->values.set(1, val);
					finalValue->htpOverride = cv->HTPOverride->boolValue();
				}
			}

			if (valuesFrom != nullptr) {
				//valuesFrom->~HashMap();
			}
			if (valuesTo != nullptr) {
				//valuesTo->~HashMap();
			}
		}
	}
	isComputing.exit();
}


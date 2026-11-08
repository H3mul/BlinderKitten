// Cuelist::refreshPaletteRefs: palette fade ticks and state changes re-resolve only the tracked cues that ref the palette.
// Lives in its own file so palette v2 does not touch Cuelist.cpp.
#include "JuceHeader.h"
#include "../Cuelist/Cuelist.h"
#include "../Cue/Cue.h"
#include "../Command/Command.h"
#include "../Command/CommandValue.h"
#include "../ChannelValue.h"
#include "../../Brain.h"


void Cuelist::refreshPaletteRefs(const Array<int>& paletteIds)
{
	if (cueA == nullptr || !isCuelistOn->boolValue()) { return; }

	double now = Time::getMillisecondCounterHiRes();
	int endIndex = cues.items.indexOf(cueA);
	int startIndex = endIndex;
	if (endIndex >= 0 && !isChaser->boolValue() && tracking->getValue() == "cuelist") { // a chaser step releases tracking
		startIndex = 0;
		for (int i = 0; i <= endIndex; i++) {
			if (cues.items[i]->releaseCurrentTracking->boolValue()) { startIndex = i; }
		}
	}

	Array<Cue*> toApply;
	if (endIndex < 0) { toApply.add(cueA); }
	else { for (int i = startIndex; i <= endIndex; i++) { toApply.add(cues.items[i]); } }

	Array<SubFixtureChannel*> toUpdate;
	isComputing.enter();
	for (Cue* c : toApply) {
		if (!c->refsPalettes(paletteIds)) continue;
		c->computeValues();
		c->csComputing.enter();
		for (auto it = c->computedValues.begin(); it != c->computedValues.end(); it.next()) {
			SubFixtureChannel* sfc = it.getKey();
			std::shared_ptr<ChannelValue> v = it.getValue();
			if (sfc == nullptr || v == nullptr) { continue; }
			std::shared_ptr<ChannelValue> old = activeValues.contains(sfc) ? activeValues.getReference(sfc) : nullptr;
			if (old != nullptr && !old->isEnded && old->values.size() > 0) {
				// the cue is still fading in: keep its timing and start value, only the target moves
				v->values.set(0, old->values[0]);
				v->TSInit = old->TSInit;
				v->TSStart = old->TSStart;
				v->TSEnd = old->TSEnd;
				v->delay = old->delay;
				v->fade = old->fade;
				v->fadeCurve = old->fadeCurve;
				v->isEnded = false;
			}
			else {
				v->values.set(0, -1);
				v->TSInit = now;
				v->TSStart = now;
				v->TSEnd = now;
				v->isEnded = true;
			}
			activeValues.set(sfc, v);
			sfc->cs.enter();
			if (!sfc->cuelistStack.contains(this)) { sfc->cuelistOnTopOfStack(this); }
			sfc->cs.exit();
			toUpdate.addIfNotAlreadyThere(sfc);
		}
		c->csComputing.exit();
	}
	isComputing.exit();

	for (auto* sfc : toUpdate) { Brain::getInstance()->pleaseUpdate(sfc); }
}

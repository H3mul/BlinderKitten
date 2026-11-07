/*
  ==============================================================================

    TempoAction.cpp
    Actions that drive a Tempo from virtual buttons, faders and cue tasks

  ==============================================================================
*/

#include "TempoAction.h"
#include "../../Brain.h"
#include "../Tempo/Tempo.h"

TempoAction::TempoAction(var params) :
    Action(params)
{
    actionType = (ActionType)(int)params.getProperty("actionType", TEMPO_TAP);

    targetId = addIntParameter("Tempo ID", "Id of the tempo to drive", 1, 0);

    if (actionType == TEMPO_SET) {
        fromValue = addFloatParameter("Value from", "BPM when the input is at 0", 60, 1);
        toValue = addFloatParameter("Value to", "BPM when the input is at 1", 180, 1);
    }

    if (actionType == TEMPO_SETAT) {
        atValue = addFloatParameter("BPM", "new bpm of the tempo", 120, 1);
    }

    if (actionType == TEMPO_SCALEAT) {
        atValue = addFloatParameter("Scale", "new scale of the tempo", 1, 0.0625f);
    }
}

TempoAction::~TempoAction()
{
}

void TempoAction::triggerInternal()
{
}

void TempoAction::setValueInternal(var value, String origin, int incrementIndex, bool isRelative) {
    Tempo* target = Brain::getInstance()->getTempoById(targetId->intValue());
    if (target == nullptr) return;

    float val = value;

    if (actionType == TEMPO_SET) {
        // continuous: fader value mapped between from and to
        float mapped = jmap(val, (float)fromValue->getValue(), (float)toValue->getValue());
        bool incrementOk = incrementIndex == 0 || incrementIndex == validIncrementIndex;
        if (incrementOk || abs(target->bpm->floatValue() - mapped) < 1) {
            target->bpm->setValue(mapped);
            validIncrementIndex = incrementIndex + 1;
        }
        return;
    }

    // all other actions fire on press only
    if (val != 1) return;

    switch (actionType)
    {
    case TEMPO_TAP:
        target->tap();
        break;

    case TEMPO_SETAT:
        target->bpm->setValue((float)atValue->getValue());
        break;

    case TEMPO_SCALEAT:
        target->scale->setValue((float)atValue->getValue());
        break;

    case TEMPO_DOUBLE:
        target->scale->setValue(target->scale->floatValue() * 2);
        break;

    case TEMPO_HALF:
        target->scale->setValue(jmax(0.0625f, target->scale->floatValue() / 2));
        break;

    case TEMPO_RESETSCALE:
        target->scale->setValue(1);
        break;

    case TEMPO_DOWNBEAT:
        target->downbeat();
        break;

    default:
        break;
    }
}

var TempoAction::getValue()
{
    var val = var();

    Tempo* target = Brain::getInstance()->getTempoById(targetId->intValue());
    if (target == nullptr) return val;

    if (actionType == TEMPO_SET) {
        val = jmap(target->bpm->floatValue(), (float)fromValue->floatValue(), (float)toValue->floatValue(), 0.0f, 1.0f);
    }
    return val;
}

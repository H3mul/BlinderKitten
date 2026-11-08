#include "PaletteAction.h"
#include "../Palette/PaletteManager.h"
#include "../Palette/PaletteRuntime.h"
#include "../../Brain.h"

PaletteAction::PaletteAction(var params) :
    Action(params)
{
    actionType = (ActionType)(int)params.getProperty("actionType", PAL_SET);

    if (actionType == PAL_SET || actionType == PAL_NEXT || actionType == PAL_PREVIOUS) {
        paletteId = addIntParameter("Palette", "ID of the target palette", 1, 1);
    }
    if (actionType == PAL_BLEND) {
        paletteId = addIntParameter("Palette", "ID of the palette to blend, 0 = every palette", 0, 0);
        stateA = addIntParameter("State A", "State shown with the fader at 0 (1 = first)", 1, 1);
        stateB = addIntParameter("State B", "State shown with the fader at 1 (1 = first)", 2, 1);
    }
    if (actionType == PAL_SHIFT_ALL_HOLD) {
        holdId = PaletteRuntime::beginHold();
    }
    if (actionType == PAL_BLEND || actionType == PAL_BLEND_ALL) {
        ownerId = PaletteRuntime::newOwnerId();
    }
    if (actionType == PAL_SET) {
        stateIndex = addIntParameter("State", "Position of the state to select (1 = first)", 1, 1);
    }
    if (actionType == PAL_SHIFT_ALL || actionType == PAL_SHIFT_ALL_HOLD || actionType == PAL_BLEND_ALL) {
        stateName = addStringParameter("State name", "Every palette that has a state with this exact name is set to it", "reverse");
    }
}

PaletteAction::~PaletteAction()
{
    if (ownerId > 0) PaletteRuntime::clearOwner(ownerId);
}

void PaletteAction::triggerInternal()
{
}

void PaletteAction::shiftAll()
{
    String name = stateName->stringValue();
    for (auto* p : PaletteManager::getInstance()->items) {
        int idx = p->findStateIndex(name);
        if (idx > 0) {
            if (actionType == PAL_SHIFT_ALL_HOLD) PaletteRuntime::holdPalette(holdId, p, idx);
            else p->setStateIndex(idx);
        }
    }
}

void PaletteAction::blendAll(float value)
{
    String name = stateName->stringValue();
    for (auto* p : PaletteManager::getInstance()->items) {
        int b = p->findStateIndex(name);
        if (b > 0) PaletteRuntime::setLayer(p, ownerId, b, value);
    }
}

void PaletteAction::setValueInternal(var value, String origin, int incrementIndex, bool isRelative)
{
    float val = value;

    if (actionType == PAL_SHIFT_ALL_HOLD) {
        if (val == 1) {
            PaletteRuntime::releaseHold(holdId); // a repeated press must not stack on itself
            shiftAll();
        }
        else if (val == 0) {
            PaletteRuntime::releaseHold(holdId);
        }
        return;
    }

    if (actionType == PAL_BLEND_ALL) { // fader: every value counts
        blendAll(val);
        return;
    }

    if (actionType == PAL_BLEND) { // fader: every value counts
        for (auto* p : PaletteManager::getInstance()->items) {
            if (paletteId->intValue() == 0 || paletteId->intValue() == p->registeredId) PaletteRuntime::setLayer(p, ownerId, stateB->intValue(), val, stateA->intValue());
        }
        return;
    }

    if (val != 1) return; // press only

    if (actionType == PAL_SHIFT_ALL) {
        shiftAll();
        return;
    }

    Palette* p = Brain::getInstance()->getPaletteById(paletteId->intValue());
    if (p == nullptr) return;
    int n = p->states.items.size();
    if (n == 0) return;
    int cur = jlimit(1, n, p->currentState->intValue());

    switch (actionType) {
    case PAL_SET:      p->setStateIndex(stateIndex->intValue()); break;
    case PAL_NEXT:     p->setStateIndex(cur % n + 1); break;
    case PAL_PREVIOUS: p->setStateIndex(cur == 1 ? n : cur - 1); break;
    default: break;
    }
}

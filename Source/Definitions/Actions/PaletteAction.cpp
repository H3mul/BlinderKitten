#include "PaletteAction.h"
#include "../Palette/PaletteManager.h"
#include "../../Brain.h"

PaletteAction::PaletteAction(var params) :
    Action(params)
{
    actionType = (ActionType)(int)params.getProperty("actionType", PAL_SET);

    if (actionType == PAL_SET || actionType == PAL_NEXT || actionType == PAL_PREVIOUS) {
        paletteId = addIntParameter("Palette", "ID of the target palette", 1, 1);
    }
    if (actionType == PAL_SET) {
        stateIndex = addIntParameter("State", "Position of the state to select (1 = first)", 1, 1);
    }
    if (actionType == PAL_SHIFT_ALL || actionType == PAL_SHIFT_ALL_HOLD) {
        stateName = addStringParameter("State name", "Every palette that has a state with this exact name is set to it", "reverse");
    }
}

PaletteAction::~PaletteAction()
{
}

void PaletteAction::triggerInternal()
{
}

void PaletteAction::shiftAll()
{
    String name = stateName->stringValue();
    for (auto* p : PaletteManager::getInstance()->items) {
        int idx = p->findStateIndex(name);
        if (idx > 0) p->setStateIndex(idx);
    }
}

void PaletteAction::restoreSnapshot()
{
    for (int i = 0; i < snapshotIds.size(); i++) {
        Palette* p = Brain::getInstance()->getPaletteById(snapshotIds[i]);
        if (p != nullptr) p->setStateIndex(snapshotStates[i]);
    }
    snapshotIds.clear();
    snapshotStates.clear();
}

void PaletteAction::setValueInternal(var value, String origin, int incrementIndex, bool isRelative)
{
    float val = value;

    if (actionType == PAL_SHIFT_ALL_HOLD) {
        if (val == 1) {
            snapshotIds.clear();
            snapshotStates.clear();
            for (auto* p : PaletteManager::getInstance()->items) {
                snapshotIds.add(p->registeredId);
                snapshotStates.add(p->currentState->intValue());
            }
            shiftAll();
        }
        else if (val == 0) {
            restoreSnapshot();
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

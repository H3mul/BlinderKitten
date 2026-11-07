/*
  ==============================================================================

    TempoManagerUI.cpp

  ==============================================================================
*/

#include "TempoManagerUI.h"
#include "TempoManager.h"

TempoManagerUI::TempoManagerUI(const String & contentName) :
	BaseManagerShapeShifterUI(contentName, TempoManager::getInstance())
{
	addItemText = "Add new tempo";
	noItemText = "Tempo is a shared BPM. Effects and carousels bound to it by tempo id follow its bpm x scale";
	setShowSearchBar(false);
	addExistingItems();
}

TempoManagerUI::~TempoManagerUI()
{
}

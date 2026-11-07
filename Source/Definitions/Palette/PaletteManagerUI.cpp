#include "PaletteManagerUI.h"
#include "PaletteManager.h"

PaletteManagerUI::PaletteManagerUI(const String& contentName) :
	BaseManagerShapeShifterUI(contentName, PaletteManager::getInstance())
{
	addItemText = "Add new palette";
	noItemText = "Store here your base / accent preset tuples and their states";
	addExistingItems();
}

PaletteManagerUI::~PaletteManagerUI()
{
}

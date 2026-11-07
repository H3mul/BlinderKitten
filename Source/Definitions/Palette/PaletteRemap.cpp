#include "JuceHeader.h"
#include "PaletteRemap.h"
#include "Palette.h"
#include "../../Brain.h"

PaletteRemap::PaletteRemap(var params) :
	BaseItem(params.getProperty("name", "Remap"))
{
	itemDataType = "PaletteRemap";
	fromId = addIntParameter("From palette", "Palette id referenced by the commands of this cue", 0, 0);
	toId = addIntParameter("To palette", "Palette id used instead", 0, 0);
	note = addStringParameter("Check", "Advisory check of the two palettes' shapes", "");
	note->isControllableFeedbackOnly = true;
	note->isSavable = false;
}

PaletteRemap::~PaletteRemap()
{
}

void PaletteRemap::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == fromId || p == toId) updateNote();
}

void PaletteRemap::afterLoadJSONDataInternal()
{
	BaseItem::afterLoadJSONDataInternal();
	updateNote();
}

void PaletteRemap::updateNote()
{
	String msg;
	if (fromId->intValue() > 0 && toId->intValue() > 0) {
		Palette* a = Brain::getInstance()->getPaletteById(fromId->intValue());
		Palette* b = Brain::getInstance()->getPaletteById(toId->intValue());
		if (b == nullptr) msg = "Target palette does not exist: the original is used";
		else if (a != nullptr && a->slotCount->intValue() != b->slotCount->intValue())
			msg = "Warning: slot count differs (" + a->slotCount->stringValue() + " vs " + b->slotCount->stringValue() + "), missing slots use the original palette";
		else if (a != nullptr && a->getSlotNameList() != b->getSlotNameList())
			msg = "Warning: slot names differ (" + a->slotNames->stringValue() + " vs " + b->slotNames->stringValue() + ")";
	}
	if (note->stringValue() != msg) note->setValue(msg);
}

#pragma once
#include "JuceHeader.h"
#include "Palette.h"

class PaletteManagerUI :
	public BaseManagerShapeShifterUI<BaseManager<Palette>, Palette, BaseItemUI<Palette>>
{
public:
	PaletteManagerUI(const String& contentName);
	~PaletteManagerUI();

	static PaletteManagerUI* create(const String& name) { return new PaletteManagerUI(name); }
};

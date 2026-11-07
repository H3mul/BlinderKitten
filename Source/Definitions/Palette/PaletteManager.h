#pragma once
#include "Palette.h"

class PaletteManager :
    public BaseManager<Palette>
{
public:
    juce_DeclareSingleton(PaletteManager, true);

    PaletteManager();
    ~PaletteManager();
};

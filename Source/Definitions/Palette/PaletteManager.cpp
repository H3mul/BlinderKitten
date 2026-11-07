#include "Palette.h"
#include "PaletteManager.h"

juce_ImplementSingleton(PaletteManager);

static int comparePalettes(Palette* A, Palette* B) {
    return (int)A->id->getValue() - (int)B->id->getValue();
}

PaletteManager::PaletteManager() :
    BaseManager("Palette")
{
    itemDataType = "Palette";
    selectItemWhenCreated = true;
    comparator.compareFunc = comparePalettes;
}

PaletteManager::~PaletteManager()
{
}

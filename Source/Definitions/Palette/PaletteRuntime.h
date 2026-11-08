/*
  ==============================================================================

    PaletteRuntime.h

    Runtime side of palettes (v2): fades between states, the scoped refresh of
    the consumers that ref a palette, momentary hold stacks, preset renumber
    upkeep and the default preset palette ref.

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class Palette;
class Cuelist;
class Carousel;
class Mapper;

class PaletteRuntime :
    private Timer
{
public:
    // A palette's currentState changed: start the fade (or snap) and refresh the consumers that ref it.
    static void stateChanged(Palette* p);
    // Fader crossfade: value 0..1 between two states, no timed fade; currentState follows the nearer end.
    static void setBlend(Palette* p, int stateA, int stateB, float value);

    // Preset renumbered / presets added or removed: keep slots and slot labels in step.
    static void presetRenumbered(int from, int to);
    static void presetsChanged();

    // Default preset id: the engine's, or the palette slot it points at when one is set.
    static int defaultPresetId();

    // Momentary shifts: one stack of holds per palette.
    static int beginHold();
    static void holdPalette(int holdId, Palette* p, int appliedState);
    static void releaseHold(int holdId);

private:
    PaletteRuntime() {}
    static PaletteRuntime& get();

    struct Consumers {
        Array<int> paletteIds;   // the palette plus palettes remapped onto it
        Array<Cuelist*> cuelists;
        Array<Carousel*> carousels;
        Array<Mapper*> mappers;
        double scanned = 0;
    };
    struct Hold { int holdId; int paletteId; int applied; int prior; };

    HashMap<int, std::shared_ptr<Consumers>> consumers;
    Array<int> pending;
    Array<int> fading;
    Array<Hold> holds;
    int nextHold = 0;
    std::atomic<bool> flushPending{ false };

    void markPending(Palette* p);
    void flush();
    void timerCallback() override;
    std::shared_ptr<Consumers> getConsumers(Palette* p);
    std::shared_ptr<Consumers> scan(Palette* p);
    void refresh(Palette* p);
};

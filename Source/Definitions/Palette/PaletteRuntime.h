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
    // Fader layers (v3): weight 0 removes the owner's layer. baseIfFirst > 0 sets the base when this creates the palette's first layer.
    static int newOwnerId();
    static void setLayer(Palette* p, int ownerId, int targetState, float weight, int baseIfFirst = 0);
    static void clearLayer(Palette* p, int ownerId);
    static void clearOwner(int ownerId); // every palette: the owner is gone

    // Preset renumbered / presets added or removed: keep slots and slot labels in step.
    static void presetRenumbered(int from, int to);
    static void presetsChanged();

    // Default preset id: the engine's, or the palette slot it points at when one is set.
    static int defaultPresetId();

    // Momentary shifts: one stack of holds per palette.
    static int beginHold();
    static void holdPalette(int holdId, Palette* p, int appliedState);
    static void releaseHold(int holdId);
    static void clearHolds(); // drop every hold without restoring (reset action)

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
    int nextOwner = 0, nextLayerSeq = 0;
    std::atomic<bool> flushPending{ false };

    void markPending(Palette* p);
    void layersChanged(Palette* p);
    void flush();
    void timerCallback() override;
    std::shared_ptr<Consumers> getConsumers(Palette* p);
    std::shared_ptr<Consumers> scan(Palette* p);
    void refresh(Palette* p);
};

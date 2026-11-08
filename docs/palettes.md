# Palettes

A palette lets a cue say "base colour" and "accent colour" instead of naming presets. The palette decides which presets those are, and you can change that decision while the show runs.

Open it from **View > Lists > Palettes**.

## The palette object

- **ID / Name**: like presets, a palette is addressed by its id.
- **Slots** and **Slot names**: the roles of the palette, for example `base,accent`. The slot's position is its role, the name is a label only. Palettes do not have to share a shape.
- **States**: a list of named states (for example `normal`, `reverse`). Each state holds one preset id per slot (0 = nothing). A preset id that no longer exists shows `MISSING PRESET` in the slot's tooltip and applies nothing.
- **Current state**: the state in use (1 = first).
- **Fade time** and **Timing preset**: see Fades.

Saved shows without palettes load as before.

## Using a palette in a command

A command value has a `Palette` type next to `Value` and `Preset`: pick the palette id, `Slot From` and `Slot To`. With a selection of several fixtures, the slots spread across it like a preset thru, so slot 1 to slot 2 gives a base to accent gradient. Commands in cues, carousel steps and mapper steps all accept it.

An empty slot, a missing preset or a missing palette applies nothing and logs nothing.

## Remap a palette per cue

A cue has a remap list (`A -> B`). While that cue plays, a reference to palette A uses palette B instead, so a duplicated cue can be recoloured with one pair. The remap applies one level only. If B does not exist or lacks the slot, the original palette is used. Commands in the programmer are not remapped.

## Changing the state

Actions (category **Palette**, usable on virtual buttons, faders and cue tasks):

| Action | Behaviour |
|---|---|
| Set palette state | Set one palette to a state number (press only). |
| Next / Previous palette state | Step one palette, wrapping (press only). |
| Shift all palettes to state | Every palette that has a state with that exact name goes to it (press only). |
| Shift all palettes to state (momentary) | Press shifts, release returns each palette to its previous state. Overlapping momentary buttons restore correctly in either release order, and a palette another action changed during the hold stays as set. |
| Blend palette states (fader) | A fader crossfades one palette (or all with id 0) between state A and state B. |
| Blend all palettes to state (fader) | A fader crossfades every palette from the state it was in when the fader left 0 to the named state. At 100% the named state is committed, back at 0 each palette returns. |

A state change reaches running cues, carousels and mappers that use the palette without re-triggering them. Only consumers that reference the palette are re-resolved.

## Fades

By default a state change is instant. Set **Fade time** (seconds) on a palette to fade between states, or set **Timing preset** to a timing preset id to use that preset's fade value (it wins over Fade time, and one timing preset can drive several palettes). Only the preset's base fade is used, not its delay, thru, symmetry, random or curves.

A state change during a fade jumps to the previous target state and fades from there. Colours between two states are interpolated per channel (a channel in only one state, or a release, switches at the midpoint).

## Default preset

In the engine settings, **Default palette ID** and **Default palette slot** make the default values follow a palette slot. With the id at 0 the normal **Default preset ID** applies.

## Renumbering

Renumbering a preset rewrites palette slots that hold the old id.

## Limits (v2)

- Effect rows cannot use palettes, and effects are not refreshed by a palette change.
- No global override layer or fader blend of every cue-bound palette at once; no cuelist-level remap.
- No OSC state feedback or dump for palettes.
- Deleting a preset leaves the slot as it was, marked missing.
- Editing a palette, preset or command while its cue plays shows only with the cuelist's **Live edit** on; otherwise it applies at the next trigger. A state change always applies at once.

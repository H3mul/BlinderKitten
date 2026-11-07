#pragma once
#include "JuceHeader.h"

// One cue-level remap pair: refs to palette `fromId` resolve through palette `toId` for this cue's commands.
class PaletteRemap :
    public BaseItem
{
public:
    PaletteRemap(var params = var());
    ~PaletteRemap();

    IntParameter* fromId;
    IntParameter* toId;
    StringParameter* note;   // advisory check, not saved

    void onContainerParameterChangedInternal(Parameter* p) override;
    void afterLoadJSONDataInternal() override;
    void updateNote();

    String getTypeString() const override { return "PaletteRemap"; }
    static PaletteRemap* create(var params) { return new PaletteRemap(params); }
};

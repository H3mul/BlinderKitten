#pragma once
#include "JuceHeader.h"

class Cuelist;

// One command layer a command, task or include row is attributed to. `id` is what is saved (a layer rename never breaks a row);
// `pick` shows the layer names of the host cuelist and is only for choosing.
class CommandLayerRefItem :
    public BaseItem
{
public:
    CommandLayerRefItem(var params = var());
    ~CommandLayerRefItem() {}

    IntParameter* refId;
    EnumParameter* pick;
    bool syncing = false;

    void refreshOptions(Cuelist* host);
    void onContainerParameterChangedInternal(Parameter* p) override;
    String getTypeString() const override { return "CommandLayerRef"; }
    static CommandLayerRefItem* create(var params) { return new CommandLayerRefItem(params); }
};

// The command layers of one command, task or include row. Empty = the base layer (commands, tasks), keep their own (Into), all (Only).
class CommandLayerRefs :
    public BaseManager<CommandLayerRefItem>
{
public:
    CommandLayerRefs(const String& name = "Command layers");
    ~CommandLayerRefs() {}

    Cuelist* hostCuelist();            // the cuelist of the cue this row is in
    String ids();                      // "1,3", for resolveCommandLayers
    String names();                    // "Body+Peak"; ids the host does not declare show as #7
    String titlePrefix();              // "> Body+Peak | " or empty, for a collapsed row
    void refreshOptions();
    void rowChanged();                 // tells the command or task that owns this list to rename itself

    void addItemInternal(CommandLayerRefItem* o, var data) override;
    void removeItemInternal(CommandLayerRefItem* o) override;
    void afterLoadJSONDataInternal() override;
};

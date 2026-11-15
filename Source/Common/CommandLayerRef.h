#pragma once

// A cuelist or one of its layers: layer 0 is the cuelist itself, n (1 to 9) its layer n.
struct CommandLayerRef
{
    int cuelist = 0;
    int commandLayer = 0;
    bool operator==(const CommandLayerRef& o) const { return cuelist == o.cuelist && commandLayer == o.commandLayer; }
    bool operator!=(const CommandLayerRef& o) const { return !(*this == o); }
};

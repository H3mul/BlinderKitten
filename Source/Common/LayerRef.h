#pragma once

// A cuelist or one of its layers: layer 0 is the cuelist itself, n (1 to 9) its layer n.
struct LayerRef
{
    int cuelist = 0;
    int layer = 0;
    bool operator==(const LayerRef& o) const { return cuelist == o.cuelist && layer == o.layer; }
    bool operator!=(const LayerRef& o) const { return !(*this == o); }
};

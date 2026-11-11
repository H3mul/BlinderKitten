#pragma once
#include "JuceHeader.h"

// Which cuelists started a size-driven object (effect, carousel, mapper, tracker) from a cue task, with "Scale effects with level" on.
// mult = largest HTP*LTP level of those cuelists, 1 when none or when something else also started it. Runtime only, never saved.
struct SizeOwners
{
    Array<int> ids;
    bool unowned = false;       // also started by a cuelist without the option
    bool ownerStopped = false;  // last stop came from the owner level reaching 0: keep links so a rise restarts it
    float mult = 1;

    void clear() { ids.clear(); unowned = false; ownerStopped = false; }
    // object stopped for good: forget the links, unless the owner level is what stopped it
    void clearOnStop() { if (!ownerStopped) { ids.clear(); unowned = false; mult = 1; } }
};

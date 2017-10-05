
#if ! defined HMAGIC_DUNGEON_LOGIC_HEADER_GUARD

    #define HMAGIC_DUNGEON_LOGIC_HEADER_GUARD

// =============================================================================

#include "structs.h"

#include "Wrappers.h"

// =============================================================================

    HM_TextResource entrance_names;

// =============================================================================

    unsigned char const *
    Drawmap(unsigned char  const * const rom,
            unsigned short       * const nbuf,
            unsigned char  const *       map,
            DUNGEDIT             * const ed);

    void
    LoadHeader(DUNGEDIT * const ed,
               int        const map);

// =============================================================================

#endif
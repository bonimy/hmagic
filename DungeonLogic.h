
#if ! defined HMAGIC_DUNGEON_LOGIC_HEADER_GUARD

    #define HMAGIC_DUNGEON_LOGIC_HEADER_GUARD

// =============================================================================

#include "structs.h"

// =============================================================================

    unsigned char const *
    Drawmap(unsigned char  const * const rom,
            unsigned short       * const nbuf,
            unsigned char  const *       map,
            DUNGEDIT             * const ed);

// =============================================================================

#endif
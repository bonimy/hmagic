
#if ! defined HMAGIC_LEVEL_MAP_LOGIC_HEADER_GUARD

    #define HMAGIC_LEVEL_MAP_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    extern char const *
    level_str[];

// =============================================================================

    extern void
    lmapblkchg(HWND             const win,
               LMAPEDIT const * const ed);

// =============================================================================

    extern void
    Paintfloor(LMAPEDIT * const ed);

// =============================================================================

    extern void
    Savedungmap(LMAPEDIT const * const  ed);


#endif

#if ! defined HMAGIC_TRACKER_LOGIC_HEADER_GUARD

    #define HMAGIC_TRACKER_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    extern int mark_sr;

    extern int mark_start;
    extern int mark_end;
    extern int mark_first;
    extern int mark_last;

    extern char op_len[32];

// =============================================================================

    extern short
    AllocScmd(FDOC*doc);

    extern short
    Getblocktime(FDOC *doc, short num, short prevtime);

    extern void
    Savesongs(FDOC *doc);

// =============================================================================

#endif
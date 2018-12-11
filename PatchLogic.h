
#if ! defined HMAGIC_PATCH_LOGIC_HEADER_GUARD

#define HMAGIC_PATCH_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    extern int
    Patch_Build
    (
        FDOC * const doc
    );

    extern void
    Removepatches
    (
        FDOC * const doc
    );

    extern int
    Doc_FreePatchInputs
    (
        CP2(FDOC) doc
    );

    extern BOOL
    PatchLogic_DeserializePatches
    (
        CP2(FDOC) p_doc
    );

// =============================================================================

#endif
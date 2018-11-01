
#if ! defined HMAGIC_SAMPLE_EDIT_LOGIC_HEADER_GUARD

    #define HMAGIC_SAMPLE_EDIT_LOGIC_HEADER_GUARD

    #include "structs.h"

    extern void
    SampleEdit_CopyToClipboard(SAMPEDIT const * const p_ed);

    extern BOOL
    SampleEdit_PasteFromClipboard(SAMPEDIT * const p_ed);

#endif

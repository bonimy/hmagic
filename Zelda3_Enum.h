

#if ! defined HMAGIC_ZELDA3_ENUM_HEADER_GUARD

#define HMAGIC_ZELDA3_ENUM_HEADER_GUARD

// =============================================================================

    #include "HMagicEnum.h"

// =============================================================================

    enum
    {
        ID_Z3Dlg_TreeView = ID_SuperDlg_FirstChild,
        
        ID_Z3Dlg_AfterLast,
        
        NUM_Z3Dlg_IDs = (ID_Z3Dlg_AfterLast - ID_SuperDlg_FirstChild)
    };

// =============================================================================

#endif
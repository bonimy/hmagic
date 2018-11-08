
#if !defined HMAGIC_OVERWORLD_ENUM_HEADER_GUARD

    #define HMAGIC_OVERWORLD_ENUM_HEADER_GUARD

// =============================================================================

    #include "HMagicEnum.h"

// =============================================================================

    enum
    {
        SD_Over_Map32_Selector = ID_SuperDlg_FirstChild,
        SD_Over_Display,
        SD_OverGrid32CheckBox = 3012,
        
        // \task Fill in others.
        
        SD_OverUpArrow = 3031,
        SD_OverDownArrow,
        SD_OverSprTileSetStatic,
        SD_OverSprTileSetEditCtl,
        SD_OverMapSearchBtn,
        SD_OverAdjustSearchBtn,
        SD_OverOverlayChkBox,
        SD_OverMap16_Selector,
        SD_OverWindowFocus,
        SD_OverAfterLast,
        
        SD_OverNumControls = (SD_OverAfterLast - ID_SuperDlg_FirstChild),
    };

    enum
    {
        SD_OverShowMarkers = 1 << 2
    };

// =============================================================================

#endif

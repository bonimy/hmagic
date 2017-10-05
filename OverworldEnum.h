
#if !defined HMAGIC_OVERWORLD_ENUM_HEADER_GUARD

    #define HMAGIC_OVERWORLD_ENUM_HEADER_GUARD

// =============================================================================

    enum
    {
        SD_GenericFirst = 3000,
        SD_OverFirst = 3000,
        
        SD_OverGrid32CheckBox = 3012,
        
        // \task Fill in others.
        
        SD_OverWindowFocus = 3039,
        SD_OverAfterLast,
        
        SD_OverNumControls = (SD_OverAfterLast - SD_OverFirst),
    };

    enum
    {
        SD_OverShowMarkers = 1 << 2
    };

// =============================================================================

#endif

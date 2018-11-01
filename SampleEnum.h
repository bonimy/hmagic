
#if ! defined HMAGIC_SAMPLE_ENUM_HEADER_GUARD

    #define HMAGIC_SAMPLE_ENUM_HEADER_GUARD

// =============================================================================

    /// IDs for controls used in the dungeon superdialog.
    enum
    {
        ID_Samp_SampleIndexStatic = 3000,
        ID_Samp_SampleIndexEdit,
        ID_Samp_Display,
        ID_Samp_SampleIsCopyCheckBox,
        ID_Samp_SampleCopyOfIndexEdit,
        ID_Samp_CopyToClipboardButton,
        ID_Samp_PasteFromClipboardButton,
        ID_Samp_LengthLabel,
        ID_Samp_SampleLengthEdit,
        ID_Samp_LoopCheckBox,
        ID_Samp_LoopPointEdit,

        // \task Once all numbers have been named, delete the assignment
        // of a specific number to this enumerated value. It will thereafter
        // be automatic.
        ID_Samp_AfterLast = 3023,
    };

    enum
    {
        Samp_Index_Changed     = 1 << 0,
        Samp_CopyIndex_Changed = 1 << 1,
    };

// =============================================================================

#endif
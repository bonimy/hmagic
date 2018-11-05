
#if ! defined HMAGIC_SAMPLE_ENUM_HEADER_GUARD

    #define HMAGIC_SAMPLE_ENUM_HEADER_GUARD

// =============================================================================

    /// IDs for controls used in the dungeon superdialog.
    enum
    {
        ID_Samp_First = 3000,
        
        ID_Samp_SampleIndexStatic = ID_Samp_First,
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
        ID_Samp_InstrumentLabel,
        ID_Samp_InstrumentIndexEdit,
        ID_Samp_InstrFrequencyLabel,
        ID_Samp_InstrFrequencyEdit,
        ID_Samp_InstrADSR_Label,
        ID_Samp_InstrADSR_Edit,
        ID_Samp_InstrGainLabel,
        ID_Samp_InstrGainEdit,
        ID_Samp_InstrSampleIndexLabel,
        ID_Samp_InstrSampleIndex_Edit,
        ID_Samp_PlayButton,
        ID_Samp_StopButton,

        // Represents an invalid, out of bounds control ID.
        ID_Samp_AfterLast,
    };

    enum
    {
        Samp_Index_Changed            = 1 << 0,
        Samp_CopyIndex_Changed        = 1 << 1,
        Samp_Unused_Flag              = 1 << 2,
        Samp_Unused_Flag2             = 1 << 3,
        Samp_Length_Changed           = 1 << 4,
        Samp_LoopPoint_Changed        = 1 << 5,
        Samp_InstrumentIndex_Changed  = 1 << 6,
        Samp_InstrumentConfig_Changed = 1 << 7

    };

// =============================================================================

#endif
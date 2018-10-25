


#if ! defined HMAGIC_ENUM_HEADER_GUARD

    #define HMAGIC_ENUM_HEADER_GUARD

// =============================================================================

    enum
    {
        CFG_NOFLAGS         = 0,
        CFG_SPRNAMES_LOADED = 1 << 0,
        CFG_MRU_LOADED      = 1 << 1,
        CFG_SNDVOL_LOADED   = 1 << 2,
        CFG_MIDI_LOADED     = 1 << 3,
        CFG_ASM_LOADED      = 1 << 4
    };

    enum
    {
        ID_MRU1 = 5000,
        ID_MRU2,
        ID_MRU3,
        ID_MRU4
    };

// =============================================================================

#endif
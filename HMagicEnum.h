


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
    
    enum
    {
        /**
            \note 
            \author MathOnNapkins
            
            This program creates numerous child frame windows (children of the
            main frame). These frames are what are commonly thought of as
            MDI child windows, and they defer their actual presentation logic,
            for the most part, to child windows within them. In Hyrule Magic,
            these child windows are called "Super Dialogs".
            
            One might ask why they have such a name, and the answer, I believe,
            is that unlike the typical dialog templates that the resource editor
            can produce, Super Dialogs as implemented in this program can be
            easily embedded as children of any window. This is not to say that
            the normal dialog templates produced by Visual Studio *can not* be
            embedded in other windows; moreso that there are more potential
            pitfalls in that approach.
            
        */
        ID_SuperDlg = 2000,
        
        ID_SuperDlg_FirstChild = 3000,
        ID_SuperDlg_LastChild  = 3999,
        
        ID_SuperDlg_Unknown = 4000,
    };

// =============================================================================

#endif
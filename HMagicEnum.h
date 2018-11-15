
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

// =============================================================================

    enum
    {
        ID_MRU1 = 5000,
        ID_MRU2,
        ID_MRU3,
        ID_MRU4
    };
    
// =============================================================================

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

    /**
        \author MathOnNapkins (of the documentation)
        \credit sephiroth3 (or whoever designed this scheme)
        
        Far-orientation flags for SuperDlg children. What does far-oriented
        mean, you ask? Let's take In typical creation and placement of controls,
        the x and y coordinates would be considered as absolute placements,
        relative to the left-hand or top-hand edges of the parent window. They
        are child windows, after all.
        
        Let's take an x coordinate as an example for how far-oriented placement
        would change how a child window is placed. Assume a parent window that
        currently has a width of 600 units. (The SuperDlg is the parent here.)
        If a child window's specified x coordinate is 70 units, normally this
        would mean that it would be placed 70 units from the left boundary
        of the parent window. If we apply a far-oriented flag to the child
        window, then x coordinate of 70 units would instead be interpreted as
        70 units from the right-hand boundary of the parent window. Thus,
        if that flag is used, we should expect that the child window will be
        placed at x coordinate (600 - 70) = 530 units. This is useful for,
        in a certain sense, anchoring a control to a far side of the SuperDlg.
        
        Let us examine what this means in the context of width of the child
        window. If the child window has a specified width of 10 units, this
        would normally indicate that the child spans 10 units horizontally from
        its left boundary to its right boundary. If the far-oriented flag
        is indicated for the child window, however, then a width of 10 units
        would be interpreted as 10 unit displacement from the right hand
        bound of the parent window. Using our previous example, this would mean
        that the child window's left bound is still at 530 units from the
        parent's left boundary, and the child window's right boundary is 
        10 units from the parent's right boundary. That puts the child's right
        boundary at 590 units, relative to the parent's left boundary.
        The calculation to determine the child's absolute width is:
        
        600 - 530 - 10 = 60 units
        
        The same logic applies to y coordinate and height parameters, with
        far-oriented interpeting them as displacements from the bottom
        boundary of the parent window.
        
        It might not be apparent why this is useful, but it allows for a
        certain degree of control over window placement and auto-sizing
        as the parent window (the SuperDlg) is itself resized.
    */
    enum
    {
        /// Read "far-oriented x coordinate for super dialog child window".
        FLG_SDCH_FOX = 1 << 0,
        
        /// Read "far-oriented width for super dialog child window".
        FLG_SDCH_FOW = 1 << 1,
        
        /// Read "far-oriented y coordinate for super dialog child window."
        FLG_SDCH_FOY = 1 << 2,
        
        /// Far "far-oriented height for super dialog child window".
        FLG_SDCH_FOH = 1 << 3,
        
        // Combination of flags.
        // Read "far-oriented x and y coordinates"
        FLG_SDCH_FOXY = (FLG_SDCH_FOX | FLG_SDCH_FOY),
        
        // Combination of flags.
        // Read "far-oriented width and height".
        FLG_SDCH_FOWH = (FLG_SDCH_FOW | FLG_SDCH_FOH),
        
        // Combination of flags.
        // Read "far-oriented absolutely".
        FLG_SDCH_FOA   = (FLG_SDCH_FOXY | FLG_SDCH_FOWH),
    };

// =============================================================================

#endif
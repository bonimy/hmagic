
    #include "structs.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "GdiObjects.h"

    #include "resource.h"

// =============================================================================

    static WNDPROC
    DefaultEditProc;

// =============================================================================

    HM_DeclareWndProc(NumEditProc)
    {
        switch(p_msg)
        {
        
        default:
            
            break;
            
        case WM_CHAR:
            
            if(always)
            {
                enum { backspace_char = '\x08' };
                
                unsigned short c = (unsigned short) p_wp;
                
                // Only pass numeric and backspace characters on to the
                // standard edit window proc.
                if('0' <= c && c <= '9')
                {
                    break;
                }
                else if(backspace_char == c)
                {
                    break;
                }
                
                return 0;
            }
        }
        
        return DefaultEditProc(p_win, p_msg, p_wp, p_lp);
    }

// =============================================================================

    HM_WindowClass frame_classes[] =
    {
        {
            MDI_FrameWnd,
            "ZEFRAME"
        },
        {
            docproc,
            "ZEDOC",
        },
        {
            overproc,
            "ZEOVER",
        },
        {
            dungproc,
            "ZEDUNGEON",
        },
        {
            PatchFrameProc,
            "PATCHLOAD"
        },
        {
            TextFrameProc,
            "ZTXTEDIT"
        },
        {
            wmapproc,
            "WORLDMAP"
        },
        {
            PalaceMapFrame,
            "LEVELMAP"
        },
        {
            tmapproc,
            "TILEMAP"
        },
        {
            AudioFrameProc,
            "MUSBANK"
        },
        {
            PerspectiveFrameProc,
            "PERSPEDIT"
        },
        {
            PaletteFrameProc,
            "PALEDIT",
            (CS_HREDRAW | CS_VREDRAW),
        },
    };

// =============================================================================

    /**
        Dialog control windows
    */
    HM_WindowClass control_classes[] =
    {
        {
            blkedit8proc,
            "BLKEDIT8"
        },
        {
            blksel8proc,
            "BLKSEL8"
        },
        {
            blksel16proc,
            "BLKSEL16"
        },
        {
            blkedit16proc,
            "BLKEDIT16"
        },
        {
            blksel32proc,
            "BLKSEL32"
        },
        {
            blkedit32proc,
            "BLKEDIT32"
        },
        {
            DungeonMapProc,
            "DUNGEON",
            0,
            0,
            &null_cursor
        },
        {
            overmapproc,
            "OVERWORLD"
        },
        {
            lmapdispproc,
            "LMAPDISP"
        },
        {
            perspdispproc,
            "PERSPDISP",
            0,
            &black_brush
        },
        {
            trackeditproc,
            "TRACKEDIT"
        },
        {
            tmapdispproc,
            "TMAPDISP",
        },
        {
            wmapdispproc,
            "WMAPDISP"
        },
        {
            lmapblksproc,
            "LMAPBLKS",
            (CS_HREDRAW | CS_VREDRAW)
        },
        {
            trackerproc,
            "TRAX0R",
            0,
            &white_brush
        },
        {
            SampleDisplayProc,
            "SAMPEDIT",
            0,
            &black_brush
        },
        {
            blk16search,
            "SEARCH16"
        },
        {
            PaletteSelector,
            "PALSELECT"
        },
        {
            dpceditproc,
            "DPCEDIT"
        },
        {
            dungselproc,
            "DUNGSEL"
        },
        {
            SuperDlg,
            "SUPERDLG",
            0,
            &super_dlg_brush,
            &normal_cursor,

            // \note This class "needs" extra allocation merely because it
            // stores a copy of the structure that was used to create it, and
            // it maintains that structure as the super dialog is resized,
            // etc. While it is possible that the program could be rewritten
            // to not require this extra space, it's probably not worth doing
            // so right now. To be clear, the issue is that GWLP_USERDATA
            // is already occupied by a pointer to the SDCREATE structure of
            // the dialog.
            DWLP_USER + sizeof(LONG_PTR)
        }
    };

// =============================================================================

    enum
    {
        NUM_FrameClasses   = MACRO_ArrayLength(frame_classes),
        NUM_ControlClasses = MACRO_ArrayLength(control_classes)
    };

// =============================================================================

    static void
    HM_RegisterFrameClasses(HINSTANCE const p_inst)
    {
        int i = 0;
        
        WNDCLASS wc = { 0 };
        
        // -----------------------------
        
        wc.hInstance = p_inst;
        
        for(i = 0; i < NUM_FrameClasses; i += 1)
        {
            CP2(HM_WindowClass) cfc = &frame_classes[i];
            
            // -----------------------------
            
            wc.style  = cfc->m_style;
            wc.style |= (CS_DBLCLKS);
            
            wc.hIcon   = alt_icon;
            
            if(cfc->m_brush)
            {
                // If a pointer to a specific brush object is supplied, use it.
                wc.hbrBackground = cfc->m_brush[0];
            }
            else
            {
                wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
            }
            
            if(cfc->m_cursor)
            {
                wc.hCursor = cfc->m_cursor[0];
            }
            else
            {
                wc.hCursor = normal_cursor;
            }
            
            wc.lpfnWndProc   = cfc->m_proc;
            wc.lpszClassName = cfc->m_class_name;
            
            cfc->m_atom = RegisterClass(&wc);
        }
    }

// =============================================================================

    /**
        These classes assume double clicking is valid, mainly. They also
        have a default brush of grey, etc.
    */
    static void
    HM_RegisterControlClasses(HINSTANCE const p_inst)
    {
        int i = 0;
        
        WNDCLASS wc = { 0 };
        
        // -----------------------------
        
        wc.hInstance = p_inst;
        
        for(i = 0; i < NUM_ControlClasses; i += 1)
        {
            CP2(HM_WindowClass) cfc = &control_classes[i];
            
            // -----------------------------
            
            if(cfc->m_brush)
            {
                // If a pointer to a specific brush object is supplied, use it.
                wc.hbrBackground = cfc->m_brush[0];
            }
            else
            {
                wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
            }
            
            wc.hIcon = alt_icon;
            
            if(cfc->m_cursor)
            {
                wc.hCursor = cfc->m_cursor[0];
            }
            else
            {
                wc.hCursor = normal_cursor;
            }
            
            wc.style  = cfc->m_style;
            wc.style |= CS_DBLCLKS;
            
            wc.lpfnWndProc   = cfc->m_proc;
            wc.lpszClassName = cfc->m_class_name;
            
            wc.cbWndExtra = cfc->m_wnd_extra;
            
            cfc->m_atom = RegisterClass(&wc);
        }
    }

// =============================================================================

    /**
        Subclass existing window classes. This is typically done to alter
        the behavior of the standard window classes provided by Microsoft.

        In our case, currently we only subclass the edit control.
    */
    static void
    HM_RegisterSubclasses(HINSTANCE const p_inst)
    {
        WNDCLASS wc = { 0 };
        
        ATOM atom = 0;
        
        // -----------------------------
        
        // Subclass the standard edit control with a different window procedure
        // that only accepts numerical characters.
        if( GetClassInfo(p_inst, WC_EDIT, &wc) )
        {
            DefaultEditProc = wc.lpfnWndProc;
            
            wc.lpszClassName = "NumEdit";
            wc.lpfnWndProc = NumEditProc;
            
            atom = RegisterClass(&wc);
        }
    }

// =============================================================================

    extern BOOL
    HM_RegisterClasses(HINSTANCE const p_inst)
    {
        HM_RegisterFrameClasses(p_inst);
        HM_RegisterControlClasses(p_inst);
        
        HM_RegisterSubclasses(p_inst);
        
        return TRUE;
    }

// =============================================================================

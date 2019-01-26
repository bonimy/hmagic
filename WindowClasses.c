
    #include "structs.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "GdiObjects.h"

    #include "resource.h"

// =============================================================================

    static BOOL
    NumEdit_ValidCharacter
    (
        unsigned short const p_c
    )
    {
        enum { backspace_char = '\x08' };
        
        // -----------------------------
        
        // Only pass numeric and backspace characters on to the
        // standard edit window proc.
        if( ('0' <= p_c) && (p_c <= '9') )
        {
            return TRUE;
        }
        else if( Is(p_c, backspace_char) )
        {
            return TRUE;
        }

        return FALSE;
    }

// =============================================================================

    static void
    NumEdit_OnPaste
    (
        HWND const p_win
    )
    {
        BOOL const open_clipboard = OpenClipboard(p_win);
        
        HANDLE h_clipboard = NULL;
        
        char * text = NULL;
        
        size_t i   = 0;
        size_t len = 0;
        
        // -----------------------------
        
        if( IsFalse(open_clipboard) )
        {
            goto error;
        }
        
        h_clipboard = GetClipboardData(CF_TEXT);
        
        if( IsNull(h_clipboard) )
        {
            goto error;
        }
        
        text = (char*) GlobalLock(h_clipboard);
        
        if( IsNull(text) )
        {
            goto error;
        }
        
        len = strlen(text);
        
        for(i = 0; i < len; i += 1)
        {
            if( NumEdit_ValidCharacter( text[i] ) )
            {
                PostMessage(p_win, WM_CHAR, text[i], 0);
            }
        }
        
        GlobalUnlock(h_clipboard);
        
        CloseClipboard();
        
        return;
        
    error:

        if(open_clipboard)
        {
            CloseClipboard();
        }
    }

// =============================================================================

    HM_DeclareWndProc(NumEditProc)
    {
        WNDPROC base_class_proc = (WNDPROC) GetClassLongPtr(p_win, 0);
        
        // -----------------------------
        
        if( IsNull(base_class_proc) )
        {
            return DefWindowProc(p_win, p_msg, p_wp, p_lp);
        }

        switch(p_msg)
        {
        
        default:
            
            break;
        
        case WM_PASTE:
            
            NumEdit_OnPaste(p_win);
            
            return 0;

        case WM_CHAR:
            
            if( IsFalse(NumEdit_ValidCharacter( (unsigned short) p_wp) ) )
            {
                return 0;
            }
        }
        
        return CallWindowProc(base_class_proc, p_win, p_msg, p_wp, p_lp);
    }

// =============================================================================

    HM_DeclareWndProc(HM_CustomTreeViewProc)
    {
        WNDPROC base_class_proc = (WNDPROC) GetClassLongPtr(p_win, 0);
        
        // -----------------------------
        
        if( IsNull(base_class_proc) )
        {
            return DefWindowProc(p_win, p_msg, p_wp, p_lp);
        }

        switch(p_msg)
        {
        
        default:
            
            break;
        
        case WM_GETDLGCODE:

            return DLGC_WANTALLKEYS;
        
        case WM_CHAR:

            switch(p_wp)
            {
            
            default:
                break;
            
            case VK_SPACE:
            case VK_RETURN:
                
                return FALSE;
            }
        }
        
        return CallWindowProc(base_class_proc, p_win, p_msg, p_wp, p_lp);
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
            HWND dummy_num_edit;
            
            WNDPROC EditProc = wc.lpfnWndProc;
            
            // -----------------------------
            
            wc.cbClsExtra    = sizeof(LONG_PTR);
            wc.lpszClassName = "NumEdit";
            wc.lpfnWndProc   = NumEditProc;
            
            atom = RegisterClass(&wc);
            
            dummy_num_edit = CreateWindow
            (
                "NumEdit",
                "Dummy",
                0,
                0, 0,
                0, 0,
                NULL,
                NULL,
                hinstance,
                NULL
            );
            
            SetClassLongPtr
            (
                dummy_num_edit,
                0,
                (LONG_PTR) EditProc
            );
            
            DestroyWindow(dummy_num_edit);
        }

        // Subclass the standard edit control with a different window procedure
        // that only accepts numerical characters.
        if( GetClassInfo(p_inst, WC_TREEVIEW, &wc) )
        {
            HWND dummy_win;
            
            WNDPROC old_proc = wc.lpfnWndProc;
            
            // -----------------------------
            
            wc.cbClsExtra    = sizeof(LONG_PTR);
            wc.lpszClassName = "HMagic-TreeView";
            wc.lpfnWndProc   = HM_CustomTreeViewProc;
            
            atom = RegisterClass(&wc);
            
            dummy_win = CreateWindow
            (
                "HMagic-TreeView",
                "Dummy",
                0,
                0, 0,
                0, 0,
                NULL,
                NULL,
                hinstance,
                NULL
            );
            
            SetClassLongPtr
            (
                dummy_win,
                0,
                (LONG_PTR) old_proc
            );
            
            DestroyWindow(dummy_win);
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

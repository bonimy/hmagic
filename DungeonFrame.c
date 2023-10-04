
    #include "structs.h"

    #include "Wrappers.h"

    #include "DungeonEnum.h"

    #include "prototypes.h"

// =============================================================================

    static BOOL
    DungeonFrame_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        CP2(DUNGEDIT) ed = (DUNGEDIT*) (mdi_cs->lParam);
        
        HWND super_dlg = NULL;
        HWND map_win   = NULL;
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, (LONG_PTR) ed);
        ShowWindow(p_win, SW_SHOW);
        
        ed->ew.doc->ents[ed->ew.param] = p_win;
        
        super_dlg = CreateSuperDialog
        (
            &dungdlg,
            p_win,
            0, 0,
            0, 0,
            (LPARAM) ed
        );
        
        if( IsNull(super_dlg) )
        {
            // This has code smell, but this isn't really supposed to happen
            // anyway.
            MessageBox(p_win, "adsfasdf", "adfasdfads!!!!!!", MB_YESNOCANCEL);
            
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            return FALSE;
        }
        
        map_win = GetDlgItem(ed->dlg, ID_DungEditWindow);
        
        SetWindowLongPtr(map_win, GWLP_USERDATA, (LPARAM) ed);
        
        Updatesize(map_win);
        InvalidateRect(map_win, 0, 0);
        
        Dungselectchg(ed, map_win, 1);
        
        return TRUE;
    }

// =============================================================================

    static void
    DungeonFrame_OnDestroy
    (
        CP2(DUNGEDIT) p_ed 
    )
    {
        Delgraphwin(p_ed);
        
        DestroyWindow(p_ed->dlg);
        
        free(p_ed->buf);
        free(p_ed->sbuf);
        free(p_ed->ebuf);
        
        free(p_ed);
    }

// =============================================================================

// This is a child frame window, but the "view" is the 'ed' window.
LRESULT CALLBACK
dungproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    CP2(DUNGEDIT) ed = (DUNGEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
    
    // -----------------------------
    
    if( Is(msg, WM_CREATE) )
    {
        return DungeonFrame_OnCreate(win, lparam);
    }
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        
        if( (HWND) lparam != win)
        {
            break;
        }
        
        activedoc = ed->ew.doc;
        
        Setdispwin(ed);
        
        goto default_case;
    
    case WM_GETMINMAXINFO:
        
        DefMDIChildProc(win,msg,wparam,lparam);
        
        if( ! ed)
        {
            goto default_case;
        }
        
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        
        SetWindowPos(ed->dlg,
                     0,
                     0,
                     0,
                     LOWORD(lparam),
                     HIWORD(lparam),
                     SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        
        goto default_case;
    
    case WM_CLOSE:
        
        if( IsZero(Closeroom(ed) ) )
        {
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            DestroyWindow(ed->dlg);
            
            goto default_case;
        }
        
        break;
    
    case WM_DESTROY:
        
        DungeonFrame_OnDestroy(ed);
        
        break;
    
    default_case:
    default:
        
        return DefMDIChildProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

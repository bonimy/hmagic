
    #include "structs.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "prototypes.h"

// =============================================================================

    static void
    TrackEdit_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        TRACKEDIT * const ed = (TRACKEDIT*) (mdi_cs->lParam);
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, (LONG_PTR) ed);
        
        ShowWindow(p_win, SW_SHOW);
        
        ed->ew.doc->sr[ed->ew.param & 0xffff].editor = p_win;
        
        CreateSuperDialog(&trackdlg,
                          p_win,
                          0, 0, 0, 0,
                          (LPARAM) ed);
    }

// =============================================================================

    LRESULT CALLBACK
    trackeditproc
    (
        HWND win,
        UINT msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        TRACKEDIT * const
        ed = (TRACKEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        // -----------------------------
        
        if(msg == WM_CREATE)
        {
            TrackEdit_OnCreate(win, lparam);
            
            return DefMDIChildProc(win, msg, wparam, lparam);
        }
        
        switch(msg)
        {
        
        case WM_MDIACTIVATE:
            
            activedoc = ed->ew.doc;
            
            goto deflt;
        
        case WM_GETMINMAXINFO:
            
            DefMDIChildProc(win,msg,wparam,lparam);
            
            if( ! ed )
            {
                goto deflt;
            }
            
            return SendMessage(ed->dlg,
                               WM_GETMINMAXINFO,
                               wparam,
                               lparam);
        
        case WM_SIZE:
            
            SetWindowPos
            (
                ed->dlg,
                0,
                0,
                0,
                LOWORD(lparam),
                HIWORD(lparam),
                (SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE)
            );
            
            goto deflt;
        
        case WM_DESTROY:
            
            ed->ew.doc->sr[ed->ew.param].editor = 0;
            
            free(ed);
            
            break;
        
        deflt:
        default:
            
            return DefMDIChildProc(win,msg,wparam,lparam);
        }
        
        return 0;
    }

// =============================================================================
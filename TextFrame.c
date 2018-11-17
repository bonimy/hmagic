
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

// =============================================================================

    static void
    TextEditFrame_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        TEXTEDIT * const ed = (TEXTEDIT*) (mdi_cs->lParam);
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, mdi_cs->lParam);
        
        ShowWindow(p_win, SW_SHOW);
        
        CreateSuperDialog(&textdlg,
                          p_win,
                          0, 0, 0, 0,
                          (LPARAM) ed);
    }

// =============================================================================

LRESULT CALLBACK
texteditproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    TEXTEDIT * const ed = (TEXTEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
    
    // -----------------------------
    
    if(msg == WM_CREATE)
    {
        TextEditFrame_OnCreate(win, lparam);
        
        return DefMDIChildProc(win, msg, wparam, lparam);
    }
    
    switch(msg)
    {

    case WM_MDIACTIVATE:
        
        activedoc = ( (TEXTEDIT*) GetWindowLongPtr(win, GWLP_USERDATA))->ew.doc;
        
        goto default_case;
    
    case WM_GETMINMAXINFO:
        
        DefMDIChildProc(win, msg, wparam, lparam);
        
        if( ! ed )
        {
            goto default_case;
        }
        
        return SendMessage(ed->dlg,
                           WM_GETMINMAXINFO,
                           wparam,
                           lparam);
    
    case WM_SIZE:
        
        SetWindowPos(ed->dlg,
                     0,
                     0, 0,
                     LOWORD(lparam), HIWORD(lparam),
                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
        
        goto default_case;
    
    case WM_DESTROY:
        
        ed->ew.doc->t_wnd = 0;
        
        free(ed);
        
        break;
    
    default_case:
    default:
        
        return DefMDIChildProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

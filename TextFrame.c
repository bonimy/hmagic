
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    // So we can address the child window, which is a super dialog.
    #include "HMagicEnum.h"

    // \task[high] Temporarily including to test something. Not actually
    // needed currently.
    #include "TextEnum.h"

// =============================================================================

    static HWND
    TextEditFrame_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        CP2(TEXTEDIT) ed = (TEXTEDIT*) (mdi_cs->lParam);
        
        HWND dlg = NULL;
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, mdi_cs->lParam);
        
        ShowWindow(p_win, SW_SHOW);
        
        dlg = CreateSuperDialog
        (
            &textdlg,
            p_win,
            0, 0, 0, 0,
            (LPARAM) ed
        );
        
        return dlg;
    }

// =============================================================================

    static void
    TextFrame_On_MDI_Activate
    (
        HWND           const p_win,
        WPARAM         const p_wp,
        LPARAM         const p_lp,
        CP2C(TEXTEDIT)       p_ed
    )
    {
        HM_MdiActivateData const ad = HM_MDI_GetActivateData
        (
            p_wp,
            p_lp
        );
        
        // -----------------------------
        
        if( Is(p_win, ad.m_activating) )
        {
            activedoc = p_ed->ew.doc;
        }
    }

// =============================================================================

    LRESULT CALLBACK
    TextFrameProc
    (
        HWND   p_win,
        UINT   p_msg,
        WPARAM p_wp,
        LPARAM p_lp
    )
    {
        HWND const dlg = GetDlgItem(p_win, ID_SuperDlg);
        
        CP2(TEXTEDIT) ed = (TEXTEDIT*) GetWindowLongPtr(p_win, GWLP_USERDATA);
        
        // -----------------------------
        
        if( Is(p_msg, WM_CREATE) )
        {
            TextEditFrame_OnCreate(p_win, p_lp);
            
            return DefMDIChildProc(p_win, p_msg, p_wp, p_lp);
        }
        
        switch(p_msg)
        {
        
        case WM_MDIACTIVATE:
            
            TextFrame_On_MDI_Activate(p_win, p_wp, p_lp, ed);
            
            goto default_case;
        
        case WM_SETFOCUS:

            SetFocus(dlg);
            
            break;

        case WM_KILLFOCUS:
            
            { SetDlgItemText(debug_window, IDC_STATIC3, "1"); }
            
            break;

        case WM_GETMINMAXINFO:
            
            DefMDIChildProc(p_win, p_msg, p_wp, p_lp);
            
            if( ! ed )
            {
                goto default_case;
            }
            
            return SendMessage
            (
                ed->dlg,
                WM_GETMINMAXINFO,
                p_wp,
                p_lp
            );
        
        case WM_SIZE:
            
            SetWindowPos
            (
                ed->dlg,
                0,
                0, 0,
                LOWORD(p_lp), HIWORD(p_lp),
                SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
            );
            
            goto default_case;
        
        case WM_DESTROY:
            
            ed->ew.doc->t_wnd = 0;
            
            free(ed);
            
            break;
        
        default_case:
        default:
            
            return DefMDIChildProc(p_win, p_msg, p_wp, p_lp);
        }
        
        return 0;
    }

// =============================================================================

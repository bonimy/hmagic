
    #include "structs.h"
    #include "prototypes.h"

    #include "Wrappers.h"

    #include "AudioLogic.h"

    #include "HMagicEnum.h"

// =============================================================================

    static HWND
    AudioFrame_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        CP2C(MUSEDIT) ed = (MUSEDIT*) mdi_cs->lParam;
        
        HWND dlg = NULL;
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, (LONG_PTR) ed);
        
        ShowWindow(p_win, SW_SHOW);
        
        dlg = CreateSuperDialog
        (
            Is(ed->ew.param, 3) ? &sampdlg : &musdlg,
            p_win,
            0, 0,
            0, 0,
            (LPARAM) ed
        );
        
        return dlg;
    }

// =============================================================================

    static void
    AudioFrame_On_MDI_Activate
    (
        HWND           const p_win,
        WPARAM         const p_wp,
        LPARAM         const p_lp,
        CP2C(MUSEDIT)        p_ed
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
            HWND const focus_control = (HWND) GetProp
            (
                p_win,
                "ControlWithFocus"
            );
            
            // -----------------------------
            
            activedoc = p_ed->ew.doc;
            
            if(focus_control)
            {
                SetFocus(focus_control);
            }
        }
        else
        {
            HWND const focused_control = GetFocus();
            
            // -----------------------------
            
            SetProp(p_win, "ControlWithFocus", focused_control);
        }
    }

// =============================================================================

    extern LRESULT CALLBACK
    AudioFrameProc
    (
        HWND   p_win,
        UINT   p_msg,
        WPARAM p_wp,
        LPARAM p_lp
    )
    {
        HWND const dlg = GetDlgItem(p_win, ID_SuperDlg);
        
        CP2(MUSEDIT) ed = (MUSEDIT*) GetWindowLongPtr(p_win, GWLP_USERDATA);
        
        // -----------------------------
        
        switch(p_msg)
        {
        
        case WM_MDIACTIVATE:
            
            AudioFrame_On_MDI_Activate
            (
                p_win,
                p_wp,
                p_lp,
                ed
            );
            
            goto deflt;
        
        case WM_SETFOCUS:
            
            if(dlg)
            {
                SetFocus(dlg);
            }
            
            break;
        
        case WM_GETMINMAXINFO:
            
            DefMDIChildProc(p_win, p_msg, p_wp, p_lp);
            
            if(!ed)
                goto deflt;
            
            return SendMessage(ed->dlg,WM_GETMINMAXINFO, p_wp, p_lp);
        
        case WM_SIZE:
            
            SetWindowPos
            (
                ed->dlg,
                0,
                0, 0,
                LOWORD(p_lp), HIWORD(p_lp),
                SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
            );
            
            goto deflt;
        
        case WM_DESTROY:
            
            ed->ew.doc->mbanks[ed->ew.param] = 0;
            
            if
            (
                sndinit
             && Is(ed->ew.doc, sounddoc)
             && IsFalse(playedsong)
            )
            {
                zwaves->pflag = 0;
            }
            
            free(ed);
            
            break;
        
        case WM_CREATE:
            
            AudioFrame_OnCreate(p_win, p_lp);
            
        deflt:
        default:
            
            return DefMDIChildProc(p_win, p_msg, p_wp, p_lp);
        }
        
        return 0;
    }

// =============================================================================

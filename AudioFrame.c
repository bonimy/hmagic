
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

    extern LRESULT CALLBACK
    AudioFrameProc
    (
        HWND   p_win,
        UINT   msg,
        WPARAM wparam,
        LPARAM p_lp
    )
    {
        HWND dlg = GetDlgItem(p_win, ID_SuperDlg);
        
        MUSEDIT * ed;
        
        // -----------------------------
        
        switch(msg)
        {
        
        case WM_MDIACTIVATE:
            
            activedoc = ((DUNGEDIT*)GetWindowLongPtr(p_win, GWLP_USERDATA))->ew.doc;
            
            goto deflt;
        
        case WM_SETFOCUS:
            
            if(dlg)
            {
                SetFocus(dlg);
            }
            
            break;
        
        case WM_GETMINMAXINFO:
            
            ed = (MUSEDIT*) GetWindowLongPtr(p_win,GWLP_USERDATA);
            
            DefMDIChildProc(p_win,msg,wparam, p_lp);
            
            if(!ed)
                goto deflt;
            
            return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam, p_lp);
        
        case WM_SIZE:
            
            ed = (MUSEDIT*) GetWindowLongPtr(p_win,GWLP_USERDATA);
            
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
            
            ed = (MUSEDIT*)GetWindowLongPtr(p_win,GWLP_USERDATA);
            ed->ew.doc->mbanks[ed->ew.param]=0;
            
            if(sndinit && ed->ew.doc == sounddoc && !playedsong)
                zwaves->pflag=0;
            
            free(ed);
            
            break;
        
        case WM_CREATE:
            
            AudioFrame_OnCreate(p_win, p_lp);
            
        deflt:
        default:
            
            return DefMDIChildProc(p_win, msg, wparam, p_lp);
        }
        return 0;
    }

// =============================================================================

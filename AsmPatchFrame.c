
#include "structs.h"
#include "prototypes.h"

// =============================================================================

    LRESULT CALLBACK
    PatchFrameProc
    (
        HWND win,
        UINT msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        PATCHLOAD * ed;
        
        // -----------------------------
        
        switch(msg)
        {
        
        case WM_MDIACTIVATE:
            activedoc = ( (PATCHLOAD*) GetWindowLongPtr(win, GWLP_USERDATA))->ew.doc;
            goto deflt;
        
        case WM_GETMINMAXINFO:
            
            ed = (PATCHLOAD*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            DefMDIChildProc(win, msg, wparam, lparam);
            
            if( ! ed )
                goto deflt;
            
            return SendMessage(ed->dlg,
                               WM_GETMINMAXINFO,
                               wparam,
                               lparam);
        
        case WM_SIZE:
            
            ed = (PATCHLOAD*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            SetWindowPos(ed->dlg,
                         0,
                         0,
                         0,
                         LOWORD(lparam),
                         HIWORD(lparam),
                         SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
            
            goto deflt;
        
        case WM_DESTROY:
            
            ed = (PATCHLOAD*) GetWindowLongPtr(win, GWLP_USERDATA);
            ed->ew.doc->hackwnd = 0;
            
            free(ed);
            
            break;
        
        case WM_CREATE:
            
            ed = (PATCHLOAD*) (((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
            
            SetWindowLongPtr(win,GWLP_USERDATA, (LONG_PTR) ed);
            
            ShowWindow(win,SW_SHOW);
            
            CreateSuperDialog(&patchdlg,win,0,0,0,0, (LPARAM) ed);
        
        default:
        deflt:
            
            return DefMDIChildProc(win, msg, wparam, lparam);
        }
        
        return 0;
    }

// =============================================================================

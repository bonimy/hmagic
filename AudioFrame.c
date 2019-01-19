
#include "structs.h"
#include "prototypes.h"

#include "AudioLogic.h"

// =============================================================================

    LRESULT CALLBACK
    AudioFrameProc
    (
        HWND win,
        UINT msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        MUSEDIT *ed;
        
        switch(msg)
        {
        
        case WM_MDIACTIVATE:
            
            activedoc = ((DUNGEDIT*)GetWindowLongPtr(win, GWLP_USERDATA))->ew.doc;
            
            goto deflt;
        
        case WM_GETMINMAXINFO:
            
            ed=(MUSEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
            DefMDIChildProc(win,msg,wparam,lparam);
            
            if(!ed)
                goto deflt;
            
            return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
        
        case WM_SIZE:
            
            ed=(MUSEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
            SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
            
            goto deflt;
        
        case WM_DESTROY:
            
            ed = (MUSEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
            ed->ew.doc->mbanks[ed->ew.param]=0;
            
            if(sndinit && ed->ew.doc == sounddoc && !playedsong)
                zwaves->pflag=0;
            
            free(ed);
            
            break;
        
        case WM_CREATE:
            
            ed = (MUSEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
            
            SetWindowLongPtr(win,GWLP_USERDATA, (LONG_PTR) ed);
            ShowWindow(win,SW_SHOW);
            CreateSuperDialog((ed->ew.param==3)?&sampdlg:&musdlg,win,0,0,0,0, (LPARAM) ed);
            
        deflt:
        default:
            
            return DefMDIChildProc(win,msg,wparam,lparam);
        }
        return 0;
    }

// =============================================================================

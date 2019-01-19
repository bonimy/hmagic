
#include "structs.h"
#include "prototypes.h"

#include "WorldMapLogic.h"

// =============================================================================

LRESULT CALLBACK
wmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT *ed;
    
    switch(msg)
    {
    case WM_CLOSE:
        
        ed=(WMAPEDIT*)GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(ed->selflag)
            Wmapselectwrite(ed);
        
        if(ed->modf)
        {
            text_buf_ty text_buf = { 0 };
            
            wsprintf(text_buf,
                     "Confirm modification of world map %d?",
                     ed->ew.param);
            
            switch
            (
                MessageBox(framewnd,
                           text_buf,
                           "World map editor",
                           MB_YESNOCANCEL)
            )
            {
            
            case IDYES:
                
                Saveworldmap(ed);
                
                break;
            
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        
        if((HWND)lparam != win)
            break;
        
        ed = ((WMAPEDIT*)GetWindowLongPtr(win, GWLP_USERDATA));
        activedoc=ed->ew.doc;
        Setdispwin((DUNGEDIT*)ed);
        
        break;
    
    case WM_GETMINMAXINFO:
        
        ed = (WMAPEDIT*)GetWindowLongPtr(win, GWLP_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        ed = (WMAPEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        
        ed=(WMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        
        SetWindowLongPtr(win, GWLP_USERDATA, (LONG_PTR) ed);
        
        ShowWindow(win,SW_SHOW);
        
        ed->dlg = CreateSuperDialog(&wmapdlg, win,0,0,0,0,(LPARAM) ed);
        
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

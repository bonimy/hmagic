
#include "structs.h"

#include "Callbacks.h"

#include "prototypes.h"

// =============================================================================

LRESULT CALLBACK
trackeditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT * ed;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        
        activedoc=((TRACKEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        
        goto deflt;
    
    case WM_GETMINMAXINFO:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    
    case WM_DESTROY:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->sr[ed->ew.param].editor=0;
        free(ed);
        break;
    
    case WM_CREATE:
        ed=(TRACKEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->ew.doc->sr[ed->ew.param&65535].editor=win;
        
        CreateSuperDialog(&trackdlg,win,0,0,0,0,(long)ed);
        
    deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================
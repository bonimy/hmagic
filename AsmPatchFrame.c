
#include "structs.h"
#include "prototypes.h"

// =============================================================================

LRESULT CALLBACK
patchproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PATCHLOAD*ed;
    switch(msg) {
    case WM_MDIACTIVATE:
        activedoc=((PATCHLOAD*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->hackwnd=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(PATCHLOAD*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        CreateSuperDialog(&patchdlg,win,0,0,0,0,(long)ed);
    default:
deflt:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

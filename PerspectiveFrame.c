
    #include "structs.h"

    #include "prototypes.h"

    #include "PerspectiveLogic.h"

// =============================================================================

LRESULT CALLBACK
PerspectiveFrameProc
(
    HWND win,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam
)
{
    PERSPEDIT*ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(PERSPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(ed->modf) {
            switch(MessageBox(framewnd,"Confirm modification of 3D objects?","3D object editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savepersp(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        activedoc=((PERSPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(PERSPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(PERSPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(PERSPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        ed->ew.doc->perspwnd=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(PERSPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLongPtr(win,GWLP_USERDATA, (LONG_PTR) ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&perspdlg,win,0,0,100,100, (LPARAM) ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

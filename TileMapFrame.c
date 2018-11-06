
    #include "structs.h"

    #include "prototypes.h"

    #include "TileMapLogic.h"

    #include "ScreenEditorLogic.h"

// =============================================================================

extern char buffer[0x400];

LRESULT CALLBACK
tmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT *ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->modf) {
            wsprintf(buffer,"Confirm modification of %s?",screen_text[ed->ew.param]);
            switch(MessageBox(framewnd,buffer,"Other screen editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savetmap(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        if((HWND)lparam!=win) break;
        ed=((TMAPEDIT*)GetWindowLong(win,GWL_USERDATA));
        activedoc=ed->ew.doc;
        Setdispwin((DUNGEDIT*)ed);
        break;
    case WM_GETMINMAXINFO:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        ed=(TMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&tmapdlg,win,0,0,256,256,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

#include "structs.h"

#include "DungeonEnum.h"

#include "prototypes.h"

// =============================================================================

// This is a child frame window, but the "view" is the 'ed' window.
LRESULT CALLBACK
dungproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    DUNGEDIT *ed;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        
        if((HWND)lparam != win)
            break;
        
        ed = ((DUNGEDIT*) GetWindowLong(win,GWL_USERDATA));
        
        activedoc = ed->ew.doc;
        
        Setdispwin(ed);
        
        goto deflt;
    
    case WM_GETMINMAXINFO:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        DefMDIChildProc(win,msg,wparam,lparam);
        
        if(!ed)
            goto deflt;
        
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        SetWindowPos(ed->dlg,
                     0,
                     0,
                     0,
                     LOWORD(lparam),
                     HIWORD(lparam),
                     SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        
        goto deflt;
    
    case WM_CLOSE:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        if(!Closeroom(ed))
        {
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            DestroyWindow(ed->dlg);
            Releaseblks(ed->ew.doc,0x79);
            Releaseblks(ed->ew.doc,0x7a);
            
            goto deflt;
        }
        
        break;
    
    case WM_DESTROY:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        Delgraphwin(ed);
        
        free(ed->buf);
        free(ed->sbuf);
        free(ed->ebuf);
        
        free(ed);
        
        break;
    
    case WM_CREATE:
        
        ed = (DUNGEDIT*) (((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        
        SetWindowLongPtr(win, GWLP_USERDATA, (LONG_PTR) ed);
        ShowWindow(win, SW_SHOW);
        
        ed->ew.doc->ents[ed->ew.param] = win;
        
        if( CreateSuperDialog(&dungdlg,win,0,0,0,0, (LPARAM) ed) == 0)
        {
            MessageBox(win, "adsfasdf", "adfasdfads!!!!!!", MB_YESNOCANCEL);
            
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            return FALSE;
        }
        
        hc = GetDlgItem(ed->dlg, ID_DungEditWindow);
        
        SetWindowLong(hc, GWL_USERDATA, (long) ed);
        
        Updatesize(hc);
        InvalidateRect(hc, 0, 0);
        Dungselectchg(ed, hc, 1);
        
deflt:
        
    default:
        
        return DefMDIChildProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

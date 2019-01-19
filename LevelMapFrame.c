
    #include "structs.h"

    #include "prototypes.h"

    #include "LevelMapLogic.h"

// =============================================================================

    LRESULT CALLBACK
    PalaceMapFrame
    (
        HWND win,
        UINT msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        LMAPEDIT * ed;
        
        switch(msg)
        {
        
        case WM_CLOSE:
            
            ed = (LMAPEDIT*) GetWindowLongPtr(win,GWLP_USERDATA);
            
            if(ed->modf)
            {
                text_buf_ty buf;
                
                wsprintf(buf,
                         "Confirm modification of %s palace?",
                         level_str[ed->ew.param+1]);
                
                switch
                (
                    MessageBox(framewnd, buf, "Level map editor", MB_YESNOCANCEL)
                )
                {
                
                case IDYES:
                    
                    Savedungmap(ed);
                    
                    break;
                
                case IDCANCEL:
                    
                    return 1;
                }
            }
            
            goto deflt;
            break;
        
        case WM_MDIACTIVATE:
            
            if((HWND)lparam!=win) break;
            
            ed=((LMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA));
            
            activedoc=ed->ew.doc;
            Setdispwin((DUNGEDIT*)ed);
            
            break;
        
        case WM_GETMINMAXINFO:
            
            ed=(LMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
            
            DefMDIChildProc(win,msg,wparam,lparam);
            
            if(!ed) goto deflt;
            
            return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
        
        case WM_SIZE:
            
            ed=(LMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
            
            SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
            
            goto deflt;
        
        case WM_CREATE:
            
            ed=(LMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
            
            SetWindowLongPtr(win,GWLP_USERDATA, (LONG_PTR) ed);
            ShowWindow(win,SW_SHOW);
            
            ed->dlg=CreateSuperDialog(&lmapdlg,win,0,0,256,256, (LPARAM) ed);
            
        deflt:
        default:
            
            return DefMDIChildProc(win,msg,wparam,lparam);
        }
        
        return 0;
    }

// =============================================================================

#include "structs.h"
#include "prototypes.h"

#include "OverworldEdit.h"


// =============================================================================

LRESULT CALLBACK
overproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT * const ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
    
    switch(msg)
    {
    
    deflt:
    default:
        return DefMDIChildProc(win, msg, wparam, lparam);
    
    case WM_CLOSE:
        
        if(ed->selflag) Overselectwrite(ed);
        
        if(ed->ew.doc->modf==2) goto deflt;
        
        if(ed->ov->modf)
        {
            text_buf_ty text_buf = { 0 };
            
            wsprintf(text_buf,
                     "Confirm modification of area %02X?",
                     ed->ew.param);
            
            switch
            (
                MessageBox(framewnd,
                           text_buf,
                           "Overworld editor",
                           MB_YESNOCANCEL)
            )
            {
            
            case IDYES:
                
                Savemap(ed);
                
                goto deflt;
            
            case IDCANCEL:
                
                break;
            
            case IDNO:
                goto deflt;
            }
        }
        else
            goto deflt;
        
        break;
    
    case WM_MDIACTIVATE:
        
        if(ed)
        {
            HM_MdiActivateData d = HM_GetMdiActivateData(wparam, lparam);
            
            if(d.m_activating != win)
            {
                break;
            }
            
            activedoc = ed->ew.doc;
            
            Setdispwin((DUNGEDIT*) ed);
        }
        
        break;
    
    case WM_GETMINMAXINFO:
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        
        if(always)
        {
            CREATESTRUCT const * const cs = (CREATESTRUCT*) lparam;
            
            MDICREATESTRUCT const * const
            mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
            
            OVEREDIT * const new_ed = (OVEREDIT*) (mdi_cs->lParam);
            
            SetWindowLong(win, GWL_USERDATA, (long) new_ed);
            
            ShowWindow(win, SW_SHOW);
            
            new_ed->dlg = CreateSuperDialog(&overdlg,
                                            win,
                                            0,
                                            0,
                                            0,
                                            0,
                                            (long) new_ed);
        }
        
        break;
    }
    
    return 0;
}

// =============================================================================

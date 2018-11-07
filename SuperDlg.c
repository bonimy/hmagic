
    #include "structs.h"

    #include "HMagicEnum.h"

// =============================================================================

HWND
CreateSuperDialog(SUPERDLG *dlgtemp,
                  HWND owner,
                  int x,
                  int y,
                  int w,
                  int h,
                  LPARAM lparam)
{
    SDCREATE * const sdc =
    (SDCREATE*) calloc(1, sizeof(SDCREATE));
    
    HWND hc;
    
    // -----------------------------
    
    sdc->dlgtemp = dlgtemp;
    sdc->owner = owner;
    sdc->lparam = lparam;
    sdc->w = w;
    sdc->h = h;
    
    hc = CreateWindowEx(0,
                        "SUPERDLG",
                        dlgtemp->title,
                        dlgtemp->style,
                        x,
                        y,
                        w,
                        h,
                        owner,
                        (HMENU) ID_SUPERDLG,
                        hinstance,
                        (void*) sdc);
    
    return hc;
}

// =============================================================================

LRESULT CALLBACK
superdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k,l,m,w,h;
    
    SDCREATE * sdc = (SDCREATE*) GetWindowLongPtr(win, GWLP_USERDATA);
    
    SD_ENTRY * sde;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_NCCREATE:
        goto deflt;
    
    case WM_GETMINMAXINFO:
        
        if(sdc)
        {
            ((LPMINMAXINFO)lparam)->ptMinTrackSize.x = sdc->dlgtemp->minw;
            ((LPMINMAXINFO)lparam)->ptMinTrackSize.y = sdc->dlgtemp->minh;
        }
        
        return 0;
    
    case WM_CREATE:
        
        sdc = (SDCREATE*) ( ( (CREATESTRUCT*) lparam)->lpCreateParams); 
        
        SetWindowLongPtr(win,
                         GWLP_USERDATA,
                         (LONG_PTR) sdc);
        
        sde = sdc->dlgtemp->sde;
        
        for(i = 0; i < sdc->dlgtemp->numsde; i++)
        {
            if(sde[i].flags & 1)
                j = sdc->w-sde[i].x;
            else
                j = sde[i].x;
            
            if(sde[i].flags & 2)
                l = sdc->w - sde[i].w - j;
            else
                l = sde[i].w;
            
            if(sde[i].flags & 4)
                k = sdc->h-sde[i].y;
            else
                k = sde[i].y;
            
            if(sde[i].flags & 8)
                m = sdc->h-sde[i].h-k;
            else
                m = sde[i].h;
            
            hc = CreateWindowEx
            (
                sde[i].exstyle,
                sde[i].cname,
                sde[i].caption,
                sde[i].style,
                j,
                k,
                l,
                m,
                win,
                (HMENU)(sde[i].id),
                hinstance,
                0
            );
            
            SendMessage(hc,
                        WM_SETFONT,
                        (WPARAM) GetStockObject(ANSI_VAR_FONT),
                        0);
        }
        
        sdc->win = win;
        sdc->prev = lastdlg;
        sdc->next = 0;
        
        if(lastdlg)
            lastdlg->next = sdc;
        else
            firstdlg = sdc;
        
        lastdlg = sdc;
        
        sdc->dlgtemp->proc(win, WM_INITDIALOG, 0, sdc->lparam);
        
        return 0;
    
    case WM_SIZE:
        
        sde = sdc->dlgtemp->sde;
        
        w = LOWORD(lparam);
        h = HIWORD(lparam);
        
        for(i = 0; i < sdc->dlgtemp->numsde; i++)
        {
            if(sde[i].flags & 1)
                j = w - sde[i].x;
            else
                j = sde[i].x;
            
            if(sde[i].flags & 2)
                l = w - sde[i].w - j;
            else
                l = sde[i].w;
            
            if(sde[i].flags & 4)
                k = h - sde[i].y;
            else
                k = sde[i].y;
            
            if(sde[i].flags & 8)
                m = h - sde[i].h - k;
            else
                m = sde[i].h;
            
            SetWindowPos(GetDlgItem(win,sde[i].id),
                         0,
                         j,
                         k,
                         l,
                         m,
                         SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        }
        
        break;
    
    case WM_DESTROY:
        
        sdc->dlgtemp->proc(win, msg, wparam, lparam);
        
        if(sdc->next)
            sdc->next->prev = sdc->prev;
        else
            lastdlg = sdc->prev;
        
        if(sdc->prev)
            sdc->prev->next = sdc->next;
        else
            firstdlg = sdc->next;
        
        free(sdc);
        
        SetWindowLongPtr(win, GWLP_USERDATA,0);
        
        break;
        
    default:
        
        if(!sdc)
            goto deflt;
        
        SetWindowLongPtr(win, DWLP_MSGRESULT, 0);
        
        if( !sdc->dlgtemp->proc(win,msg,wparam,lparam) )
deflt:
            return DefWindowProc(win,msg,wparam,lparam);
        else
            return GetWindowLongPtr(win, DWLP_MSGRESULT);
    }
    
    return 0;
}

// =============================================================================

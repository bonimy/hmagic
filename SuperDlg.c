
    #include "structs.h"

    #include "HMagicEnum.h"

// =============================================================================

HWND
CreateSuperDialog(SUPERDLG * const dlgtemp,
                  HWND       const owner,
                  int        const x,
                  int        const y,
                  int        const w,
                  int        const h,
                  LPARAM     const lparam)
{
    SDCREATE * const sdc = (SDCREATE*) calloc(1, sizeof(SDCREATE));
    
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
                        (HMENU) ID_SuperDlg,
                        hinstance,
                        (void*) sdc);
    
    return hc;
}

// =============================================================================

void
SuperDlg_OnCreate
(
    HWND   const p_win,
    LPARAM const p_lp
)
{
    CREATESTRUCT const * const cs = (CREATESTRUCT*) p_lp;
    
    SDCREATE * const sdc = (SDCREATE*) cs->lpCreateParams;
    
    int i = 0;
    
    int const w = cs->cx;
    int const h = cs->cy;
    
    SD_ENTRY const * sde = sdc->dlgtemp->sde;
    
    // -----------------------------
    
    SetWindowLongPtr(p_win,
                     GWLP_USERDATA,
                     (LONG_PTR) sdc);
    
    for(i = 0; i < sdc->dlgtemp->numsde; i += 1)
    {
        // X and Y coordinates of the super dialog entry.
        int sde_x = 0;
        int sde_y = 0;
        
        // width and height of the super dialog entry.
        // Can be negative, which doesn't appear to be documented
        // as acceptable for CreateDialogEx(). \task Try to find
        // any reference on this on the internet. Update: This is
        // apparently not kosher, but doesn't hurt anything,
        // And the subsequent WM_SIZE issued later on corrects it
        // before the end user can observe the problem.
        int sde_w = 0;
        int sde_h = 0;
        
        HWND sde_win;
        
        // -----------------------------
        
        if(sde[i].flags & FLG_SDCH_FOX)
            sde_x = (w - sde[i].x);
        else
            sde_x = sde[i].x;
        
        if(sde[i].flags & FLG_SDCH_FOW)
            sde_w = (w - sde[i].w - sde_x);
        else
            sde_w = sde[i].w;
        
        if(sde[i].flags & FLG_SDCH_FOY)
            sde_y = (h - sde[i].y);
        else
            sde_y = sde[i].y;
        
        if(sde[i].flags & FLG_SDCH_FOH)
            sde_h = (h - sde[i].h - sde_y);
        else
            sde_h = sde[i].h;
        
        sde_win = CreateWindowEx
        (
            sde[i].exstyle,
            sde[i].cname,
            sde[i].caption,
            sde[i].style,
            sde_x,
            sde_y,
            sde_w,
            sde_h,
            p_win,
            (HMENU) (sde[i].id),
            hinstance,
            0
        );
        
        if(sde_win != NULL)
        {
            SendMessage(sde_win,
                        WM_SETFONT,
                        (WPARAM) GetStockObject(ANSI_VAR_FONT),
                        0);
        }
        else
        {
            MessageBox(p_win,
                       "Failed to create SD entry. Why?",
                       "Error",
                       MB_OK);
        }
    }
    
    sdc->win  = p_win;
    sdc->prev = lastdlg;
    sdc->next = 0;
    
    if(lastdlg)
        lastdlg->next = sdc;
    else
        firstdlg = sdc;
    
    lastdlg = sdc;
    
    sdc->dlgtemp->proc(p_win,
                       WM_INITDIALOG,
                       0, sdc->lparam);
}

// =============================================================================

void
SuperDlg_OnSize
(
    SDCREATE const * const p_sdc,
    HWND             const p_win,
    LPARAM           const p_lp
)
{
    int i = 0;
    
    int const w = LOWORD(p_lp);
    int const h = HIWORD(p_lp);
    
    SD_ENTRY const * const sde = p_sdc->dlgtemp->sde;
    
    // -----------------------------
    
    for(i = 0; i < p_sdc->dlgtemp->numsde; i++)
    {
        HWND const sd_child = GetDlgItem(p_win, sde[i].id);
        
        int sde_x = 0;
        int sde_y = 0;
        
        int sde_w = 0;
        int sde_h = 0;
        
        // -----------------------------
        
        if(sde[i].flags & FLG_SDCH_FOX)
            sde_x = (w - sde[i].x);
        else
            sde_x = sde[i].x;
        
        if(sde[i].flags & FLG_SDCH_FOW)
            sde_w = (w - sde[i].w - sde_x);
        else
            sde_w = sde[i].w;
        
        if(sde[i].flags & FLG_SDCH_FOY)
            sde_y = (h - sde[i].y);
        else
            sde_y = sde[i].y;
        
        if(sde[i].flags & FLG_SDCH_FOH)
            sde_h = (h - sde[i].h - sde_y);
        else
            sde_h = sde[i].h;
        
        SetWindowPos
        (
            sd_child,
            0,
            sde_x,
            sde_y,
            sde_w,
            sde_h,
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
        );
    }
}

// =============================================================================

LRESULT CALLBACK
superdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SDCREATE * sdc = (SDCREATE*) GetWindowLongPtr(win, GWLP_USERDATA);
    
    // -----------------------------
    
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
        
        SuperDlg_OnCreate(win, lparam);
        
        return 0;
    
    case WM_SIZE:
        
        SuperDlg_OnSize(sdc, win, lparam);
        
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
        
        SetWindowLongPtr(win, GWLP_USERDATA, 0);
        
        break;
        
    default:
        
        if( ! sdc )
        {
            goto deflt;
        }
        
        SetWindowLongPtr(win, DWLP_MSGRESULT, 0);
        
        if( ! sdc->dlgtemp->proc(win,msg,wparam,lparam) )
deflt:
            return DefWindowProc(win,msg,wparam,lparam);
        else
            return GetWindowLongPtr(win, DWLP_MSGRESULT);
    }
    
    return 0;
}

// =============================================================================

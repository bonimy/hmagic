
    #include "structs.h"

    #include "Wrappers.h"

    #include "HMagicEnum.h"

// =============================================================================

    /// Dimensions for a Super Dialog Entry (child window).
    typedef
    struct
    {
        /// X coordinate.
        int x;
        
        /// Y coordinate.
        int y;
        
        /// Width.
        int w;
        
        /// Height.
        int h;
        
    }
    SDE_Dimensions;

// =============================================================================

    extern HWND
    CreateSuperDialog
    (
        CP2(SUPERDLG)       dlgtemp,
        HWND          const owner,
        int           const x,
        int           const y,
        int           const w,
        int           const h,
        LPARAM        const lparam
    )
    {
        CP2(SDCREATE) sdc = (SDCREATE*) calloc(1, sizeof(SDCREATE));
        
        HWND hc;
        
        // -----------------------------
        
        sdc->dlgtemp = dlgtemp;
        sdc->owner = owner;
        sdc->lparam = lparam;
        sdc->w = w;
        sdc->h = h;
        
        hc = CreateWindowEx
        (
            0,
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
            (void*) sdc
        );
        
        return hc;
    }

// =============================================================================

    static void
    SuperDlg_SizeChild
    (
        P2C(SD_ENTRY)             p_sde,
        CP2(SDE_Dimensions)       p_dim,
        int                 const p_parent_width,
        int                 const p_parent_height
    )
    {
        if(p_sde->flags & FLG_SDCH_FOX)
        {
            p_dim->x = (p_parent_width - p_sde->x);
        }
        else
        {
            p_dim->x = p_sde->x;
        }
        
        if(p_sde->flags & FLG_SDCH_FOW)
        {
            p_dim->w = (p_parent_width - p_sde->w - p_dim->x);
        }
        else
        {
            p_dim->w = p_sde->w;
        }
        
        if(p_sde->flags & FLG_SDCH_FOY)
        {
            p_dim->y = (p_parent_height - p_sde->y);
        }
        else
        {
            p_dim->y = p_sde->y;
        }
        
        if(p_sde->flags & FLG_SDCH_FOH)
        {
            p_dim->h = (p_parent_height - p_sde->h - p_dim->y);
        }
        else
        {
            p_dim->h = p_sde->h;
        }
    }

// =============================================================================

    static void
    SuperDlg_OnCreate
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2(SDCREATE) sdc = (SDCREATE*) cs->lpCreateParams;
        
        int i = 0;
        
        int const w = cs->cx;
        int const h = cs->cy;
        
        SD_ENTRY const * sde = sdc->dlgtemp->sde;
        
        // -----------------------------
        
        SetWindowLongPtr
        (
            p_win,
            GWLP_USERDATA,
            (LONG_PTR) sdc
        );
        
        /**
            \note The widths and heights of the controls can be negative
            at creation time, and this is owing to the fact that we don't yet
            have size information on the dialog itself at this point. So don't
            worry too much about the negative dimensions, it will be sorted
            out by the WM_SIZE message that will be issued soon after
            creation.
        */
        for(i = 0; i < sdc->dlgtemp->numsde; i += 1)
        {
            SDE_Dimensions sde_dim = { 0};
            
            HWND sde_win;
            
            // -----------------------------
            
            SuperDlg_SizeChild
            (
                &sde[i],
                &sde_dim,
                w,
                h
            );
            
            sde_win = CreateWindowEx
            (
                sde[i].exstyle,
                sde[i].cname,
                sde[i].caption,
                sde[i].style,
                sde_dim.x,
                sde_dim.y,
                sde_dim.w,
                sde_dim.h,
                p_win,
                (HMENU) (sde[i].id),
                hinstance,
                0
            );
            
            if(sde_win != NULL)
            {
                SendMessage
                (
                    sde_win,
                    WM_SETFONT,
                    (WPARAM) GetStockObject(ANSI_VAR_FONT),
                    0
                );
            }
            else
            {
                MessageBox
                (
                    p_win,
                    "Failed to create SD entry. Why?",
                    "Error",
                    MB_OK
                );
            }
        }
        
        sdc->win  = p_win;
        sdc->prev = lastdlg;
        sdc->next = 0;
        
        if(lastdlg)
        {
            lastdlg->next = sdc;
        }
        else
        {
            firstdlg = sdc;
        }
        
        lastdlg = sdc;
        
        sdc->dlgtemp->proc
        (
            p_win,
            WM_INITDIALOG,
            0,
            sdc->lparam
        );
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
            
            SDE_Dimensions sde_dim = { 0 };
            
            // -----------------------------
            
            SuperDlg_SizeChild
            (
                &sde[i],
                &sde_dim,
                w,
                h
            );
            
            SetWindowPos
            (
                sd_child,
                0,
                sde_dim.x,
                sde_dim.y,
                sde_dim.w,
                sde_dim.h,
                SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
            );
        }
    }

// =============================================================================

    extern LRESULT CALLBACK
    SuperDlg(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

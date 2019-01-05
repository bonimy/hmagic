
#include "structs.h"
#include "prototypes.h"

#include "Wrappers.h"
#include "GdiObjects.h"

#include "HMagicUtility.h"

#include "PaletteEdit.h"

// =============================================================================

COLORREF custcols[16];

// =============================================================================

    static char const pal_w[] =
    {
        3,4,15,7,15,7,7,7,15,16,16,7,7,16,8,8
    };

    static char const pal_h[] =
    {
        1,1,1,5,4,3,1,1,6,2,8,1,1,6,1,1
    };

// =============================================================================

    void
    PaletteFrame_OnCreate
    (
        HWND const p_win,
        LPARAM const p_lp
    )
    {
        short * b = NULL;
        
        int i = 0;
        int j = 0;
        int k = 0;
        int l = 0;
        
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) (cs->lpCreateParams);
        
        CP2(PALEDIT) ed = (PALEDIT*) (mdi_cs->lParam);

        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, (LONG_PTR) ed);
        
        ShowWindow(p_win, SW_SHOW);
        
        i = (ed->ew.param >> 10) & 63;
        
        ed->palw = pal_w[i];
        ed->palh = pal_h[i];
        
        k = ed->palw*ed->palh;
        
        ed->pal = malloc(k * 2);
        ed->brush = malloc(k * 4);
        ed->modf = 0;
        
        b = (short*)
        (
            ed->ew.doc->rom
          + romaddr(pal_addr[i] + (k * (ed->ew.param >> 16) << 1) )
        );
        
        for(j = 0; j < k; j += 1)
        {
            l = ed->pal[j] = b[j];
            
            ed->brush[j] = CreateSolidBrush
            (
                ( (l & 0x1f) << 3) + ((l & 0x3e0) << 6) + ((l & 0x7c00) << 9)
            );
        }
        
        Updatesize(p_win);
    }

// =============================================================================

    static void
    PaletteFrame_OnPaint
    (
        PALEDIT const * const p_ed,
        HWND            const p_win
    )
    {
        PAINTSTRUCT ps;
        
        HDC const hdc = BeginPaint(p_win, &ps);
        
        RECT rc = HM_GetClientRect(p_win);
        
        int i = 0;
        int j = 0;
        int k = 0;
        
        int m = ( rc.right - (p_ed->palw << 4) ) >> 1;
        int n = ( rc.bottom - (p_ed->palh << 4) ) >> 1;
        
        HGDIOBJ const oldobj = SelectObject(hdc, white_pen);
        
        // -----------------------------
        
        for(j = 0; j < p_ed->palh; j += 1)
        {
            for(i = 0; i < p_ed->palw; i++, k++)
            {
                rc.left   = m + (i << 4);
                rc.right  = rc.left + 16;
                rc.top    = n + (j << 4);
                rc.bottom = rc.top + 16;
                
                FillRect(hdc, &rc, p_ed->brush[k]);
                
                if( p_ed->pal[k] & 0x8000 )
                {
                    // I see that this paints a diagonal line on the
                    // palette entry, but does this do anything practical?
                    // In some programs this indicates that a default color
                    // is used instead, but hard to say here.
                    
                    MoveToEx(hdc, rc.left + 3, rc.top + 13, 0);
                    LineTo(hdc, rc.left + 13, rc.top + 3);
                }
            }
        }
        
        SelectObject(hdc, oldobj);
        
        rc.left   = m;
        rc.top    = n;
        rc.right  = rc.left + (i << 4);
        rc.bottom = rc.top + (j << 4);
        
        FrameRect(hdc, &rc, black_brush);
        
        EndPaint(p_win, &ps);
    }

// =============================================================================

    extern LRESULT CALLBACK
    PaletteFrameProc
    (
        HWND   win,
        UINT   msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        int i = 0,j,k,m,n;
        short l;
        RECT rc;
        
        CHOOSECOLOR cc;
        
        CP2(PALEDIT) ed = (PALEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
    
        // -----------------------------
        
        if(msg == WM_CREATE)
        {
            PaletteFrame_OnCreate(win, lparam);
            
            return DefMDIChildProc(win, msg, wparam, lparam);
        }
        
        switch(msg)
        {
        
        case WM_CLOSE:
            
            if(ed->ew.doc->modf == 2)
                goto deflt;
            
            if(ed->modf)
            {
                char text_buf[0x200] = { 0 };
                
                // -----------------------------
                
                wsprintf(text_buf,
                         "Confirm palette modification?",
                         ed->ew.param);
                
                switch
                (
                    MessageBox(framewnd,
                               text_buf,
                               "Palette editor",
                               MB_YESNOCANCEL)
                )
                {
                
                case IDYES:
                    
                    Savepal(ed);
                    
                    goto deflt;
                
                case IDCANCEL:
                    break;
                
                case IDNO:
                    
                    goto deflt;
                }
            }
            
            goto deflt;
        case WM_MDIACTIVATE:
            
            activedoc=((WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA))->ew.doc;
            
            break;
        
        case WM_GETMINMAXINFO:
            
            DefMDIChildProc(win,msg,wparam,lparam);
            
            if(!ed)
                goto deflt;
            
            break;
        
        case WM_PAINT:
            
            if(ed)
            {
                PaletteFrame_OnPaint(ed, win);
            }
            
            break;
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            
            rc = HM_GetClientRect(win);
            
            m = ( rc.right - (ed->palw << 4) ) >> 1;
            n = ( rc.bottom - (ed->palh << 4) ) >> 1;
            i = ( (lparam & 65535) - m ) >> 4;
            j = ( (lparam >> 16) - n ) >> 4;
            
            if(i<0 || i>=ed->palw || j<0 || j>=ed->palh) break;
            
            k = i + j * ed->palw;
            
            if(msg == WM_RBUTTONDOWN)
            {
                ed->pal[k] ^= 0x8000;
                
                goto upd;
            }
            
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = framewnd;
            
            // Not using a custom template.
            cc.hInstance = NULL;
            
            l = ed->pal[k];
            
            cc.rgbResult = ((l & 0x1f) << 3) + ((l & 0x3e0) << 6) + ((l & 0x7c00) << 9);
            cc.lpCustColors=custcols;
            cc.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;
            
            if(ChooseColor(&cc))
            {
                ed->pal[k] = HM_ColRefTo5bpc(cc.rgbResult);
                
                DeleteObject(ed->brush[k]);
                ed->brush[k]=CreateSolidBrush(cc.rgbResult);
                
            upd:
                
                ed->modf=1;
                rc.left=m+(i<<4);
                rc.top=n+(j<<4);
                rc.right=rc.left+16;
                rc.bottom=rc.top+16;
                
                InvalidateRect(win,&rc,0);
            }
            
            break;
        
        case WM_DESTROY:
            
            for(i = ed->palw * ed->palh - 1; i >= 0; i--)
            {
                DeleteObject(ed->brush[i]);
            }
            
            ed->ew.doc->pals[ed->ew.param&1023]=0;
            
            free(ed->pal);
            free(ed->brush);
            free(ed);
            
            break;
        
            
        deflt:
        default:
            
            return DefMDIChildProc(win,msg,wparam,lparam);
        }
        
        return 0;
    }

// =============================================================================

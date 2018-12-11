
    #include "structs.h"
    
    #include "Wrappers.h"

    #include "GdiObjects.h"

// =============================================================================

    static void
    HM_ErrorControl_OnPaint
    (
        HWND const p_win
    )
    {
        PAINTSTRUCT ps;
        
        HDC const hdc = BeginPaint(p_win, &ps);
        
        HGDIOBJ const old_brush = SelectObject(hdc, red_brush);
        
        HGDIOBJ const old_pen = SelectObject(hdc, green_pen);
        
        // -----------------------------
        
        FillRect(hdc, &ps.rcPaint, red_brush);
        
        SelectObject(hdc, old_brush);
        SelectObject(hdc, old_pen);
        
        EndPaint(p_win, &ps);
    }

// =============================================================================

    extern LRESULT CALLBACK
    HM_ErrorControlProc
    (
        HWND   p_win,
        UINT   p_msg,
        WPARAM p_wp,
        LPARAM p_lp
    )
    {
        switch(p_msg)
        {
        
        case WM_PAINT:
            
            HM_ErrorControl_OnPaint(p_win);
            
            break;

        default:
            
            return DefWindowProc(p_win, p_msg, p_wp, p_lp);
        }
        
        return 0;
    }


// =============================================================================

    static void
    Palselrect
    (
        CP2C(BLKEDIT8)       ed,
        HWND           const win,
        CP2(RECT)            rc
    )
    {
        (void) win;
        
        rc->left   =   (ed->psel & 15)       * ed->pwidth  >> 4;
        rc->top    =   (ed->psel >> 4)       * ed->pheight >> 4;
        rc->right  = ( (ed->psel & 15) + 1 ) * ed->pwidth  >> 4;
        rc->bottom = ( (ed->psel >> 4) + 1 ) * ed->pheight >> 4;
    }

// =============================================================================

    static void
    PaletteSelector_OnPaint
    (
        CP2C(BLKEDIT8)       ed,
        HWND          const win
    )
    {
        int i = 0,
            j = 0,
            k = 0,
            l = 0,
            m = 0;
        
        OVEREDIT * const oed = ed->oed;
        
        RECT rc;
        
        PAINTSTRUCT ps;
        
        HDC const hdc = BeginPaint(win, &ps);
        
        HPALETTE const oldpal = SelectPalette(hdc, ed->oed->hpal, 1);
        
        HGDIOBJ const oldobj = GetCurrentObject(hdc, OBJ_BRUSH);
        
        HBRUSH newobj = 0;
        
        // -----------------------------  
        
        k =   ( ps.rcPaint.left  << 4 ) / ed->pwidth;
        j =   ( ps.rcPaint.top   << 4 ) / ed->pheight;
        l = ( (ps.rcPaint.right  << 4 ) + ed->pwidth - 1 ) / ed->pwidth;
        m = ( (ps.rcPaint.bottom << 4 ) + ed->pheight - 1 ) / ed->pheight;
        
        RealizePalette(hdc);
        
        for( ; j < m; j += 1)
        {
            for(i = k; i < l; i += 1)
            {
                HBRUSH const oldob2 = newobj;
                
                // -----------------------------
                
                if(oed->hpal)
                {
                    // \task Test this in 256-color mode.
                    newobj = CreateSolidBrush
                    (
                        PALETTEINDEX( *(WORD*) &oed->pal[i + (j << 4)] )
                    );
                }
                else
                {
                    COLORREF cr = HM_ColQuadToRef( oed->pal[ i + (j << 4) ] );
                    
                    newobj = CreateSolidBrush(cr);
                }
                
                SelectObject(hdc, newobj);
                
                if(oldob2)
                {
                    DeleteObject(oldob2);
                }
                
                Rectangle
                (
                    hdc,
                    i * ed->pwidth >> 4,
                    j * ed->pheight >> 4,
                    (i + 1) * ed->pwidth >> 4,
                    (j + 1) * ed->pheight >> 4
                );
            }
        }
        
        Palselrect(ed, win, &rc);
        FrameRect(hdc, &rc, green_brush);
        
        SelectPalette(hdc, oldpal, 1);
        SelectObject(hdc, oldobj);
        
        if(newobj)
        {
            DeleteObject(newobj);
        }
        
        EndPaint(win, &ps);
    }

// =============================================================================

    static void
    PaletteSelector_OnLeftMouseDown
    (
        CP2(BLKEDIT8)       p_ed,
        HWND          const p_win,
        LPARAM        const p_lp
    )
    {
        int i = 0,
            j = 0,
            k = 0;
        
        RECT rc;
        
        // -----------------------------
        
        Palselrect(p_ed, p_win, &rc);
        InvalidateRect(p_win, &rc, 0);
        
        if(p_ed->blknum == 260)
            j = p_ed->psel & 0xf0;
        else if(p_ed->blknum >= 256)
            j = p_ed->psel & 0xfc;
        else j = p_ed->psel & 0xf8;
        
        p_ed->psel = (((short) p_lp) << 4) / p_ed->pwidth
                 +((p_lp >> 12) / p_ed->pheight << 4);
        
        if(p_ed->blknum == 260)
            i = p_ed->psel & 0xf0, k = 15;
        else if(p_ed->blknum >= 256)
            i = p_ed->psel & 0xfc, k = 3;
        else
            i = p_ed->psel & 0xf8, k = 7;
        
        if(p_ed->oed->gfxtmp != 0xff && i != j)
        {
            for(j = (p_ed->size << 6) - 1; j >= 0; j--)
            {
                p_ed->buf[j] &= k;
                
                if(p_ed->buf[j])
                    p_ed->buf[j]|=i;
            }
            
            InvalidateRect(p_ed->blkwnd,0,0);
        }
        
        Palselrect(p_ed,
                   p_win,
                   &rc);
        
        InvalidateRect(p_win,
                       &rc,
                       0);
    }

// =============================================================================

    // Palette selection window that gets used when editing 8x8 tiles
    // (at the very least in the case of dungeon objects)
    extern LRESULT CALLBACK
    PaletteSelector
    (
        HWND   win,
        UINT   msg,
        WPARAM wparam,
        LPARAM lparam
    )
    {
        CP2(BLKEDIT8) ed = (BLKEDIT8*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        // -----------------------------
        
        if( IsNull(ed) || 1 )
        {
            return HM_ErrorControlProc(win, msg, wparam, lparam);
        }
        
        switch(msg)
        {
        
        case WM_PAINT:
            
            PaletteSelector_OnPaint(ed, win);
            
            break;
        
        case WM_LBUTTONDOWN:
            
            PaletteSelector_OnLeftMouseDown(ed, win, lparam);
            
            break;
        
        default:
            
            return DefWindowProc(win, msg, wparam, lparam);
        }
        
        return 0;
    }

// =============================================================================

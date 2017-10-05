
#include "structs.h"
#include "Wrappers.h"

#include "GdiObjects.h"

// =============================================================================

void Palselrect(BLKEDIT8 *ed, HWND win, RECT *rc)
{
    (void) win;
    
    rc->left = (ed->psel & 15) * ed->pwidth >> 4;
    rc->top  = (ed->psel >> 4) * ed->pheight >> 4;
    rc->right  = ((ed->psel & 15) + 1) * ed->pwidth >> 4;
    rc->bottom = ((ed->psel >> 4) + 1) * ed->pheight >> 4;
}

// =============================================================================

void
PaletteSelector_OnPaint(BLKEDIT8 * const ed,
                        HWND       const win)
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
    
    k=(ps.rcPaint.left<<4)/ed->pwidth;
    j=(ps.rcPaint.top<<4)/ed->pheight;
    l=((ps.rcPaint.right<<4)+ed->pwidth-1)/ed->pwidth;
    m=((ps.rcPaint.bottom<<4)+ed->pheight-1)/ed->pheight;
    
    RealizePalette(hdc);
    
    for(;j<m;j++)
    {
        for(i=k;i<l;i++)
        {
            HBRUSH const oldob2 = newobj;
            
            if(oed->hpal)
            {
                // \task Perhaps test this in 256 color mode if possible.
                // MSDN seems to indicate that this is wrong, but ......
                // there is some evidence that this is how you generate
                // a COLORREF from the currently realized palette maybe?
                newobj = CreateSolidBrush(0x1000000 + ((short*)(oed->pal))[ i + (j << 4) ]);
            }
            else
            {
                COLORREF cr = HM_ColQuadToRef( oed->pal[ i + (j << 4) ] );
                
                newobj = CreateSolidBrush(cr);
            }
            
            SelectObject(hdc, newobj);
            
            if(oldob2)
                DeleteObject(oldob2);
            
            Rectangle(hdc,
                      i * ed->pwidth >> 4,
                      j * ed->pheight >> 4,
                      (i + 1) * ed->pwidth >> 4,
                      (j + 1) * ed->pheight >> 4);
        }
    }
    
    Palselrect(ed,win,&rc);
    FrameRect(hdc,&rc,green_brush);
    
    SelectPalette(hdc,oldpal,1);
    SelectObject(hdc,oldobj);
    
    if(newobj)
        DeleteObject(newobj);
    
    EndPaint(win, &ps);
}

// =============================================================================

// Palette selection window that gets used when editing 8x8 tiles
// (at the very least in the case of dungeon objects)
LRESULT CALLBACK
palselproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int i = 0,
        j = 0,
        k = 0;
    
    BLKEDIT8 *ed;
    
    RECT rc;
    
    switch(msg)
    {
    
    case WM_PAINT:
        
        ed = (BLKEDIT8*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            PaletteSelector_OnPaint(ed, win);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(BLKEDIT8*)GetWindowLong(win,GWL_USERDATA);
        Palselrect(ed,win,&rc);
        InvalidateRect(win,&rc,0);
        if(ed->blknum==260)
            j=ed->psel&0xf0;
        else if(ed->blknum>=256)
            j=ed->psel&0xfc; else j=ed->psel&0xf8;
        ed->psel=(((short)lparam)<<4)/ed->pwidth+((lparam>>12)/ed->pheight<<4);
        
        if(ed->blknum == 260)
            i=ed->psel&0xf0,k=15;
        else if(ed->blknum>=256)
            i=ed->psel&0xfc,k=3; else i=ed->psel&0xf8,k=7;
        
        if(ed->oed->gfxtmp!=0xff && i!=j)
        {
            for(j=(ed->size<<6)-1;j>=0;j--)
            {
                ed->buf[j]&=k;
                if(ed->buf[j]) ed->buf[j]|=i;
            }
            
            InvalidateRect(ed->blkwnd,0,0);
        }
        
        Palselrect(ed,win,&rc);
        
        InvalidateRect(win,&rc,0);
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

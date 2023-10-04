
#include "structs.h"

// =============================================================================

LRESULT CALLBACK
blkedit8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLKEDIT8*ed;
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch(msg)
    {
    
    case WM_PAINT:
        
        ed = (BLKEDIT8*)GetWindowLongPtr(win,GWLP_USERDATA);
        
        if(!ed) break;
        
        hdc=BeginPaint(win,&ps);
        
        if(ed->hpal!=ed->oed->hpal) memcpy(ed->pal,ed->oed->pal,1024);
        
        ed->hpal=ed->oed->hpal;
        
        if(ed->oed->hpal)
        {
            HPALETTE const oldpal = SelectPalette(hdc,ed->oed->hpal,1);
            
            RealizePalette(hdc);
            
            SetDIBitsToDevice(hdc,
                              -ed->scrollh,
                              -ed->scrollv,
                              128,
                              ed->size >> 1,
                              0,
                              0,
                              0,
                              ed->size >> 1,
                              ed->buf,
                              (BITMAPINFO*) &(ed->bmih),
                              DIB_PAL_COLORS);
            
            SelectPalette(hdc, oldpal, 1);
        }
        else
            SetDIBitsToDevice(hdc,-ed->scrollh,-ed->scrollv,128,ed->size>>1,0,0,0,ed->size>>1,ed->buf,(BITMAPINFO*)&(ed->bmih),DIB_RGB_COLORS);
        
        EndPaint(win,&ps);
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================
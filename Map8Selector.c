
    #include "structs.h"

    #include "GdiObjects.h"

    #include "prototypes.h"

// =============================================================================

LRESULT CALLBACK
blksel8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLOCKSEL8*ed;
    SCROLLINFO si;
    RECT rc;
    PAINTSTRUCT ps;
    int i,j,k,n;
    switch(msg) {
//  case WM_DESTROY:
//      ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
//      DeleteDC(ed->bufdc);
//      DeleteObject(ed->bufbmp);
//      break;
    case WM_SIZE:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=16;
        si.nMin=0;
        si.nMax=ed->ed->gfxtmp==0xff?0:63;
        SetScrollInfo(win,SB_VERT,&si,1);
        break;
    case WM_VSCROLL:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        j=i=ed->scroll;
        switch(wparam&65535) {
        case SB_BOTTOM:
            i=56;
            break;
        case SB_TOP:
            i=0;
            break;
        case SB_LINEDOWN:
            i++;
            break;
        case SB_LINEUP:
            i--;
            break;
        case SB_PAGEDOWN:
            i+=8;
            break;
        case SB_PAGEUP:
            i-=8;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        }
        if(i<0) i=0;
        if(i>48) i=48;
        if(i==ed->scroll) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        si.nPos=i;
        SetScrollInfo(win,SB_VERT,&si,1);
        ScrollWindowEx(win,0,(j*ed->h>>4)-(i*ed->h>>4),0,0,0,0,SW_INVALIDATE|SW_ERASE);
        ed->scroll=i;
        i<<=4;
        j<<=4;
        
        if(j<i) j+=256,k=i+256; else k=j,j=i;
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->ed->hpal, 1);
            HPALETTE const oldpal2 = SelectPalette(ed->bufdc, ed->ed->hpal, 1);
            
            for( ; j < k; j++)
            {
                n = (j - i);
                
                Updateblk8sel(ed, n);
            }
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(ed->bufdc, oldpal2, 1);
        }
        
        break;
    
    case WM_RBUTTONDOWN:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        j=17;
        goto edit;
    case WM_LBUTTONDBLCLK:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        i=(ed->scroll<<4)+((lparam&65535)<<4)/ed->w+((((lparam>>16)<<4)/ed->h)<<4);
        if(ed->dfl==16) if(i<0x180) j=256+(i>>7); else break;
        else if(ed->dfl==8) if(i<0x100) j=259; else break;
        else if(ed->ed->gfxnum==0xff) j=0;
        else if(i<0x200) j=i>>6;
        else if(i<0x240) break;
        else if(i<0x280) j=10;
        else if(i<0x300) j=5+(i>>6);
        else j=(i>>6)-1;
        j|=(ed->flags&0x1c00)<<6;
        
        if(ed->sel>=0x200)
            j |= 0x80000;
        
    edit:
        
        if( Editblocks(ed->ed, j, win) )
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->ed->hpal, 1);
            
            HPALETTE const oldpal2 = SelectPalette(ed->bufdc, ed->ed->hpal, 1);
            
            for(i = 0; i < 256; i++)
                Updateblk8sel(ed,i);
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(ed->bufdc, oldpal2, 1);
            
            InvalidateRect(win, &rc, 0);
            
            SendMessage(GetParent(win), 4001, 0, 0);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        SendMessage(GetParent(win),4000,(ed->scroll<<4)+((lparam&65535)<<4)/ed->w+((((lparam>>16)<<4)/ed->h)<<4)+ed->flags,0);
        break;
    case WM_PAINT:
        
        ed = (BLOCKSEL8*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        if(always)
        {
            HDC const hdc = BeginPaint(win, &ps);
            
            HPALETTE const oldpal = SelectPalette(hdc, ed->ed->hpal, 1);
            
            RealizePalette(hdc);
            
            k=(ed->scroll&15)*ed->h>>4;
            i=ed->h-k;
            if(i>ps.rcPaint.bottom) j=ps.rcPaint.bottom; else j=i;
            if(j>ps.rcPaint.top)
                BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,j-ps.rcPaint.top,ed->bufdc,ps.rcPaint.left,ps.rcPaint.top+k,SRCCOPY);
            if(i<ps.rcPaint.top) j=ps.rcPaint.top; else j=i;
            if(j<ps.rcPaint.bottom)
                BitBlt(hdc,ps.rcPaint.left,j,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-j,ed->bufdc,ps.rcPaint.left,j+k-ed->h,SRCCOPY);
            i=ed->sel-(ed->scroll<<4);
            if(i>=0 && i<256) {
                rc.left=(i&15)*ed->w>>4;
                rc.top=(i&240)*ed->h>>8;
                rc.right=rc.left+(ed->w>>4);
                rc.bottom=rc.top+(ed->h>>4);
                FrameRect(hdc,&rc,green_brush);
            }
            
            SelectPalette(hdc, oldpal, 1);
            EndPaint(win, &ps);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================
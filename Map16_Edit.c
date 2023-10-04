
    #include "structs.h"

    #include "GdiObjects.h"

    #include "MetatileLogic.h"

// =============================================================================

LRESULT CALLBACK
blkedit16proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    BLOCKEDIT16*ed;
    PAINTSTRUCT ps;
    HPALETTE oldpal;
    HDC hdc;
    HBRUSH hbr;
    int i;
    switch(msg) {
    case WM_LBUTTONDOWN:
        ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        ed->blks[i]=ed->bs.sel|ed->bs.flags;
        oldpal=SelectPalette(objdc,ed->bs.ed->hpal,1);
        SelectPalette(ed->bufdc,ed->bs.ed->hpal,1);
        Updateblk16disp(ed,i);
        SelectPalette(objdc,oldpal,1);
        SelectPalette(ed->bufdc,oldpal,1);
        InvalidateRect(win,0,0);
        break;
    case WM_RBUTTONDOWN:
        ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        SendMessage(GetParent(win),4000,ed->blks[i],0);
        break;
    case WM_PAINT:
        ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(!ed) break;
        hdc=BeginPaint(win,&ps);
        oldpal=SelectPalette(hdc,ed->bs.ed->hpal,1);
        RealizePalette(hdc);
        BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,ed->bufdc,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
        hbr=SelectObject(hdc,white_pen);
        MoveToEx(hdc,ed->w>>1,0,0);
        LineTo(hdc,ed->w>>1,ed->h-1);
        MoveToEx(hdc,0,ed->h>>1,0);
        LineTo(hdc,ed->w-1,ed->h>>1);
        SelectObject(hdc,hbr);
        SelectPalette(hdc,oldpal,1);
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

    #include "structs.h"

    #include "GdiObjects.h"

    #include "prototypes.h"

    #include "LevelMapLogic.h"

// =============================================================================

LRESULT CALLBACK
lmapblksproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k,l,n,o,p,q;
    
    HDC hdc;
    HPALETTE oldpal;
    HGDIOBJ oldobj;
    SCROLLINFO si;
    LMAPEDIT*ed;
    PAINTSTRUCT ps;
    RECT rc;
    
    unsigned char*rom;
    
    switch(msg)
    {
    
    case WM_LBUTTONDOWN:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        i=(lparam&65535)>>4;
        j=lparam>>20;
        if(i<0 || i>=ed->blkrow || j<0) break;
        i+=(j+ed->blkscroll)*ed->blkrow;
        if(i>=0xba) break;
        lmapblkchg(win,ed);
        ed->blksel=i;
        lmapblkchg(win,ed);
        InvalidateRect(win,&rc,0);
        break;
    case WM_PAINT:
        
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        hdc=BeginPaint(win,&ps);
        oldpal=SelectPalette(hdc,ed->hpal,1);
        RealizePalette(hdc);
        k=((ps.rcPaint.right+31)&0xffffffe0);
        l=((ps.rcPaint.bottom+31)&0xffffffe0);
        j=ps.rcPaint.top&0xffffffe0;
        o=((j>>4)+ed->blkscroll)*ed->blkrow;
        rom=ed->ew.doc->rom;
        oldobj=SelectObject(hdc,white_pen);
        for(;j<l;j+=32) {
            i=ps.rcPaint.left&0xffffffe0;
            n=i>>4;
            for(;i<k;i+=32) {
                if(n>=ed->blkrow) break;
                for(q=0;q<32;q+=16) {
                    for(p=0;p<32;p+=16) {
                        if(n+o<0xba) {
                            Drawblock((OVEREDIT*)ed,p,q,((short*)(rom + 0x57009))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p+8,q,((short*)(rom + 0x5700b))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p,q+8,((short*)(rom + 0x5700d))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p+8,q+8,((short*)(rom + 0x5700f))[(n+o) << 2],0);
                        } else {
                            Drawblock((OVEREDIT*)ed,p,q,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p+8,q,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p,q+8,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p+8,q+8,0xf00,0);
                        }
                        n++;
                    }
                    n+=ed->blkrow-2;
                }
                n+=2;
                n-=ed->blkrow<<1;
                Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)ed);
                if(ed->disp&1) {
                    MoveToEx(hdc,i+32,j,0);
                    LineTo(hdc,i,j);
                    LineTo(hdc,i,j+32);
                    MoveToEx(hdc,i+16,j,0);
                    LineTo(hdc,i+16,j+32);
                    MoveToEx(hdc,i,j+16,0);
                    LineTo(hdc,i+32,j+16);
                }
            }
            o+=ed->blkrow<<1;
        }
        
        rc.left=(ed->blksel%ed->blkrow)<<4;
        rc.top=(ed->blksel/ed->blkrow-ed->blkscroll)<<4;
        rc.right=rc.left+16;
        rc.bottom=rc.top+16;
        
        FrameRect(hdc, &rc, green_brush);
        
        SelectObject(hdc, oldobj);
        SelectPalette(hdc, oldpal, 1);
        
        EndPaint(win,&ps);
        
        break;
    
    case WM_SIZE:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        ed->blkrow=(lparam&65535)>>4;
        si.nMax=(0xba + ed->blkrow-1)/ed->blkrow;
        ed->maxrow=si.nMax;
        si.nPage=(lparam>>20);
        ed->blkpage=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->blkscroll=Handlescroll(win,-1,ed->blkscroll,ed->blkpage,SB_VERT,si.nMax,16);
        break;
    case WM_VSCROLL:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->blkscroll=Handlescroll(win,wparam,ed->blkscroll,ed->blkpage,SB_VERT,ed->maxrow,16);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

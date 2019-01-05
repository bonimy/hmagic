
#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"

#include "OverworldEdit.h"

// =============================================================================

int
Keyscroll(HWND win,int wparam,int sc,int page,int scdir,int size,int size2)
{
    int i;
    switch(wparam) {
    case VK_UP:
        i=SB_LINEUP;
        break;
    case VK_DOWN:
        i=SB_LINEDOWN;
        break;
    case VK_PRIOR:
        i=SB_PAGEUP;
        break;
    case VK_NEXT:
        i=SB_PAGEDOWN;
        break;
    case VK_HOME:
        i=SB_TOP;
        break;
    case VK_END:
        i=SB_BOTTOM;
        break;
    default:
        return sc;
    }
    return Handlescroll(win,i,sc,page,scdir,size,size2);
}

// =============================================================================

void
Blk16Search_OnPaint(OVEREDIT const * const p_ed,
                    HWND             const p_win)
{
    text_buf_ty text_buf = { 0 };

    unsigned char const * const rom = p_ed->ew.doc->rom;
    
    int i = 0,
        j = 0,
        m = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal,1);
    
    HGDIOBJ const oldbrush = SelectObject(hdc, trk_font);
    HGDIOBJ const oldobj2 = SelectObject(hdc, white_pen);
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    i = (ps.rcPaint.top >> 4);
    j = ( (ps.rcPaint.bottom + 15) >> 4);
    
    FillRect(hdc,&(ps.rcPaint), black_brush);
    
    m = i + p_ed->schscroll;
    
    SetTextColor(hdc, 0xffffff);
    
    SetBkColor(hdc, 0);
    
    for( ; i < j; i += 2)
    {
        int k = 0;
        
        for(k = 0; k < 1024; k++)
            drawbuf[k] = 0;
        
        for(k = 0; k < 2; k++)
        {
            int l = 0;
            
            unsigned short const * o = 0;
            
            RECT rc;
            
            if(m >= 0xea8)
                break;
            
            o = (unsigned short*) (rom + 0x78000 + (m << 3));
            
            for(l = 0; l < 4; l++)
                Drawblock(p_ed,
                          blkx[l] + 8,
                          blky[l] + (k << 4),
                          *(o++),
                          0);
            rc.left=4;
            rc.right=20;
            rc.top = ( (i + k) << 4) + 2;
            rc.bottom=rc.top+12;
            
            DrawFrameControl(hdc,&rc,DFC_BUTTON,
                             ((p_ed->selsch[m>>3]&(1<<(m&7))) ? (DFCS_BUTTONCHECK | DFCS_CHECKED):DFCS_BUTTONCHECK)
                             +((p_ed->schpush == m) ? DFCS_PUSHED : 0));
            
            rc.left=24;
            rc.right=40;
            
            DrawFrameControl(hdc,
                             &rc,
                             DFC_BUTTON,
                             ((p_ed->selsch[0x1d5 + (m >> 3)] & (1 << (m & 7)))
                           ? (DFCS_BUTTONCHECK|DFCS_CHECKED)
                           : DFCS_BUTTONCHECK)
                           +((p_ed->schpush==m + 0xea8)?DFCS_PUSHED:0));
            
            rc.left=44;
            rc.right=60;
            
            DrawFrameControl(hdc,
                             &rc,
                             DFC_BUTTON,
                             ((p_ed->selsch[0x3aa + (m >> 3)] & (1 << (m & 7)))
                           ? (DFCS_BUTTONCHECK|DFCS_CHECKED)
                           : DFCS_BUTTONCHECK)
                           + ((p_ed->schpush== m + 0x1d50) ? DFCS_PUSHED : 0));
            
            wsprintf(text_buf,
                     "%04d",
                     m);
            
            TextOut(hdc, 64, rc.top, text_buf, 4);
            
            m++;
        }
        
        k = i << 4;
        
        Paintblocks(&(ps.rcPaint),hdc,100,k, (DUNGEDIT*) p_ed);
        
        MoveToEx(hdc,108,k,0);
        LineTo(hdc,124,k);
        
        MoveToEx(hdc,108,k+16,0);
        LineTo(hdc,124,k+16);
    }
    
    SelectObject(hdc, oldbrush);
    SelectObject(hdc, oldobj2);
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
blk16search(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT *ed;
    RECT rc;
    SCROLLINFO si;
    int i,j,k;
    
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS|DLGC_WANTARROWS;
    case WM_KEYDOWN:
        
        ed = (OVEREDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(wparam >= 48 && wparam < 58)
        {
            wparam -= 48;
            
            ed->schtyped /* <<=4 */ *=10;
            ed->schtyped+=wparam;
            ed->schtyped%=5000;
            ed->schscroll=Handlescroll(win,SB_THUMBPOSITION|(ed->schtyped<<16),ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        } else ed->schscroll=Keyscroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    case WM_SIZE:
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=0xea8;
        si.nPage=lparam>>20;
        ed->schpage=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->schscroll=Handlescroll(win,-1,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    case WM_VSCROLL:
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        ed->schscroll=Handlescroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    
    case WM_PAINT:
        
        ed = (OVEREDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(ed)
        {
            Blk16Search_OnPaint(ed, win);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        i=(short)lparam;
        j=lparam>>16;
        k=(j>>4)+ed->schscroll;
        if(k<0 || k>=0xea8) break;
        if(ed->schpush!=-1)
            InvalidateRect(win,&(ed->schrc),0);
        if((j&15)>=2 && (j&15)<14) {
            if(i>4 && i<20) j=0,rc.left=4;
            else if(i>24 && i<40) j=0xea8,rc.left=24;
            else if(i>44 && i<60) j=0x1d50,rc.left=44;
            else break;
            ed->schpush=k+j;
            rc.right=rc.left+16;
            rc.top = ( (k - ed->schscroll) << 4) + 2;
            rc.bottom=rc.top+12;
            ed->schrc=rc;
            InvalidateRect(win,&rc,0);
            SetCapture(win);
        }
        break;
    case WM_LBUTTONUP:
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(ed->schpush!=-1) {
            i=(short)lparam;
            j=lparam>>16;
            rc=ed->schrc;
            if(i>=rc.left && i<rc.right && j>=rc.top && j<rc.bottom)
            ed->selsch[ed->schpush>>3]^=(1<<(ed->schpush&7));
            ed->schpush=-1;
            InvalidateRect(win,&(ed->schrc),0);
            ReleaseCapture();
        }
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

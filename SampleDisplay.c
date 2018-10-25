
    #include "structs.h"
    #include "prototypes.h"

    // For pen objects.
    #include "GdiObjects.h"

    // Access to other window procedures.
    #include "Callbacks.h"

    // For wrapper of GetClientRect()
    #include "Wrappers.h"

// =============================================================================

    void
    Getsampsel(SAMPEDIT *ed, RECT *rc)
    {
        rc->left=(ed->sell<<16)/ed->zoom-ed->scroll;
        rc->right=(ed->selr<<16)/ed->zoom+1-ed->scroll;
    }

// =============================================================================

LRESULT CALLBACK
sampdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SAMPEDIT*ed;
    ZWAVE*zw;
    PAINTSTRUCT ps;
    HDC hdc,oldobj;
    SCROLLINFO si;
    int i,j,k,l;
    RECT rc,rc2;
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    case WM_SIZE:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        i=ed->height;
        ed->width=(short)lparam;
        ed->height=lparam>>16;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->zw->end;
        ed->page=si.nPage=(ed->width<<16)/ed->zoom;
        SetScrollInfo(win,SB_HORZ,&si,1);
        if(i!=ed->height) InvalidateRect(win,0,1);
        break;
    case WM_HSCROLL:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->scroll=Handlescroll(win,wparam,ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
        break;
    
    case WM_KEYDOWN:
        
        ed = (SAMPEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        switch(wparam)
        {
        
        case VK_DELETE:
            
            zw = ed->zw;
            
            if(zw->copy != -1)
                break;
            
            if(ed->sell == ed->selr)
            {
                // \task This logic deletes the whole sample if there is
                // is not a selected region of width greater than zero.
                // Is this really intuitive? Seems like a great way to lose
                // work.
                
                zw->end = 0;
                
                zw->buf = (short*) realloc(zw->buf, 2);
                
                zw->buf[0]=0;
                zw->lopst=0;
                ed->sell=0;
            }
            else
            {
                memcpy(zw->buf+ed->sell,zw->buf+ed->selr,zw->end-ed->selr);
                
                zw->end += ed->sell-ed->selr;
                
                zw->buf = realloc(zw->buf, (zw->end + 1) << 1);
                
                if(zw->lopst >= ed->selr)
                    zw->lopst += ed->sell - ed->selr;
                
                zw->buf[zw->end]=zw->buf[zw->lopst];
            }
            ed->selr=ed->sell;
            ed->init=1;
            SetDlgItemInt(ed->dlg,3008,zw->end,0);
            SetDlgItemInt(ed->dlg,3010,zw->lopst,0);
            InvalidateRect(win,0,1);
            ed->init=0;
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            break;
        }
        break;
    case WM_CHAR:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case 1:
            ed->sell=0;
            ed->selr=ed->zw->end;
            InvalidateRect(win,0,1);
            break;
        case 3:
            sampdlgproc(ed->dlg,WM_COMMAND,3005,0);
            break;
        case 22:
            sampdlgproc(ed->dlg,WM_COMMAND,3006,0);
            break;
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetFocus(win);
        SetCapture(win);
        ed->flag|=4;
        ed->flag&=-9;
        i=((short)lparam+ed->scroll)*ed->zoom>>16;
        rc = HM_GetClientRect(win);
        if(wparam&MK_SHIFT) {
            if(i<ed->sell) ed->flag|=8;
            goto selectmore;
        }
        Getsampsel(ed,&rc);
        InvalidateRect(win,&rc,1);
        ed->selr=ed->sell=i;
        Getsampsel(ed,&rc);
        InvalidateRect(win,&rc,1);
        break;
    case WM_LBUTTONUP:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->flag&=-5;
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->flag&4) {
            rc = HM_GetClientRect(win);
selectmore:
            rc2=rc;
            Getsampsel(ed,&rc);
            InvalidateRect(win,&rc,1);
            i=((short)lparam+ed->scroll)*ed->zoom>>16;
            if(i>ed->selr && (ed->flag&8)) {
                ed->flag&=-9;
                ed->sell=ed->selr;
            }
            if(i<ed->sell && !(ed->flag&8)) {
                ed->flag|=8;
                ed->selr=ed->sell;
            }
            if(ed->flag&8) ed->sell=i;
            else ed->selr=i;
            if(ed->sell<0) ed->sell=0;
            if(ed->selr>ed->zw->end) ed->selr=ed->zw->end;
            Getsampsel(ed,&rc);
            InvalidateRect(win,&rc,1);
            if((short)lparam>=rc2.right) ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(((short)lparam+ed->scroll-ed->page)<<16),ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
            if((short)lparam<rc2.left) ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(((short)lparam+ed->scroll)<<16),ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
        }
        break;
    case WM_PAINT:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        hdc=BeginPaint(win,&ps);
        zw=ed->ew.doc->waves+ed->editsamp;
        oldobj=SelectObject(hdc,green_pen);
        l=0;
        for(i=ps.rcPaint.left-1;i<=ps.rcPaint.right;i++) {
            j=((i+ed->scroll)*ed->zoom>>16);
            if(j<0) continue;
            if(j>=zw->end) break;
            k=((int)(zw->buf[j])+32768)*ed->height>>16;
            if(!l) MoveToEx(hdc,i,k,0),l=1; else LineTo(hdc,i,k);
        }
        SelectObject(hdc,oldobj);
        Getsampsel(ed,&rc);
        if(rc.left<ps.rcPaint.left) rc.left=ps.rcPaint.left;
        if(rc.right>ps.rcPaint.right) rc.right=ps.rcPaint.right;
        if(rc.left<rc.right) {
            rc.top=ps.rcPaint.top;
            rc.bottom=ps.rcPaint.bottom;
            InvertRect(hdc,&rc);
        }
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

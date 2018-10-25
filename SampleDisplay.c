﻿
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
    Getsampsel(SAMPEDIT const * const ed,
               RECT           * const rc)
    {
        rc->left  = (ed->sell << 16) / ed->zoom     - ed->scroll;
        rc->right = (ed->selr << 16) / ed->zoom + 1 - ed->scroll;
    }

// =============================================================================

    void
    SampleDisplay_OnPaint(SAMPEDIT * const p_ed,
                          HWND       const p_win)
    {
        int i = 0;
        
        RECT rc;
        
        PAINTSTRUCT ps;
        
        HDC const dc = BeginPaint(p_win, &ps);
        
        ZWAVE const * const zw = p_ed->ew.doc->waves + p_ed->editsamp;

        HGDIOBJ const oldobj = SelectObject(dc, green_pen);
        
        // -----------------------------
        
        for(i = ps.rcPaint.left - 1; i <= ps.rcPaint.right; i++)
        {
            int j = 0;
            int k = 0;
            int l = 0;
            
            // -----------------------------

            j = ((i + p_ed->scroll) * p_ed->zoom >> 16);
            
            if(j < 0)
                continue;
            
            if(j >= zw->end)
                break;
            
            k = ((int)(zw->buf[j]) + 32768) * p_ed->height >> 16;
            
            if(!l)
                MoveToEx(dc, i, k, 0), l = 1;
            else
                LineTo(dc,i,k);
        }
        
        SelectObject(dc, oldobj);
        Getsampsel(p_ed, &rc);
        
        if(rc.left < ps.rcPaint.left)
            rc.left = ps.rcPaint.left;
        
        if(rc.right > ps.rcPaint.right)
            rc.right = ps.rcPaint.right;
        
        if(rc.left < rc.right)
        {
            rc.top    = ps.rcPaint.top;
            rc.bottom = ps.rcPaint.bottom;
            InvertRect(dc, &rc);
        }
        
        EndPaint(p_win, &ps);
    }

// =============================================================================

/**
    Returns true if we need a valid editor pointer to handle the message,
    but none is available.
*/
BOOL
SampleDisplay_UnmetPrereqs(MSG    const p_packed_msg,
                           void * const p_edit_structure)
{
    // Some messages require a valid editor pointer to do anything useful.
    switch(p_packed_msg.message)
    {
    
    // The default assumption is that it is needed, the rest are
    // special exceptions, in essence.
    default:
        
        // If this comparison is true, we have an unmet prerequisite.
        return (NULL == p_edit_structure);
    
    case WM_GETDLGCODE:
        
        return FALSE;
    }
}

// =============================================================================

LRESULT CALLBACK
SampleDisplayProc(HWND p_win,UINT p_msg, WPARAM p_wp,LPARAM p_lp)
{
    ZWAVE * zw;
    SCROLLINFO si;
    int i;
    RECT rc,rc2;
    
    MSG const packed_msg = HM_PackMessage(p_win, p_msg, p_wp, p_lp);
    
    SAMPEDIT * const ed = (SAMPEDIT*) GetWindowLongPtr(p_win, GWLP_USERDATA);
    
    // -----------------------------
    
    if( SampleDisplay_UnmetPrereqs(packed_msg, ed) )
    {
        // Can't be handled.
        return 0;
    }
    
    switch(p_msg)
    {
    
    case WM_GETDLGCODE:
        
        return DLGC_WANTCHARS;
    
    case WM_SIZE:
        
        i = ed->height;
        
        ed->width  = (short) p_lp;
        ed->height = p_lp >> 16;
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE|SIF_PAGE;
        si.nMin = 0;
        si.nMax = ed->zw->end;
        
        ed->page = si.nPage=(ed->width<<16)/ed->zoom;
        
        SetScrollInfo(p_win,SB_HORZ,&si,1);
        
        if(i!=ed->height)
            InvalidateRect(p_win,0,1);
        
        break;
    
    case WM_HSCROLL:
        
        ed->scroll = Handlescroll(p_win,
                                  p_wp,
                                  ed->scroll,
                                  ed->page,
                                  SB_HORZ,
                                  ed->zw->end * ed->zoom >> 16,
                                  1);
        
        break;
    
    case WM_KEYDOWN:
        
        switch(p_wp)
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
            InvalidateRect(p_win,0,1);
            ed->init=0;
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            break;
        }
        
        break;
    
    case WM_CHAR:
        
        switch(p_lp)
        {
        
        case 1:
            ed->sell=0;
            ed->selr=ed->zw->end;
            InvalidateRect(p_win,0,1);
            
            break;
        
        case 3:
            sampdlgproc(ed->dlg, WM_COMMAND, 3005, 0);
            
            break;
        
        case 22:
            sampdlgproc(ed->dlg, WM_COMMAND, 3006, 0);
            
            break;
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        
        SetFocus(p_win);
        SetCapture(p_win);
        ed->flag|=4;
        ed->flag&=-9;
        
        i = ((short)p_lp + ed->scroll)*ed->zoom>>16;
        rc = HM_GetClientRect(p_win);
        
        if(p_wp & MK_SHIFT)
        {
            if(i<ed->sell) ed->flag|=8;
            goto selectmore;
        }
        
        Getsampsel(ed,&rc);
        InvalidateRect(p_win,&rc,1);
        ed->selr=ed->sell=i;
        Getsampsel(ed,&rc);
        InvalidateRect(p_win,&rc,1);
        break;
    
    case WM_LBUTTONUP:
        
        ed->flag &= -5;
        
        ReleaseCapture();
        
        break;
    
    case WM_MOUSEMOVE:
        
        if(ed->flag & 4)
        {
            rc = HM_GetClientRect(p_win);
selectmore:
            rc2=rc;
            Getsampsel(ed,&rc);
            InvalidateRect(p_win,&rc,1);
            i=((short)p_lp+ed->scroll)*ed->zoom>>16;
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
            InvalidateRect(p_win,&rc,1);
            
            if((short)p_lp >= rc2.right)
                ed->scroll = Handlescroll
                (
                    p_win,
                    SB_THUMBPOSITION|(((short)p_lp + ed->scroll-ed->page)<<16),
                    ed->scroll,
                    ed->page,
                    SB_HORZ,
                    ed->zw->end * ed->zoom >> 16,
                    1
                );
            
            if((short)p_lp <  rc2.left)
                ed->scroll = Handlescroll
                (
                    p_win,
                    SB_THUMBPOSITION|(((short)p_lp + ed->scroll) << 16),
                    ed->scroll,
                    ed->page,
                    SB_HORZ,
                    ed->zw->end * ed->zoom>>16,
                    1
                );
        }
        
        break;
    
    case WM_PAINT:
        
        SampleDisplay_OnPaint(ed, p_win);
        
        break;
    
    default:
        
        return DefWindowProc(p_win, p_msg, p_wp, p_lp);
    }
    
    return 0;
}

// =============================================================================

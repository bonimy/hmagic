
    #include "structs.h"
    #include "prototypes.h"

    // For pen objects.
    #include "GdiObjects.h"

    // Access to other window procedures.
    #include "Callbacks.h"

    // For wrapper of GetClientRect()
    #include "Wrappers.h"

    // To interface to the sample edit dialog (which is the parent to this
    // control.
    #include "SampleEnum.h"

    // For copy / paste functionality.
    #include "SampleEditLogic.h"

// =============================================================================

    void
    Getsampsel(SAMPEDIT const * const ed,
               RECT           * const rc)
    {
        rc->left  = (ed->sell << 16) / ed->zoom     - ed->scroll;
        rc->right = (ed->selr << 16) / ed->zoom + 1 - ed->scroll;
    }

// =============================================================================

    static void
    SampleDisplay_OnKeydown
    (
        CP2(SAMPEDIT) p_ed,
        CP2C(MSG)     p_packed_msg)
    {
        CP2(ZWAVE) zw = p_ed->ew.doc->waves + p_ed->editsamp;
        
        HWND const win = p_packed_msg->hwnd;
        
        // -----------------------------
        
        switch(p_packed_msg->wParam)
        {
        
        case VK_DELETE:
            
            if(zw->copy != -1)
                break;
            
            if(p_ed->sell == p_ed->selr)
            {
                /*
                    \task This logic deletes the whole sample if there is
                    is not a selected region of width greater than zero. We
                    really ought to change this behavior, as there is no
                    undo feature currently, and this isn't even intuitive
                    behavior! Without a way to manually enter data, not to
                    mention confirm modifications like many other sorts of
                    editors in the program, this is a real problem.
                */
                
                zw->end = 0;
                
                zw->buf = (short*) realloc(zw->buf, 2);
                
                zw->buf[0]=0;
                zw->lopst=0;
                
                p_ed->sell=0;
            }
            else
            {
                int const sel_len = (p_ed->selr - p_ed->sell);
                
                // -----------------------------
                
                memcpy(zw->buf + p_ed->sell,
                       zw->buf + p_ed->selr,
                       (zw->end - p_ed->selr) );
                
                zw->end -= sel_len;
                
                zw->buf = (short*) realloc(zw->buf,
                                           (zw->end + 1) << 1);
                
                if(zw->lopst >= p_ed->selr)
                    zw->lopst -= (sel_len);
                
                zw->buf[zw->end] = zw->buf[zw->lopst];
            }
            
            p_ed->selr = p_ed->sell;
            
            p_ed->init = 1;
            
            SetDlgItemInt(p_ed->dlg, ID_Samp_SampleLengthEdit, zw->end, 0);
            SetDlgItemInt(p_ed->dlg, ID_Samp_LoopPointEdit, zw->lopst, 0);
            
            InvalidateRect(win, 0, 1);
            
            p_ed->init = 0;
            
            p_ed->ew.doc->m_modf = 1;
            p_ed->ew.doc->w_modf = 1;
            
            break;
        }
    }

// =============================================================================

    static void
    SampleDisplay_OnChar(SAMPEDIT * const p_ed,
                         MSG        const p_packed_msg)
    {
        HWND const win = p_packed_msg.hwnd;
        
        WPARAM const wp = p_packed_msg.wParam;
        
        // -----------------------------
        
        switch(wp)
        {
        
        // Ctrl-A
        // "Select All"
        case 1:
            
            p_ed->sell = 0;
            p_ed->selr = p_ed->zw->end;
            
            InvalidateRect(win, 0, 1);
            
            break;
        
        // Ctrl-C
        // "Copy Selected Region"
        case 3:
            
            SampleEdit_CopyToClipboard(p_ed);
            
            break;
        
        // Ctrl-V
        // "Paste"
        case 22:
            
            sampdlgproc(p_ed->dlg,
                        WM_COMMAND,
                        ID_Samp_PasteFromClipboardButton,
                        0);
            
            break;
        }
    }


// =============================================================================

    void
    SampleDisplay_OnPaint(SAMPEDIT * const p_ed,
                          HWND       const p_win)
    {
        int i = 0;
        int l = 0;
        
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
        return Is(p_edit_structure, NULL);
    
    case WM_GETDLGCODE:
    case WM_NCCREATE:
    case WM_NCDESTROY:
    case WM_CREATE:
    case WM_DESTROY:
        
        return FALSE;
    }
}

// =============================================================================

LRESULT CALLBACK
SampleDisplayProc(HWND p_win,UINT p_msg, WPARAM p_wp,LPARAM p_lp)
{
    SCROLLINFO si;
    
    int i;
    
    RECT rc,rc2;
    
    MSG const packed_msg = HM_PackMessage(p_win, p_msg, p_wp, p_lp);
    
    CP2(SAMPEDIT) ed = (SAMPEDIT*) GetWindowLongPtr(p_win, GWLP_USERDATA);
    
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
        
        SampleDisplay_OnKeydown(ed,
                                &packed_msg);

        
        break;
    
    case WM_CHAR:
        
        SampleDisplay_OnChar(ed,
                             packed_msg);
        
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

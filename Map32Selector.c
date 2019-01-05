
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "GdiObjects.h"

    #include "HMagicEnum.h"

    #include "OverworldEdit.h"

    #include "MetatileLogic.h"

// =============================================================================

BOOL CALLBACK
editblock32(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT*oe;
    BLOCKEDIT32*ed;
    BLOCKSEL16*bs;
    HWND hc,hc2;
    HDC hdc;
    
    RECT rc;
    FDOC*doc;
    int i,j;
    short*l;
    
    unsigned char * rom;
    
    switch(msg)
    {
    
    case WM_QUERYNEWPALETTE:
        
        ed = (BLOCKEDIT32*) GetWindowLongPtr(win,GWLP_USERDATA);
        
        SetPalette(win,ed->bs.ed->hpal);
        
        return 1;
    
    case WM_PALETTECHANGED:
        
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        
        break;
    
    case WM_INITDIALOG:
        ed = (BLOCKEDIT32*) malloc(sizeof(BLOCKEDIT32));
        bs=&(ed->bs);
        oe=bs->ed=(OVEREDIT*)lparam;
        Getblock32(oe->ew.doc->rom,oe->selblk,ed->blks);
        bs->scroll=0;
        bs->sel=0;
        hc=GetDlgItem(win,IDC_CUSTOM2);
        
        SetWindowLongPtr(win, GWLP_USERDATA, (LONG_PTR) ed);
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        Updatesize(hc);
        
        hc=GetDlgItem(win,IDC_CUSTOM1);
        
        rc = HM_GetClientRect(hc);
        
        ed->w=rc.right;
        ed->h=rc.bottom;
        hdc=GetDC(win);
        
        ed->bufdc=CreateCompatibleDC(hdc);
        ed->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        
        ReleaseDC(win,hdc);
        
        SelectObject(ed->bufdc,ed->bufbmp);
        SelectPalette(ed->bufdc,ed->bs.ed->hpal,1);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->bs.ed->hpal, 1);
            
            for(i = 0; i < 4; i++)
                Updateblk32disp(ed,i);
            
            SelectPalette(objdc, oldpal, 1);
        }
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        wparam = 0;
    
    // \task Is the fallthrough here intentional?
    
    case 4000:
        SetDlgItemInt(win,IDC_EDIT1,wparam,0);
        break;
    case WM_DESTROY:
        ed=(BLOCKEDIT32*)GetWindowLongPtr(win,GWLP_USERDATA);
        DeleteDC(ed->bufdc);
        DeleteObject(ed->bufbmp);
        free(ed);
        break;
    
    case 4001:
        
        ed = (BLOCKEDIT32*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->bs.ed->hpal, 1);
            
            for(i = 0; i < 4; i++)
                Updateblk32disp(ed, i);
            
            SelectPalette(objdc, oldpal, 1);
        }
        
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        
        break;
    
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT1|(EN_CHANGE<<16):
            bs=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
            SetBS16(bs,GetDlgItemInt(win,IDC_EDIT1,0,0),GetDlgItem(win,IDC_CUSTOM2));
            break;
        case IDOK:
            ed=(BLOCKEDIT32*)GetWindowLongPtr(win,GWLP_USERDATA);
            oe=ed->bs.ed;
            doc=oe->ew.doc;
            rom=doc->rom;
            i=oe->selblk;
            j=(i>>2)*6;
            l=ed->blks;
            switch(i&3) {
            case 0:
                rom[0x18000 + j] = (l[0] & 0xff);
                rom[0x18004 + j] = (rom[0x18004 + j] & 15) + ((l[0] >> 4) & 240);
                rom[0x1b400 + j] = (l[1] & 0xff);
                rom[0x1b404 + j] = (rom[0x1b404 + j]&15)+((l[1]>>4)&240);
                rom[0x20000 + j] = (l[2] & 0xff);
                rom[0x20004 + j] = (rom[0x20004 + j]&15)+((l[2]>>4)&240);
                rom[0x23400 + j] = (l[3] & 0xff);
                rom[0x23404 + j] = (rom[0x23404 + j]&15)+((l[3]>>4)&240);
                break;
            case 1:
                rom[0x18001 + j] = (l[0] & 0xff);
                rom[0x18004 + j] = (rom[0x18004 + j]&240)+(l[0]>>8);
                rom[0x1b401 + j] = (l[1] & 0xff);
                rom[0x1b404 + j] = (rom[0x1b404 + j]&240)+(l[1]>>8);
                rom[0x20001 + j] = (l[2] & 0xff);
                rom[0x20004 + j] = (rom[0x20004 + j]&240)+(l[2]>>8);
                rom[0x23401 + j] = (l[3] & 0xff);
                rom[0x23404 + j] = (rom[0x23404 + j]&240)+(l[3]>>8);
                break;
            case 2:
                rom[0x18002 + j] = (l[0] & 0xff);
                rom[0x18005 + j] = (rom[0x18005 + j]&15)+((l[0]>>4)&240);
                rom[0x1b402 + j] = (l[1] & 0xff);
                rom[0x1b405 + j] = (rom[0x1b405 + j]&15)+((l[1]>>4)&240);
                rom[0x20002 + j] = (l[2] & 0xff);
                rom[0x20005 + j] = (rom[0x20005 + j]&15)+((l[2]>>4)&240);
                rom[0x23402 + j] = (l[3] & 0xff);
                rom[0x23405 + j] = (rom[0x23405 +j]&15)+((l[3]>>4)&240);
                break; 
            case 3:
                rom[0x18003 + j] = (l[0] & 0xff);
                rom[0x18005 + j] = (rom[0x18005 + j]&240)+(l[0]>>8);
                rom[0x1b403 + j] = (l[1] & 0xff);
                rom[0x1b405 + j] = (rom[0x1b405 + j] & 240) + (l[1] >> 8);
                rom[0x20003 + j] = (l[2] & 0xff);
                rom[0x20005 + j] = (rom[0x20005 + j] & 240) + (l[2] >> 8);
                rom[0x23403 + j] = (l[3] & 0xff);
                rom[0x23405 + j] = (rom[0x23405 + j] & 240) + (l[3] >> 8);
                break;
            }
            
            for(i=0;i<160;i++)
            {
                hc2 = doc->overworld[i].win;
                
                if(hc2 != 0)
                {
                    hc2 = GetDlgItem(hc2, ID_SuperDlg);
                    
                    hc=GetDlgItem(hc2,3000);
                    InvalidateRect(hc,0,0);
                    hc=GetDlgItem(hc2,3001);
                    InvalidateRect(hc,0,0);
                }
            }
            
            doc->modf=1;
        
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

// =============================================================================

LRESULT CALLBACK
blksel32proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    int i,j,m;
    OVEREDIT*ed;
    RECT rc,rc2;
    HBRUSH oldbrush;
    HPALETTE oldpal;
    SCROLLINFO si;
    switch(msg)
    {
    
    case WM_SIZE:
        
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=lparam>>21;
        si.nMin=0;
        
        si.nMax = ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa;
        
        SetScrollInfo(win,SB_VERT,&si,1);
        
        ed->sel_page=si.nPage;
        
        ed->sel_scroll = Handlescroll(win,
                                      -1,
                                      ed->sel_scroll,
                                      ed->sel_page,
                                      SB_VERT,
                                      ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        
        break;
    
    case WM_MOUSEWHEEL:
        
        {
            HM_MouseWheelData const d = HM_GetMouseWheelData(wparam, lparam);
            
            unsigned scroll_type = SB_LINEUP;
            
            WPARAM fake_wp;
            
            ed = (OVEREDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            if(d.m_distance > 0)
            {
                // wheel moving up or left
                if(d.m_control_key)
                {
                    scroll_type = SB_PAGEUP;
                }
                else
                {
                    scroll_type = SB_LINEUP;
                }
            }
            else
            {
                if(d.m_control_key)
                {
                    scroll_type = SB_PAGEDOWN;
                }
                else
                {
                    scroll_type = SB_LINEDOWN;
                }
            }
            
            fake_wp =  MAKEWPARAM(scroll_type, 0);
            
            ed->sel_scroll = Handlescroll(win,
                                          fake_wp,
                                          ed->sel_scroll,
                                          ed->sel_page,
                                          SB_VERT,
                                          ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        }
        
        break;
    
    case WM_VSCROLL:
        
        ed = (OVEREDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        ed->sel_scroll = Handlescroll(win,
                                      wparam,
                                      ed->sel_scroll,
                                      ed->sel_page,
                                      SB_VERT,
                                      ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(OVEREDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        rc = HM_GetClientRect(win);
        i=(rc.right>>1)-64;
        j=lparam&65535;
        
        if(j < i || j >= rc.right - i)
            break;
        
        m = ( (ed->sel_scroll + (lparam >> 21) ) << 2) + ( (j - i) >> 5);
        
        if(m<0 || m>=(ed->schflag?ed->schnum:0x22a8)) break;
        if(ed->schflag) m=ed->schbuf[m];
        SetDlgItemInt(ed->dlg,3005,m,0);
        break;
    case WM_LBUTTONDBLCLK:
        ShowDialog(hinstance,(LPSTR)IDD_DIALOG7,framewnd,editblock32,GetWindowLongPtr(win,GWLP_USERDATA));
        break;
    case WM_PAINT:
        
        ed = (OVEREDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(!ed)
            break;
        
        hdc = BeginPaint(win, &ps);
        
        oldpal = SelectPalette(hdc, ed->hpal, 1);
        
        RealizePalette(hdc);
        rc = HM_GetClientRect(win);
        
        j = ((ps.rcPaint.bottom + 31) >> 5) << 2;
        
        oldbrush=SelectObject(hdc,white_pen);
        
        for(i = (ps.rcPaint.top >> 5) << 2; i < j; i++)
        {
            m = i + (ed->sel_scroll << 2);
            
            if(m >= (ed->schflag ? ed->schnum : 0x22a8))
                break;
            
            rc2.top = (i >> 2) << 5;
            rc2.left = (rc.right >> 1) - 64 + ((i & 3) << 5);
            
            if(rc2.top < ps.rcPaint.top - 31 || rc2.left < ps.rcPaint.left - 31 || rc2.left >= ps.rcPaint.right || rc2.top >= ps.rcPaint.bottom)
                continue;
            
            Drawblock32(ed,ed->schflag?ed->schbuf[m]:m,0);
            Paintblocks(&(ps.rcPaint),hdc,rc2.left,rc2.top,(DUNGEDIT*)ed);
            
            if(m == ed->sel_select)
            {
                rc2.bottom = rc2.top + 32;
                rc2.right = rc2.left+32;
                FrameRect(hdc, &rc2, green_brush);
            }
            else if(ed->disp & 2)
            {
                MoveToEx(hdc, rc2.left + 31, rc2.top, 0);
                LineTo(hdc, rc2.left + 31, rc2.top + 31);
                LineTo(hdc, rc2.left - 1, rc2.top + 31);
            }
        }
        
        SelectObject(hdc,oldbrush);
        SelectPalette(hdc,oldpal,1);
        
        EndPaint(win,&ps);
        
        break;
    
    default:
        
        return DefWindowProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================
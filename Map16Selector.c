
    #include "structs.h"

    #include "GdiObjects.h"

    #include "HMagicEnum.h"
    #include "HMagicUtility.h"

    #include "prototypes.h"

    #include "OverworldEnum.h"
    #include "OverworldEdit.h"

    #include "MetatileLogic.h"

// =============================================================================

BOOL CALLBACK editblock16(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT*oe;
    BLOCKEDIT16*ed;
    BLOCKSEL8*bs;
    BLOCKEDIT32*be;
    HWND hc,hc2;
    HDC hdc;
    RECT rc;
    FDOC*doc;
    int i,j;
    short*l;
    unsigned char*rom;
    
    switch(msg)
    {
    
    case WM_QUERYNEWPALETTE:
        
        ed = (BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        
        SetPalette(win, ed->bs.ed->hpal);
        
        return 1;
    
    case WM_PALETTECHANGED:
        
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        
        break;
    
    case WM_INITDIALOG:
        ed=malloc(sizeof(BLOCKEDIT16));
        bs=&(ed->bs);
        be=ed->be=(BLOCKEDIT32*)lparam;
        oe=bs->ed=be->bs.ed;
        doc=oe->ew.doc;
        rom=doc->rom;
        l=(short*)(rom + 0x78000 + (be->bs.sel<<3));
        *(int*)(ed->blks)=*(int*)l;
        *(int*)(ed->blks+2)=((int*)l)[1];
        
        SetWindowLongPtr(win,GWLP_USERDATA, (LONG_PTR) ed);
        
        hc=GetDlgItem(win,IDC_CUSTOM1);
        rc = HM_GetClientRect(hc);
        ed->w=rc.right;
        ed->h=rc.bottom;
        hdc=GetDC(win);
        ed->bufdc=CreateCompatibleDC(hdc);
        ed->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        SelectObject(ed->bufdc,ed->bufbmp);
        SelectPalette(ed->bufdc,oe->hpal,1);
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        hc=GetDlgItem(win,IDC_CUSTOM2);
        InitBlksel8(hc,bs,oe->hpal,hdc);
        ReleaseDC(win,hdc);
        
        for(i=0;i<4;i++)
            Updateblk16disp(ed, i);
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        Updatesize(hc);
        
        wparam=0;
    
    case 4000:
        bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
        bs->flags=wparam&0xfc00;
        CheckDlgButton(win,IDC_CHECK1,(wparam&16384)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK2,(wparam&32768)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK3,(wparam&8192)?BST_CHECKED:BST_UNCHECKED);
        SetDlgItemInt(win,IDC_EDIT2,wparam&1023,0);
        SetDlgItemInt(win,IDC_EDIT1,(wparam>>10)&7,0);
        break;
    case 4001:
        ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        for(i=0;i<4;i++) Updateblk16disp(ed,i);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        hc=GetParent(win);
        be=ed->be;
        for(i=0;i<4;i++) Updateblk32disp(be,i);
        InvalidateRect(GetDlgItem(hc,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(hc,IDC_CUSTOM2),0,0);
        break;
    case WM_DESTROY:
        ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
        DeleteDC(ed->bufdc);
        DeleteObject(ed->bufbmp);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        free(ed);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT2|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            i=GetDlgItemInt(win,IDC_EDIT2,0,0);
            if(i<0) i=0;
            if(i>0x3ff) i=0x3ff;
            j=bs->scroll<<4;
            if(i<j) j=i;
            if(i>=j+256) j=i-240;
            hc=GetDlgItem(win,IDC_CUSTOM2);
            SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
            Changeblk8sel(hc,bs);
            bs->sel=i;
            if(i<0x200) {
                SetDlgItemInt(win,IDC_EDIT3,bs->ed->ew.doc->rom[0x71459 + i],0);
                ShowWindow(GetDlgItem(win,IDC_EDIT3),SW_SHOW);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_SHOW);
            } else {
                ShowWindow(GetDlgItem(win,IDC_EDIT3),SW_HIDE);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            }
            Changeblk8sel(hc,bs);
            break;
        case IDC_EDIT1|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            bs->flags&=0xe000;
            bs->flags|=(GetDlgItemInt(win,IDC_EDIT1,0,0)&7)<<10;
            goto updflag;
            break;
        case IDC_EDIT3|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            if(bs->sel<0x200)
            bs->ed->ew.doc->rom[0x71459 + bs->sel]=GetDlgItemInt(win,IDC_EDIT3,0,0);
            break;
        case IDC_CHECK1:
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            bs->flags&=0xbc00;
            if(IsDlgButtonChecked(win,IDC_CHECK1)) bs->flags|=0x4000;
            goto updflag;
        case IDC_CHECK2:
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            bs->flags&=0x7c00;
            if(IsDlgButtonChecked(win,IDC_CHECK2)) bs->flags|=0x8000;
updflag:
            if((bs->flags&0xdc00)!=bs->oldflags)
            {
                HPALETTE const
                oldpal = SelectPalette(objdc, bs->ed->hpal, 1);
                
                HPALETTE const
                oldpal2 = SelectPalette(bs->bufdc, bs->ed->hpal, 1);
                
                bs->oldflags = bs->flags & 0xdc00;
                
                for(i = 0; i < 256; i++)
                    Updateblk8sel(bs, i);
                
                SelectPalette(objdc, oldpal, 1);
                SelectPalette(bs->bufdc, oldpal2, 1);
                
                InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
            }
            
            break;
        case IDC_CHECK3:
            bs=(BLOCKSEL8*)GetWindowLongPtr(win,GWLP_USERDATA);
            bs->flags&=0xdc00;
            if(IsDlgButtonChecked(win,IDC_CHECK3)) bs->flags|=0x2000;
            break;
        case IDOK:
            ed=(BLOCKEDIT16*)GetWindowLongPtr(win,GWLP_USERDATA);
            oe=ed->bs.ed;
            doc=oe->ew.doc;
            rom=doc->rom;
            i=oe->selblk;
            l=(short*)(rom + 0x78000 + (ed->be->bs.sel<<3));
            *(int*)l=*(int*)(ed->blks);
            ((int*)l)[1]=*(int*)(ed->blks+2);
            
            for(i=0;i<160;i++)
            {
                hc2 = doc->overworld[i].win;
                
                if(hc2 != 0)
                {
                    hc2 = GetDlgItem(hc2, ID_SuperDlg);
                    
                    hc=GetDlgItem(hc2, SD_Over_Map32_Selector);
                    InvalidateRect(hc,0,0);
                    
                    hc=GetDlgItem(hc2, SD_Over_Display);
                    InvalidateRect(hc,0,0);
                }
            }
            
            doc->modf=1;
            
            EndDialog(win,1);
            
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

// =============================================================================

void
Blksel16_OnPaint(BLOCKSEL16 const * const p_ed,
                 HWND               const p_win)
{
    unsigned char const * const rom = p_ed->ed->ew.doc->rom;
    
    int i = 0,
        j = 0;
    
    RECT rc;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->ed->hpal, 1);
    
    HGDIOBJ const oldbrush = SelectObject(hdc, white_pen);
    
    RealizePalette(hdc);
    
    rc = HM_GetClientRect(p_win);
    
    j = ( (ps.rcPaint.bottom + 31) >> 5) << 2;
    
    for(i = (ps.rcPaint.top >> 5) << 2; i < j; i++)
    {
        int const m = i + (p_ed->scroll << 2);
        
        int l = 0,
            n = 0,
            p = 0;
        
        RECT rc2;
        
        if(m > 0xea7)
            break;
        
        n = ((m & 3) << 1) + ((m & 0xffc) << 2);
        
        for(l = 0; l < 4; l++)
        {
            int k = 0;
            
            unsigned char const * const o =
            (
                rom + 0x78000
              + ( ( n + (l & 1) + ( (l & 2) << 2 ) ) << 3 )
            );
            
            for(k = 0; k < 4; k++)
            {
                Drawblock(p_ed->ed,
                          blkx[p],
                          blky[p],
                          ldle16b_i(o, k),
                          0);
                
                p++;
            }
        }
        
        if(m >= 0xea0)
            for(l = 512; l < 1024; l++)
                drawbuf[l] = 0;
        
        rc2.top = (i >> 2) << 5;
        rc2.left=(rc.right >> 1) - 64 + ((i & 3) << 5);
        
        Paintblocks(&(ps.rcPaint),
                    hdc,
                    rc2.left,
                    rc2.top,
                    (DUNGEDIT*) (p_ed->ed));
        // MoveToEx(hdc,rc2.left+31,rc2.top,0);
        // LineTo(hdc,rc2.left+31,rc2.top+31);
        // LineTo(hdc,rc2.left-1,rc2.top+31);
        // MoveToEx(hdc,rc2.left+15,rc2.top,0);
        // LineTo(hdc,rc2.left+15,rc2.top+31);
        // MoveToEx(hdc,rc2.left,rc2.top+15,0);
        // LineTo(hdc,rc2.left+31,rc2.top+15);
        
        if(m == ( (p_ed->sel & 7) >> 1) + ( (p_ed->sel & 0xfff0) >> 2 ) )
        {
            rc2.left += (p_ed->sel & 1) << 4;
            rc2.top += (p_ed->sel & 8) << 1;
            rc2.bottom = rc2.top + 16;
            rc2.right = rc2.left + 16;
            
            FrameRect(hdc,
                      &rc2,
                      green_brush);
        }
    }
    
    SelectObject(hdc, oldbrush);
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
blksel16proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLOCKSEL16* ed;
    SCROLLINFO si;
    RECT rc;
    int i,j;
    switch(msg) {
    case WM_SIZE:
        ed=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=lparam>>21;
        si.nMin=0;
        si.nMax=0xea;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->page=si.nPage;
        break;
    
    case WM_MOUSEWHEEL:
        
        ed=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
        i=ed->scroll;
        
        {
#if 0
            signed distance = HM_GetSignedHiword(wparam);
            
            unsigned flags = LOWORD(wparam);
            
            unsigned x_coord = LOWORD(lparam);
            unsigned y_coord = LOWORD(lparam);
            
            unsigned const is_horiz = (flags & MK_CONTROL);
            
            unsigned which_sb = (is_horiz) ? SB_HORZ : SB_VERT;
            
            int const which_scroll = (is_horiz) ? ed->mapscrollh : ed->mapscrollv;
            int const which_page   = (is_horiz) ? ed->mappageh : ed->mappagev;
            
            int * const which_return = (is_horiz) ? &ed->mapscrollh : &ed->mapscrollv;
            
            (*which_return) = Handlescroll(win,
                                           (distance > 0) ? 0 : 1,
                                           which_scroll,
                                           which_page,
                                           which_sb,
                                           ed->mapsize ? 32 : 16, 32);
#endif
        }
        
        break;
    
    case WM_VSCROLL:
        ed=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
        i=ed->scroll;
        switch(wparam&65535) {
        case SB_BOTTOM:
            i=0x200 - ed->page;
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
            i+=ed->page;
            break;
        case SB_PAGEUP:
            i-=ed->page;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        }
        if(i<0) i=0;
        if(i>0xea - ed->page) i = 0xea - ed->page;
        if(i==ed->scroll) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        si.nPos=i;
        SetScrollInfo(win,SB_VERT,&si,1);
        ScrollWindowEx(win,0,(ed->scroll-i)<<5,0,0,0,0,SW_INVALIDATE|SW_ERASE);
        ed->scroll=i;
        break;
    case WM_LBUTTONDOWN:
        ed=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
        rc = HM_GetClientRect(win);
        
        i = (rc.right >> 1) - 64;
        
        j = lparam & 65535;
        
        if(j < i || j >= rc.right - i)
            break;
        
        SendMessage(GetParent(win),
                    4000,
                    ( (ed->scroll << 4) + ( (lparam >> 20) << 3) )
                  + ( (j - i) >> 4),
                    0);
        
        break;
    
    case WM_LBUTTONDBLCLK:
        ed=(BLOCKSEL16*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(ShowDialog(hinstance,(LPSTR)IDD_DIALOG8,win,editblock16,GetWindowLongPtr(win,GWLP_USERDATA)))
            SendMessage(GetParent(win),4001,0,0);
        break;
    
    case WM_PAINT:
        
        ed = (BLOCKSEL16*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(ed)
        {
            Blksel16_OnPaint(ed, win);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

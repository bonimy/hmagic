
    #include "structs.h"

    #include "GdiObjects.h"
    #include "wrappers.h"
    
    #include "prototypes.h"

    #include "DungeonEnum.h"
    #include "DungeonLogic.h"

// =============================================================================

unsigned short dp_tab[]={
    68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,
    67,67,67,67,52,52,52,52,34,34,34,34,68,50,34,34,
    34,50,0x1054,52,68,68,50,34,0x1054,68,0x1024,50,54,68,68,68,
    68,68,68,68,34,0x1024,0x1024,74,52,52,52,52,0xf000,51,54,113,
    34,66,66,66,66,66,66,34,34,81,81,81,81,81,81,81,
    81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,
    81,51,19,19,19,19,19,19,19,19,19,19,19,19,19,22,
    22,0,0,68,17,0,68,68,50,66,52,52,34,66,34,19,
    19,19,19,19,19,19,19,83,51,0x1024,0x1024,34,51,68,68,68,
    17,54,54,34,0,0x1024,0x1024,0,0,0,0,54,54,53,34,19,
    34,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,34,34,49,17,17,97,97,0,0,
    68,17,0,68,68,66,67,67,34,17,17,34,17,34,0,66,
    66,67,67,67,67,0x1063,0x1063,66,0xf04e,34,49,17,17,17,17,17,
    0x1024,0x1024,34,34,68,34,34,0,0,0,0,0,0,0,0,0,
    17,17,17,17,0xf003,17,17,17,17,17,17,17,17,17,17,17,
    17,17,68,19,19,66,66,66,34,34,68,34,34,34,0,0,
    17,0xf008,17,49,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0,0,0xf010,0xf017,0,
    0,0x1024,0x1024,0,0,0,0x1024,17,0x1024,0x1024,0x1024,0xf01e,0x1035,0x1044,34,0x1024,
    0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0x1034,0x1054,0x1074,17,17,17,17,17,17,17,17,17,17,0xf02f,17,17,
    34,34,33,34,52,0x108a,34,0xf02f,34,34,34,68,68,68,68,68,
    68,68,34,34,34,34,68,68,68,68,0xf032,34,0xf052,0xf05a,0xf03b,34,
    34,52,52,68,35,35,54,54,35,35,0x1064,0x1064,70,70,34,34,
    34,34,34,34,34,34,34,68,68,34,34,56,0x1086,54,52,34,
    34,34,34,34,0xf043,0x1033,34,34,34,34,0x1024,0x1053,70,54,34,34,
    0xf057,0xf057,0xf048,34,34,34,68,52,52,67,67,68,52,52,67,67,
    72,0x2110,0x108a,0xf04b,0x2110,34,56,56,72,52,68,0x1024,34,34,34,0
};

unsigned short dp_x[]={
    2,98,52,
    3,17,0x863c,0x1023,18,
    6,56,0x1023,34,0x1023,56,0x8590,34,
    4,0x83d8,49,0x872a,51,0x1023,51,
    4,0x875a,51,0x1023,51,0x83d8,49,
    16,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,
    0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,
    1,0x9488,97,
    4,0x96dc,0x10cc,0x97f6,0x10cc,0x9914,0x10cc,0x9a2a,0x10cc,
    6,0x9bf2,70,81,0x1064,0x1026,0x1062,85,
    4,18,0x1023,18,68,
    2,0x22d6,0x1023,
    1,0x80e0,0x1024,
    3,34,18,50,
    4,34,34,34,34,
    2,51,51,
    1,0x9b4a,230
};

// =============================================================================

void
Getdungselrect(int                const i,
               RECT             * const rc,
               CHOOSEDUNG const * const ed)
{
    rc->left   = (i & 3) * ed->w >> 2;
    rc->top    = (i >> 2) * ed->h >> 2;
    rc->right  = rc->left + (ed->w >> 2);
    rc->bottom = rc->top + (ed->h >> 2);
}

// =============================================================================

void
Updateobjdisplay(CHOOSEDUNG const * const ed,
                 int                const num)
{
    char text_buf[0x200];
    
    int i = num + (ed->scroll << 2);
    
    int j,k,l,m,n,o,p;
    
    uint16_t objbuf[0x2000];
    
    unsigned char obj[6];
    
    RECT rc;
    
    // Fills tilemap with transparent tiles.
    // \note This assumes that this particular constant is always a blank
    // tile. Whether someone could screw that up via editing with HM itself,
    // I don't know.
    for(j = 0; j < 0x2000; j++)
    {
        objbuf[j] = 0x01e9;
    }
    
    // Create a fake object stream that contains just one object.
    if(i >= 0x260)
    {
        return;
    }
    else if(i >= 0x1b8)
    {
        // Fake a directive that switches to loading doors.
        obj[0] = 0xf0;
        obj[1] = 0xff;
        
        // Set door type
        obj[2] = (i - 0x1b8) / 42;
        obj[3] = ((i - 0x1b8) % 42) << 1;
        
        // Terminate the simulated object stream.
        obj[4] = 0xff;
        obj[5] = 0xff;
        
        getdoor(obj + 2, ed->ed->ew.doc->rom);
        
        rc.left = (dm_x & 0x3f) << 3;
        rc.top  = (dm_x & 0xfc0) >> 3;
    }
    else
    {
        if(i < 0x40)
        {
            obj[0] = 0xfc;
            obj[1] = 0;
            obj[2] = i;
        }
        else if(i < 0x138)
        {
            obj[0] = 0;
            obj[1] = 0;
            
            switch( obj3_t[i - 0x40] & 15 )
            {
            case 0: case 2:
                
                obj[1] = 1;
                
                break;  
            }
            
            obj[2] = i - 0x40;
        }
        else
        {
            obj[0] = i & 3;
            obj[1] = ( (i - 0x138) >> 2) & 3;
            obj[2] = 0xf8 + ( ( (i - 0x138) >> 4) & 7);
        }
        
        // Terminate the object stream.
        obj[3] = 255;
        obj[4] = 255;
        
        getobj(obj);
        
        rc.left = 0;
        rc.top  = 0;
    }
    
    Getdungobjsize(ed->ed->selchk, &rc, 0, 0, 1);
    
    if(i < 0x1b8)
    {
        if(rc.top < 0)
            j=-rc.top >> 3;
        else j = 0;
        
        if(rc.left < 0)
            k = -rc.left >> 3;
        else
            k = 0;
        
        if(i < 0x40)
        {
            obj[0] |= k >> 4;
            obj[1]  = (k << 4) | (j >> 2);
            obj[2] |= k << 6;
        }
        else
        {
            obj[0] |= k << 2;
            obj[1] |= j << 2;
        }
        
        rc.right  += k << 3;
        rc.bottom += j << 3;
        rc.left   += k << 3;
        rc.top    += j << 3;
    }
    
    dm_buf = objbuf;
    
    Drawmap(ed->ed->ew.doc->rom,objbuf,obj,ed->ed);
    
    {
        BITMAPINFOHEADER backup_bmih = ed->ed->bmih;
        
        ed->ed->bmih.biWidth  = 0x20;
        ed->ed->bmih.biHeight = 0x20;
        
        Paintdungeon(ed->ed,
                     objdc,
                     &rc,
                     rc.left,
                     rc.top,
                     rc.right,
                     rc.bottom,
                     0,
                     0,
                     objbuf);
        
        ed->ed->bmih = backup_bmih;
    }
    
    rc.right  -= rc.left;
    rc.bottom -= rc.top;
    n = rc.right;
    
    if(rc.bottom > n)
        n = rc.bottom;
    
    n++;
    
    j = ed->w >> 2;
    k = ed->h >> 2;
    l = (rc.right) * j / n;
    m = (rc.bottom) * k / n;
    o = ((((i&3)<<1)+1)*ed->w>>3);
    p = (((((i>>2)&3)<<1)+1)*ed->h>>3);
    
    SetStretchBltMode(ed->bufdc, HALFTONE);
    
    StretchBlt(ed->bufdc,
               o - (l >> 1),
               p - (m >> 1),
               l,
               m,
               objdc,
               rc.left,
               rc.top,
               rc.right,
               rc.bottom,
               SRCCOPY);
    
    if(i < 0x40)
        l = i + 0x100;
    else if(i < 0x138)
        l = i - 0x40;
    else if(i < 0x1b8)
        l = i + 0xe48;
    else
        l = i - 0x1b8;
    
    wsprintf(text_buf, "%03X", l);
    
    SetTextColor(ed->bufdc, 0);
    
    TextOut(ed->bufdc, o - 1, p - 1, text_buf, 3);
    TextOut(ed->bufdc, o + 1, p - 1, text_buf, 3);
    TextOut(ed->bufdc, o - 1, p + 1, text_buf, 3);
    TextOut(ed->bufdc, o + 1, p + 1, text_buf, 3);
    
    SetTextColor(ed->bufdc, 0xefefef);
    
    TextOut(ed->bufdc, o, p, text_buf, 3);
}

// =============================================================================

LRESULT CALLBACK
dpceditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    unsigned short*rdbuf;
    int i,j,k,l,m,n,o,p,q,r;
    unsigned short*b;
    RECT rc;
    switch(msg) {
    case WM_LBUTTONUP:
        if(dpce->flag&1) {
            dpce->flag&=-2;
            ReleaseCapture();
        }
        break;
    case WM_LBUTTONDOWN:
        SetCapture(win);
        dpce->flag|=1;
    case WM_MOUSEMOVE:
        if(dpce->flag&1) {
    case WM_RBUTTONDOWN:
            k=dpce->sel;
            l=dpce->dp_w[k];
            m=dpce->dp_h[k];
            rc.left=(dpce->w>>1)-(l<<2);
            rc.top=(dpce->h>>1)-((m&127)<<2);
            i=((short)lparam-rc.left)>>3;
            j=((lparam>>16)-rc.top)>>3;
            if(i<0 || j<0 || i>=l || j>=(m&127)) break;
            b=(unsigned short*)(dpce->buf+dpce->dp_st[k]);
            if(m&128)
                b+=i+j*l;
            else b+=j+i*m;
            if(msg==WM_RBUTTONDOWN) {SendMessage(GetParent(win),4000,*b,0);break;}
            *b=dpce->bs.sel|dpce->bs.flags;
            rc.left+=i<<3;
            rc.top+=j<<3;
            rc.right=rc.left+8;
            rc.bottom=rc.top+8;
            InvalidateRect(win,&rc,0);
        }
        break;
    case WM_PAINT:
        hdc=BeginPaint(win,&ps);
        i=dpce->sel;
        j=dpce->dp_w[i];
        k=dpce->dp_h[i];
        l=dpce->dp_st[i];
        r=k;
        k&=127;
        rdbuf=(unsigned short*)(dpce->buf+l);
        l=0;
        rc.left=(dpce->w>>1)-(j<<2);
        rc.right=rc.left+(j<<3);
        rc.top=(dpce->h>>1)-(k<<2);
        rc.bottom=rc.right+(k<<3);
        
        if(r & 128)
        {
            BITMAPINFOHEADER backup = dunged->bmih;
            
            dunged->bmih.biWidth  = 32;
            dunged->bmih.biHeight = 32;
            
            for(n = 0; n < k; n += 4)
            {
                for(m = 0; m < j; m += 4)
                {
                    o=4;
                    p=4;
                    
                    if(m > j - 4)
                        o = j & 3;
                    
                    if(n > k - 4)
                        p = k & 3;
                    
                    for(q=0;q<p;q++)
                    {
                        for(i=0;i<o;i++)
                        {
                            Drawblock((OVEREDIT*)dunged,i<<3,q<<3,rdbuf[l+i],0);
                        }
                        
                        l+=j;
                    }
                    
                    l -= (j * p - o);
                    
                    o <<= 3;
                    p <<= 3;
                    
                    SetDIBitsToDevice(hdc,
                                      rc.left + (m << 3),
                                      rc.top + (n << 3),
                                      o,
                                      p,
                                      0,
                                      0,
                                      0,
                                      32,
                                      drawbuf + ( (32 - p) << 5),
                                      (BITMAPINFO*) &(dunged->bmih),
                                      dunged->hpal ? DIB_PAL_COLORS : DIB_RGB_COLORS);
                }
                
                l += 3 * j;
            }
            
            dunged->bmih = backup;
        }
        else
        {
            BITMAPINFOHEADER backup = dunged->bmih;
            
            dunged->bmih.biWidth  = 32;
            dunged->bmih.biHeight = 32;
            
            for(m=0;m<j;m+=4)
            {
                for(n=0;n<k;n+=4)
                {
                    o=4;
                    p=4;
                    
                    if(m>j-4) o=j&3;
                    if(n>k-4) p=k&3;
                    
                    for(i=0;i<o;i++)
                    {
                        for(q=0;q<p;q++)
                        {
                            Drawblock((OVEREDIT*)dunged,i<<3,q<<3,rdbuf[l+q],0);
                        }
                        
                        l += k;
                    }
                    
                    l -= k * o - p;
                    
                    o <<= 3;
                    p <<= 3;
                    
                    SetDIBitsToDevice(hdc,
                                      rc.left + (m << 3),
                                      rc.top + (n << 3),
                                      o,
                                      p,
                                      0,
                                      0,
                                      0,
                                      32,
                                      drawbuf + ( (32 - p) << 5),
                                      (BITMAPINFO*) &(dunged->bmih),
                                      dunged->hpal ? DIB_PAL_COLORS : DIB_RGB_COLORS);
                }
                
                l += (3 * k);
            }
            
            dunged->bmih = backup;
        }
        
        EndPaint(win,&ps);
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

int loadpiece(int k,int j,int m)
{
    int n,o;
    if(j<0x2000) {
        n=dpce->dp_w[k]=j&15;
        n*=o=(j>>4)&15;
        o|=(j&0x1000)>>5;
        dpce->dp_h[k]=o;
        dpce->dp_st[k]=m;
        m+=n<<1;
    } else if(j<0x4000) {
        n=dpce->dp_w[k]=j&63;
        n*=o=(j>>6)&63;
        o|=(j&0x1000)>>5;
        dpce->dp_h[k]=o;
        dpce->dp_st[k]=m;
        m+=n<<1;        
    }
    return m;
}




void Upddpcbuttons(HWND win)
{
    EnableWindow(GetDlgItem(win,IDC_BUTTON1),dpce->sel);
    EnableWindow(GetDlgItem(win,IDC_BUTTON8),dpce->sel<dpce->num-1);
    if(!IsWindowEnabled(GetFocus())) SetFocus(GetDlgItem(win,IDOK));
}

int GetBTOfs(DPCEDIT*ed)
{
    if(ed->bs.sel<0x140)
        return ed->bs.sel + 0x71659;
    else if(ed->bs.sel>=0x1c0)
        return ed->bs.sel + 0x715d9;
    else
        return ed->bs.sel + 0x70eea
             + ((short*)(ed->bs.ed->ew.doc->rom + 0x71000))[((DUNGEDIT*)(ed->bs.ed))->gfxnum];
}

// "Dungeon piece" editor. Guess this edits dungeon objects.
BOOL CALLBACK
dpcdlgproc(HWND win,
           UINT msg,
           WPARAM wparam,
           LPARAM lparam)
{
    HDC hdc;
    
    int i = 0,
        j = 0,
        k = 0,
        l = 0,
        m = 0;
    
    uint16_t const * b;
    
    HPALETTE oldpal;
    RECT rc = empty_rect;
    HWND hc;
    BLOCKSEL8*bs;
    
    switch(msg)
    {
    
    case WM_QUERYNEWPALETTE:
        
        SetPalette(win,dunged->hpal);
        
        return 1;
    
    case WM_PALETTECHANGED:
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        break;
    case WM_INITDIALOG:
        dpce=malloc(sizeof(DPCEDIT));
        dpce->sel=0;
        dpce->flag=0;
        dpce->bs.ed=(OVEREDIT*)dunged;
        SendDlgItemMessage(win,IDC_BUTTON1,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[0]);
        SendDlgItemMessage(win,IDC_BUTTON8,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[1]);
        memcpy(dpce->buf,dunged->ew.doc->rom + 0x1b52,0x342e);
        
        hdc = GetDC(win);
        
        InitBlksel8(GetDlgItem(win,IDC_CUSTOM1),&(dpce->bs),dunged->hpal,hdc);
        
        ReleaseDC(win, hdc);
        
        rc = HM_GetDlgItemRect(win, IDC_CUSTOM2);
        
        dpce->w=rc.right;
        dpce->h=rc.bottom;
        dpce->obj=i=lparam;
        j=dp_tab[i];
        if(i<0x40)
        m=((unsigned short*)(dunged->ew.doc->rom + 0x83f0))[i];
        else if(i<0x138) m=((unsigned short*)(dunged->ew.doc->rom + 0x7f80))[i];
        else if(i<0x1b8) m=((unsigned short*)(dunged->ew.doc->rom + 0x8280))[i];
        if((j&0xf000)==0xf000) {
            b=dp_x + (j & 4095);
            l=dpce->num=*(b++);
            for(k=0;k<l;k++) {
again:
                j=*(b++);
                if(j&32768) { m=j&32767; goto again; }
                m=loadpiece(k,j,m);
            }
        } else {
            dpce->num=1;
            m=loadpiece(0,j,m);
        }
        Upddpcbuttons(win);
        wparam=0;
    case 4000:
        bs=&(dpce->bs);
        bs->flags=wparam&0xfc00;
        CheckDlgButton(win,IDC_CHECK1,(wparam&16384)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK2,(wparam&32768)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK3,(wparam&8192)?BST_CHECKED:BST_UNCHECKED);
        SetDlgItemInt(win,IDC_EDIT2,wparam&1023,0);
        SetDlgItemInt(win,IDC_EDIT1,(wparam>>10)&7,0);
        break;
    case WM_DESTROY:
        DeleteDC(dpce->bs.bufdc);
        DeleteObject(dpce->bs.bufbmp);
        free(dpce);
        break;
    case WM_COMMAND:
        switch(wparam)
        {
        case IDC_EDIT2|(EN_CHANGE<<16):
            bs=&(dpce->bs);
            i=GetDlgItemInt(win,IDC_EDIT2,0,0);
            if(i<0) i=0;
            if(i>0x3ff) i=0x3ff;
            j=bs->scroll<<4;
            if(i<j) j=i;
            if(i>=j+256) j=i-240;
            hc=GetDlgItem(win,IDC_CUSTOM1);
            SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
            Changeblk8sel(hc,bs);
            bs->sel=i;
            if(i<0x200) {
                SetDlgItemInt(win,IDC_EDIT8,dunged->ew.doc->rom[GetBTOfs(dpce)],0);
                ShowWindow(GetDlgItem(win,IDC_EDIT8),SW_SHOW);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_SHOW);
            } else {
                ShowWindow(GetDlgItem(win,IDC_EDIT8),SW_HIDE);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            }
            Changeblk8sel(hc,bs);
            break;

        case IDC_EDIT1|(EN_CHANGE<<16):
            bs=&(dpce->bs);
            bs->flags&=0xe000;
            bs->flags|=(GetDlgItemInt(win,IDC_EDIT1,0,0)&7)<<10;
            goto updflag;
            break;

        case IDC_EDIT3|(EN_CHANGE<<16):
            bs=&(dpce->bs);
            if(bs->sel<0x200)
            bs->ed->ew.doc->rom[GetBTOfs(dpce)]=GetDlgItemInt(win,IDC_EDIT3,0,0);
            break;
        case IDC_CHECK1:
            bs=&(dpce->bs);
            bs->flags&=0xbc00;
            if(IsDlgButtonChecked(win,IDC_CHECK1)) bs->flags|=0x4000;
            goto updflag;
        case IDC_CHECK2:
            bs=&(dpce->bs);
            bs->flags&=0x7c00;
            if(IsDlgButtonChecked(win,IDC_CHECK2)) bs->flags|=0x8000;
updflag:
            if((bs->flags&0xdc00)!=bs->oldflags) {
                bs->oldflags=bs->flags&0xdc00;
                oldpal=SelectPalette(objdc,dunged->hpal,1);
                SelectPalette(bs->bufdc,dunged->hpal,1);
                for(i=0;i<256;i++) Updateblk8sel(bs,i);
                SelectPalette(objdc,oldpal,1);
                SelectPalette(bs->bufdc,oldpal,1);
                InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
            }
            break;
        case IDC_CHECK3:
            bs=&(dpce->bs);
            bs->flags&=0xdc00;
            if(IsDlgButtonChecked(win,IDC_CHECK3)) bs->flags|=0x2000;
            break;
        case IDOK:
            memcpy(dunged->ew.doc->rom + 0x1b52,dpce->buf,0x342e);
            dunged->ew.doc->modf=1;
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        case IDC_BUTTON1:
            dpce->sel--;
            InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,1);
            Upddpcbuttons(win);
            break;
        case IDC_BUTTON8:
            dpce->sel++;
            InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,1);
            Upddpcbuttons(win);
        }
    }
    return 0;
}

// =============================================================================

    void
    DungeonObjSelector_OnPaint(CHOOSEDUNG const * const ed,
                               HWND               const win)
    {
        int j = 0;
        int k = (ed->scroll & 3) * ed->h >> 2;
        int i = ed->h - k;
        
        PAINTSTRUCT ps;
        
        HDC const hdc = BeginPaint(win, &ps);
        
        HPALETTE const oldpal = SelectPalette(hdc, ed->ed->hpal, 1);
        
        // -----------------------------
        
        RealizePalette(hdc);
        
        if(i > ps.rcPaint.bottom)
            j = ps.rcPaint.bottom;
        else
            j = i;
        
        if(j > ps.rcPaint.top)
        {
            BitBlt(hdc,
                   ps.rcPaint.left,
                   ps.rcPaint.top,
                   ps.rcPaint.right - ps.rcPaint.left,
                   j - ps.rcPaint.top,
                   ed->bufdc,
                   ps.rcPaint.left,
                   ps.rcPaint.top + k,
                   SRCCOPY);
        }
        
        if(i < ps.rcPaint.top)
            j = ps.rcPaint.top;
        else
            j = i;
        
        if(j < ps.rcPaint.bottom)
        {
            BitBlt(hdc,
                   ps.rcPaint.left,j,
                   ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - j,
                   ed->bufdc,
                   ps.rcPaint.left,
                   j + k - ed->h,
                   SRCCOPY);
        }
        
        i = ed->sel - (ed->scroll << 2);
        
        if(i >= 0 && i < 16)
        {
            RECT rc;
            
            Getdungselrect(i, &rc, ed);
            FrameRect(hdc, &rc, green_brush);
        }
        
        SelectPalette(hdc, oldpal, 0);
        
        EndPaint(win, &ps);
    }

// =============================================================================

/// Dungeon object selector dialog's object viewport.
/// Window class is "DUNGSEL".
LRESULT CALLBACK
dungselproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    CHOOSEDUNG *ed;
    HPALETTE oldpal;
    SCROLLINFO si;
    int i,j,k,l,m,n;
    RECT rc;
    
    switch(msg)
    {
    
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    
    case WM_KEYDOWN:
        if(wparam>=65 && wparam<71) { wparam-=55; goto digscr; }
        if(wparam>=48 && wparam<58) {
            wparam-=48;
digscr:
            ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
            ed->typednum=((ed->typednum<<4)|wparam)&0xfff;
            i=ed->typednum;
            if(!(ed->ed->selchk&1))
                if(i>=0xf80) i-=0xe48;
                else if(i>=0x140) break;
                else if(i>=0x100) i-=0x100;
                else if(i>=0xf8) break;
                else i+=0x40;
                else i+=0x1b8;
            i>>=2;
            j=ed->scroll;
            goto scroll;
        }
        break;
    case WM_VSCROLL:
        ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
        j=i=ed->scroll;
        switch(wparam&65535) {
        case SB_LINEDOWN:
            i++;
            break;
        case SB_LINEUP:
            i--;
            break;
        case SB_PAGEDOWN:
            i+=4;
            break;
        case SB_PAGEUP:
            i-=4;
            break;
        case SB_TOP:
            i=0;
            break;
        case SB_BOTTOM:
            i=0x6d;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            i=wparam>>16;
        }
scroll:
        if(ed->ed->selchk&1) {
            if(i<0x6e) i=0x6e;
            if(i>0x94) i=0x94;
        } else {
            if(i<0) i=0;
            if(i>0x6a) i=0x6a;
        }
        if(i==j) break;
        ed->scroll=i;
        if(j<i-4) j=i-4;
        if(j>i+4) j=i+4;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        si.nPos=i;
        SetScrollInfo(win,SB_VERT,&si,1);
        ScrollWindowEx(win,0,(j-i)*ed->h>>2,0,0,0,0,SW_INVALIDATE);
        i<<=2;
        j<<=2;
        if(j<i) j+=16,k=i+16; else k=j,j=i;
        oldpal=SelectPalette(objdc,ed->ed->hpal,0);
        for(;j<k;j++) {
            n=j-i;
            m=(j&3)*ed->w>>2;
            l=((j>>2)&3)*ed->h>>2;
            Rectangle(ed->bufdc,m,l,m+(ed->w>>2),l+(ed->h>>2));
            Updateobjdisplay(ed,n);
        }
        SelectPalette(objdc,oldpal,1);
        break;
    
    case WM_PAINT:
        
        ed = (CHOOSEDUNG*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            DungeonObjSelector_OnPaint(ed, win);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        
        ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
        SetFocus(win);
        i=ed->sel-(ed->scroll<<2);
        if(i>=0 && i<16) {
            Getdungselrect(i,&rc,ed);
            InvalidateRect(win,&rc,0);
        }
        i=((lparam&65535)<<2)/ed->w;
        i|=(((lparam>>16)<<2)/ed->h)<<2;
        ed->sel=i+(ed->scroll<<2);
        if(i>=0 && i<16) {
            Getdungselrect(i,&rc,ed);
            InvalidateRect(win,&rc,0);
        }
        EnableWindow(GetDlgItem(ed->dlg,IDC_BUTTON1),dp_tab[ed->sel]);
        break;
    case WM_LBUTTONDBLCLK:
        SendMessage(GetParent(win),WM_COMMAND,IDOK,0);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

    /// Procedure for the Dungeon object selector dialog box.
    BOOL CALLBACK
    choosedung(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
    {
        HWND hc;
        HDC hdc;
        CHOOSEDUNG *ed;
        DUNGEDIT *de;
        RECT rc;
        SCROLLINFO si;
        HPALETTE oldpal;
        HWND *p;
        
        int i;
        
        // -----------------------------
        
        switch(msg)
        {
        
        case WM_QUERYNEWPALETTE:
            
            ed = (CHOOSEDUNG*) GetWindowLong(win, DWL_USER);
            
            SetPalette(win, ed->ed->hpal);
            
            return 1;
        
        case WM_PALETTECHANGED:
            
            InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
            
            break;
        
        case WM_INITDIALOG:
            
            ed = (CHOOSEDUNG*) calloc(1, sizeof(CHOOSEDUNG) );
            
            ed->dlg=win;
            
            de = ed->ed = (DUNGEDIT*) lparam;
            
            if(de->selchk & 1)
            {
                getdoor(de->buf + de->selobj, de->ew.doc->rom);
                
                i = dm_k * 42 + 0x1b8 + dm_l;
            }
            else
            {
                getobj(de->buf+de->selobj);
                
                if(dm_k<0xf8)
                    i = dm_k + 0x40;
                else if(dm_k<0x100)
                    i = ( (dm_k - 0xf8) << 4) + dm_l + 0x138;
                else
                    i = dm_k - 0x100;
            }
            
            ed->sel=i;
            ed->typednum=0;
            
            i >>= 2;
            
            if(de->selchk&1)
            {
                if(i>0x94)
                    i=0x94;
            }
            else if(i>0x6a)
                i=0x6a;
            
            ed->scroll=i;
            hdc = GetDC(win);
            
            hc = GetDlgItem(win, IDC_CUSTOM1);
            
            rc = HM_GetClientRect(hc);
            
            ed->w = rc.right;
            ed->h = rc.bottom;
            
            ed->bufdc = CreateCompatibleDC(hdc);
            ed->bufbmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            
            ReleaseDC(win,hdc);
            
            SetBkMode(ed->bufdc,TRANSPARENT);
            SelectObject(ed->bufdc,ed->bufbmp);
            
            SetWindowLong(win,DWL_USER,(long)ed);
            SetWindowLong(hc,GWL_USERDATA,(long)ed);
            
            si.cbSize=sizeof(si);
            si.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
            si.nPos=ed->scroll;
            
            if(de->selchk&1)
            {
                si.nMin=0x6e;
                si.nMax=0x97;
            }
            else
            {
                si.nMin=0;
                si.nMax=0x6d;
            }
            
            si.nPage=4;
            SetScrollInfo(hc,SB_VERT,&si,0);
            SelectObject(ed->bufdc,black_brush);
            Rectangle(ed->bufdc,0,0,ed->w,ed->h);
            oldpal=SelectPalette(objdc,de->hpal,0);
            SelectPalette(ed->bufdc,de->hpal,0);
            SelectPalette(objdc,oldpal,1);
            SelectPalette(ed->bufdc,oldpal,1);
            
            EnableWindow(GetDlgItem(win,IDC_BUTTON1),dp_tab[ed->sel]);
            
    upddisp:
            
            for(i = 0; i < 16; i++)
            {
                Updateobjdisplay(ed, i);
            }
            
            InvalidateRect(hc, 0, 0);
            
            break;
        
        case WM_DESTROY:
            
            ed = (CHOOSEDUNG*) GetWindowLong(win,DWL_USER);
            
            DeleteDC(ed->bufdc);
            DeleteObject(ed->bufbmp);
            
            free(ed);
            
            break;
        
        case WM_COMMAND: // handle dialogue commands
            
            switch(wparam)
            {
            case IDOK:
                ed = (CHOOSEDUNG*) GetWindowLong(win,DWL_USER);
                EndDialog(win, ed->sel);
                
                break;
            
            case IDCANCEL:
                EndDialog(win,-1);
                
                break;
            
            case IDC_BUTTON1:
                ed = (CHOOSEDUNG*) GetWindowLong(win, DWL_USER);
                dunged = ed->ed;
                
                if
                (
                    DialogBoxParam
                    (
                        hinstance,
                        (LPSTR) IDD_DIALOG22,
                        win,
                        dpcdlgproc,
                        ed->sel
                    )
                )
                {
                    p = dunged->ew.doc->ents;
                    
                    for(i = 0; i < 168; i++, p++)
                    {
                        if(*p)
                        {
                            de = (DUNGEDIT*) GetWindowLong(*p, GWL_USERDATA);
                            Updatemap(de);
                            InvalidateRect( GetDlgItem(de->dlg, ID_DungEditWindow),
                                           0,
                                           0);
                        }
                    }
                    
                    hc=GetDlgItem(win,IDC_CUSTOM1);
                    
                    goto upddisp;
                }
                
                break;
            }
        }
        return FALSE;
    }

// =============================================================================

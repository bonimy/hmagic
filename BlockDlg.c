
    #include "structs.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "Wrappers.h"

    #include "HMagicEnum.h"

    #include "DungeonEnum.h"

    #include "OverworldEnum.h"

// =============================================================================

void
Blkedit8load(BLKEDIT8 const * const ed)
{
    int i, j, k, l, m, n = 0;
    int *b, *c;
    
    if(ed->blknum == 260)
        l = 0, n = 24;
    else if(ed->blknum == 259)
        l = 0, n = 8;
    else if(ed->blknum >= 256)
    {
        l = (ed->blknum - 256) << 7,
        n = 16;
    }
    else if(ed->blknum < 8)
        l = ed->blknum << 6;
    else if(ed->blknum == 10)
        l = 0x240;
    else if(ed->blknum >= 32)
        l = (ed->blknum - 31) << 16;
    else if(ed->blknum >= 15)
        l = (ed->blknum - 5) << 6;
    else
    {
        l = (ed->blknum + 1) << 6;
        n = 4;
    }
    
    if(ed->oed->gfxtmp==0xff) m=-1; else if(n==24) m=0x0f0f0f0f; else if(n>=8) m=0x3030303; else m=0x7070707;
    for(i=ed->size-16;i>=0;i-=16) {
        for(j=0;j<128;j+=8) {
            Drawblock(ed->oed,0,24,(j>>3)+i+l,n);
            b = (int*) (ed->buf + ( (ed->size - 16 - i) << 6) + j);
            c = (int*) drawbuf;
            for(k=0;k<8;k++) {
                b[0]=c[0]&m;
                b[1]=c[1]&m;
                b+=32;
                c+=8;
            }
        }
    }
}

// =============================================================================

extern BOOL CALLBACK
blockdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLKEDIT8 *ed;
    OVEREDIT *oed;
    ZBLOCKS *blk;
    FDOC *doc;
    HWND hc, hc2;
    RECT rc;
    
    unsigned char *b, *b2;
    
    HGLOBAL hgl;
    
    int i, j, k, l, m[2], n, o[8];
    
    BITMAPINFOHEADER bmih;
    
    switch(msg)
    {
    
    case WM_QUERYNEWPALETTE:
        
        ed = (BLKEDIT8*) GetWindowLongPtr(win, DWLP_USER);
        
        SetPalette(win, ed->oed->hpal);
        
        break;
    
    case WM_PALETTECHANGED:
        
        InvalidateRect(GetDlgItem(win, IDC_CUSTOM1), 0, 0);
        InvalidateRect(GetDlgItem(win, IDC_CUSTOM2), 0, 0);
        
        break;
    
    case WM_INITDIALOG:
        
        ed = malloc(sizeof(BLKEDIT8));
        SetWindowLongPtr(win, DWLP_USER, (LONG_PTR) ed);
        
        oed = (OVEREDIT*) *(int*) lparam;
        hc = GetDlgItem(win, IDC_CUSTOM1);
        SetWindowLongPtr(hc,GWLP_USERDATA, (LONG_PTR) ed);
        
        i = ((short*) lparam)[2];
        ed->oed = oed;
        
        if(i >= 145 && i < 147)
            i += 111;
        
        if(i == 260)
            ed->size = 896;
        else if(i == 259)
            ed->size = 256;
        else if(i >= 256)
            ed->size = 128;
        else if(oed->gfxtmp == 0xff)
            ed->size = 256, i = 0;
        else
            ed->size = 64;
        
        ed->blknum = i;
        ed->buf = malloc(ed->size << 6);
        Blkedit8load(ed);
        
        ed->scrollh = 0;
        ed->scrollv = 0;
        ed->bmih = zbmih;
        ed->bmih.biWidth = 128;
        ed->bmih.biHeight = ed->size >> 1;
        memcpy(ed->pal, oed->pal, 1024);
        
        ed->blkwnd = hc;
        
        hc = GetDlgItem(win, IDC_CUSTOM2);
        
        rc = HM_GetClientRect(hc);
        
        ed->pwidth = rc.right;
        ed->pheight = rc.bottom;
        ed->psel = i = ((short*) lparam)[3] << 4;
        
        if(ed->blknum != 260 && ed->blknum >= 256)
            i >>= 2;
        
        for(j = (ed->size << 6) - 1; j >= 0; j--)
        {
            if(ed->buf[j])
                ed->buf[j] |= i;
        }
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        break;
    
    case WM_DESTROY:
        
        ed = (BLKEDIT8*) GetWindowLongPtr(win, DWLP_USER);
        
        // \note This line commented out in the original source
        // for(i=0;i<256;i++) DeleteObject(ed->brush[i]);
        
        free(ed->buf);
        free(ed);
        
        break;
    
    case WM_COMMAND:
        
        ed = (BLKEDIT8*) GetWindowLongPtr(win, DWLP_USER);
        
        switch(wparam)
        {
        case IDC_BUTTON1:
            
            if(ed->oed->gfxtmp == 0xff)
            {
                HPALETTE const hpal = ed->oed->hpal;
                
                hgl = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,17448);
                b = GlobalLock(hgl);
                
                ed->bmih.biSizeImage = ed->bmih.biWidth * ed->bmih.biHeight;
                
                if(hpal)
                {
                    memcpy(b, &(ed->bmih), 40);
                    
                    b += 40;
                    
                    for(j = 0; j < 256; j += 8)
                    {
                        GetPaletteEntries(hpal,
                                          ((short*)(ed->pal))[j],
                                          8,
                                          (void*)(o));
                        
                        for(i = 0; i < 8; i++)
                        {
                            ((int*)(b))[i] = ((o[i] & 0xff) << 16) | ((o[i] & 0xff0000) >> 16) | (o[i] & 0xff00);
                        }
                        
                        b += 32;
                    }
                }
                else
                {
                    memcpy(b, &(ed->bmih), 1064);
                    b += 1064;
                }
                
                memcpy(b,ed->buf,16384);
            }
            else
            {
                HPALETTE const hpal = ed->oed->hpal;
                
                bmih.biSize=40;
                bmih.biWidth=128;
                bmih.biHeight=ed->bmih.biHeight;
                bmih.biPlanes=1;
                bmih.biBitCount=4;
                bmih.biCompression=BI_RGB;
                bmih.biSizeImage=bmih.biWidth*bmih.biHeight>>1;
                bmih.biClrUsed=ed->blknum==260?16:8;
                bmih.biClrImportant=0;
                hgl=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,40+(bmih.biClrUsed<<2)+(ed->size<<5));
                b=GlobalLock(hgl);
                *(BITMAPINFOHEADER*)b=bmih;
                j=ed->psel;
                if(ed->blknum>=256) j&=0xfc; else j&=0xf8;
                k=ed->size<<5;
                b2=ed->buf;
                b+=40;
                l=bmih.biClrUsed;
                
                if(hpal != 0)
                {
                    GetPaletteEntries(hpal, 0, 1, (void*) o);
                    GetPaletteEntries(hpal, ((short*) (ed->pal))[1 + j], l - 1, (void*) (o + 1));
                    
                    for(i = 0; i < l; i++)
                    {
                        ((int*) (b))[i] = ((o[i] & 0xff) << 16) | ( (o[i] & 0xff0000) >> 16) | (o[i] & 0xff00);
                    }
                }
                else
                {
                    *(int*)(b) = *(int*)ed->pal;
                    
                    for(i = 1; i < l; i++)
                        ((int*) (b))[i] = ((int*) ed->pal)[i + j];
                }
                
                b += bmih.biClrUsed << 2;
                j = 0;
                l--;
                
                for(i = 0; i < k; i++)
                {
                    *(b++) = ((b2[j] & l) << 4) | (b2[j + 1] & l);
                    j += 2;
                }
            }
            
            GlobalUnlock(hgl);
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_DIB, hgl);
            CloseClipboard();
            
            break;
        case IDC_BUTTON2:
            OpenClipboard(0);
            hgl=GetClipboardData(CF_DIB);
            if(!hgl) {
                MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
                CloseClipboard();
                break;
            }
            b=GlobalLock(hgl);
            bmih=*(BITMAPINFOHEADER*)b;
            if(bmih.biSize!=40 || bmih.biWidth!=128 ||
                (bmih.biHeight!=ed->bmih.biHeight&&bmih.biHeight!=-ed->bmih.biHeight) || bmih.biPlanes!=1
                || (bmih.biBitCount!=4 && bmih.biBitCount!=8) ||
                bmih.biCompression!=BI_RGB) {
                GlobalUnlock(hgl);
                CloseClipboard();
                MessageBox(framewnd,"The image does not have the correct dimensions or is not a 16 color image.","Bad error happened",MB_OK);
                break;
            }
            b+=40+(((bmih.biClrUsed?bmih.biClrUsed:(1<<bmih.biBitCount)))<<2);
            if(ed->blknum==260) k=15; else k=7;
            if(bmih.biBitCount==8)
                memcpy(ed->buf,b,ed->size<<6);
            else {
                j=ed->size<<5;
                b2=ed->buf;
                for(i=0;i<j;i++) {
                    *(b2++)=((*b)>>4);
                    *(b2++)=(*(b++));
                }
            }
            if(ed->oed->gfxtmp!=0xff) {
                if(ed->blknum==260) j=ed->psel&0xf0; else j=ed->psel&0xf8;
                for(i=(ed->size<<6)-1;i>=0;i--) {
                    ed->buf[i]&=k;
                    if(ed->buf[i]) ed->buf[i]|=j;
                }
            }
            if(bmih.biHeight<0) {
                j=(ed->size<<5);
                k=(ed->size<<6)-128;
                for(i=0;i<j;i++) {
                    l=ed->buf[i];
                    ed->buf[i]=ed->buf[i^k];
                    ed->buf[i^k]=l;
                }
            }
            GlobalUnlock(hgl);
            CloseClipboard();
            InvalidateRect(ed->blkwnd,0,0);
            break;
        case IDOK:
            oed=ed->oed;
            doc=oed->ew.doc;
            blk=doc->blks;
            j=ed->size<<6;
            b2=malloc(j);
            for(i=0;i<j;i++)
                b2[(i&7)+((i&0x78)<<3)+((i&0x380)>>4)+(i&0xfc00)]=ed->buf[j-128-(i&0xff80)+(i&127)];
            i=2;
            if(ed->blknum==260) {memcpy(blk[k=225].buf,b2,0xe000);goto endsave3;}
            if(ed->blknum==259) {memcpy(blk[k=224].buf,b2,0x4000);goto endsave3;}
            if(ed->blknum>=256) {memcpy(blk[k=bg3blkofs[ed->blknum-256]].buf,b2,0x2000);goto endsave3;}
            if(ed->blknum>=32) {memcpy(blk[k=ed->blknum-32].buf,b2,0x1000);goto endsave3;}
            if(oed->gfxtmp==0xff) {
                memcpy(blk[223].buf,b2,16384);
                blk[223].modf=1;
                goto endsave2;
            } else {
                j=ed->size<<6;
                for(i=0;i<j;i++) {
                    b2[i]&=7;
                    if(b2[i]) b2[i]|=8;
                }
                j=ed->blknum;
                m[0]=0;
                m[1]=0;
                if(oed->gfxtmp>=0x20) { if(ed->blknum==7) {
                    memcpy(blk[oed->blocksets[7]].buf + 0x800,b2 + 0x800,0x800);
                    if(oed->anim==2) {
                        memcpy(blk[oed->blocksets[9]].buf,b2,0x800);
                        m[1]=1;
                    } else if(oed->anim==1) {
                        memcpy(blk[oed->blocksets[8]].buf + 0x800,b2,0x800);
                        m[0]=1;
                    } else {
                        memcpy(blk[oed->blocksets[8]].buf,b2,0x800);
                        m[0]=1;
                    }
                    goto endsave;
                } } else {
                    if(ed->blknum==6) {
                        memcpy(blk[oed->blocksets[6]].buf,b2,0xc00);
                        memcpy(blk[oed->blocksets[8]].buf+(oed->anim<<10),b2 + 0xc00,0x400);
                        m[0]=1;
                        goto endsave;
                    }
                    else if(ed->blknum==7) {
                        memcpy(blk[oed->blocksets[7]].buf + 0x400, b2 + 0x400,0xc00);
                        memcpy(blk[oed->blocksets[9]].buf+(oed->anim<<10),b2,0x400);
                        m[1] = 1;
                        
                        goto endsave;
                    }
                }
            }
            
            memcpy(blk[(j >= 15) ? (0x6a + j) : oed->blocksets[j]].buf, b2, 0x1000);
endsave:
            for(i = 0; i < 3; i++)
            {
                if(i == 2)
                    k = (j >= 15) ? (0x6a + j) : oed->blocksets[j];
                else if(m[i])
                    k = oed->blocksets[i + 8];
                else
                    continue;
endsave3:
                b = blk[k].buf;
                
                if(k == 225)
                    l = 0xe000;
                else if(k == 224)
                    l = 0x4000;
                else if(k >= 220)
                    l = 0x2000;
                else
                    l = 0x1000;
                
                for(n = 0; n < l; n++)
                    b[n + l] = masktab[b[n]];
                
                for(n = 0; n < l; n++)
                    b[n+l+l] = b[n^7];
                
                b += l << 1;
                
                for(n = 0; n < l; n++)
                    b[n + l] = masktab[b[n]];
                
                blk[k].modf=1;
            }
            
            for(i = 0; i < 160; i++)
            {
                hc = doc->overworld[i].win;
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, ID_SuperDlg);
                    
                    hc = GetDlgItem(hc2, SD_Over_Map32_Selector);
                    
                    InvalidateRect(hc, 0, 0);
                    
                    hc = GetDlgItem(hc2, SD_Over_Display);
                    
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            for(i = 0; i < 0xa8; i++)
            {
                hc = doc->ents[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, ID_SuperDlg);
                    hc = GetDlgItem(hc2, ID_DungEditWindow);
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            for(i = 0; i < 11; i++)
            {
                hc = doc->tmaps[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, ID_SuperDlg);
                    hc = GetDlgItem(hc2, 3000);
                    InvalidateRect(hc, 0, 0);
                    
                    hc = GetDlgItem(hc2, 3001);
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            goto nowmap;
endsave2:
            
            for(i = 0; i < 2; i++)
            {
                hc = doc->wmaps[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, ID_SuperDlg);
                    hc = GetDlgItem(hc2, 3000);
                    
                    InvalidateRect(hc, 0, 0);
                    
                    hc = GetDlgItem(hc2, 3001);
                    
                    SendMessage(hc, 4001, 0, 0);
                }
            }
            
        nowmap:
            
            doc->modf = 1;
            
            free(b2);
            
            EndDialog(win, 1);
            
            break;
        
        case IDCANCEL:
            
            EndDialog(win, 0);
        }
    }
    return 0;
}

// =============================================================================
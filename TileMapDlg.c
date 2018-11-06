
    #include "structs.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "HMagicUtility.h"

    #include "TileMapLogic.h"

// =============================================================================

extern BOOL CALLBACK
tmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT*ed;
    int i,k,l,m;
    unsigned char*rom;
    BLOCKSEL8*bs;
    HWND hc;
    HPALETTE oldpal;
    HDC hdc;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(TMAPEDIT*)lparam;
        ed->hpal=0;
        ed->bmih=zbmih;
        ed->anim=3;
        ed->modf=0;
        ed->sel=-1;
        ed->selbg=0;
        ed->tool=0;
        ed->withfocus=0;
        ed->mapscrollh=ed->mapscrollv=0;
        rom=ed->ew.doc->rom;
        ed->disp=3;
        ed->layering=98;
        if(!ed->ew.param) {
            ed->gfxtmp=rom[0x64207];
            ed->sprgfx=rom[0x6420c];
            ed->gfxnum=rom[0x64211];
            ed->comgfx=rom[0x6433d];
            ed->anigfx=rom[0x64223];
            ed->pal1=rom[0x145d6];
            ed->pal6=ed->pal2=rom[0x145db];
            ed->pal3=rom[0x145e8];
            ed->pal4=rom[0x145ed];
            ed->pal5=rom[0x145e3];
            
            i = 0;
            
            Loadpal(ed, rom, 0x1be6c8 + 70 * ed->pal1, 0x21, 7, 5);
            Loadpal(ed, rom, 0x1be86c + 42 * ed->pal2, 0x29, 7, 3);
            Loadpal(ed, rom, 0x1be86c + 42 * ed->pal6, 0x59, 7, 3);
            Loadpal(ed, rom, 0x1bd446 + 14 * ed->pal3, 0xe9, 7, 1);
            Loadpal(ed, rom, 0x1bd39e + 14 * ed->pal4, 0x81, 7, 1);
            Loadpal(ed, rom, 0x1be604 + 14 * ed->pal5, 0x71, 7, 1);
            Loadpal(ed, rom, 0x1bd218, 0x91, 15, 4);
            Loadpal(ed, rom, 0x1bd6a0, 0, 16, 2);
            
            ed->back1 = ed->back2 = 0x1ec;
        } else {
            i=ed->ew.param+1;
            if(i==6) {
                Getblocks(ed->ew.doc,220);
                Getblocks(ed->ew.doc,221);
                Getblocks(ed->ew.doc,222);
                ed->gfxtmp=32;
                ed->gfxnum=64;
                ed->sprgfx=128;
                ed->anigfx=90;
                ed->comgfx=116;
                Loadpal(ed,rom,0x1be544,0x20,16,6);
                Loadpal(ed,rom,0x1bd6a0,0,16,2);
                Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
                Loadpal(ed,rom,0x1bd642,0xd9,3,1);
                Loadpal(ed,rom,0x1bd658,0xdc,4,1);
                Loadpal(ed,rom,0x1bd344,0xf1,15,1);
                ed->layering=100;
                ed->back1=768;
                ed->back2=127;
            } else {
                ed->gfxtmp=rom[0x64dd5];
                ed->sprgfx=rom[0x6420c];
                ed->gfxnum=rom[0x64dda];
                ed->comgfx=rom[0x64dd0];
                ed->anigfx=rom[0x64223];
                ed->pal1=rom[0x64db4]>>1;
                ed->pal2=rom[0x64dc4];
                Loadpal(ed,rom, 0x1bd734 + 180 * ed->pal1,0x21,15,6);
                Loadpal(ed,rom, 0x1bd734 + 180 * ed->pal1,0x91,7,1);
                Loadpal(ed,rom, 0x1be604, 0x71, 7, 1);
                Loadpal(ed,rom, 0x1bd660 + 32 * ed->pal2,0,16,2);
                ed->back1=127;
                ed->back2=169;
            }
        }
        ed->datnum=i;
        if(i<7) {
            k = romaddr(rom[0x137d + i] + (rom[0x1386 + i]<<8) + (rom[0x138f + i] << 16));
            i=0;
            for(;;) {
                if(rom[k+i]>=128) break;
                if(rom[k+i+2]&64) i+=6; else i+=((rom[k+i+2]&63)<<8)+rom[k+i+3]+5;
            }
            i++;
        } else {
            k=*(short*)(rom+tmap_ofs[i-7]) + 0x68000;
            if(i==7 || i==9) k++;
            switch(i) {
            case 7:
                i=*(unsigned short*)(rom + 0x64ecd);
                break;
            case 8:
                i=*(unsigned short*)(rom + 0x64e5c)+3;
                ed->buf=malloc(i);
                memcpy(ed->buf,rom+k,i-1);
                ed->buf[i-1]=255;
                goto loaded;
            case 9:
                i=*(unsigned short*)(rom + 0x654bd);
                break;
            case 10:
                i=*(unsigned short*)(rom + 0x65142)+1;
                break;
            case 11:
                i=*(unsigned short*)(rom + 0x6528d)+1;
                break;
            }
        }
        ed->buf=malloc(i);
        memcpy(ed->buf,rom+k,i);
loaded:
        ed->len=i;
        
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        k = 0x6073 + (ed->gfxtmp << 3);
        l = 0x5d97 + (ed->gfxnum << 2);
        for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
        for(i=3;i<7;i++) if(m=rom[l++]) ed->blocksets[i]=m;
        ed->blocksets[8]=ed->anigfx;
        ed->blocksets[9]=ed->anigfx+1;
        ed->blocksets[10]=ed->comgfx + 0x73;
        k = 0x5b57 + (ed->sprgfx << 2);
        for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k+i] + 0x73;
        for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
        if(ed->datnum!=6) Getblocks(ed->ew.doc,224);
        bs=&(ed->bs);
        Addgraphwin((DUNGEDIT*)ed,1);
        Setdispwin((DUNGEDIT*)ed);
        hdc=GetDC(win);
        hc=GetDlgItem(win,3001);
        bs->ed=(OVEREDIT*)ed;
        InitBlksel8(hc,bs,ed->hpal,hdc);
        ReleaseDC(win,hdc);
        Updtmap(ed);
        CheckDlgButton(win,3002,BST_CHECKED);
        CheckDlgButton(win,3004,BST_CHECKED);
        CheckDlgButton(win,3012,BST_CHECKED);
        CheckDlgButton(win,3013,BST_CHECKED);
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,(long)ed);
        Updatesize(hc);
        SetDlgItemInt(win,3008,0,0);
        break;
    case WM_DESTROY:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        ed->ew.doc->tmaps[ed->ew.param]=0;
        Delgraphwin((DUNGEDIT*)ed);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        for(i=0;i<15;i++) Releaseblks(ed->ew.doc,ed->blocksets[i]);
        if(ed->datnum==6) {
            Releaseblks(ed->ew.doc,220);
            Releaseblks(ed->ew.doc,221);
            Releaseblks(ed->ew.doc,222);
        } else Releaseblks(ed->ew.doc,224);
        free(ed->buf);
        free(ed);
        break;
    case 4002:
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        break;
    case 4000:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        i=wparam&0x3ff;
        hc=GetDlgItem(win,3001);
        Changeblk8sel(hc,&(ed->bs));
        ed->bs.sel=i;
        Changeblk8sel(hc,&(ed->bs));
        break;
    case WM_COMMAND:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        bs=&(ed->bs);
        switch(wparam) {
        case (EN_CHANGE<<16)+3008:
            bs->flags&=0xe000;
            bs->flags|=(GetDlgItemInt(win,3008,0,0)&7)<<10;
updflag:
            if((bs->flags&0xdc00)!=bs->oldflags) {
                bs->oldflags=bs->flags&0xdc00;
updblk:
                oldpal=SelectPalette(objdc,ed->hpal,1);
                SelectPalette(bs->bufdc,ed->hpal,1);
                for(i=0;i<256;i++) Updateblk8sel(bs,i);
                SelectPalette(objdc,oldpal,1);
                SelectPalette(bs->bufdc,oldpal,1);
                InvalidateRect(GetDlgItem(win,3001),0,0);
            }
            break;
        case 3002:
            ed->selbg=0;
            ed->bs.dfl=0;
            goto updblk;
            break;
        case 3003:
            ed->selbg=1;
            ed->bs.dfl=0;
            goto updblk;
        case 3004:
            ed->tool=0;
            break;
        case 3005:
            ed->tool=1;
            break;
        case 3006:
            ed->selbg=2;
            if(ed->datnum==6) ed->bs.dfl=16;
            else ed->bs.dfl=8;
            goto updblk;
        case 3009:
            bs->flags^=0x4000;
            goto updflag;
        case 3010:
            bs->flags^=0x8000;
            goto updflag;
        case 3011:
            bs->flags^=0x2000;
            goto updflag;
        case 3012:
            ed->disp^=1;
upddisp:
            InvalidateRect(GetDlgItem(win,3000),0,1);
            break;
        case 3013:
            ed->disp^=2;
            goto upddisp;
        }
    }
    return FALSE;
}

// =============================================================================

#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"
#include "Wrappers.h"

#include "WorldMapLogic.h"

BOOL CALLBACK
wmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT*ed;
    BLOCKSEL8*bs;
    unsigned char*rom;
    HWND hc;
    HDC hdc;
    RECT rc;
    int i,j,k;
    int *b,*b2;
    static char*mapmark_str[10]={
        "Hyrule castle",
        "Village guy",
        "Sahasrahla",
        "Pendants",
        "Master sword",
        "Agahnim's tower",
        "First crystal",
        "All crystals",
        "Agahnim's other tower",
        "Flute locations"
    };
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(win,DWLP_USER,lparam);
        ed=(WMAPEDIT*)lparam;
        Getblocks(ed->ew.doc,223);
        ed->hpal=0;
        ed->anim=0;
        ed->gfxtmp=255;
        ed->mapscrollh=0;
        ed->mapscrollv=0;
        ed->bmih=zbmih;
        
        rom = ed->ew.doc->rom;
        
        b = (int*)(rom + 0x54727 + (ed->ew.param << 12));
        
        j=ed->ew.param?1:4;
        
        for(k=0;k<j;k++)
        {
            b2=(int*)(ed->buf+wmap_ofs[k]);
            for(i=0;i<32;i++) {
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                b2+=8;
            }
        }
        memcpy(ed->undobuf,ed->buf,0x1000);
        Loadpal(ed,rom,ed->ew.param?0xadc27:0xadb27,0,16,8);
        hc=GetDlgItem(win,3000);
        SetWindowLongPtr(hc,GWLP_USERDATA, (LPARAM) ed);
        Updatesize(hc);
        bs=&(ed->bs);
        
        hdc = GetDC(win);
        
        hc = GetDlgItem(win, 3001);
        
        rc = HM_GetClientRect(hc);
        
        bs->ed = (OVEREDIT*)ed;
        bs->sel=0;
        bs->scroll=0;
        bs->flags=0;
        bs->dfl=0;
        bs->w=rc.right;
        bs->h=rc.bottom;
        bs->bufdc=CreateCompatibleDC(hdc);
        bs->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        
        ReleaseDC(win,hdc);
        Addgraphwin((DUNGEDIT*)ed,2);
        
        Setdispwin((DUNGEDIT*)ed);
        SelectObject(bs->bufdc,bs->bufbmp);
        SelectObject(bs->bufdc,white_pen);
        SelectObject(bs->bufdc,black_brush);
        Rectangle(bs->bufdc,0,0,bs->w,bs->h);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->hpal, 1);
            HPALETTE const oldpal2 = SelectPalette(bs->bufdc, ed->hpal, 1);
            
            for(i = 0; i < 256; i++)
                Updateblk8sel(bs,i);
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(bs->bufdc, oldpal2, 1);
        }
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) bs);
        Updatesize(hc);
        
        ed->tool=1;
        ed->dtool=0;
        ed->selflag=0;
        ed->modf=0;
        ed->undomodf=0;
        ed->selbuf=0;
        ed->marknum=0;
        
        CheckDlgButton(win,3003,BST_CHECKED);
        
        hc=GetDlgItem(win,3005);
        
        for(i=0;i<10;i++)
            SendMessage(hc,CB_ADDSTRING,0,(LPARAM) mapmark_str[i]);
        
        SendMessage(hc,CB_SETCURSEL,0,0);
        
        break;
    case 4002:
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        break;
    case WM_DESTROY:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,DWLP_USER);
        ed->ew.doc->wmaps[ed->ew.param]=0;
        Delgraphwin((DUNGEDIT*)ed);
        Releaseblks(ed->ew.doc,223);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        free(ed);
        break;
    
    // \task[med] What is this constant?
    case 4000:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,DWLP_USER);
        j=wparam;
        if(j<0) j=0;
        if(j>0xff) j=0xff;
        hc=GetDlgItem(win,3001);
        Changeblk8sel(hc,&(ed->bs));
        ed->bs.sel=j;
        Changeblk8sel(hc,&(ed->bs));
        break;
    case WM_COMMAND:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,DWLP_USER);
        j=ed->tool;
        switch(wparam) {
        case 3002:
            ed->tool=0;
            break;
        case 3003:
            ed->tool=1;
            break;
        case 3004:
            ed->tool=2;
            break;
        case 3005|(CBN_SELCHANGE<<16):
            
            ed->marknum = (short) SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
            
            if(j==4)
                break;
updscrn:
            j=0;
            InvalidateRect(GetDlgItem(win,3000),0,0);
            break;
        case 3006:
            ed->tool=3;
            ed->selmark=0;
            break;
        case 3008:
            ed->tool=4;
            goto updscrn;
        case 3007:
            if(ed->selflag) Wmapselectwrite(ed);
            b2=malloc(0x1000);
            memcpy(b2,ed->buf,0x1000);
            memcpy(ed->buf,ed->undobuf,0x1000);
            memcpy(ed->undobuf,b2,0x1000);
            free(b2);
            i=ed->undomodf;
            ed->undomodf=ed->modf;
            ed->modf=i;
            goto updscrn;
        }
        if(j==4 && ed->tool!=4) goto updscrn;
    }
    return FALSE;
}
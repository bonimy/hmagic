
    #include "structs.h"

    #include "GdiObjects.h"

    #include "prototypes.h"

    #include "LevelMapLogic.h"

// =============================================================================

BOOL CALLBACK lmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    LMAPEDIT*ed;
    unsigned char*rom;
    HWND hc;
    int i,j,k,l,m;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(LMAPEDIT*)lparam;
        rom=ed->ew.doc->rom;
        ed->hpal=0;
        ed->init=1;
        ed->modf=0;
        m=ed->ew.param;
        SetDlgItemInt(win,3002,ed->level=(((char)rom[0x56196 + m])>>1)+1,0);
        i=((short*)(rom + 0x575d9))[m];
        ed->floors=(i>>4)&15;
        ed->basements=i&15;
        ed->bg=i>>8;
        if(i&256) CheckDlgButton(win,3008,BST_CHECKED);
        if(i&512) CheckDlgButton(win,3009,BST_CHECKED);
        ed->bossroom=((short*)(rom + 0x56807))[m];
        ed->bossofs=((short*)(rom + 0x56e5d))[m];
        SendDlgItemMessage(win,3006,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[2]);
        SendDlgItemMessage(win,3007,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[3]);
        CheckDlgButton(win,3011,BST_CHECKED);
        ed->tool=0;
        i=25*(ed->floors+ed->basements);
        ed->rbuf=malloc(i);
        memcpy(ed->rbuf,rom + 0x58000 + ((short*)(rom + 0x57605))[m],i);
        k=0;
        for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
        ed->buf=malloc(k);
        memcpy(ed->buf,rom+((short*)(rom+*(short*)(rom + 0x56640) + 0x58000))[m] + 0x58000,k);
        if(ed->bossroom==15) ed->bosspos=-1;
        else for(j=0;j<i;j++) if(ed->rbuf[j]==ed->bossroom) {ed->bosspos=j;break;}
        ed->len=k;
        ed->init=0;
        ed->gfxtmp=0x20;
        ed->disp=0;
        ed->blksel=0;
        k=0x6173;
        j=0x5e97;
        for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
        for(i=3;i<7;i++) if(l=rom[j++]) ed->blocksets[i]=l;
        k = 0x5b57 + ((128 | m) << 2);
        for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k++] + 0x73;
        ed->blocksets[8]=90;
        ed->blocksets[9]=91;
        ed->blocksets[10]=116;
        for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
        ed->bmih=zbmih;
        ed->anim=0;
        Loadpal(ed,rom,0x1be544,0x20,16,6);
        Loadpal(ed,rom,0x1bd6a0,0,16,2);
        Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
        Loadpal(ed,rom,0x1bd642,0xd9,3,1);
        Loadpal(ed,rom,0x1bd658,0xdc,4,1);
        Loadpal(ed,rom,0x1bd344,0xf1,15,1);
        
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        ed->curfloor=0;
        ed->sel=0;
        if(!ed->floors) ed->curfloor=-1;
        hc=GetDlgItem(win,3010);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        ed->blkscroll=0;
        hc=GetDlgItem(win,3004);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        Updatesize(hc);
        Addgraphwin((DUNGEDIT*)ed,2);
paintfloor:
        Paintfloor(ed);
updscrn:
        hc=GetDlgItem(win,3010);
        InvalidateRect(hc,0,0);
        break;
    case WM_COMMAND:
        ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
        if(!ed) break;
        rom=ed->ew.doc->rom;
        if(ed->init) break;
        switch(wparam) {
        case 3000:
            ed->tool=2;
            goto updscrn;
        case 3002|(EN_CHANGE<<16):
            ed->level=GetDlgItemInt(win,3002,0,0);
            ed->modf=1;
            break;
        case 3005: // The grid checkbox in the dungeon map editor.
            ed->disp^=1;
            InvalidateRect(GetDlgItem(win,3004),0,0);
            InvalidateRect(GetDlgItem(win,3010),0,0);
            break;
        case 3006:
            if(ed->curfloor==ed->floors-1) break;
            ed->curfloor++;
            goto paintfloor;
        case 3007:
            if(ed->curfloor==-ed->basements) break;
            ed->curfloor--;
            goto paintfloor;
        case 3008:
            ed->bg^=1;
            ed->modf=1;
            break;
        case 3009:
            ed->bg^=1;
            ed->modf=1;
            break;
        case 3011:
            ed->tool=0;
            goto updscrn;
        case 3012:
            ed->tool=1;
            goto updscrn;
        case 3013:
            if(ed->basements+ed->floors==2) {
                MessageBox(framewnd,"There must be at least two floors.","Bad error happened",MB_OK);
                break;
            }
            i=(ed->curfloor+ed->basements)*25;
            k=0;
            for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
            l=0;
            for(j=0;j<25;j++) if(ed->rbuf[j+i]!=15) l++;
            memcpy(ed->buf+k,ed->buf+k+l,ed->len-k-l);
            ed->len-=l;
            ed->buf=realloc(ed->buf,ed->len);
            memcpy(ed->rbuf+i,ed->rbuf+i+25,(ed->floors-ed->curfloor-1)*25);
            if(ed->bosspos>=i) ed->bosspos-=25;
            if(ed->bosspos>=i) ed->bossroom=15,ed->bosspos=-1;
            ed->rbuf=realloc(ed->rbuf,(ed->floors+ed->basements)*25);
            if(ed->curfloor>=0) ed->floors--;
            else ed->basements--;
            if(ed->curfloor>=ed->floors) ed->curfloor--;
            if(-ed->curfloor>ed->basements) ed->curfloor++;
            ed->modf=1;
            Paintfloor(ed);
            goto updscrn;
        case 3016:
            ed->curfloor++;
        case 3017:
            i=(ed->curfloor+ed->basements)*25;
            k=0;
            for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
            if(ed->curfloor>=0) ed->floors++;
            else ed->basements++,ed->curfloor--;
            l=(ed->floors+ed->basements)*25;
            ed->rbuf=realloc(ed->rbuf,l);
            memmove(ed->rbuf+i+25,ed->rbuf+i,l-i-25);
            if(ed->bosspos>i) ed->bosspos+=25;
            for(j=0;j<25;j++) ed->rbuf[i+j]=15;
            ed->modf=1;
            Paintfloor(ed);
            goto updscrn;
        }
        break;
    case WM_DESTROY:
        
        ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
        
        Delgraphwin((DUNGEDIT*)ed);
        
        ed->ew.doc->dmaps[ed->ew.param]=0;
        
        for(i=0;i<15;i++)
            Releaseblks(ed->ew.doc,ed->blocksets[i]);
        
        free(ed->rbuf);
        free(ed->buf);
        free(ed);
        
        break;
    }
    return FALSE;
}

// =============================================================================
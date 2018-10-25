
#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"

#include "HMagicUtility.h"

#include "OverworldEnum.h"
#include "OverworldEdit.h"


#include "Wrappers.h"

#include "Callbacks.h"

// =============================================================================

uint8_t const map16ofs[] = {0, 1, 64, 65, 0, 1, 32, 33};

// =============================================================================

void changeposition(unsigned char*rom,int x,int y,int sx,int sy,int cx,int cy,int dx,int dy,int sp,int map,int i,int q,int door1,int door2)
{
    int a=(map&7)<<9,b=(map&56)<<6,d,e,f,g,h,j,k,l;
    k=((short*)(rom+x))[i];
    l=((short*)(rom+y))[i];
    h=((short*)(rom+x))[i]=k+dx;
    j=((short*)(rom+y))[i]=l+dy;
    if(door1) if(((unsigned short*)(rom+door2))[i]+1>=2) j+=39; else j+=19;
    d=((short*)(rom+sx))[i];
    f=((short*)(rom+cx))[i];
    if((dx<0 && h<f) || (dx>0 && h>f)) dx=h-f; else dx=0;
    d+=dx;
    if(d<a) dx+=a-d,d=a;
    if(d>a+q) dx+=a+q-d,d=a+q;
    ((short*)(rom+sx))[i]=d;
    ((short*)(rom+cx))[i]+=dx;
    e=((short*)(rom+sy))[i];
    g=((short*)(rom+cy))[i];
    if((dy<0 && j<g) || (dy>0 && j>g)) dy=j-g; else dy=0;
    e+=dy;
    if(e<b) dy+=b-e,e=b;
    if(e>b+q+32) dy+=b+q+32-e,e=b+q+32;
    ((short*)(rom+sy))[i]=e;
    ((short*)(rom+cy))[i]+=dy;
    ((short*)(rom+sp))[i]=(((e-b)&0xfff0)<<3)+(((d-a)&0xfff0)>>3);
    
    if(door1)
    {
        if( ((unsigned short*)(rom + door1))[i] + 1 >= 2 )
            ((short*)(rom + door1))[i] +=
            ( ( ( (j - 19) & 0xfff0) - (l & 0xfff0) ) << 3 )
          + ( ( ( h & 0xfff0) - (k & 0xfff0) ) >> 3 );
        
        if( ((unsigned short*)(rom + door2))[i] + 1 >= 2 )
            ((short*)(rom + door2))[i] +=
            ( ( ( (j - 39) & 0xfff0 ) - (l & 0xfff0) ) << 3 )
          + ( ( (h & 0xfff0) - (k & 0xfff0) ) >> 3 );
    }
}

// =============================================================================

HGDIOBJ
Paintovlocs(HDC hdc,OVEREDIT*ed,int t,int n,int o,int q,
            uint8_t const * const rom)
{
    text_buf_ty text_buf = { 0 };
    
    int i,j,k,m;
    uint16_t const * b2;
    uint16_t const * b3;
    unsigned char*b6;
    
    HGDIOBJ oldobj = 0;
    
    RECT rc;
    RECT text_r;
    
    switch(t)
    {
    
    case 0:
        
        // Sprites
        oldobj = SelectObject(hdc, purple_brush);
        
        if(ed->ew.param < 0x90)
        {
            m  = ed->sprset;
            b6 = ed->ebuf[m];
            
            for(i = ed->esize[m] - 3; i >= 0; i -= 3)
            {
                if( (ed->tool == 5) && (i == ed->selobj) )
                {
                    continue;
                }
                
                rc.left = ( (b6[i + 1]) << 4 ) - n;
                rc.top  = ( (b6[i])     << 4 ) - o;
                
                Drawdot(hdc, &rc, q, n, o);
                GetOverString(ed, 5, i, text_buf);
                
                text_r = rc;
                
                Getstringobjsize(text_buf, &text_r);
                
                PaintSprName(hdc, rc.left,
                             rc.top,
                             &text_r,
                             text_buf);
            }
        }
        
        break;
    
    case 1:
        
        b2=(ed->ew.doc->rom + 0xdb96f);
        b3=(ed->ew.doc->rom + 0xdba71);
        
        // Entrances
        oldobj = SelectObject(hdc, yellow_brush);
        
        for(i=0;i<129;i++)
        {
            
            if(ed->tool==3 && i==ed->selobj)
                continue;
            
            if(b2[i]==ed->ew.param)
            {
                j=b3[i];
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                wsprintf(text_buf,"%02X",rom[0xdbb73 + i]);
                
                PaintSprName(hdc,
                             rc.left,
                             rc.top,
                             &rc,
                             text_buf);
            }
        }
        
        break;
    
    case 2:
        
        b2=(rom + 0x160ef);
        b3=(rom + 0x16051);
        
        // Exits
        oldobj = SelectObject(hdc, white_brush);
        
        for(i=0;i<79;i++) {
            if(ed->tool==7 && i==ed->selobj) continue;
            if(rom[0x15e28 + i]==ed->ew.param) {
                j=b2[i];
                k=b3[i];
                j-=((ed->ew.param&7)<<9);
                k-=((ed->ew.param&56)<<6);
                rc.left=j-n;
                rc.top=k-o;
                Drawdot(hdc,&rc,q,n,o);
                
                wsprintf(text_buf,
                         "%04X",
                         ((unsigned short*)(rom + 0x15d8a))[i]);
                
                text_r = rc;
                
                Getstringobjsize(text_buf, &text_r);
                
                PaintSprName(hdc,
                             rc.left, rc.top,
                             &text_r,
                             text_buf);
            }
        }
        
        break;
    
    case 3:
        
        b2 = (ed->ew.doc->rom + 0xdb826);
        b3 = (ed->ew.doc->rom + 0xdb800);
        
        // Holes
        oldobj = SelectObject(hdc, black_brush);
        
        for(i = 0; i < 19; i++)
        {
            if( (ed->tool == 8) && (i == ed->selobj) )
                continue;
            
            if(b2[i] == ed->ew.param)
            {
                j=b3[i];
                j+=0x400;
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                
                wsprintf(text_buf,
                         "%02X",
                         rom[0xdb84c + i]);
                
                text_r = rc;
                
                Getstringobjsize(text_buf, &text_r);
                
                PaintSprName(hdc,
                             rc.left,
                             rc.top,
                             &text_r,
                             text_buf);
            }
        }
        
        break;
    
    case 4:
        
        // Items
        oldobj = SelectObject(hdc, red_brush);
        
        if(ed->ew.param < 0x80)
        {
            for(i=0;i<ed->ssize;i+=3)
            {
                if(ed->tool==10 && i==ed->selobj) continue;
                j=ed->sbuf[i];
                k=ed->sbuf[i+1];
                j+=k<<8;
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                
                GetOverString(ed, 10, i, text_buf);
                
                text_r = rc;
                
                Getstringobjsize(text_buf, &text_r);
                
                PaintSprName(hdc,
                             rc.left,
                             rc.top,
                             &text_r,
                             text_buf);
            }
        }
        
        break;
    
    case 5:
        
        b2 = (rom + 0x16b8f);
        b3 = (rom + 0x16b6d);
        
        // Bird ("Fly-{N}") locations.
        oldobj = SelectObject(hdc, blue_brush);
        
        for(i = 0; i < 17; i++)
        {
            if(ed->tool == 9 && i == ed->selobj)
                continue;
            
            if(((short*)(rom + 0x16ae5))[i]==ed->ew.param)
            {
                j = ldle16h_i(b2, i);
                k = ldle16h_i(b3, i);
                
                j -= ( (ed->ew.param & 0x07) << 9);
                k -= ( (ed->ew.param & 0x38) << 6);
                
                rc.left = (j - n);
                rc.top  = (k - o);
                
                Drawdot(hdc, &rc, q, n, o);
                
                GetOverString(ed, 9, i, text_buf);
                
                text_r = rc;
                
                Getstringobjsize(text_buf, &text_r);
                
                PaintSprName(hdc,
                             rc.left,
                             rc.top,
                             &text_r,
                             text_buf);
            }
        }
    }
    
    return oldobj;
}

// =============================================================================

void
OverworldMap_OnPaint(OVEREDIT * const ed,
                     HWND       const win)
{
    RECT rc = { 0 };
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, ed->hpal, 1);
    
    HGDIOBJ oldobj = SelectObject(hdc, white_pen);
    
    int i = 0;
    int j = 0;
    int k = ( (ps.rcPaint.right + 31) & 0xffffffe0 );
    int l = ( (ps.rcPaint.bottom + 31) & 0xffffffe0 );
    int m = 0;
    int n = ed->mapscrollh << 5;
    int o = ed->mapscrollv << 5;
    int p = 0;
    int q = 0;
    int r = 0;
    
    uint16_t * b2 = 0;
    uint16_t * b3 = 0;
    uint16_t * b4 = 0;
    uint16_t * b5 = 0;
    
    uint8_t const * b6 = 0;
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    if(ed->dtool == 2 || ed->dtool == 3 || ed->selflag)
        Getselectionrect(ed, &rc);
    
    b4 = ed->ov->buf;
    
    if(ed->ew.param == 128)
        p = 8;
    else if(ed->ew.param == 136)
        p = 0x20;
    else
        p = 0x10;
    
    if(ed->mapsize)
        q = 0x400;
    else
        q = 0x200;
    
    for(j = ps.rcPaint.top & 0xffffffe0; j < l; j += 32)
    {
        if(j+o >= q)
            break;
        
        i = ps.rcPaint.left&0xffffffe0;
        
        b2 = b4 + ed->mapscrollh + (i >> 5) + j + o;
        
        if(p == 0x20)
        {
            if(j + o < 256)
            {
                b3 = b2 + 8;
                b5 = b4 + j + o;
            }
            else
            {
                p = 16;
                goto bgchange;
            }
        }
        else
        {
            
        bgchange:
            
            b5 = b4 + 0x400 + ( (j + o) >> 1);
            b3 = b2 + 0x400 - ( (j + o) >> 1);
            
            if(b5 >= b4 + 0x500)
            {
                b3 -= 0x100;
                b5 -= 0x100;
            }
        }
        
        for(; i < k; i += 32)
        {
            if(b3 >= b5 + p)
                if(ed->ew.param==128)
                {
                    b3 = b2 + 256;
                    b5 = b4 + 256 + j + o;
                }
                else
                    b3 -= p;
            
            if(i + n >= q)
                break;
            
            m = *b2;
            
            if(ed->dtool == 3)
            {
                if(i >= rc.left && i < rc.right && j >= rc.top && j < rc.bottom)
                    m = ed->selblk;
            }
            else if(ed->selflag)
            {
                if(i >= rc.left && i < rc.right && j >= rc.top && j < rc.bottom)
                    m = ed->selbuf
                    [
                        ( (i + n) >> 5 )
                      - ed->rectleft
                      + ( ( (j + o) >> 5 ) - ed->recttop )
                      * (ed->rectright - ed->rectleft)
                    ];
            }
            
            if(ed->disp & 8)
            {
                if(ed->disp & 1)
                {
                    b6=map16ofs+4;
                    r=(((i+n)&0x1ff)>>4)+(((j+o)&0x1ff)<<1) + 0x1000;
                    ovlblk[0]=ed->ovlmap[r];
                    ovlblk[1]=ed->ovlmap[r+1];
                    ovlblk[2]=ed->ovlmap[r+32];
                    ovlblk[3]=ed->ovlmap[r+33];
                    Drawblock32(ed,*b3,0);
                }
                else
                {
                    b6=map16ofs;
                    r = ( (i + n) >> 4 ) + ( (j + o) << 2 );
                    ovlblk[0]=ed->ovlmap[r];
                    ovlblk[1]=ed->ovlmap[r+1];
                    ovlblk[2]=ed->ovlmap[r+64];
                    ovlblk[3]=ed->ovlmap[r+65];
                    Drawblock32(ed,m,0);
                }
            }
            else
            {
                if(!(ed->disp & 1))
                    goto noback;
                
                if((!ed->layering) || (ed->ew.param==128 && ((b3<b5+16 && b3>=b5+8 && b2<b4+256) || (ed->sprset<2 && b2<b4+512 && b3<b5+8))) ||
                    ((ed->ew.param==136) && b2<b5+8 && b2<b4+256)) {
                    Drawblock32(ed,m,0);
                    Drawblock32(ed,*b3,3);
                }
                else if(ed->layering==1)
                {
                    Drawblock32(ed,*b3,0);
                    Drawblock32(ed,m,1);
                }
                else
                {
                
                noback:
                    Drawblock32(ed,m,0);
                }
            }
            
            Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)ed);
            
            if(ed->disp & 8)
            {
                for(m=0;m<4;m++)
                {
                    if(!(j+o+(m&2)))
                        continue;
                    if(((ed->ovlmap[r+b6[m]]+1)^(ed->ovlmap[r+b6[m]-b6[2]]+1)) & 65536)
                    {
                        MoveToEx(hdc,i+blkx[m<<2],j+blky[m<<2],0);
                        LineTo(hdc,i+blkx[m<<2]+16,j+blky[m<<2]);
                    }
                }
                
                for(m=0;m<4;m++)
                {
                    if(!(i+n+(m&1)))
                        continue;
                    if(((ed->ovlmap[r+b6[m]]+1)^(ed->ovlmap[r+b6[m]-1]+1))&65536)
                    {
                        MoveToEx(hdc,i+blkx[m<<2],j+blky[m<<2],0);
                        LineTo(hdc,i+blkx[m<<2],j+blky[m<<2]+16);
                    }
                }
            }
            
            if(ed->disp & 2)
            {
                MoveToEx(hdc,i+31,j,0);
                LineTo(hdc,i+31,j+31);
                LineTo(hdc,i-1,j+31);
                if(ed->disp&8) {
                    MoveToEx(hdc,i,j+16,0);
                    LineTo(hdc,i+31,j+16);
                    MoveToEx(hdc,i+16,j,0);
                    LineTo(hdc,i+16,j+31);
                }
            }
            
            b2++;
            b3++;
        }
    }
    SelectObject(hdc, oldobj);
    
    if(rc.right>q-n) rc.right=q-n;
    
    if(rc.bottom>q-o) rc.bottom=q-o;
    
    if(ed->dtool==2) {
        if(rc.right>rc.left && rc.bottom>rc.top) FrameRect(hdc,&rc,white_brush);
    }
    
    if(ed->selflag)
    {
        FrameRect(hdc, &rc, green_brush);
    }
    
    if(ed->disp & SD_OverShowMarkers)
    {
         uint8_t const * const rom = ed->ew.doc->rom;
        
        HGDIOBJ oldobj2 = SelectObject(hdc, trk_font);
        
        SetBkMode(hdc, TRANSPARENT);
        
        q = ed->mapsize ? 1024 : 512;
        oldobj = 0;
        
        for(i = 0; i < 6; i++)
        {
            if(tool_ovt[ed->tool] == i)
                continue;
            
            oldobj2 = Paintovlocs(hdc,ed,i,n,o,q,rom);
            
            if(!oldobj)
                oldobj = oldobj2;
        }
        
        Paintovlocs(hdc,ed,tool_ovt[ed->tool],n,o,q,rom);
        
        // Highlight the currently selected entity with a rectanglar
        // border and background fill.
        if(ed->selobj != -1)
        {
            text_buf_ty text_buf;
            
            HGDIOBJ const oldobj2 = SelectObject(hdc, green_pen);
            HGDIOBJ const oldobj3 = SelectObject(hdc, black_brush);
            
            GetOverString(ed, ed->tool, ed->selobj, text_buf);
            
            rc.left = ed->objx-n;
            rc.top  = ed->objy-o;
            
            Getstringobjsize(text_buf, &rc);
            
            if(rc.right > q - n)
                rc.right = q - n;
            
            if(rc.bottom > q - o)
                rc.bottom = q - o;
            
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            
            PaintSprName(hdc, rc.left, rc.top, &rc, text_buf);
            
            SelectObject(hdc, oldobj2);
            SelectObject(hdc, oldobj3);
        }
        
        SelectObject(hdc, oldobj);
    }
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(win, &ps);
}

// =============================================================================

LRESULT CALLBACK
overmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SCROLLINFO si;
    
    int i = 0; int j = 0; int k = 0; int l = 0;
    int m = 0; int n = 0; int o = 0;
    int q = 0;
    
    POINT pt;
    RECT rc = {0, 0, 0, 0};
    HMENU menu,menu2;
    
    const static int whirl_ofs[8]={
        0x16b29,0x16b4b,0x16b6d,0x16b8f,0x16ae5,0x16b07,0x16bb1,0x16bd3
    };
    
    uint16_t * b4 = 0;
    uint16_t * b5 = 0;
    
    unsigned char*b6 = 0,*rom = 0;
    
    OVEREDIT * const ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
    
    switch(msg)
    {
    
    case WM_SIZE:
        
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax = ( ed->mapsize ? 32 : 16 ) - 1;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,si.nMax,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,si.nMax,32);
        
        break;
    
    case WM_MOUSEWHEEL:
        
        {
            HM_MouseWheelData d = HM_GetMouseWheelData(wparam, lparam);
            
            unsigned const is_horiz = (d.m_control_key);
            
            unsigned which_sb = (is_horiz) ? SB_HORZ : SB_VERT;
            
            int const which_scroll = (is_horiz) ? ed->mapscrollh : ed->mapscrollv;
            int const which_page   = (is_horiz) ? ed->mappageh : ed->mappagev;
            
            int * const which_return = (is_horiz) ? &ed->mapscrollh : &ed->mapscrollv;
            
            (*which_return) = Handlescroll(win,
                                           (d.m_distance > 0) ? 0 : 1,
                                           which_scroll,
                                           which_page,
                                           which_sb,
                                           ed->mapsize ? 32 : 16, 32);
        }
        
        break;
    
    case WM_VSCROLL:
        
        ed->mapscrollv = Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,ed->mapsize?32:16,32);
        
        break;
    
    case WM_HSCROLL:
        
        ed->mapscrollh = Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,ed->mapsize?32:16,32);
        
        break;
    
    case WM_RBUTTONDOWN:
        
        l=ed->tool;
        if(l==6 || l==4) break;
        if(l>2) {
            menu=CreatePopupMenu();
            switch(l) {
            case 3:
                AppendMenu(menu,MF_STRING,2,"Insert entrance");
                break;
            case 5:
                if(ed->ew.param<0x90)
                AppendMenu(menu,MF_STRING,3,"Insert sprite");
                break;
            case 7:
                AppendMenu(menu,MF_STRING,5,"Insert exit");
                break;
            case 8:
                AppendMenu(menu,MF_STRING,6,"Insert hole");
                break;
            case 9:
                AppendMenu(menu,MF_STRING,7,"Insert bird location");
                AppendMenu(menu,MF_STRING,8,"Insert whirlpool");
                break;
            case 10:
                if(ed->ew.param<0x80) AppendMenu(menu,MF_STRING,4,"Insert item");
                break;
            }
            if(ed->selobj!=-1) AppendMenu(menu,MF_STRING,1,"Remove");
            if(ed->ew.param<0x90 && l==5) {
                j=0,k=1;
                for(i=ed->ew.param>>7;i<3;i++,k<<=1) {
                    if(i==ed->sprset || ed->ecopy[i]!=-1 || i==ed->ecopy[ed->sprset]) continue;
                    j|=k;
                }
                if(j) {
                    k=1;
                    menu2=CreatePopupMenu();
                    for(i=0;i<3;i++) {
                        if(k&j) AppendMenu(menu2,MF_STRING,i+9,sprset_str[i]);
                        k<<=1;
                    }
                    AppendMenu(menu,MF_STRING|MF_POPUP,(int)menu2,"Use same sprites as");
                }
            }
            if(ed->disp&8) AppendMenu(menu,MF_STRING,12,"Clear overlay");
            GetCursorPos(&pt);
            k=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY,pt.x,pt.y,0,win,0);
            DestroyMenu(menu);
            i=(lparam&65535)+(ed->mapscrollh<<5);
            j=(lparam>>16)+(ed->mapscrollv<<5);
            rom=ed->ew.doc->rom;
            switch(k) {
            case 1:
                Overselchg(ed,win);
                switch(l) {
                case 3:
                    ((short*)(rom + 0xdb96f))[ed->selobj]=-1;
                    break;
                case 5:
                    b6=ed->ebuf[ed->sprset];
                    ed->esize[ed->sprset]-=3;
                    memcpy(b6+ed->selobj,b6+ed->selobj+3,ed->esize[ed->sprset]-ed->selobj);
                    ed->ebuf[ed->sprset]=realloc(b6,ed->esize[ed->sprset]);
                    ed->e_modf[ed->sprset]=1;
                    ed->ov->modf=1;
                    break;
                case 7:
                    rom[0x15e28 + ed->selobj]=255;
                    ((short*)(rom + 0x15d8a))[ed->selobj]=-1;
                    break;
                case 8:
                    ((short*)(rom + 0xdb826))[ed->selobj]=-1;
                    break;
                case 9:
                    ((short*)(rom + 0x16ae5))[ed->selobj]=-1;
                    break;
                case 10:
                    ed->ssize-=3;
                    b6=ed->sbuf;
                    memcpy(b6+ed->selobj,b6+ed->selobj+3,ed->ssize-ed->selobj);
                    ed->sbuf=realloc(b6,ed->ssize);
                    ed->ov->modf=1;
                    break;
                }
                ed->selobj=-1;
                break;
            case 2:
                for(m=0;m<129;m++)
                {
                    if( is16b_neg1_i(rom + 0xdb96f, m) )
                    {
                        Overselchg(ed,win);
                        ((short*)(rom + 0xdb96f))[m]=ed->ew.param;
                        ((short*)(rom + 0xdba71))[m]=((j<<3)&0x1f80)+((i>>3)&0x7e);
                        rom[0xdbb73 + m]=0;
                        ed->selobj=m;
                        ed->objx=i&0xfff0;
                        ed->objy=j&0xfff0;
                        Overselchg(ed,win);
                        return 0;
                    }
                }
                MessageBox(framewnd,"Can't add anymore entrances.","Bad error happened",MB_OK);
                break;
            case 3:
                m=ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,0);
                Overselchg(ed,win);
                if(m==-1) break;
                if(ed->selobj==-1) ed->selobj=ed->esize[ed->sprset];
                ed->esize[ed->sprset]+=3;
                b6=ed->ebuf[ed->sprset]=realloc(ed->ebuf[ed->sprset],ed->esize[ed->sprset]);
                memmove(b6+ed->selobj+3,b6+ed->selobj,ed->esize[ed->sprset]-ed->selobj-3);
                b6[ed->selobj]=j>>4;
                b6[ed->selobj+1]=i>>4;
                b6[ed->selobj+2]=m;
                ed->e_modf[ed->sprset]=1;
                ed->ov->modf=1;
                ed->objx=i&0xfff0;
                ed->objy=j&0xfff0;
                Overselchg(ed,win);
                break;
            case 4:
                Overselchg(ed,win);
                b6=ed->sbuf=realloc(ed->sbuf,ed->ssize+=3);
                if(ed->selobj==-1) ed->selobj=ed->ssize-3;
                memmove(b6+ed->selobj+3,b6+ed->selobj,ed->ssize-ed->selobj-3);
                *(short*)(b6+ed->selobj)=((i>>3)&0x7e)+((j<<3)&0x1f80);
                ed->objx=i&0xfff0;
                ed->objy=j&0xfff0;
                b6[ed->selobj+2]=0;
                ed->ov->modf=1;
                Overselchg(ed,win);
                break;
            
            case 5: // add exit command?
                
                for(n = 0; n < 79; n++)
                    if(rom[0x15e28 + n] == 255)
                    {
                        rom[0x15e28 + n] = ed->ew.param;
                        
                        ((short*) (rom + 0x15d8a))[n] = 0;
addexit:
                        
                        l = i - 128;
                        m = j - 112;
                        q = ed->mapsize ? 1024 : 512;
                        
                        if(l < 0)
                            l = 0;
                        
                        if(l > q - 256)
                            l = q - 256;
                        
                        if(m < 0)
                            m = 0;
                        
                        if(m > q - 224)
                            m = q - 224;
                        
                        l += (ed->ew.param & 7) << 9;
                        m += (ed->ew.param & 56) << 6;
                        
                        Overselchg(ed,win);
                        
                        ed->selobj = n;
                        ed->objx = i;
                        ed->objy=j;
                        i += (ed->ew.param & 7) << 9;
                        j += (ed->ew.param & 56) << 6;
                        o = ((l & 0xfff0) >> 3) + ((m & 0xfff0) << 3);
                        
                        if(k == 5)
                        {
                            ((short*)(rom + 0x160ef))[n]=i;
                            ((short*)(rom + 0x16051))[n]=j;
                            ((short*)(rom + 0x15fb3))[n]=l;
                            ((short*)(rom + 0x15f15))[n]=m;
                            ((short*)(rom + 0x1622b))[n]=l+128;
                            ((short*)(rom + 0x1618d))[n]=m+112;
                        }
                    else
                    {
                        ((short*)(rom + 0x16b8f))[n]=i;
                        ((short*)(rom + 0x16b6d))[n]=j;
                        ((short*)(rom + 0x16b4b))[n]=l;
                        ((short*)(rom + 0x16b29))[n]=m;
                        ((short*)(rom + 0x16bd3))[n]=l+128;
                        ((short*)(rom + 0x16bb1))[n]=m+112;
                    }
                    Overselchg(ed,win);
                    return 0;
                }
                MessageBox(framewnd,"You can't add anymore exits.","Bad error happened",MB_OK);
                break;
            case 6:
                
                for(n=0;n<19;n++)
                    if( is16b_neg1_i(rom + 0xdb826, n) )
                    {
                        Overselchg(ed,win);
                        ((short*)(rom + 0xdb826))[n]=ed->ew.param;
                        ((short*)(rom + 0xdb800))[n]=((j<<3)&0x1f80)+((i>>3)&0x7e) - 0x400;
                        rom[0xdb84c + n]=0;
                        ed->selobj=n;
                        ed->objx=i&0xfff0;
                        ed->objy=j&0xfff0;
                        Overselchg(ed,win);
                        return 0;
                    }
                
                MessageBox(framewnd,"You can't add anymore holes.","Bad error happened",MB_OK);
                break;
            case 7:
                
                for(n=0;n<9;n++)
                    if( is16b_neg1_i(rom + 0x16ae5, n) )
                    {
                        ((short*)(rom + 0x16ae5))[n]=ed->ew.param;
                        goto addexit;
                    }
                
                MessageBox(framewnd,"You can't add anymore bird locations.","Bad error happened",MB_OK);
                break;
            case 8:
                
                for(n=9;n<17;n++)
                {
                    if( is16b_neg1_i(rom + 0x16ae5, n) )
                    {
                        ((short*)(rom + 0x16ae5))[n]=ed->ew.param;
                        ((short*)(rom + 0x16ce6))[n]=0;
                        goto addexit;
                    }
                }
                
                MessageBox(framewnd,"You can't add anymore whirlpools.","Bad error happened",MB_OK);
                break;
            case 9: case 10: case 11:
                
                i = ed->sprset;
                
                if(ed->ecopy[i] == -1)
                    free(ed->ebuf[i]);
                
                k-=9;
                ed->ebuf[i]=ed->ebuf[k];
                ed->esize[i]=ed->esize[k];
                ed->e_modf[i]=1;
                if(k>i) { i=k; k=ed->sprset; }
                ed->ecopy[i]=k;
                ed->ecopy[k]=-1;
                InvalidateRect(win,0,0);
            }
        } else {
            if(ed->disp&8) {
                l=(lparam>>20)+(ed->mapscrollv<<1);
                k=((lparam&65535)>>4)+(ed->mapscrollh<<1);
                if(ed->mapsize) q=63; else q=31;
                if(k>q || l>q) break;
                SetDlgItemInt(ed->dlg,3005,ed->ovlmap[(ed->disp&1)?((k&31)+((l&31)<<5) + 0x1000):k+(l<<6)],0);
            } else {
                l=(lparam>>21)+ed->mapscrollv;
                k=((lparam&65535)>>5)+ed->mapscrollh;
                if(ed->mapsize) q=31; else q=15;
                if(k>q || l>q) break;
                SetDlgItemInt(ed->dlg,3005,ed->ov->buf[k+(l<<5)],0);
            }
        }
        break;
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    case WM_CHAR:
        
        if(wparam==26) {overdlgproc(GetParent(win),WM_COMMAND,3014,0);break;}
        i=ed->selobj;
        
        if(i == -1)
            break;
        
        rom=ed->ew.doc->rom;
        switch(ed->tool) {
        case 3:
            j=rom[0xdbb73 + i];
            break;
        case 5:
            m=ed->sprset;
            b6=ed->ebuf[m];
            j=b6[i+2];
            break;
        case 7:
            j=((unsigned short*)(rom + 0x15d8a))[i];
            break;
        case 8:
            j=rom[0xdb84c + i];
            break;
        case 9:
            if(i>8) j=((unsigned short*)(rom + 0x16ce6))[i];
            else j=i;
            break;
        case 10:
            j=ed->sbuf[i+2];
            if(j>=128) j=(j>>1)-41;
            if(j==26) ((short*)(rom + 0x16dc5))[ed->ew.param]=0;
            break;
        default:
            return 0;
        }
        Overselchg(ed,win);
        if(wparam>=64) wparam&=0xdf;
        switch(wparam) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            j<<=4;
            j+=wparam-'0';
            goto updent;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            j<<=4;
            j+=wparam-'A'+10;
            goto updent;
        case 'N':
            k=-1;
            goto updadd;
        case 'M':
            k=1;
            goto updadd;
        case 'J':
            k=-16;
            goto updadd;
        case 'K':
            k=16;
updadd:
            j+=k;
updent:
            switch(ed->tool) {
            case 3:
                rom[0xdbb73 + i]=j;
                ed->ew.doc->modf=1;
                break;
            case 5:
                Overselchg(ed,win);
                b6[i+2]=j;
                ed->ov->modf=1;
                ed->e_modf[ed->sprset]=1;
                break;
            case 7:
                ((unsigned short*)(rom + 0x15d8a))[i]=j;
                ed->ew.doc->modf=1;
                break;
            case 8:
                rom[0xdb84c + i]=j;
                break;
            case 9:
                if(i>8) rom[0x16cec + i*2]=j;
                else {
                    j--;
                    j&=15;
                    if(j>8) break;
                    for(k=0;k<8;k++) {
                        l=((short*)(rom+whirl_ofs[k]))[i];
                        ((short*)(rom+whirl_ofs[k]))[i]=((short*)(rom+whirl_ofs[k]))[j];
                        ((short*)(rom+whirl_ofs[k]))[j]=l;
                    }
                    l=rom[0x16bf5 + i];
                    rom[0x16bf5 + i]=rom[0x16bf5 + j];
                    rom[0x16bf5 + j]=l;
                    l=rom[0x16c17 + i];
                    rom[0x16c17 + i]=rom[0x16c17 + j];
                    rom[0x16c17 + j]=l;
                    Overselchg(ed,win);
                    ed->selobj=j;
                }
                break;
            case 10:
                if(j>27) j-=28;
                if(j<0) j+=28;
                if(j==26) {
                    if(((short*)(rom + 0x16dc5))[ed->ew.param]) if(k) j+=k; else return 0;
                    else ((short*)(rom + 0x16dc5))[ed->ew.param]=*(short*)(ed->sbuf+i);
                }
                if(j>22) j=(j+41)<<1;
                ed->sbuf[i+2]=j;
            }
            Overselchg(ed,win);
        }
        break;
    
    case WM_LBUTTONDBLCLK:
        
        if(ed->selobj==-1) break;
        switch(ed->tool)
        {
        case 3:
            SendMessage(ed->ew.doc->editwin,4000,0x30000 + ed->ew.doc->rom[0xdbb73 + ed->selobj],0);
            break;
        case 5:
            
            if(always)
            {
                text_buf_ty text_buf;
                
                m=ed->sprset;
                b6=ed->ebuf[m];
                j=b6[ed->selobj+2];
                i=ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,j);
                if(i==-1) break;
                n=ed->mapscrollh<<5;
                o=ed->mapscrollv<<5;
                ed->ov->modf=1;
                ed->e_modf[ed->sprset]=1;
                rc.left=ed->objx-n;
                rc.top=ed->objy-o;
                
                wsprintf(text_buf,
                         "%02X-%s",
                         j,
                         sprname[j]);
                
                Getstringobjsize(text_buf, &rc);
                InvalidateRect(win,&rc,0);
                
                wsprintf(text_buf,
                         "%02X-%s",
                         i,
                         sprname[i]);
                
                Getstringobjsize(text_buf, &rc);
                InvalidateRect(win,&rc,0);
                b6[ed->selobj+2]=i;
            }
            
            break;
        case 7:
            oved=ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG10,framewnd,editexit,0);
            break;
        case 9:
            oved=ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG12,framewnd,editwhirl,0);
            break;
        }
        break;
    
    case WM_LBUTTONDOWN:
        
        if(ed->tool == 4)
        {
            text_buf_ty text_buf;
            
            l = ( ( (lparam >> 21) + ed->mapscrollv ) << 8)
              | ( ( ( ( (short) lparam) >> 5) + ed->mapscrollh ) << 2)
              | 0x7e2000;
            
            wsprintf(text_buf,
                     "Position %04X,%04X:\n%06X %06X\n%06X %06X",
                     ((short)lparam) + (ed->mapscrollh << 5) + ((ed->ew.param & 7) << 9),
                     (lparam >> 16) + (ed->mapscrollv << 5) + ((ed->ew.param & 56) << 6),
                     l,
                     l +    2,
                     l + 0x80,
                     l + 0x82);
            
            MessageBox(framewnd, text_buf, "Address calculator",MB_OK);
            
            break;
        }
        if(ed->dtool) break;
        if(ed->tool!=1 && ed->selflag) {
            Overselectwrite(ed);
            Getselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
        }
        ed->dtool=ed->tool+1;
        if(ed->tool!=6) SetCapture(win);
        SetFocus(win);
        goto mousemove;
    
    mousemove:
    case WM_MOUSEMOVE:
        
        {
            HWND const par = GetParent(win);
            
            RECT const par_rect = HM_GetWindowRect(par);
            
            HM_MouseMoveData const d =
            HM_GetMouseMoveData(win, wparam, lparam);
            
            POINT const rel_pos =
            {
                d.m_screen_pos.x - par_rect.left,
                d.m_screen_pos.y - par_rect.top
            };
            
            LPARAM const new_lp = MAKELPARAM(rel_pos.x, rel_pos.y);
            
            PostMessage(GetParent(win), msg, wparam, new_lp);
        }
        
        if(!ed->dtool)
        {
            break;
        }
        
        n=ed->mapscrollh<<5;
        o=ed->mapscrollv<<5;
        l=(lparam>>16)+o;
        k=((short)lparam)+n;
        if(ed->mapsize) q=0x3ff; else q=0x1ff;
        if(k>q) k=q-1;
        if(l>q) l=q-1;
        if(k<0) k=0;
        if(l<0) l=0;
        switch(ed->dtool)
        {
        
        case 1:
            if(ed->disp&8) {
                k>>=4;
                l>>=4;
                rc.left=(k<<4)-n;
                rc.right=rc.left+17;
                rc.top=(l<<4)-o;
                rc.bottom=rc.top+17;
                InvalidateRect(win,&rc,0);
                if(ed->disp&1) {
                    ed->ovlmap[(k&31)+((l&31)<<5) + 0x1000]=ed->selblk;
                    rc.left=(k^16<<4)-n;
                    rc.right=rc.left+17;
                    InvalidateRect(win,&rc,0);
                    rc.top=(l^16<<4)-o;
                    rc.bottom=rc.top+17;
                    InvalidateRect(win,&rc,0);
                    rc.left=(k<<4)-n;
                    rc.right=rc.left+17;
                    InvalidateRect(win,&rc,0);
                } else ed->ovlmap[k+(l<<6)]=ed->selblk;
                ed->ovlmodf=1;
                ed->ov->modf=1;
                break;
            }
            if(msg==WM_LBUTTONDOWN) {
                memcpy(ed->undobuf,ed->ov->buf,0x800);
                ed->undomodf=ed->ov->modf;
            }
            k>>=5;
            l>>=5;
            ed->ov->buf[k+(l<<5)]=ed->selblk;
            rc.left=(k<<5)-n;
            rc.right=rc.left+32;
            rc.top=(l<<5)-o;
            rc.bottom=rc.top+32;
            InvalidateRect(win,&rc,0);
            if((ed->ew.param==136) && k>=8 && l<8) {
                rc.left-=256;
                rc.right-=256;
                InvalidateRect(win,&rc,0);
            } else if(ed->ew.param==128 && k>=8 && l>=8 && l<16) {
                rc.top-=256;
                rc.bottom-=256;
                InvalidateRect(win,&rc,0);
            }
            ed->ov->modf=1;
            break;
        
        case 2:
        case 3:
            k>>=5;
            l>>=5;
            if(msg==WM_LBUTTONDOWN) {
                if(ed->selflag) if(k>=ed->rectleft && k<ed->rectright && l>=ed->recttop && l<ed->rectbot) {
                    ed->selx=k;
                    ed->sely=l;
                } else {
                    Overselectwrite(ed);
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    goto newsel;
                } else newsel: ed->rectleft=ed->rectright=k,ed->recttop=ed->rectbot=l;
            } else {
                if(ed->selflag) {
                    if(k==ed->selx && l==ed->sely) break;
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectleft+=k-ed->selx;
                    ed->rectright+=k-ed->selx;
                    ed->recttop+=l-ed->sely;
                    ed->rectbot+=l-ed->sely;
                    ed->selx=k;
                    ed->sely=l;
                } else {
                    if(k>=ed->rectleft) k++;
                    if(l>=ed->recttop) l++;
                    if(ed->rectright==k && ed->rectbot==l) break;
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectright=k,ed->rectbot=l;
                }
                Getselectionrect(ed,&rc);
                InvalidateRect(win,&rc,0);
            }
            break;
        case 4: case 6: case 8: case 9: case 10: case 11:
            
            rom=ed->ew.doc->rom;
            
            if(ed->disp & SD_OverShowMarkers)
            {
                if(msg == WM_LBUTTONDOWN)
                {
                    Overselchg(ed,win);
                    switch(ed->tool) {
                    case 3:
                        b4 = (uint16_t*) (rom + 0xdb96f);
                        b5 = (uint16_t*) (rom + 0xdba71);
                        i=128;
                        m=0;
searchtile:
                        k&=0xfff0;
                        l&=0xfff0;
                        for(;i>=0;i--) {
                            if(b4[i]==ed->ew.param) {
                                j=b5[i]+m;
                                if(((j&0x7e)<<3)==k && ((j&0x1f80)>>3)==l) {
                                    ed->objx=k;
                                    ed->objy=l;
foundobj:
                                    ed->selx=k;
                                    ed->sely=l;
                                    ed->selobj=i;
                                    Overselchg(ed,win);
                                    return 0;
                                }
                            }
                        }
                        break;
                    case 5:
                        k&=0xfff0;
                        l&=0xfff0;
                        m=ed->sprset;
                        b6=ed->ebuf[ed->sprset];
                        for(i=ed->esize[m]-3;i>=0;i-=3)
                        {
                            text_buf_ty text_buf;
                            
                            rc.left=b6[i+1]<<4;
                            rc.top=b6[i]<<4;
                            
                            GetOverString(ed, 5, i, text_buf);
                            
                            Getstringobjsize(text_buf, &rc);
                            
                            if
                            (
                                k >= rc.left
                             && l >= rc.top
                             && k < rc.right
                             && l < rc.bottom
                            )
                            {
                                ed->selobj=i;
                                ed->selx=k;
                                ed->sely=l;
                                
                                ed->objx = (short) rc.left;
                                ed->objy = (short) rc.top;
                                
                                rc.left-=n;
                                rc.top-=o;
                                rc.right-=n;
                                rc.bottom-=o;
                                InvalidateRect(win,&rc,0);
                                return 0;
                            }
                        }
                        break;
                    case 7:
                        b4=(uint16_t*)(rom + 0x16051);
                        b5=(uint16_t*)(rom + 0x160ef);
                        j=k+((ed->ew.param&7)<<9);
                        m=l+((ed->ew.param&56)<<6);
                        for(i=78;i>=0;i--) {
                            if(rom[0x15e28 + i]==ed->ew.param) {
                                if(j>=b5[i] && j<b5[i]+16 && m>=b4[i] && m<b4[i]+16) {
                                    ed->objx=b5[i]-j+k;
                                    ed->objy=b4[i]-m+l;
                                    goto foundobj;
                                }
                            }
                        }
                        break;
                    case 8:
                        
                        b4 = (uint16_t*) (rom + 0xdb826);
                        b5 = (uint16_t*) (rom + 0xdb800);
                        i=18;
                        m=0x400;
                        goto searchtile;
                    case 9:
                        
                        b4 = (uint16_t*) (rom + 0x16b8f);
                        b5 = (uint16_t*) (rom + 0x16b6d);
                        
                        j=k+((ed->ew.param&7)<<9);
                        m=l+((ed->ew.param&56)<<6);
                        for(i=16;i>=0;i--) {
                            if(((short*)(rom + 0x16ae5))[i]==ed->ew.param) {
                                if(j>=b4[i] && j<b4[i]+16 && m>=b5[i] && m<b5[i]+16) {
                                    ed->objx=b4[i]-j+k;
                                    ed->objy=b5[i]-m+l;
                                    goto foundobj;
                                }
                            }
                        }
                        break;
                    case 10:
                        k&=0xfff0;
                        l&=0xfff0;
                        m=ed->sprset;
                        b6=ed->sbuf;
                        for(i=ed->ssize-3;i>=0;i-=3) {
                            j=*(short*)(b6+i);
                            rc.left=(j&0x7e)<<3;
                            rc.top=(j&0x1f80)>>3;
                            
                            if(k>=rc.left && k<rc.left+16 && l>=rc.top && l<rc.top+16)
                            {
                                ed->objx = (short) rc.left;
                                ed->objy = (short) rc.top;
                                ed->ov->modf=1;
                                goto foundobj;
                            }
                        }
                        break;
                    }
                    ed->selobj=-1;
                    ed->dtool=0;
                    ReleaseCapture();
                }
                else
                {
                    switch(ed->tool)
                    {
                    case 3:
                        b5=(uint16_t*)(ed->ew.doc->rom + 0xdba71);
                        m=0;
movetile:
                        k&=0xfff0;
                        l&=0xfff0;
                        if(k==ed->selx && l==ed->sely) return 0;
                        rc.left=ed->objx-n;
                        rc.top=ed->objy-o;
                        Getstringobjsize("00",&rc);
                        InvalidateRect(win,&rc,0);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        
                        b5[ed->selobj] = ((ed->objy << 3) | (ed->objx >> 3)) - m;
                        
                        ed->selx=k;
                        rc.left=k-n;
                        ed->sely=l;
                        rc.top=l-o;
                        Getstringobjsize("00",&rc);
                        InvalidateRect(win,&rc,0);
                        break;
                    
                    case 5:
                        
                        k &= ~0xf;
                        l &= ~0xf;
                        
                        if(k == ed->selx && l == ed->sely)
                            return 0;
                        
                        if(always)
                        {
                            text_buf_ty text_buf;
                            
                            m=ed->sprset;
                            
                            b6=ed->ebuf[ed->sprset];
                            
                            rc.left=ed->objx-n;
                            rc.top=ed->objy-o;
                            
                            GetOverString(ed, 5, ed->selobj, text_buf);
                            
                            Getstringobjsize(text_buf, &rc);
                            
                            InvalidateRect(win,&rc,0);
                            
                            ed->objx+=k-ed->selx;
                            ed->objy+=l-ed->sely;
                        }

                        // Clamp X and Y coordinates of the sprite to acceptable
                        // ranges.
                        if(always)
                        {
                            int max_pos = (512 << ed->mapsize) - 0x10;
                            
                            if(ed->objx < 0)
                                ed->objx = 0;
                            else if(ed->objx > max_pos)
                                ed->objx = max_pos;
                            
                            if(ed->objy < 0)
                                ed->objy = 0;
                            else if(ed->objy > max_pos)
                                ed->objy = max_pos;
                        }
                        
                        b6[ed->selobj]=ed->objy>>4;
                        b6[ed->selobj+1]=ed->objx>>4;
                        
                        rc.right+=k-ed->selx;
                        rc.bottom+=l-ed->sely;
                        rc.left=ed->objx-n;
                        rc.top=ed->objy-o;
                        
                        ed->selx=k;
                        ed->sely=l;
                        
                        InvalidateRect(win,&rc,0);
                        
                        ed->e_modf[m]=1;
                        ed->ov->modf=1;
                        
                        break;
                    
                    case 7:
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        changeposition(rom,0x160ef,0x16051,0x15fb3,0x15f15,0x1622b,0x1618d,k-ed->selx,l-ed->sely,0x15e77,ed->ew.param,ed->selobj,ed->mapsize?768:256,0x16367,0x16405);
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    case 8:
                        b5 = (uint16_t*)(ed->ew.doc->rom + 0xdb800);
                        m=0x400;
                        goto movetile;
                    case 9:
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        changeposition(rom,0x16b8f,0x16b6d,0x16b4b,0x16b29,0x16bd3,0x16bb1,k-ed->selx,l-ed->sely,0x16b07,ed->ew.param,ed->selobj,ed->mapsize?768:256,0,0);
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    case 10:
                        k&=0xfff0;
                        l&=0xfff0;
                        if(k==ed->selx && l==ed->sely) return 0;
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        i=ed->selobj;
                        j=*(short*)(ed->sbuf+i)=(ed->objx>>3)+(ed->objy<<3);
                        if(ed->sbuf[i+2]==0x86 && ed->ew.param<0x80) ((short*)(rom + 0x16dc5))[ed->ew.param]=j;
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    }
                    ed->ew.doc->modf=1;
                }
            }
            break;
        case 7:
            ReleaseCapture();
            if(!copybuf) {
                MessageBox(framewnd,"You must copy before you can paste.","Bad error happened",MB_OK);
                ed->dtool=0;
                break;
            }
            k>>=5;
            l>>=5;
            ed->rectleft=k;
            ed->recttop=l;
            ed->rectright=k+copy_w;
            ed->rectbot=l+copy_h;
            ed->stselx=-1;
            ed->stsely=-1;
            q=copy_w*copy_h<<1;
            ed->selbuf=malloc(q);
            ed->selflag=1;
            memcpy(ed->selbuf,copybuf,q);
            Getselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
            ed->dtool=0;
            ed->ov->modf=1;
        }
        break;
    
    case WM_LBUTTONUP:
        
        if(ed->dtool > 1 && ed->dtool < 4)
        {
            Getselectionrect(ed, &rc);
            
            InvalidateRect(win, &rc, 0);
            
            if(ed->rectleft < ed->rectright)
            {
                i = ed->rectleft;
                j = ed->rectright;
            }
            else
            {
                i = ed->rectright;
                j = ed->rectleft;
            }
            
            if(ed->recttop < ed->rectbot)
            {
                k = ed->recttop;
                l = ed->rectbot;
            }
            else
            {
                k = ed->rectbot;
                l = ed->recttop;
            }
            
            if(ed->dtool == 3)
            {
                ed->undomodf=ed->ov->modf;
                memcpy(ed->undobuf,ed->ov->buf,0x800);
                
                for(m = i; m < j; m++)
                    for(n = k; n < l; n++)
                        ed->ov->buf[m + (n << 5)] = ed->selblk;
                
                ed->ov->modf = 1;
            }
            else if(!ed->selflag)
            {
                ed->rectleft = i;
                ed->rectright = j;
                ed->recttop = k;
                ed->rectbot = l;
                ed->selflag = 1;
                ed->stselx = i;
                ed->stsely = k;
                j-=i;
                l-=k;
                l<<=5;
                i+=k<<5;
                n=0;
                b4=ed->ov->buf;
                ed->selbuf=malloc((ed->rectright-ed->rectleft)*(ed->rectbot-ed->recttop)<<1);
                for(o=0;o<l;o+=32) for(m=0;m<j;m++) ed->selbuf[n++]=b4[m+i+o];
            };
        }
        
        if(ed->dtool)
            ReleaseCapture();
        
        ed->dtool=0;
        
        break;
    
    case WM_PAINT:
        
        OverworldMap_OnPaint(ed, win);
        
        break;
    
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

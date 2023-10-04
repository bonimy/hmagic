
#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"
#include "Callbacks.h"

#include "HMagicUtility.h"

#include "WorldMapLogic.h"

// For map_ind.
#include "OverworldEdit.h"

// =============================================================================

static int
Fixscrollpos(unsigned char*rom,int x,int y,int sx,int sy,int cx,int cy,int dp,int m,int l,int door1,int door2)
{
    int j,k,n,o,p,q,r,s,t,u;
    r=((short*)(rom+x))[m];
    s=((short*)(rom+y))[m];
    if(door1) if(((unsigned short*)(rom+door2))[m]+1>=2) s+=39; else s+=19;
    n=rom[0x125ec + (r>>9)+(s>>9<<3)]+(l&64);
    j=((short*)(rom+sx))[m];
    k=((short*)(rom+sy))[m];
    o=(n&56)<<6;
    p=(n&7)<<9;
    q=rom[0x12844 + n]?1024:512;
    t=((short*)(rom+cy))[m];
    u=((short*)(rom+cx))[m];
    j+=r-u;
    k+=s-t;
    t=s;
    u=r;
    if(k<o) {
        t+=o-k;
        k=o;
    }
    if(k>o+q-256) {
        t+=o-k+q-256;
        k=o+q-256;
    }
    if(j<p) {
        u+=p-j;
        j=p;
    }
    if(j>p+q-240) {
        u+=p-j+q-240;
        j=p+q-240;
    }
    ((short*)(rom+cy))[m]=t;
    ((short*)(rom+cx))[m]=u;
    ((short*)(rom+sx))[m]=j;
    ((short*)(rom+sy))[m]=k;
    ((short*)(rom+dp))[m]=(((k-((n&56)<<6))&0xfff0)<<3)|(((j-((n&7)<<9))&0xfff0)>>3);
    if(door1) {
        if(((unsigned short*)(rom+door1))[m]+1>=2) ((short*)(rom+door1))[m]+=(((l-n)&7)<<6)+(((l&56)-(n&56))<<3);
        if(((unsigned short*)(rom+door2))[m]+1>=2) ((short*)(rom+door2))[m]+=(((l-n)&7)<<6)+(((l&56)-(n&56))<<3);
    }
    return n;
}

// =============================================================================

void
WorldMapDisplay_OnPaint(WMAPEDIT const * const p_ed,
                        HWND             const p_win)
{
    unsigned char const * const rom = p_ed->ew.doc->rom;
    
    unsigned char const * b2 = 0;
    unsigned char const * b4 = 0;
    
    int j = 0,
        k = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0,
        t = 0;
    
    RECT rc = empty_rect;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    HGDIOBJ oldobj2;
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    k = ((ps.rcPaint.right+31)&0xffffffe0);
    l = ((ps.rcPaint.bottom+31)&0xffffffe0);
    n = p_ed->mapscrollh<<5;
    o = p_ed->mapscrollv<<5;
    j = ps.rcPaint.top&0xffffffe0;
    
    b4 = p_ed->buf + ( (j + o) << 3);
    
    m = p_ed->ew.param ? 0x100 : 0x200;
    
    t = (j + o) >> 3;
    
    if(p_ed->selflag || p_ed->dtool==2 || p_ed->dtool==3)
        Wmapselectionrect(p_ed,&rc);
    
    for(;j<l;j+=32)
    {
        int i = ps.rcPaint.left & 0xffffffe0;
        int s = (i + n) >> 3;
        
        if(j + o >= m)
            break;
        
        b2 = b4 + s;
        
        for(;i<k;i+=32)
        {
            int q = 0;
            
            if(i + n >= m)
                break;
            
            for(q=0;q<32;q+=8)
            {
                int p = 0;
                
                for(p = 0; p < 32; p += 8)
                {
                    int r = b2[ (p >> 3) + (q << 3) ];
                    
                    if(p_ed->dtool == 3)
                    {
                        if(i + p >= rc.left && i + p < rc.right && j + q >= rc.top && j + q < rc.bottom)
                            r = p_ed->bs.sel;
                    }
                    else if(p_ed->selflag)
                    {
                        if(i + p >= rc.left && i + p < rc.right && j + q >= rc.top && j + q < rc.bottom)
                            r = p_ed->selbuf[s + (p >> 3) - p_ed->rectleft + (t + (q >> 3) - p_ed->recttop) * (p_ed->rectright - p_ed->rectleft)];
                    }
                    
                    Drawblock((OVEREDIT*) p_ed,
                              p,
                              q,
                              r,
                              0);
                }
            }
            
            b2 += 4;
            s += 4;
            
            Paintblocks(&(ps.rcPaint),hdc,i,j, (DUNGEDIT*) p_ed);
        }
        
        b4+=256;
        t+=4;
    }
    
    if(rc.right > m - n)
        rc.right = m - n;
    
    if(rc.bottom > m - o)
        rc.bottom = m - o;
    
    if(p_ed->dtool == 2)
    {
        if(rc.right > rc.left && rc.bottom > rc.top)
            FrameRect(hdc, &rc, white_brush);
    }
    
    if(p_ed->selflag)
        FrameRect(hdc,&rc,green_brush);
    
    oldobj2 = SelectObject(hdc, trk_font);
    
    SetBkMode(hdc, TRANSPARENT);
    
    if(p_ed->tool == 4)
    {
        text_buf_ty text_buf = { 0 };
        
        int i = 0;
        
        RECT text_r = { 0 };
        
        HGDIOBJ const oldobj = SelectObject(hdc, white_pen);
        
        for(i = 0; i < 64; i++)
        {
            j=((i&7)<<5)-n;
            k=((i&56)<<2)-o;
            if(m==0x200) j+=128,k+=128;
            l=rom[0x125ec + i];
            
            wsprintf(text_buf,
                     "%02X",
                     l);
            
            text_r.left = j + 8;
            text_r.top  = k + 8;
            
            Getstringobjsize(text_buf, &text_r);
            
            PaintSprName(hdc, j + 8, k + 8, &text_r, text_buf);
            
            if(l!=i)
                continue;
            
            l = rom[0x12844 + i] ? 64 : 32;
            
            MoveToEx(hdc,j+l,k,0);
            LineTo(hdc,j,k);
            LineTo(hdc,j,k+l);
        }
        
        j = 0;
        k = 0;
        
        if(m == 0x200)
            j = 128, k = 128;
        
        j -= n;
        k -= o;
        
        MoveToEx(hdc,j+256,k,0);
        LineTo(hdc,j+256,k+256);
        LineTo(hdc,j,k+256);
        
        SelectObject(hdc, oldobj);
    }
    else
    {
        int i = 0;
        
        int const l = p_ed->marknum;
        
        text_buf_ty text_buf = { 0 };
        
        RECT text_r = { 0 };
        
        HGDIOBJ const oldobj = SelectObject(hdc, purple_brush);
        
        if(l==9)
            i=7;
        else
            i=6;
        
        for( ; i >= 0; i--)
        {
            if( p_ed->tool == 3 && i == p_ed->selmark )
                continue;
            
            if( Getwmappos(p_ed, rom, i, &rc, n, o) )
                continue;
            
            Drawdot(hdc,&rc,m,n,o);
            
            Getwmapstring(p_ed,i, text_buf);
            
            text_r = rc;
            
            Getstringobjsize(text_buf, &rc);
            
            PaintSprName(hdc, rc.left, rc.top, &text_r, text_buf);
        }
        
        if(p_ed->tool == 3)
        {
            i = p_ed->selmark;
            
            if(i!=-1)
            {
                if(!Getwmappos(p_ed, rom,i,&rc,n,o))
                {
                    Getwmapstring(p_ed, i, text_buf);
                    Getstringobjsize(text_buf, &rc);
                    
                    if(rc.right > m - n)
                        rc.right = m - n;
                    
                    if(rc.bottom > m - o)
                        rc.bottom = m - o;
                    
                    FillRect(hdc, &rc, black_brush);
                    FrameRect(hdc, &rc, green_brush);
                    
                    PaintSprName(hdc, rc.left, rc.top, &rc, text_buf);
                }
            }
        }
        
        SelectObject(hdc,oldobj);
    }
    
    SelectObject(hdc, oldobj2);
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
wmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT*ed;
    int i,j,k,l,m,n,o,p,q;
    FDOC*doc;
    SCROLLINFO si;
    unsigned char*b2,*b4,*rom;
    short*b1,*b3;
    static int sprxpos[4]={0,32,0,32};
    static int sprypos[4]={0,0,32,32};
    static int sctpos[4]={0,64,4096,4160};
    int u[20];
    
    RECT rc = {0,0,0,0};
    
    POINT pt;
    switch(msg)
    {
    
    case WM_GETDLGCODE:
        
        return DLGC_WANTCHARS;
    
    case WM_CHAR:
        
        if(wparam == 26)
        {
            // Ctrl-Z
            // Undo, apparently? 
            wmapdlgproc(GetParent(win),WM_COMMAND,3007,0);
        }
        
        break;
    
    case WM_SIZE:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->ew.param?8:16;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,si.nMax,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,si.nMax,32);
        break;
    case WM_VSCROLL:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        ed->mapscrollv=Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,ed->ew.param?8:16,32);
        break;
    case WM_HSCROLL:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        ed->mapscrollh=Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,ed->ew.param?8:16,32);
        break;
    case WM_LBUTTONDOWN:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(ed->dtool) break;
        if(ed->tool!=1 && ed->selflag) {
            Wmapselectwrite(ed);
            Wmapselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
        }
        ed->dtool=ed->tool+1;
        SetCapture(win);
        SetFocus(win);
        goto mousemove;
    case WM_RBUTTONDOWN:
    case WM_MOUSEMOVE:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
mousemove:
        
        n=ed->mapscrollh<<5;
        o=ed->mapscrollv<<5;
        j=((short)lparam)+n;
        k=(lparam>>16)+o;
        q=ed->ew.param?256:512;
        if(j<0) j=0;
        if(j>=q) j=q-1;
        if(k<0) k=0;
        if(k>=q) k=q-1;
        if(msg==WM_RBUTTONDOWN) {
            if(ed->tool==3) {
                rom=ed->ew.doc->rom;
                if(ed->marknum==9) l=8; else l=7;
                if(q==512) j-=128,k-=128;
                if(j<0) j=0;
                if(k<0) k=0;
                if(j>255) j=255;
                if(k>255) k=255;
                
                for(i = 0; i < l; i++)
                    if( Getwmappos(ed,rom,i,&rc,n,o) )
                        break;
                
                if(i < l || ed->selmark != -1)
                {
                    HMENU menu = CreatePopupMenu();
                    
                    if(i<l)
                    {
                        text_buf_ty text_buf = { 0 };
                        
                        for(i = 0; i < l; i++)
                        {
                            if(Getwmappos(ed,rom,i,&rc,n,o))
                            {
                                wsprintf(text_buf,"Insert icon %d",i+1);
                                AppendMenu(menu, MF_STRING, i + 1, text_buf);
                            }
                        }
                    }
                    
                    if(ed->selmark != -1)
                        AppendMenu(menu, MF_STRING, 10, "Remove icon");
                    
                    GetCursorPos(&pt);
                    
                    i = TrackPopupMenu(menu,
                                       TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY,
                                       pt.x,
                                       pt.y,
                                       0,
                                       win,
                                       0);
                    
                    DestroyMenu(menu);
                    
                    if(i==10) {
                        Wmapselchg(ed,win,1);
                        ((short*)(rom+wmmark_ofs[ed->selmark]))[ed->marknum]=-1;
                        ed->selmark=-1;
                    } else if(i) {
                        Wmapselchg(ed,win,0);
                        i--;
                        b1=((short*)(rom+wmmark_ofs[i]))+ed->marknum;
                        ed->selmark=i;
                        *b1=j<<4;
                        b1[9]=k<<4;
                        ed->objx=j;
                        ed->objy=k;
                        Wmapselchg(ed,win,1);
                    }
                }
                break;
            }
            i=ed->buf[(j>>3)+(k>>3<<6)];
            SendMessage(GetParent(win),4000,i,0);
            break;
        }
        switch(ed->dtool) {
        case 1:
            if(msg==WM_LBUTTONDOWN) {
                memcpy(ed->undobuf,ed->buf,0x1000);
                ed->undomodf=ed->modf;
            }
            j&=0xfff8;
            k&=0xfff8;
            ed->buf[(j>>3)+(k<<3)]=ed->bs.sel;
            ed->modf=1;
            rc.left=j-n;
            rc.right=j-n+8;
            rc.top=k-o;
            rc.bottom=k-o+8;
            InvalidateRect(win,&rc,0);
            break;
        case 2:
        case 3:
            j>>=3;
            k>>=3;
            if(msg==WM_LBUTTONDOWN) {
                if(ed->selflag) if(j>=ed->rectleft && j<ed->rectright && k>=ed->recttop && k<ed->rectbot) {
                    ed->selx=j;
                    ed->sely=k;
                } else {
                    Wmapselectwrite(ed);
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    goto newsel;
                } else newsel: ed->rectleft=ed->rectright=j,ed->recttop=ed->rectbot=k;
            } else {
                if(ed->selflag) {
                    if(j==ed->selx && k==ed->sely) break;
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectleft+=j-ed->selx;
                    ed->rectright+=j-ed->selx;
                    ed->recttop+=k-ed->sely;
                    ed->rectbot+=k-ed->sely;
                    ed->selx=j;
                    ed->sely=k;
                } else {
                    if(j>=ed->rectleft) j++;
                    if(k>=ed->recttop) k++;
                    if(ed->rectright==j && ed->rectbot==k) break;
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectright=j,ed->rectbot=k;
                }
                Wmapselectionrect(ed,&rc);
                InvalidateRect(win,&rc,0);
            }
            break;
        case 4:
            rom=ed->ew.doc->rom;
            p=ed->marknum;
            if(msg==WM_LBUTTONDOWN) {
                q=ed->ew.param?0:128;
                Wmapselchg(ed,win,0);
                ed->selmark=-1;
                if(p==9) l=8; else l=7;
                for(i=0;i<l;i++) {
                    if(Getwmappos(ed,rom,i,&rc,n,o)) continue;
                    rc.left+=n;
                    rc.top+=o;
                    if(j>=rc.left && k>=rc.top && j<rc.left+16 && k<rc.top+16) {
                        ed->selmark=i;
                        ed->objx = (short) (rc.left - q);
                        ed->objy = (short) (rc.top - q);
                        ed->selx=j;
                        ed->sely=k;
                        Wmapselchg(ed,win,0);
                        return 0;
                    }
                }
                ed->dtool=0;
                ReleaseCapture();
                break;
            } else {
                i=ed->selmark;
                j-=ed->selx;
                k-=ed->sely;
                if(!(j||k)) break;
                Wmapselchg(ed,win,1);
                ed->objx+=j;
                ed->objy+=k;
                if(ed->objx<0) j-=ed->objx,ed->objx=0;
                if(p==9) {
                    l=rom[0x53763 + i]+(rom[0x5376b + i]<<8)+(j<<4);
                    rom[0x53763 + i]=l;
                    rom[0x5376b + i]=l>>8;
                    l=rom[0x53773 + i]+(rom[0x5377b + i]<<8)+(k<<4);
                    rom[0x53773 + i]=l;
                    rom[0x5377b + i]=l>>8;
                } else {
                    ((short*)(rom+wmmark_ofs[i]))[p]+=j<<4;
                    ((short*)(rom+wmmark_ofs[i]+18))[p]+=k<<4;
                }
                ed->selx+=j;
                ed->sely+=k;
                Wmapselchg(ed,win,1);
            }
            break;
        case 5:
            doc=ed->ew.doc;
            rom=doc->rom;
            if(q==512) j-=128,k-=128;
            if(j>=0 && k>=0 && j<256 && k<256) {
                i=(j>>5)+(k>>5<<3);
                if(rom[0x12844 + i]) {
                    i=rom[0x125ec + i];
                    if(doc->overworld[i].win) {
                        MessageBox(framewnd,"You can't change the map size while it is being edited.","Bad error happened",MB_OK);
                        goto maperror;
                    }
                    for(n=0;n<128;n+=64) {
                        m=0xe0000 + ((short*)(rom + 0xdc2f9))[n+i];
                        for(o=0;*(short*)(rom+m+o)!=-1;o+=3);
                        for(l=1;l<4;l++) ((short*)(rom + 0xdc2f9))[n+i+map_ind[l]]=m+o;
                    }
                    b2=malloc(512);
                    b4=malloc(512);
                    for(n=0;n<5;n++) {
                        u[n]=((short*)(rom + 0x4c881))[sprs[n]+i];
                    }
                    for(n=0;n<5;n++) {
                        p=((short*)(rom + 0x4c881))[sprs[n]+i];
                        for(o=0;o<n;o++) {
                            if(u[n]==u[o]) {
                                ((short*)(rom + 0x4c881))[sprs[n]+i]=((short*)(rom + 0x4c881))[sprs[o]+i];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+1]=((short*)(rom + 0x4c881))[sprs[o]+i+1];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+8]=((short*)(rom + 0x4c881))[sprs[o]+i+8];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+9]=((short*)(rom + 0x4c881))[sprs[o]+i+9];
                                goto nextspr;
                            }
                        }
                        memcpy(b2,rom + 0x50000 + p,512);
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            p=0;
                            for(o=0;b2[o]!=0xff;o+=3) {
                                if(((b2[o]&32)==sprypos[l]) &&
                                    ((b2[o+1]&32)==sprxpos[l])) {
                                    b4[p++]=b2[o]&31;
                                    b4[p++]=b2[o+1]&31;
                                    b4[p++]=b2[o+2];
                                }
                            }
                            Savesprites(ed->ew.doc,sprs[n]+m,b4,p);
                        }
nextspr:;
                    }
                    for(n=0;n<128;n+=64) {
                        memcpy(b2,rom + 0xe0000 + ((short*)(rom + 0xdc2f9))[n+i],512);
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            p=0;
                            for(o=0;*(short*)(b2+o)!=-1;o+=3) {
                                q=*(short*)(b2+o);
                                k=(q&0x1f80)>>7;
                                j=(q&0x7e)>>1;
                                if(k>=sprypos[l] && k<sprypos[l]+32 &&
                                    j>=sprxpos[l] && j<sprxpos[l]+32) {
                                    *(short*)(b4+p)=q&0xfbf;
                                    b4[p+2]=b2[o+2];
                                    p+=3;
                                }
                            }
                            if(Savesecrets(ed->ew.doc,n+m,b4,p)) Savesecrets(ed->ew.doc,n+m,0,0);
                        }
                    }
                    free(b4);
                    free(b2);
                    for(l=0;l<4;l++) {
                        m=i+map_ind[l];
                        rom[0x125ec + m]=m;
                        rom[0x12844 + m]=0;
                        rom[0x12884 + m]=1;
                        rom[0x1788d + m]=1;
                        rom[0x178cd + m]=1;
                        rom[0x4c635 + m]=2;
                        rom[0x4c675 + m]=2;
                        rom[0x4c6b5 + m]=2;
                        ((short*)(rom + 0x12634))[m]=96;
                        ((short*)(rom + 0x126b4))[m]=64;
                        ((short*)(rom + 0x12734))[m]=6144;
                        ((short*)(rom + 0x127b4))[m]=4096;
                        ((short*)(rom + 0x13ee2))[m]=(((short*)(rom + 0x128c4))[m]=(m&56)<<6)-224;
                        ((short*)(rom + 0x13f62))[m]=(((short*)(rom + 0x12944))[m]=(m&7)<<9)-256;
                    }
                    goto updlayout;
                } else if(((i&7)<7) && ((i&56)<56) && (!rom[0x12845 + i])
                    && (!rom[0x1284c + i]) && (!rom[0x1284d + i])) {
                    if(doc->overworld[i].win || doc->overworld[i+1].win || doc->overworld[i+8].win || doc->overworld[i+9].win ) {
                        MessageBox(framewnd,"You can't change the map size while it is being edited.","Bad error happened",MB_OK);
                        goto maperror;
                    }
                    for(n=0;n<5;n++) {
                        u[n]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i];
                        u[n+5]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+1];
                        u[n+10]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+8];
                        u[n+15]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+9];
                    }
                    for(n=0;n<5;n++)
                        for(o=0;o<n;o++)
                            if(((u[n]==u[o]&&u[n]!=0xcb41)
                                || (u[n+5]==u[o+5]&&u[n+5]!=0xcb41)
                                || (u[n+10]==u[o+10]&&u[n+10]!=0xcb41)
                                || (u[n+15]==u[o+15]&&u[n+15]!=0xcb41)) &&
                                !(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15])) {
                                MessageBox(framewnd,"The manner in which sprites sets are reused in the different parts of the game is not the same throughout the 4 areas.","Bad error happened",MB_OK);
                                goto maperror;
                            }
                    ((short*)(rom + 0x12634))[i]=96;
                    ((short*)(rom + 0x12636))[i]=96;
                    ((short*)(rom + 0x12644))[i]=4192;
                    ((short*)(rom + 0x12646))[i]=4192;
                    ((short*)(rom + 0x126b4))[i]=128;
                    ((short*)(rom + 0x126b6))[i]=128;
                    ((short*)(rom + 0x126c4))[i]=4224;
                    ((short*)(rom + 0x126c6))[i]=4224;
                    ((short*)(rom + 0x12734))[i]=6144;
                    ((short*)(rom + 0x12736))[i]=6208;
                    ((short*)(rom + 0x12744))[i]=6144;
                    ((short*)(rom + 0x12746))[i]=6208;
                    ((short*)(rom + 0x127b4))[i]=8192;
                    ((short*)(rom + 0x127b6))[i]=8256;
                    ((short*)(rom + 0x127c4))[i]=8192;
                    ((short*)(rom + 0x127c6))[i]=8256;
                    b4=malloc(512);
                    for(n=0;n<5;n++) {
                        for(o=0;o<n;o++) if(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15]) goto nextspr2;
                        p=0;
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            b2=rom + 0x50000 + ((short*)(rom + 0x4c881))[sprs[n]+m];
                            for(o=0;b2[o]!=0xff;o+=3) {
                                b4[p++]=b2[o]+sprypos[l];
                                b4[p++]=b2[o+1]+sprxpos[l];
                                b4[p++]=b2[o+2];
                            }
                        }
                        Savesprites(ed->ew.doc,sprs[n]+i+1,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i+8,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i+9,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i,b4,p);
nextspr2:;
                    }
                    for(n=0;n<5;n++)
                        for(o=0;o<n;o++)
                            if(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15])
                                ((short*)(rom + 0x4c881))[sprs[n]+i]=((short*)(rom + 0x4c881))[sprs[o]+i];
                    for(n=0;n<128;n+=64) {
                        p=0;
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            b2=rom + 0xe0000 + ((short*)(rom + 0xdc2f9))[n+m];
                            for(o=0;*(short*)(b2+o)!=-1;o+=3) {
                                *(short*)(b4+p)=(*(short*)(b2+o))+sctpos[l];
                                b4[p+2]=b2[o+2];
                                p+=3;
                            }
                        }
                        Savesecrets(ed->ew.doc,n+i+1,0,0);
                        Savesecrets(ed->ew.doc,n+i+8,0,0);
                        Savesecrets(ed->ew.doc,n+i+9,0,0);
                        Savesecrets(ed->ew.doc,n+i,b4,p);
                    }
                    free(b4);
                    for(l=0;l<4;l++) {
                        m=i+map_ind[l];
                        rom[0x125ec + m]=i;
                        rom[0x12844 + m]=32;
                        rom[0x12884 + m]=3;
                        rom[0x1788d + m]=0;
                        rom[0x178cd + m]=0;
                        rom[0x4c635 + m]=4;
                        rom[0x4c675 + m]=4;
                        rom[0x4c6b5 + m]=4;
                        ((short*)(rom + 0x13ee2))[m]=(((short*)(rom + 0x128c4))[m]=(i&56)<<6)-224;
                        ((short*)(rom + 0x13f62))[m]=(((short*)(rom + 0x12944))[m]=(i&7)<<9)-256;
                    }
updlayout:
                    b1=(short*)(rom + 0xdb96f);
                    b3=(short*)(rom + 0xdba71);
                    for(m=0;m<129;m++) {
                        l=b1[m];
                        if(l>=128) continue;
                        n=b3[m];
                        j=((l&7)<<9)+((n&0x7e)<<3);
                        k=((l&56)<<6)+((n&0x1f80)>>3);
                        o=rom[0x125ec + (((j>>9)+(k>>9<<3))&63)];
                        b1[m]=o|(l&64);
                        b3[m] =
                        ( ( ( j - ( (short*) (rom + 0x12944))[o] ) >> 3 ) & 0x7e)
                      + ( ( ( k - ( (short*) (rom + 0x128c4))[o] ) << 3 ) & 0x1f80);
                    }
                    for(m=0;m<79;m++) {
                        l=rom[0x15e28 + m];
                        if(l>=128) continue;
                        rom[0x15e28 + m]=Fixscrollpos(rom,0x160ef,0x16051,0x15fb3,0x15f15,0x1622b,0x1618d,0x15e77,m,l,0x16367,0x16405);
                    }
                    for(m=0;m<17;m++) {
                        l=((short*)(rom + 0x16ae5))[m];
                        if(l>=128) continue;
                        ((short*)(rom + 0x16ae5))[m]=Fixscrollpos(rom,0x16b8f,0x16b6d,0x16b4b,0x16b29,0x16bd3,0x16bb1,0x16b07,m,l,0,0);
                    }
                    b1=(short*)(rom + 0xdb826);
                    b3=(short*)(rom + 0xdb800);
                    for(m=0;m<19;m++) {
                        l=b1[m];
                        if(l>=128) continue;
                        n=b3[m] + 0x400;
                        j=((l&7)<<9)+((n&0x7e)<<3);
                        k=((l&56)<<6)+((n&0x1f80)>>3);
                        o=rom[0x125ec + (((j>>9)+(k>>9<<3))&63)];
                        b1[m]=o|(l&64);
                        
                        b3[m] =
                        ( ( ( j - ldle16b_i(rom + 0x12944, o) ) >> 3 ) & 0x7e )
                      + ( ( ( k - ldle16b_i(rom + 0x128c4, o) ) << 3 ) & 0x1f80 )
                      - 0x400;
                    }
                    ed->ew.doc->modf=1;
                    ed->objx=((i&7)<<5);
                    ed->objy=((i&56)<<2);
                    Wmapselchg(ed,win,1);
                }
            }
maperror:
            ed->dtool=0;
            ReleaseCapture();
            break;
        }
        break;
    case WM_LBUTTONUP:
        ed=(WMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(ed->dtool>1 && ed->dtool<4) {
            Wmapselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
            if(ed->rectleft<ed->rectright) i=ed->rectleft,j=ed->rectright;
            else i=ed->rectright,j=ed->rectleft;
            if(ed->recttop<ed->rectbot) k=ed->recttop,l=ed->rectbot;
            else k=ed->rectbot,l=ed->recttop;
            if(ed->dtool==3) {
                ed->undomodf=ed->modf;
                memcpy(ed->undobuf,ed->buf,0x1000);
                for(m=i;m<j;m++)
                    for(n=k;n<l;n++)
                        ed->buf[m+(n<<6)]=ed->bs.sel;
                ed->modf=1;
            } else if(!ed->selflag) {
                ed->rectleft=i;
                ed->rectright=j;
                ed->recttop=k;
                ed->rectbot=l;
                ed->selflag=1;
                ed->stselx=i;
                ed->stsely=k;
                j-=i;
                l-=k;
                l<<=6;
                i+=k<<6;
                n=0;
                b4=ed->buf;
                ed->selbuf=malloc((ed->rectright-ed->rectleft)*(ed->rectbot-ed->recttop));
                for(o=0;o<l;o+=64) for(m=0;m<j;m++) ed->selbuf[n++]=b4[m+i+o];
            }
        }
        if(ed->dtool) ReleaseCapture();
        ed->dtool=0;
        break;
    
    case WM_PAINT:
        
        ed = (WMAPEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(ed)
        {
            WorldMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    case WM_KEYDOWN:
        ed = (WMAPEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        i=ed->selmark;
        if(ed->tool!=3 || i==-1 || ed->marknum==9) break;
        if(wparam>=65 && wparam<71) { wparam-=55; goto digit; }
        if(wparam>=48 && wparam<58) {
            wparam-=48;
digit:
            b1=((short*)(ed->ew.doc->rom+wmpic_ofs[i]))+ed->marknum;
            *b1<<=4;
            *b1|=wparam;
            Wmapselchg(ed,win,1);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}
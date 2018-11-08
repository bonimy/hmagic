
    #include "structs.h"

    #include "GdiObjects.h"

    #include "HMagicUtility.h"

    #include "prototypes.h"

    #include "TileMapLogic.h"

// =============================================================================

void Gettmapsize(unsigned char*b,RECT*rc,int*s,int*t,int*u)
{
    int k,l,m,n,o,p,q,r;
    k=(*b<<8)+b[1];
    m=b[2];
    l=((m&63)<<8)+b[3]+1;
    n=(k&31)+((k&1024)>>5);
    o=((k&992)>>5)+((k&14336)>>6);
    if(!s) s=t=u=&r;
    if(m&64) l++,*s=6; else *s=l+4;
    *t=k;
    *u=l;
    l>>=1;
    
    if(m & 128)
        p = n + 1, q = o + l;
    else
        p = n + l, q = o + 1;
    
    r = ( (p - 1) >> 5) - (n >> 5);
    if(r) q+=r,n-=(n&31),p=n+32;
    rc->left=n<<3;
    rc->top=o<<3;
    rc->right=p<<3;
    rc->bottom=q<<3;
}

// =============================================================================

void Tmapobjchg(TMAPEDIT*ed,HWND win)
{
    RECT rc;
    int i,j;
    
    if(ed->sel != -1)
    {
        Gettmapsize(ed->buf+ed->sel,&rc,0,0,0);
        ed->selrect=rc;
        rc.top&=511;
        rc.bottom&=511;
        i=ed->mapscrollh<<5;
        j=ed->mapscrollv<<5;
        rc.left-=i;
        rc.right-=i;
        rc.top-=j;
        rc.bottom-=j;
        InvalidateRect(win,&rc,0);
    }
}

// =============================================================================

void
TileMapDisplay_OnPaint(TMAPEDIT const * const p_ed,
                       HWND             const p_win)
{
    int k = 0,
        l = 0,
        n = 0,
        o = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    RealizePalette(hdc);
    
    k = ((ps.rcPaint.right + 31) & 0xffffffe0);
    l = ((ps.rcPaint.bottom + 31) & 0xffffffe0);
    n = p_ed->mapscrollh << 5;
    o = p_ed->mapscrollv << 5;
    
    if(l + o > 0x200)
        l = 0x200 - o;
    
    if(k + n > 0x200)
        k = 0x200 - n;
    
    Paintdungeon((DUNGEDIT*) p_ed,
                 hdc,
                 &(ps.rcPaint),
                 ps.rcPaint.left & 0xffffffe0,
                 ps.rcPaint.top & 0xffffffe0,
                 k,
                 l,
                 n,
                 o,
                 p_ed->nbuf);
    
    if(p_ed->sel != -1)
    {
        RECT rc;
        
        Gettmapsize(p_ed->buf + p_ed->sel, &rc, 0, 0, 0);
        
        rc.top    &= 511;
        rc.bottom &= 511;
        
        rc.left -= n;
        rc.top  -= o;
        
        rc.right  -= n;
        rc.bottom -= o;
        
        FrameRect(hdc,
                  &rc,
                  (p_ed->withfocus & 1) ? green_brush : gray_brush);
    }
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

// Tilemap... display portion window procedure?
LRESULT CALLBACK
tmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT*ed;
    SCROLLINFO si;
    HWND hc;
    int i,j,k,l,n,o,p;
    unsigned char*b;
    
    RECT rc;
    short bg_ids[3]={3002,3003,3006};
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS|DLGC_WANTARROWS;
    case WM_SIZE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=16;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,16,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,16,32);
        
        break;
    
    case WM_VSCROLL:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollv=Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,16,32);
        
        break;
    
    case WM_HSCROLL:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollh=Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,16,32);
        break;
    case WM_SETFOCUS:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withfocus|=1;
        Tmapobjchg(ed,win);
        break;
    case WM_KILLFOCUS:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withfocus&=-2;
        Tmapobjchg(ed,win);
        break;  
    
    case WM_PAINT:
        
        ed = (TMAPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            TileMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetFocus(win);
        o=(lparam&65535)+(ed->mapscrollh<<5);
        p=(lparam>>16)+(ed->mapscrollv<<5)+(ed->selbg<<9);
        if(msg==WM_RBUTTONDOWN) {
            if(ed->tool) goto draw;
            Tmapobjchg(ed,win);
            ed->len+=6;
            ed->buf=realloc(ed->buf,ed->len);
            
            i = ((o & 0xf8) >> 3) + ( ( (o & 0x100) + (p & 0xf8) ) << 2) + ((p & 0x700) << 3);
            
            if(i >= 0x2000)
                i += 0x4000;
            
            *(short*)(ed->buf+ed->len-7)=(i>>8)|(i<<8);
            *(short*)(ed->buf+ed->len-5)=256;
            *(short*)(ed->buf+ed->len-3)=0;
            ed->buf[ed->len-1] = u8_neg1;
            ed->sel=ed->len-7;
            ed->selofs=i;
            ed->sellen=1;
            Tmapobjchg(ed,win);
            break;
        }
        b=ed->buf;
        if(ed->sel!=-1 && o>=ed->selrect.left && o<ed->selrect.right && p>=ed->selrect.top && p<ed->selrect.bottom) goto movesel;
        Tmapobjchg(ed,win);
        ed->sel=-1;
        for(i=0;;) {
            if(b[i]>=128) break;
            Gettmapsize(b+i,&rc,&k,&l,&n);
            if(o>=rc.left && o<rc.right && p>=rc.top && p<rc.bottom) {
                ed->sel=i;
                ed->selofs=l;
                ed->sellen=n>>1;
            }
            i+=k;
        }
        if(ed->sel!=-1) {
movesel:
            if(ed->tool) o&=-9,p&=-9;
            ed->dragx=o;
            ed->dragy=p;
            ed->withfocus|=2;
            Tmapobjchg(ed,win);
            SetCapture(win);
            if(ed->tool) goto draw;
        }
        break;
    case WM_LBUTTONUP:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withfocus&2) ed->withfocus&=-3,ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        o=(lparam&65535)+(ed->mapscrollh<<5);
        p=(lparam>>16)+(ed->mapscrollv<<5)+(ed->selbg<<9);
        if(ed->withfocus&2) {
            if(o>ed->dragx+7 || o<ed->dragx || p>ed->dragy+7 || p<ed->dragy) {
draw:
                if(ed->tool) {
                    if(ed->buf[ed->sel+2]&64)
                    {
                        if(msg==WM_RBUTTONDOWN)
                        {
                            k=*(short*)(ed->buf+ed->sel+4);
                            goto getblk;
                        } else *(short*)(ed->buf+ed->sel+4)=ed->bs.sel+ed->bs.flags; goto updblk;
                    }
                    else if(ed->buf[ed->sel+2]&128)
                    {
                        if(p>=ed->selrect.top && p<ed->selrect.bottom)
                        {
                            if(msg==WM_RBUTTONDOWN)
                            {
                                k = ldle16b
                                (
                                    ed->buf + ed->sel + 4
                                  + ( ( ( p - ed->selrect.top ) >> 2) & -2 )
                                );
                                
                                goto getblk;
                            }
                            
                            stle16b
                            (
                                ed->buf + ed->sel + 4
                              + ( ( (p - ed->selrect.top) >> 2) & -2 ),
                                ed->bs.sel + ed->bs.flags
                            );
                            
                            goto updblk;
                        }
                    }
                    else
                    {
                        i = ( (o & 0xf8) >> 3 )
                          + ( ( (o & 0x100) + (p & 0xf8) ) << 2)
                          + ( (p & 0x700) << 3 );
                        
                        if(i >= 0x2000)
                            i += 0x4000;
                        
                        if(i>=ed->selofs && i<ed->selofs+ed->sellen)
                        {
                            if(msg==WM_RBUTTONDOWN)
                            {
                                k = *(short*)( ed->buf + ed->sel + 4 + ( (i - ed->selofs) << 1) );
getblk:
                                hc=GetDlgItem(ed->dlg,3001);
                                Changeblk8sel(hc,&(ed->bs));
                                ed->bs.sel=k;
                                ed->bs.flags=ed->bs.sel&0xfc00;
                                ed->bs.sel&=0x3ff;
                                
                                if((ed->bs.flags&0xdc00)!=ed->bs.oldflags)
                                {
                                    HPALETTE const
                                    oldpal = SelectPalette(objdc,
                                                           ed->bs.ed->hpal,
                                                           1);
                                    
                                    HPALETTE const
                                    oldpal2 = SelectPalette(ed->bs.bufdc,
                                                            ed->bs.ed->hpal,
                                                            1);
                                    
                                    ed->bs.oldflags=ed->bs.flags;
                                    
                                    for(i=0;i<256;i++)
                                        Updateblk8sel(&(ed->bs),i);
                                    
                                    SelectPalette(objdc,oldpal,1);
                                    SelectPalette(ed->bs.bufdc,oldpal2,1);
                                    
                                    InvalidateRect(hc,0,0);
                                }
                                else
                                    Changeblk8sel(hc,&(ed->bs));
                                
                                CheckDlgButton(ed->dlg,3011,(ed->bs.flags&0x2000)>>13);
                                CheckDlgButton(ed->dlg,3009,(ed->bs.flags&0x4000)>>14);
                                CheckDlgButton(ed->dlg,3010,(ed->bs.flags&0x8000)>>15);
                                
                                break;
                            }
                            
                            *(short*)(ed->buf + ed->sel + 4 + ( (i - ed->selofs) << 1))
                                = ed->bs.sel+ed->bs.flags;
updblk:
                            Updtmap(ed);
                            Tmapobjchg(ed,win);
                            ed->modf=1;
                        }
                    }
                    
                    ed->dragx = o & -9;
                    ed->dragy = p & -9;
                    
                    break;
                }
                
                i = (o - ed->dragx) >> 3;
                j = (p - ed->dragy) >> 3;
                
                if(!(i||j)) break;
                
                Tmapobjchg(ed,win);
                k=((ed->buf[ed->sel])<<8)+ed->buf[ed->sel+1];
                l=(k&31)+((k&1024)>>5)+i;
                n=((k&992)>>5)+((k&2048)>>6)+j;
                if(l<0 || l>63 || n<0 || n>63) ed->withfocus|=4,SetCursor(forbid_cursor); else {
                    n+=ed->selbg<<6;
                    k=(l&31)+((l&32)<<5)+((n&31)<<5)+((n&224)<<6);
                    if(k>=0x2000) k+=0x4000;
                    ed->buf[ed->sel]=k>>8;
                    ed->buf[ed->sel+1]=k;
                    ed->dragx+=i<<3;
                    ed->dragy+=j<<3;
                    ed->modf=1;
                    Updtmap(ed);
                    Tmapobjchg(ed,win);
                    if(ed->withfocus&4) {SetCursor(normal_cursor);ed->withfocus&=-5;}
                }
            }
        }
        break;
    case WM_KEYDOWN:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_RIGHT:
            Tmapobjchg(ed,win);
            i=ed->buf[ed->sel+2];
            j=((i&63)<<8)+ed->buf[ed->sel+3]+1;
            if(i&64) ed->sel+=6; else ed->sel+=j+4;
            if(ed->buf[ed->sel]>=128) ed->sel=-1;
            Tmapobjchg(ed,win);
            break;
        case VK_LEFT:
            if(ed->sel==-1) break;
            Tmapobjchg(ed,win);
            if(!ed->sel) i=-1; else for(i=0;;) {
                k=ed->buf[ed->sel+2];
                j=((k&63)<<8)+ed->buf[ed->sel+3]+1;
                if(i&64) l=i+6;
                else l=i+4+j;
                if(l==ed->sel) break;
            }
            ed->sel=i;
            Tmapobjchg(ed,win);
            break;
        }
        break;
    case WM_CHAR:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(wparam>=96) wparam-=32;
        switch(wparam) {
        case '1': case '2': case '3':
            if(ed->sel==-1) break;
            i=ed->buf[ed->sel];
            ed->buf[ed->sel]=(i&15)|"\0\20\140"[wparam-49];
            CheckDlgButton(ed->dlg,3002,wparam==49);
            CheckDlgButton(ed->dlg,3003,wparam==50);
            CheckDlgButton(ed->dlg,3006,wparam==51);
            SendMessage(ed->dlg,WM_COMMAND,bg_ids[wparam-49],0);
            Updtmap(ed);
            Tmapobjchg(ed,win);
            break;
        case 8:
            Tmapobjchg(ed,win);
            Gettmapsize(ed->buf+ed->sel,&rc,&k,&l,&n);
            ed->len-=k;
            memcpy(ed->buf+ed->sel,ed->buf+ed->sel+k,ed->len-ed->sel);
            ed->buf=realloc(ed->buf,ed->len);
            ed->sel=-1;
            Updtmap(ed);
            break;
        case 'M':
            if(ed->sel!=-1) {
                Tmapobjchg(ed,win);
                ed->buf[ed->sel+2]^=128;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            break;
        case ',':
            if(ed->sel!=-1) {
                Tmapobjchg(ed,win);
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]-1;
                if(!j) break;
                if(!(i&64)) {
                    memcpy(ed->buf+ed->sel+4+j,ed->buf+ed->sel+6+j,ed->len-ed->sel-6-j);
                    ed->buf=realloc(ed->buf,ed->len-=2);
                }
                j--;
                ed->sellen--;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j;
                Updtmap(ed);
                ed->modf=1;
            }
            break;
        case '.':
            if(ed->sel!=-1) {
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]+3;
                if(!(i&64)) {
                    ed->buf=realloc(ed->buf,ed->len+=2);
                    memmove(ed->buf+ed->sel+4+j,ed->buf+ed->sel+2+j,ed->len-4-j-ed->sel);
                    *(short*)(ed->buf+ed->sel+2+j)=0;
                }
                j--;
                ed->sellen++;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            break;
        case '-':
            if(ed->sel!=-1) {
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]-1;
                if(i&64) {
                    j++;
                    ed->buf=realloc(ed->buf,ed->len+=j),memmove(ed->buf+ed->sel+6+j,ed->buf+ed->sel+6,ed->len-ed->sel-6-j);
                    for(k=j;k;k-=2) *(short*)(ed->buf+ed->sel+4+k)=*(short*)(ed->buf+ed->sel+4);
                } else memcpy(ed->buf+ed->sel+6,ed->buf+ed->sel+6+j,ed->len-ed->sel-6-j),ed->buf=realloc(ed->buf,ed->len-=j),j--;
                i^=64;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j+1;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            
            break;
        }
        
        break;
    
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================
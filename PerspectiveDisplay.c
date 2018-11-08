
    #include "structs.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "PerspectiveLogic.h"

// =============================================================================

static int
inpoly(int x,int y,POINT*pts,int k)
{
    int i,j,l,m,n=0;
    for(i=0;i<k;i++) {
        j=i+1;
        if(j==k) j=0;
        if(pts[j].x==pts[i].x) continue;
        if(pts[j].x<pts[i].x) l=pts[j].x,m=pts[i].x;
        else l=pts[i].x,m=pts[j].x;
        if(x>=l && x<m && (y>pts[i].y+(x-pts[i].x)*(pts[j].y-pts[i].y)/(pts[j].x-pts[i].x))) n^=1;
    }
    return n;
}

// =============================================================================

static void
rotvec2d(POINT *vec, POINT *pt)
{
    int a=vec->x,b=vec->y,c,d,e;
    
    if(a == 0 && b == 0)
        return;
    
    c = (int) sqrt(1048576 / (a * a + b * b));
    d = ((pt->x*a)-(pt->y*b))*c>>10;
    e = ((pt->y*a)+(pt->x*b))*c>>10;
    pt->x=d;
    pt->y=e;
}

// =============================================================================

static void
pnormal(int x1,int y1,int z1,int x2,int y2,int z2,int x3,int y3,int z3)
{
    static int a,b,c,d;
    
    a = ( (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1) ) >> 1;
    b = ( (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1) ) >> 1;
    c = ( (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1) ) >> 1;
    
    if(a==0 && b==0 && c==0) {rptx=0,rpty=0,rptz=1023;return;}
    
    d = (int) sqrt(a*a+b*b+c*c);
    
    rptx=a*1024/d;
    rpty=b*1024/d;
    rptz=c*1024/d;
}

// =============================================================================

static int
fronttest(POINT*pt,POINT*pt2,POINT*pt3)
{
    if(pt2->x==pt->x)
        if(pt2->y<pt->y) return pt3->x<pt->x; else return pt3->x>=pt->x;
    if(pt2->x>pt->x)
        return pt3->y<pt->y+(pt3->x-pt->x)*(pt2->y-pt->y)/(pt2->x-pt->x);
    return pt3->y>=pt->y+(pt3->x-pt->x)*(pt2->y-pt->y)/(pt2->x-pt->x);
}

// =============================================================================

static void
Get3dpt(PERSPEDIT*ed,int x,int y,int z,POINT*pt)
{
    int cx,cy,cz,sx,sy,sz,a,b,c,d;
    cx=cost[ed->xrot];
    cy=cost[ed->yrot];
    cz=cost[ed->zrot];
    sx=cost[(ed->xrot+192)&255];
    sy=cost[(ed->yrot+192)&255];
    sz=cost[(ed->zrot+192)&255];
    
    a = (cy * x - sy * z) >> 14; // A=X1
    b = (cy * z + sy * x) >> 14; // B=Z1
    c = (cx * b - sx * y) >> 14; // C=Z2
    b = (cx * y + sx * b) >> 14; // B=Y2
    d = (cz * a - sz * b) >> 14; // D=X3
    a = (cz * b + sz * a) >> 14; // A=Y3
    
    c+=250;
//  a=cx*x-sx*y>>14;
//  b=cx*y+sx*x>>14;
//  c=cz*z-sz*b>>14;
//  b=cz*b+sz*z>>14;
//  d=cy*a-sy*c>>14;
//  a=(cy*c+sy*a>>14)+250;
    pt->x=d*ed->scalex/c+(ed->width>>1);
    pt->y=a*ed->scaley/c+(ed->height>>1);
    rptx=d;
    rpty=a;
    rptz=c;
}

// =============================================================================

/*
void Perspselchg(PERSPEDIT*ed,HWND win)
{
    RECT rc;
    POINT pt;
    int j;
    if(ed->selpt==-1) return;
    j=*(unsigned short*)(ed->buf+2+ed->objsel)- 0xff8c + 3*ed->selpt;
    Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],&pt);
    rc.left=pt.x-3;
    rc.right=pt.x+3;
    rc.top=pt.y-3;
    rc.bottom=pt.y+3;
    InvalidateRect(win,&rc,1);
    rc.left=0;
    rc.right=200;
    rc.top=0;
    rc.bottom=24;
    InvalidateRect(win,&rc,1);
}
*/

// =============================================================================

extern LRESULT CALLBACK
perspdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PERSPEDIT*ed;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;
    int i,j,k,l,m,n,o = 0,q,r;
    short p[3];
    const static char a[8]={0,1,2,0,1,2};
    const static char c[8]={2,0,0,2,0,0};
    const static char d[8]={1,2,1,1,2,1};
    char e[4]={0,0,0,0};
    const static short b[6]={-128,-128,128,128,128,-128};
    const static char xyz_text[]="XYZ";
    POINT pt,pt2,pt3,pts[16],pts2[16];
    int rx[16],ry[16],rz[16];
    HBRUSH oldbrush,oldbrush2,oldbrush3;
    switch(msg) {
    case WM_SIZE:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        ed->width=lparam&65535;
        ed->height=lparam>>16;
updscale:
        ed->scalex=ed->width*ed->enlarge>>8;
        ed->scaley=ed->height*ed->enlarge>>8;
        goto upddisp;
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS+DLGC_WANTARROWS;
    case WM_KEYDOWN:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!(lparam&0x1000000)) switch(wparam) {
        case VK_RIGHT: goto moveright;
        case VK_LEFT: goto moveleft;
        case VK_DOWN: goto movedown;
        case VK_UP: goto moveup;
        }
        switch(wparam) {
        case VK_ADD:
            ed->enlarge+=4;
            if(ed->enlarge>640) ed->enlarge=640;
            goto updscale;
        case VK_SUBTRACT:
            ed->enlarge-=4;
            if(ed->enlarge<12) ed->enlarge=12;
            goto updscale;
        case VK_NUMPAD2:
movedown:
            l=1;
            m=-1;
updpos:
            if(!ed->tool) {
                if(ed->selpt==-1) break;
                if(GetKeyState(VK_CONTROL)&128) m<<=2;
                j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
                ed->buf[j+l]+=m;
                ed->modf=1;
                goto upddisp;
            }
            break;
        case VK_NUMPAD4:
moveleft:
            l=0;
            m=-1;
            goto updpos;
        case VK_NUMPAD6:
moveright:
            l=0;
            m=1;
            goto updpos;
        case VK_NUMPAD8:
moveup:
            l=1;
            m=1;
            goto updpos;
        case 'A':
            l=2;
            m=-1;
            goto updpos;
        case 'Z':
            l=2;
            m=1;
            goto updpos;
        case 'Q':
            ed->zrot-=4;
            goto upd;
        case 'W':
            ed->zrot+=4;
            goto upd;
        case VK_UP:
            ed->xrot+=4;
            goto upd;
        case VK_DOWN:
            ed->xrot-=4;
            goto upd;
        case VK_RIGHT:
            ed->yrot+=4;
upd:
            ed->xrot&=255;
            ed->yrot&=255;
            ed->zrot&=255;
upddisp:
            InvalidateRect(win,0,1);
            break;
        case VK_LEFT:
            ed->yrot-=4;
            goto upd;
        case VK_BACK:
            if(ed->tool==1) {
                if(ed->newptp==-1) break;
                ed->newptp=-1;
                goto upddisp;
            }
            if(!ed->tool) {
                if(ed->selpt==-1) break;
                j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
                memcpy(ed->buf+j,ed->buf+j+3,ed->len-j-3);
                *(unsigned short*)(ed->buf+4+ed->objsel)-=3;
                ed->buf[ed->objsel]--;
                if(!ed->objsel) {
                    *(unsigned short*)(ed->buf+8)-=3;
                    *(unsigned short*)(ed->buf+10)-=3;
                }
                ed->len-=3;
                k=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
                for(i=ed->buf[ed->objsel+1];i;i--) {
                    l=ed->buf[k];
                    for(m=0;m<l;m++) if(ed->buf[m+k+1]==ed->selpt) {
                        memcpy(ed->buf+k,ed->buf+k+l+2,ed->len-k-l-2);
                        ed->buf[ed->objsel+1]--;
                        if(!ed->objsel) {
                            *(unsigned short*)(ed->buf+8)-=l+2;
                            *(unsigned short*)(ed->buf+10)-=l+2;
                        }
                        ed->len-=l+2;
                        goto delnext;
                    } else if(ed->buf[m+k+1]>ed->selpt) ed->buf[m+k+1]--;
                    k+=l+2;
delnext:;
                }
                if(ed->selpt==ed->buf[ed->objsel]) ed->selpt--;
                ed->modf=1;
                goto updsize;
            } else if(ed->newlen) {
                ed->newlen--;
                if(ed->newlen) ed->selpt=ed->newface[ed->newlen-1];
                else ed->selpt=-1;
                goto upddisp;
            }
            break;
        }
        break;
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        k=lparam&65535;
        l=lparam>>16;
        switch(ed->tool) {
        case 2:
            if(ed->len+ed->newlen+2>=116) break;
            if(ed->newlen==16) break;
            goto addf;
        case 0:
addf:
            q=ed->selpt;
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8f + ed->buf[ed->objsel]*3;
            for(i=ed->buf[ed->objsel]-1;i>=0;i--) {
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],&pt);
                if(k>=pt.x-3 && k<pt.x+3 && l>=pt.y-3 && l<pt.y+3) {
                    if(ed->tool==2) {
                        for(l=0;l<ed->newlen;l++) if(ed->newface[l]==i) {
                            if(ed->newlen<3) return 0;
                            if(l) return 0;
                            if(!ed->objsel) {
                                o=*(unsigned short*)(ed->buf+8) - 0xff8c;
                                memcpy(ed->buf+o+ed->newlen+2,ed->buf+o,ed->len-o);
                                *(short*)(ed->buf+8)+=ed->newlen+2;
                                *(short*)(ed->buf+10)+=ed->newlen+2;
                            } else o=ed->len;
                            ed->buf[o]=ed->newlen;
                            ed->buf[o+ed->newlen+1]=0;
                            memcpy(ed->buf+o+1,ed->newface,ed->newlen);
                            ed->len+=ed->newlen+2;
                            ed->newlen=0;
                            ed->buf[ed->objsel+1]++;
                            ed->modf=1;
                            goto updsize;
                        }
                        ed->newface[ed->newlen++]=i;
                    }
                    ed->selpt=i;
                    goto upddisp;
                }
                j-=3;
            }
            if(!ed->tool) ed->selpt=-1;
            if(ed->selpt!=q) goto upddisp;
            break;
        case 1:
            if(ed->len>113) break;
            k-=ed->width>>1;
            l-=ed->height>>1;
            for(j=0;j<6;j++) {
                i=a[j];
                p[i]=b[j];
                n=c[j];
                o=d[j];
                p[n]=-128;
                p[o]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                rx[0]=rptx;
                ry[0]=rpty;
                rz[0]=rptz;
                p[n]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt2);
                rx[1]=rptx;
                ry[1]=rpty;
                rz[1]=rptz;
                p[n]=-128;
                p[o]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt3);
                rx[2]=rptx;
                ry[2]=rpty;
                rz[2]=rptz;
                if((j<3) ^fronttest(&pt,&pt2,&pt3)) {
                    pnormal(rx[0],ry[0],rz[0],rx[1],ry[1],rz[1],rx[2],ry[2],rz[2]);
                    m=k*rptx/ed->scalex+l*rpty/ed->scaley+rptz;
                    if(!m) continue;
                    
                    m=(rx[0]*rptx+ry[0]*rpty+rz[0]*rptz)/m;
                    
                    q =
                    (
                        ( (rx[1] - rx[0]) * (m * k / ed->scalex - rx[0]) )
                      + ( (ry[1] - ry[0]) * (m * l / ed->scaley - ry[0]) )
                      + ( (rz[1] - rz[0]) * (m - rz[0]) )
                      - 32768
                    ) >> 8;
                    
                    r =
                    (
                        ( (rx[2] - rx[0]) * (m * k / ed->scalex - rx[0]) )
                      + ( (ry[2] - ry[0]) * (m * l / ed->scaley - ry[0]) )
                      + ( (rz[2] - rz[0]) * (m - rz[0]) )
                      - 32768
                    ) >> 8;
                    
                    if(q<-128 || q>127 || r<-128 || r>127) continue;
                    if(ed->newptp==-1 || ed->newptp==a[j]) {
                        ed->newptp=a[j];
                        ed->newptx=q;
                        ed->newpty=r;
                    } else {
                        o=*(unsigned short*)(ed->buf+ed->objsel+4) - 0xff8c;
                        memcpy(ed->buf+o+3,ed->buf+o,ed->len-o);
                        ed->buf[o+ed->newptp]=(c[ed->newptp]==a[j])?r:q;
                        ed->buf[o+c[ed->newptp]]=ed->newptx;
                        ed->buf[o+d[ed->newptp]]=ed->newpty;
                        ed->buf[o+1]=-ed->buf[o+1];
                        ed->selpt=ed->buf[ed->objsel]++;
                        *(short*)(ed->buf+ed->objsel+4)+=3;
                        if(!ed->objsel) {
                            *(short*)(ed->buf+8)+=3;
                            *(short*)(ed->buf+10)+=3;
                        }
                        ed->len+=3;
                        ed->newptp=-1;
                        ed->modf=1;
                        goto updsize;
                    }
                    goto upddisp;
                }
            }
            break;
        case 3:
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c;
            for(i=0;i<ed->buf[ed->objsel];i++) {
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],pts+i);
                j+=3;
            }
            j=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
            n=-1;
            for(i=ed->buf[ed->objsel+1];i;i--) {
                m=ed->buf[j++];
                if(fronttest(pts+ed->buf[j],pts+ed->buf[j+1],pts+ed->buf[j+2])) {
                    for(q=0;q<m;q++) pts2[q]=pts[ed->buf[j+q]];
                    if(inpoly(k,l,pts2,m)) {
                        n=j;
                        o=m;
                    }
                }
                j+=m+1;
            }
            
            if(n != -1)
            {
                text_buf_ty buf; 
                
                memcpy(ed->buf + n - 1, ed->buf + n + o + 1, ed->len - n - o - 1);
                ed->len-=o+2;
                ed->buf[ed->objsel+1]--;
                
                if(!ed->objsel)
                {
                    *(short*) (ed->buf + 8) -= o + 2;
                    *(short*) (ed->buf + 10) -= o + 2;
                }
                
                ed->modf = 1;
                
updsize:
                
                wsprintf(buf, "Free: %d", 116 - ed->len);
                SetDlgItemText(ed->dlg, 3003, buf);
                
                goto upddisp;
            }
        }
        
        break;
    
    case WM_PAINT:
        
        ed = (PERSPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        hdc = BeginPaint(win,&ps);
        
        oldbrush=GetCurrentObject(hdc,OBJ_PEN);
        oldbrush2=GetCurrentObject(hdc,OBJ_BRUSH);
        oldbrush3=SelectObject(hdc,trk_font);
        
        SetBkMode(hdc,TRANSPARENT);
        
        for(j = 0; j < 6; j++)
        {
            i=a[j];
            p[i]=b[j];
            k=c[j];
            l=d[j];
            p[k]=-128;
            p[l]=-128;
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            p[l]=128;
            Get3dpt(ed,p[0],p[1],p[2],&pt2);
            p[k]=128;
            Get3dpt(ed,p[0],p[1],p[2],&pt3);
            SelectObject(hdc,white_pen);
            
            if( (j < 3) ^ fronttest(&pt,&pt2,&pt3) )
            {
                text_buf_ty buf = { 0 };
                
                m=0;
                for(;;) {
                    if(e[l]) goto nextaxis;
                    if(p[i]<0) p[i]=-144; else p[i]=144;
                    p[k]=-128;
                    p[l]=-128;
                    Get3dpt(ed,p[0],p[1],p[2],&pt);
                    MoveToEx(hdc,pt.x,pt.y,0);
                    p[l]=128;
                    Get3dpt(ed,p[0],p[1],p[2],&pt2);
                    LineTo(hdc,pt2.x,pt2.y);
                    pt.x-=pt2.x;
                    pt.y-=pt2.y;
                    pt3.x=20;
                    pt3.y=8;
                    rotvec2d(&pt,&pt3);
                    LineTo(hdc,pt3.x+pt2.x,pt3.y+pt2.y);
                    MoveToEx(hdc,pt2.x,pt2.y,0);
                    pt3.x=20;
                    pt3.y=-8;
                    rotvec2d(&pt,&pt3);
                    LineTo(hdc,pt3.x+pt2.x,pt3.y+pt2.y);
                    p[l]=0;
                    Get3dpt(ed,p[0],p[1],p[2],&pt2);
                    buf[0]=xyz_text[l];
                    buf[1]=0;
                    Paintspr(hdc,pt2.x,pt2.y,0,0,ed->width);
                    e[l]=1;
nextaxis:
                    if(m==1) break;
                    m=k;
                    k=l;
                    l=m;
                    m=1;
                }
                continue;
            }
            for(m=-128;m<=128;m+=16) {
                p[k]=m;
                p[l]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[l]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
            for(m=-128;m<=128;m+=16) {
                p[l]=m;
                p[k]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[k]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
            if(ed->newptp!=-1 && ed->newptp!=i) {
                SelectObject(hdc,green_pen);
                p[ed->newptp]=-128;
                if(c[ed->newptp]==i) m=ed->newpty; else m=ed->newptx;
                if(ed->newptp==k) p[l]=m;
                else p[k]=m;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[ed->newptp]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
        }
        SelectObject(hdc,white_pen);
        j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c;
        for(i=0;i<ed->buf[ed->objsel];i++) {
            Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],pts+i);
            rx[i]=rptx;
            ry[i]=rpty;
            rz[i]=rptz;
            j+=3;
        }
        j=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
        for(i=ed->buf[ed->objsel+1];i;i--) {
            k=ed->buf[j++];
            l=ed->buf[j];
            m=ed->buf[j+1];
            n=ed->buf[j+2];
            if(fronttest(pts+l,pts+m,pts+n)) {
                pnormal(rx[l],ry[l],rz[l],rx[m],ry[m],rz[m],rx[n],ry[n],rz[n]);
                rptz=-rptz>>7;
                if(rptz<0) rptz=0;
                if(rptz>7) rptz=7;
                SelectObject(hdc,shade_brush[rptz]);
                for(l=0;l<k;l++) pts2[l]=pts[ed->buf[j+l]];
                Polygon(hdc,pts2,k);
            }
            j+=k+1;
        }
        for(i=0;i<ed->buf[ed->objsel];i++) {
            rc.left=pts[i].x-2;
            rc.right=pts[i].x+2;
            rc.top=pts[i].y-2;
            rc.bottom=pts[i].y+2;
            FillRect(hdc,&rc,(ed->selpt==i)?green_brush:red_brush);
        }
        if(ed->newlen>1) {
            SelectObject(hdc,blue_pen);
            MoveToEx(hdc,pts[ed->newface[0]].x,pts[ed->newface[0]].y,0);
            for(i=1;i<ed->newlen;i++) {
                k=ed->newface[i];
                LineTo(hdc,pts[k].x,pts[k].y);
            }
        }
        if(ed->tool==1 && ed->newptp!=-1) {
            SelectObject(hdc,blue_pen);
            p[ed->newptp]=b[ed->newptp];
            p[c[ed->newptp]]=ed->newptx;
            p[d[ed->newptp]]=ed->newpty;
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            MoveToEx(hdc,pt.x,pt.y,0);
            p[ed->newptp]=-b[ed->newptp];
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            LineTo(hdc,pt.x,pt.y);
        }
        
        if(ed->selpt!=-1)
        {
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
            if(!ed->tool)
            {
                extern char buffer[0x400];
                
                SelectObject(hdc,green_pen);
                wsprintf(buffer,"X: %02X Y: %02X Z: %02X",ed->buf[j]+128,ed->buf[j+1]+128,ed->buf[j+2]+128);
                Paintspr(hdc,0,0,0,0,ed->width);
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],-128,&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],128,&pt);
                LineTo(hdc,pt.x,pt.y);
                Get3dpt(ed,ed->buf[j],-128,ed->buf[j+2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,ed->buf[j],128,ed->buf[j+2],&pt);
                LineTo(hdc,pt.x,pt.y);
                Get3dpt(ed,-128,-ed->buf[j+1],ed->buf[j+2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,128,-ed->buf[j+1],ed->buf[j+2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
        }
        SelectObject(hdc,oldbrush);
        SelectObject(hdc,oldbrush2);
        SelectObject(hdc,oldbrush3);
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================
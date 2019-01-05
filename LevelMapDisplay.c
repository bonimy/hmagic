
    #include "structs.h"

    #include "GdiObjects.h"

    #include "LevelMapLogic.h"

    #include "prototypes.h"

// =============================================================================

static void
LevelMapDisplay_OnPaint(LMAPEDIT const * const p_ed,
                        HWND             const p_win)
{
    unsigned short const * b2 = 0;
    
    unsigned short const * b4 = 0;
    
    int k = 0,
        l = 0,
        j = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    k = ( (ps.rcPaint.right + 31) & 0xffffffe0 );
    l = ( (ps.rcPaint.bottom + 31) & 0xffffffe0 );
    j = ps.rcPaint.top & 0xffffffe0;
    b4 = p_ed->nbuf + (j << 2) + 0x6f;
    
    for( ; j < l; j += 32)
    {
        int i = ps.rcPaint.left&0xffffffe0;
        int s = i >> 3;
        
        if(j >= 0xe0)
            break;
        
        b2 = b4 + s;
        
        for( ; i < k; i += 32)
        {
            int q = 0;
            
            if(i >= 0x1f0)
                break;
            
            for(q=0;q<32;q+=8)
            {
                int p = 0;
                
                for(p=0;p<32;p+=8)
                {
                    Drawblock((OVEREDIT*) p_ed,p,q,b2[(p>>3)+(q<<2)],0);
                }
            }
            
            b2 += 4;
            s += 4;
            
            Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)p_ed);
        }
        
        b4 += 128;
    }
    
    if(p_ed->tool)
    {
        RECT rc;
        
        HGDIOBJ oldfont;
        
        k = (p_ed->basements + p_ed->curfloor) * 25;
        
        if(p_ed->tool==2 && p_ed->bosspos>=k && p_ed->bosspos<k+25)
        {
            HGDIOBJ const oldobj = SelectObject(hdc, red_brush);
            
            l=p_ed->bosspos-k;
            
            rc.left=((l%5)<<4)+20+(p_ed->bossofs>>8);
            rc.top=((l/5)<<4)+4+(p_ed->bossofs&255);
            rc.right=rc.left+16;
            rc.bottom=rc.top+16;
            
            Drawdot(hdc,&rc,0x1f0,0,0);
            SelectObject(hdc, oldobj);
        }
        
        oldfont = SelectObject(hdc, trk_font);
        SetBkMode(hdc,TRANSPARENT);
        
        for(j = 0; j < 5; j++)
        {
            int i = 0;
            
            for(i=0;i<5;i++)
            {
                l=p_ed->rbuf[k];
                rc.left=(i<<4)+24;
                rc.top=(j<<4)+8;
                
                if(p_ed->tool==1 && k==p_ed->sel)
                {
                    HGDIOBJ const oldobj2 = SelectObject(hdc, green_pen);
                    HGDIOBJ const oldobj3 = SelectObject(hdc, black_brush);
                    
                    rc.right=rc.left+16;
                    rc.bottom=rc.top+16;
                    
                    Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom);
                    
                    SelectObject(hdc,oldobj2);
                    SelectObject(hdc,oldobj3);
                }
                
                if(l != 15)
                {
                    extern char buffer[0x400];

                    wsprintf(buffer, "%02X", l);
                    
                    Paintspr(hdc,rc.left,rc.top,0,0,0x1f0);
                }
                
                k++;
            }
        }
        
        SelectObject(hdc, oldfont);
    }
    
    if( (p_ed->disp & 1) && !p_ed->tool )
    {
        int i = 0;
        
        HGDIOBJ const oldobj = SelectObject(hdc, white_pen);
        
        // -----------------------------
        
        k = (p_ed->basements + p_ed->curfloor) * 25;
        
        for(i = 0; i < 5; i++)
        {
            if((p_ed->rbuf[k] != 15))
            {
                MoveToEx(hdc,(i<<4)+24,8,0);
                LineTo(hdc,(i<<4)+41,8);
            }
            k++;
        }
        
        for(j = 1; j < 5; j++)
        {
            for(i=0;i<5;i++) {
                if((p_ed->rbuf[k]==15)==(p_ed->rbuf[k-5]!=15)) {
                    MoveToEx(hdc,(i<<4)+24,(j<<4)+8,0);
                    LineTo(hdc,(i<<4)+41,(j<<4)+8);
                }
                k++;
            }
        }
        
        k-=5;
        
        for(i=0;i<5;i++) {
            if((p_ed->rbuf[k]!=15)) {
                MoveToEx(hdc,(i<<4)+24,88,0);
                LineTo(hdc,(i<<4)+41,88);
            }
            k++;
        }
        
        k-=25;
        
        for(j=0;j<5;j++)
        {
            int i = 0;
            
            if((p_ed->rbuf[k]!=15))
            {
                MoveToEx(hdc,24,(j<<4)+8,0);
                LineTo(hdc,24,(j<<4)+24);
            }
            
            k++;
            
            for(i=1;i<5;i++)
            {
                if((p_ed->rbuf[k]==15)==(p_ed->rbuf[k-1]!=15)) {
                    MoveToEx(hdc,(i<<4)+24,(j<<4)+8,0);
                    LineTo(hdc,(i<<4)+24,(j<<4)+24);
                }
                k++;
            }
            
            if((p_ed->rbuf[k-1]!=15))
            {
                MoveToEx(hdc,104,(j<<4)+8,0);
                LineTo(hdc,104,(j<<4)+24);
            }
        }
        
        SelectObject(hdc, oldobj);
    }
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
lmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k = 0,l = 0,p,q,s;
    
    LMAPEDIT *ed;
    
    RECT rc;
    
    switch(msg)
    {
    
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS | DLGC_WANTARROWS;
    
    case WM_CHAR:
        ed=(LMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        if(wparam>=0x40) wparam&=0xdf;
        switch(wparam) {
        case 8:
            if(ed->tool==2) goto setboss;
            if(!ed->tool) break;
            if(ed->rbuf[ed->sel]==15) break;
            k=0;
            for(j=0;j<ed->sel;j++) if(ed->rbuf[j]!=15) k++;
            ed->len--;
            memcpy(ed->buf+k,ed->buf+k+1,ed->len-k);
            ed->buf=realloc(ed->buf,ed->len);
            ed->rbuf[ed->sel]=15;
            goto updblk;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            wparam-=55;
            goto digit;
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
            wparam-=48;
digit:
            if(ed->tool!=1) break;
            i=ed->rbuf[ed->sel];
            if(i==15) {
                k=0;
                for(j=0;j<ed->sel;j++) if(ed->rbuf[j]!=15) k++;
                ed->len++;
                ed->buf=realloc(ed->buf,ed->len);
                memmove(ed->buf+k+1,ed->buf+k,ed->len-k-1);
                ed->buf[k]=0;
            }
            i=((i<<4)+wparam)&255;
            if(i==15) i=255;
            ed->rbuf[ed->sel]=i;
updblk:
            ed->modf=1;
            Paintfloor(ed);
            s=(ed->curfloor+ed->basements)*25;
            if(ed->sel>=s && ed->sel<s+25) {
                rc.left=(((ed->sel-s)%5)<<4)+24;
                rc.top=(((ed->sel-s)/5)<<4)+8;
                rc.right=rc.left+16;
                rc.bottom=rc.top+16;
                InvalidateRect(win,&rc,0);
            }
        }
        return 0;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(LMAPEDIT*)GetWindowLongPtr(win,GWLP_USERDATA);
        k=lparam&65535;
        l=lparam>>16;
        if(k>=24 && k<104 && l>=8 && l<88) {
            k-=24;
            l-=8;
setboss:
            s=(ed->curfloor+ed->basements)*25;
            switch(ed->tool) {
            case 1:
                j=ed->sel;
                p=q=0;
                goto chgloc;
            case 2:
                j=ed->bosspos;
                p=(ed->bossofs>>8)-4;
                q=(ed->bossofs&255)-4;
chgloc:
                if(j>=s && j<s+25) {
                    rc.left=(((j-s)%5)<<4)+24+p;
                    rc.top=(((j-s)/5)<<4)+8+q;
                    rc.right=rc.left+16;
                    rc.bottom=rc.top+16;
                    InvalidateRect(win,&rc,0);
                }
            }
            s+=(k>>4)+(l>>4)*5;
            switch(ed->tool) {
            case 0:
                j=ed->rbuf[s];
                if(j==15) break;
                q=0;
                for(i=0;;i++) {
                    p=ed->rbuf[i];
                    if(p!=15) if(p==j) break; else q++;
                }
                if(msg==WM_RBUTTONDOWN) {
                    lmapblkchg(win,ed);
                    ed->blksel=ed->buf[q];
                    lmapblkchg(win,ed);
                    break;
                }
                ed->buf[q] = (unsigned char) ed->blksel;
                Paintfloor(ed);
                ed->modf=1;
                k&=0xfff0;
                l&=0xfff0;
                break;
            case 1:
                ed->sel=s;
                k&=0xfff0;
                l&=0xfff0;
                break;
            case 2:
                if(msg==WM_CHAR) {
                    ed->bosspos=-1;
                    ed->bossroom=15;
                } else {
                    ed->bosspos=((ed->bossroom=ed->rbuf[s])!=15)?s:-1;
                    ed->bossofs=((k&15)<<8)+(l&15);
                    ed->modf=1;
                }
                if(ed->bossroom==15) return 0;
                k-=4;
                l-=4;
                break;
            }
            rc.left=k+24;
            rc.top=l+8;
            rc.right=rc.left+16;
            rc.bottom=rc.top+16;
            InvalidateRect(win,&rc,0);
        } else if(k<16 && l>=80 && l<88) {
            if(msg==WM_RBUTTONDOWN) {
                if(ed->basements) ed->basements--,ed->floors++,ed->curfloor++;
                else break;
            } else {
                if(ed->floors) ed->basements++,ed->floors--,ed->curfloor--;
                else break;
            }
            Paintfloor(ed);
            rc.left=0;
            rc.top=80;
            rc.right=16;
            rc.bottom=88;
            InvalidateRect(win,&rc,0);
            ed->modf=1;
        }
        
        break;
    
    case WM_PAINT:
        
        ed = (LMAPEDIT*) GetWindowLongPtr(win, GWLP_USERDATA);
        
        if(ed)
        {
            LevelMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================


    #include "structs.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "Wrappers.h"

    #include "AudioLogic.h"

    #include "TrackerLogic.h"

// =============================================================================

static char
*note_str[12] = {"C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B "};

static char
cmd_str[32][9] =
{
    "Instr.  ",
    "Pan     ",
    "Pansweep",
    "Vibrato ",
    "Vib.Off ",
    "Glob.Vol",
    "GV ramp ",
    "Speed   ",
    "Spd ramp",
    "G. Trans",
    "Transpos",
    "Tremolo ",
    "Trm.Off ",
    "Volume  ",
    "Vol.Ramp",
    "CallLoop",
    "Cmd F0  ",
    "PSlideTo",
    "PSlideFr",
    "Cmd F3  ",
    "Finetune",
    "Cmd F5  ",
    "Cmd F6  ",
    "Cmd F7  ",
    "Echo    ",
    "PSlide  ",
    "Cmd FA  ",
    "Cmd FB  ",
    "Cmd FC  ",
    "Cmd FD  ",
    "Cmd FE  ",
    "Cmd FF  "
};

// =============================================================================

void Trackchgsel(HWND win,RECT*rc,TRACKEDIT*ed)
{
    rc->top=(mark_start-ed->scroll)*textmetric.tmHeight;
    rc->bottom=(mark_end+1-ed->scroll)*textmetric.tmHeight;
    
    InvalidateRect(win,rc,1);
}

LRESULT CALLBACK
trackerproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT *ed, *ed2;
    HDC hdc;
    PAINTSTRUCT ps;
    HANDLE oldobj,oldobj2,oldobj3;
    SCROLLINFO si;
    SCMD*sc,*sc2,*sc3;
    FDOC*doc;
    RECT rc;
    
    const static int csel_l[]={48,72,96,168,192,216};
    const static int csel_r[]={64,88,160,184,208,232};
    int i, j = 0, k, l, m;
    
    switch(msg)
    {
    case WM_ACTIVATE:
        
        if(wparam == WA_ACTIVE)
            SetFocus(win);
        
        break;
    
    case WM_SIZE:
        
        ed = (TRACKEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        si.cbSize = sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->len+1;
        
        ed->page=si.nPage=(lparam>>16)/textmetric.tmHeight;
        
        SetScrollInfo(win,SB_VERT,&si,1);
        
        ed->scroll=Handlescroll(win,-1,ed->scroll,ed->page,SB_VERT,ed->len,textmetric.tmHeight);
        
        break;
    
    case WM_VSCROLL:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->scroll=Handlescroll(win,wparam,ed->scroll,ed->page,SB_VERT,ed->len,textmetric.tmHeight);
        break;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        SetFocus(win);
        SetCapture(win);
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withcapt=1;
selchg:
        j=(lparam>>16)/textmetric.tmHeight+ed->scroll;
        if(j>ed->len) j=ed->len;
        if(j<0) j=0;
        rc = HM_GetClientRect(win);
        if(msg!=WM_MOUSEMOVE) if(j<ed->len) {
            sc=ed->ew.doc->scmd+ed->tbl[j];     
            i=lparam&65535;
            if(wparam&MK_SHIFT) if(ed->csel==2) m=12; else m=16; else m=1;
            if(msg==WM_LBUTTONDOWN) m=-m;
            if(sc->cmd>=0xe0) l=op_len[sc->cmd - 0xe0]; else l=0;
            if(i>=48 && i<64) k=0;
            else if(i>=72 && i<88) k=1;
            else if(i>=96 && i<160) k=2;
            else if(l && i>=168 && i<184) k=3;
            else if(l>1 && i>=192 && i<208) k=4;
            else if(l>2 && i>=216 && i<232) k=5;
            else k=-1;
            goto dontgetrc2;
updfld:
            rc = HM_GetClientRect(win);
dontgetrc2:
            if(k==ed->csel && j==ed->sel) {
                switch(k) {
                case 0:
                    if(!(sc->flag&1)) sc->flag|=1; else sc->b1+=m;
                    if(!(sc->b1&127)) sc->b1+=m;
                    sc->b1&=0x7f;
                    goto modfrow;
                case 1:
                    if((sc->flag&3)!=3) sc->flag|=2; else sc->b2+=m;
                    if(!(sc->b2&127)) sc->b2+=m;
                    sc->b2&=0x7f;
                    goto modfrow;
                case 2:
                    if(sc->cmd<0xe0) {
                        sc->cmd+=m;
                        if(sc->cmd<0x80) sc->cmd+=0x4a;
                        else if(sc->cmd>=0xca) sc->cmd-=0x4a;
                    } else {
                        sc->cmd+=m;
                        if(sc->cmd<0x80) sc->cmd-=0x20;
                        else if(sc->cmd<0xe0) sc->cmd+=0x20;
                    }
                    goto modfrow;
                case 3:
                    sc->p1+=m;
                    goto modfrow;
                case 4:
                    sc->p2+=m;
                    goto modfrow;
                case 5:
                    sc->p3+=m;
                    goto modfrow;
                }
            } else {
                ed->csel=k;
            }
        } else ed->csel=-1;
setrow:
        if(ed->sel!=-1) {
            rc.top=(ed->sel-ed->scroll)*textmetric.tmHeight;
            rc.bottom=rc.top+textmetric.tmHeight;
            InvalidateRect(win,&rc,1);
        }
        ed->sel=j;
        goto dontgetrc;
updrow:
        rc = HM_GetClientRect(win);
dontgetrc:
        if(j>=ed->scroll+ed->page) {
            k=j-ed->page;
            goto scroll;
        }
        if(j<ed->scroll) {
            k=j;
scroll:
            ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(k<<16),ed->scroll,ed->page,SB_VERT,ed->len+1,textmetric.tmHeight);
        }
        rc.top=(j-ed->scroll)*textmetric.tmHeight;
        rc.bottom=rc.top+textmetric.tmHeight;
        InvalidateRect(win,&rc,1);
        break;
modfrow:
        ed->ew.doc->m_modf=1;
        goto updrow;
    case WM_MOUSEMOVE:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withcapt) goto selchg;
        break;
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withcapt) ReleaseCapture(),ed->withcapt=0;
        break;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_SYSKEYDOWN:
        if(!(lparam&0x20000000)) break;
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_F12:
            ed->debugflag=!ed->debugflag;
            InvalidateRect(win,0,1);
            break;
        case VK_F11:
            sc=ed->ew.doc->scmd;
            for(i=0;i<ed->len;i++) sc[ed->tbl[i]].flag&=-5;
            Getblocktime(ed->ew.doc,ed->tbl[0],0);
            InvalidateRect(win,0,1);
            break;
        case 'N':
            NewSR(ed->ew.doc,ed->ew.doc->sr[ed->ew.param].bank);
            break;
        case 'U':
            if(!mark_doc) break;
            goto unmark;
        case 'L':
            
            if(mark_doc!=ed->ew.doc || mark_sr!=ed->ew.param)
            {
                
            unmark:
                
                if(mark_doc)
                {
                    HWND w = mark_doc->sr[mark_sr].editor;
                    
                    if(w)
                    {
                        HWND sw = GetDlgItem(GetDlgItem(w, 2000), 3000);
                        rc = HM_GetClientRect(sw);
                        Trackchgsel(sw, &rc,
                                    (TRACKEDIT*) GetWindowLongPtr(sw, GWLP_USERDATA));
                    }
                }
                
                if(wparam=='U')
                {
                    mark_doc = 0;
                    break;
                }
                
                mark_doc=ed->ew.doc;
                mark_sr=ed->ew.param;
                mark_first=mark_last=ed->tbl[mark_start=mark_end=ed->sel];
                rc = HM_GetClientRect(win);
            }
            else
            {
                rc = HM_GetClientRect(win);
                Trackchgsel(win,&rc,ed);
                j=ed->sel;
                if(j>=ed->len) j=ed->len-1;
                if(ed->sel>=mark_start) mark_last=ed->tbl[mark_end=j];
                else mark_first=ed->tbl[mark_start=j];
            }
            
            Trackchgsel(win,&rc,ed);
            
            break;
        
        case 'M':
            if(!mark_doc) break;
            if(mark_doc!=ed->ew.doc) {
                MessageBox(framewnd,"Selection is in another file","Bad error happened",MB_OK);
                break;
            }
            mark_doc->m_modf=1;
            oldobj=mark_doc->sr[mark_sr].editor;
            if(oldobj) {
                oldobj=GetDlgItem(GetDlgItem(oldobj,2000),3000);
                InvalidateRect(oldobj,0,1);
                Updatesize(oldobj);
                ed2=(TRACKEDIT*)GetWindowLong(oldobj,GWL_USERDATA);
                
                CopyMemory(ed2->tbl + mark_start,
                           ed2->tbl + mark_end + 1,
                           (ed2->len - mark_end - 1) << 1);
                
                ed2->tbl=realloc(ed2->tbl,(ed2->len+=mark_start-1-mark_end)<<1);
            }
            sc=mark_doc->scmd;
            if(ed->sel)
                sc[i=ed->tbl[ed->sel-1]].next=mark_first; else i=-1,mark_doc->sr[ed->ew.param].first=mark_first;
            if(ed->sel<ed->len)
                sc[j=ed->tbl[ed->sel]].prev=mark_last; else j=-1;
            if(sc[mark_first].prev!=-1) sc[sc[mark_first].prev].next=sc[mark_last].next;
            else mark_doc->sr[mark_sr].first=sc[mark_last].next;
            if(sc[mark_first].next!=-1) sc[sc[mark_last].next].prev=sc[mark_first].prev;
            sc[mark_first].prev=i;
            sc[mark_last].next=j;
            ed->tbl=realloc(ed->tbl,(ed->len+=mark_end+1-mark_start)<<1);
            for(i=ed->sel,j=mark_first;j!=-1;i++,j=sc[j].next) ed->tbl[i]=j;
            mark_sr=ed->ew.param;
            mark_end=ed->sel+mark_end-mark_start;
            mark_start=ed->sel;
            InvalidateRect(win,0,1);
            Updatesize(win);
            break;
        case 'C':
            if(!mark_doc) break;
            sc=mark_doc->scmd;
            sc2=ed->ew.doc->scmd;
            l=ed->sel;
            if(l) k=ed->tbl[l-1]; else k=-1;
            sc3=sc2+k;
            m=mark_end+1-mark_start;
            ed->tbl=realloc(ed->tbl,(ed->len+=mark_end+1-mark_start)<<1);
            
            CopyMemory(ed->tbl + l + m,
                       ed->tbl + l,
                       (ed->len - l - m) << 1);
            
            m=mark_first;
            for(i=mark_start;i<=mark_end;i++) {
                j=AllocScmd(ed->ew.doc);
                if(k!=-1) sc3->next=j; else ed->ew.doc->sr[ed->ew.param].first=j;
                sc3=sc2+j;
                sc3->prev=k;
                *(int*)(&sc3->flag)=*(int*)(&sc[m].flag);
                *(int*)(&sc3->p3)=*(int*)(&sc[m].p3);
                k=ed->tbl[l++]=j;
                m=sc[m].next;
            }
            
            if(l == ed->len)
                sc3->next = -1;
            else
                sc3->next = ed->tbl[l], sc2[sc3->next].prev = j;
            
            ed->ew.doc->m_modf = 1;
            
            InvalidateRect(win,0,1);
            Updatesize(win);
            
            break;
        
        case 'Q':
            
            sc = ed->ew.doc->scmd;
            
            for(i = 0; i < ed->len; i++)
            {
                sc2 = sc + ed->tbl[i];
                
                if(i == 0)
                    j = -1;
                else
                    j = ed->tbl[i - 1];
                
                if(i == ed->len - 1)
                    k = -1;
                else
                    k = ed->tbl[i+1];
                
                if(sc2->prev != j || sc2->next != k)
                {
                    text_buf_ty buf;
                    
                    wsprintf(buf,"Data is bad. Ofs %04X Prev=%04X instead of %04X, Next=%04X instead of %04X",i,sc2->prev,j,sc2->next,k);
                    MessageBox(framewnd, buf, "Bad error happened", MB_OK);
                    
                    return 0;
                }
            }
            MessageBox(framewnd,"Data is okay.","Check",MB_OK);
        }
        break;
    case WM_KEYDOWN:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_DOWN:
            if(ed->sel==ed->len) break;
            rc = HM_GetClientRect(win);
            j=ed->sel+1;
            goto setrow;
        case VK_UP:
            if(ed->sel<=0) break;
            GetClientRect(win,&rc);
            j=ed->sel-1;
            goto setrow;
        case VK_DELETE:
            if(ed->sel==ed->len) return 0;
            doc=ed->ew.doc;
            sc3=doc->scmd;
            j=ed->tbl[ed->sel];
            if(doc==mark_doc && ed->ew.param==mark_sr && j>=mark_start && j<=mark_end) {
                mark_end--;
                if(mark_end<mark_start) mark_doc=0;
            }
            sc=sc3+j;
            if(sc->prev!=-1) sc3[sc->prev].next=sc->next;
            else doc->sr[ed->ew.param].first=sc->next;
            if(sc->next!=-1) sc3[sc->next].prev=sc->prev;
            sc->next=doc->m_free;
            sc->prev=-1;
            doc->m_free=j;
            ed->len--;
            if(ed->len<ed->sel) ed->sel=ed->len;
            
            CopyMemory(ed->tbl + ed->sel,
                       ed->tbl + ed->sel + 1,
                       (ed->len - ed->sel) << 1);
            
            ed->tbl=realloc(ed->tbl,ed->len<<2);
            ed->ew.doc->m_modf=1;
            InvalidateRect(win,0,1);
            Updatesize(win);
            return 0;
        case VK_INSERT:
            doc=ed->ew.doc;
            k=ed->sel;
            i=AllocScmd(doc);
            sc3=doc->scmd;
            sc2=sc3+i;
            if(doc==mark_doc && ed->ew.param==mark_sr && k>mark_start && k<=mark_end) mark_end++;
            if(k) {
                sc2->prev=ed->tbl[k-1];
                sc3[sc2->prev].next=i;
            } else sc2->prev=-1,ed->ew.doc->sr[ed->ew.param].first=i;
            if(k<ed->len) {
                sc3[j=ed->tbl[k]].prev=i;
                sc2->next=j;
            } else sc2->next=-1;
            sc2->cmd=128;
            sc2->flag=0;
            ed->len++;
            ed->tbl=realloc(ed->tbl,ed->len<<2);
            MoveMemory(ed->tbl + k + 1, ed->tbl + k, (ed->len - k) << 1);
            ed->tbl[k]=i;
            ed->ew.doc->m_modf=1;
            InvalidateRect(win,0,1);
            Updatesize(win);
            return 0;
        case 13:
            if(ed->sel==ed->len) return 0;
            doc=ed->ew.doc;
            sc=doc->scmd+ed->tbl[ed->sel];
            if(sc->cmd==0xef) Edittrack(doc,*(short*)&(sc->p1));
            return 0;
        }
        if(ed->csel==2) switch(wparam) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(sc->cmd>0xc7) break;
            sc->cmd=((sc->cmd-128)%12)+(wparam-49)*12+128;
            goto modfrow;
        case 'Z':
            i=0;
            goto updnote;
        case 'S':
            i=1;
            goto updnote;
        case 'X':
            i=2;
            goto updnote;
        case 'D':
            i=3;
            goto updnote;
        case 'C':
            i=4;
            goto updnote;
        case 'F':
            i=5;
            goto updnote;
        case 'V':
            i=6;
            goto updnote;
        case 'G':
            i=7;
            goto updnote;
        case 'B':
            i=8;
            goto updnote;
        case 'H':
            i=9;
            goto updnote;
        case 'N':
            i=10;
            goto updnote;
        case 'J':
            i=11;
updnote:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(sc->cmd>0xc7) sc->cmd=128+i;
            else sc->cmd=sc->cmd-((sc->cmd-128)%12)+i;
            goto modfrow;
        case ' ':
        case 219:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xc8;
            goto modfrow;
        case VK_BACK:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xc9;
            goto modfrow;
        case 'M':
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xe0;
            goto modfrow;
        } else switch(wparam) {
        case VK_BACK:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(ed->csel==0) sc->flag&=-4;
            if(ed->csel==1) sc->flag&=-3;
            goto modfrow;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            wparam-=7;
            goto digkey;
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
digkey:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            k=ed->csel;
            if(j==-1 || k==-1) break;
            rc = HM_GetClientRect(win);
            m=wparam-'0';
            switch(k) {
            case 0:
                sc->flag|=1;
                sc->b1<<=4;
                if(!(sc->b1|m)) sc->b1=112;
                goto updfld;
            case 1:
                sc->flag|=3;
                sc->b2<<=4;
                if(!(sc->b2|m)) sc->b2=112;
                goto updfld;
            case 3:
                sc->p1<<=4;
                goto updfld;
            case 4:
                sc->p2<<=4;
                goto updfld;
            case 5:
                sc->p3<<=4;
                goto updfld;
            }
            break;
        }
        break;
    
    case WM_PAINT:
        
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        hdc=BeginPaint(win,&ps);
        
        j=(ps.rcPaint.bottom+textmetric.tmHeight-1)/textmetric.tmHeight+ed->scroll;
        i=ps.rcPaint.top/textmetric.tmHeight+ed->scroll;
        
        if(j>=ed->len) j=ed->len;
        
        k=(i-ed->scroll)*textmetric.tmHeight;
        oldobj=SelectObject(hdc,trk_font);
        doc=ed->ew.doc;
        SetROP2(hdc,R2_COPYPEN);
        
        for(;i<j;i++)
        {
            text_buf_ty buf;
            
            l=ed->tbl[i];
            sc=doc->scmd+l;
            wsprintf(buf,"%04X: ",l);
            
            if(sc->flag&2)
                wsprintf(buf+6,"%02X %02X ",sc->b1,sc->b2);
            else if(sc->flag&1)
                wsprintf(buf+6,"%02X    ",sc->b1);
            else
                wsprintf(buf+6,"      ");
            if(sc->cmd<0xe0)
            {
                l=sc->cmd - 0x80;
                if(l==0x48) wsprintf(buf+12,"--       ");
                else if(l==0x49) wsprintf(buf+12,"Off      ");
                else wsprintf(buf+12,"%s %d     ",note_str[l%12],l/12+1);
            }
            else
            {
                l=sc->cmd - 0xe0;
                wsprintf(buf+12,"%s ",cmd_str[l]);
                if(op_len[l]) wsprintf(buf+21,"%02X ",sc->p1);
                if(op_len[l]>1) wsprintf(buf+24,"%02X ",sc->p2);
                if(op_len[l]>2) wsprintf(buf+27,"%02X ",sc->p3);
            }
            
            TextOut(hdc,0,k,buf,lstrlen(buf));
            
            if(ed->debugflag)
            {
                wsprintf(buf,"%04X: Flag %d Time %04X Time2 %04X",sc->addr,sc->flag,sc->tim,sc->tim2);
                TextOut(hdc,256,k,buf,lstrlen(buf));
            }
            
            k += textmetric.tmHeight;
        }
        
        k = (ed->sel - ed->scroll) * textmetric.tmHeight;
        
        SetROP2(hdc, R2_NOTXORPEN);
        
        oldobj3=SelectObject(hdc,null_pen);
        if(mark_doc==ed->ew.doc && mark_sr==ed->ew.param) {
            rc.top=(mark_start-ed->scroll)*textmetric.tmHeight;
            rc.bottom=(mark_end+1-ed->scroll)*textmetric.tmHeight;
            oldobj2=SelectObject(hdc,green_brush);
            Rectangle(hdc,ps.rcPaint.left-1,rc.top,ps.rcPaint.right+1,rc.bottom+1);
            SelectObject(hdc,black_brush);
        } else oldobj2=SelectObject(hdc,black_brush);
        rc.top=k;
        rc.bottom=k+textmetric.tmHeight;
        Rectangle(hdc,ps.rcPaint.left-1,rc.top,ps.rcPaint.right+1,rc.bottom+1);
        SelectObject(hdc,oldobj3);
        if(ed->csel!=-1)
        {
            rc.left=csel_l[ed->csel];
            rc.right=csel_r[ed->csel];
            DrawFocusRect(hdc,&rc);
        }
        
        SelectObject(hdc,oldobj);
        SelectObject(hdc,oldobj2);
        EndPaint(win,&ps);
        break;
        
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "AudioLogic.h"

// =============================================================================

char mus_min[3]={0,15,31};
char mus_max[3]={15,31,34};

// =============================================================================

BOOL CALLBACK musdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k;
    HWND hc;
    MUSEDIT*ed;
    FDOC*doc;
    SONG*s;
    SONGPART*sp;
    
    char const * st;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        SetWindowLongPtr(win,DWLP_USER,lparam);
        ed=(MUSEDIT*)lparam;
        ed->dlg=win;
        ed->sel_song=-1;
        if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
        hc=GetDlgItem(win,3000);
        j=ed->ew.doc->numsong[ed->ew.param];
        for(i=0;i<j;i++)
        {
            text_buf_ty buf;
            
            if(i < mus_min[ed->ew.param] || i >= mus_max[ed->ew.param])
                st = buf, wsprintf(buf, "Song %d", i + 1);
            else
                st = mus_str[i + 2];
            
            SendMessage(hc,LB_ADDSTRING,0, (LPARAM) st);
        }
        break;
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case 3000|(LBN_SELCHANGE<<16):
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            doc=ed->ew.doc;
            j=ed->ew.param;
            while(j--) i+=doc->numsong[j];
            if(ed->sel_song!=i) {
                ed->sel_song=i;
songchg:
                hc=GetDlgItem(win,3009);
                SendMessage(hc,LB_RESETCONTENT,0,0);
                s=doc->songs[i];
                if(s)
                {
                    text_buf_ty buf;
                    
                    ShowWindow(hc,SW_SHOW);
                    
                    for(j=0;j<s->numparts;j++)
                    {
                        wsprintf(buf, "Part %d", j);
                        
                        SendMessage(hc,LB_ADDSTRING,0, (LPARAM) buf);
                    }
                    SetDlgItemInt(win,3025,s->lopst,0);
                    CheckDlgButton(win,3026,(s->flag&2)?BST_CHECKED:BST_UNCHECKED);
                    ShowWindow(GetDlgItem(win,3025),(s->flag&2)?SW_SHOW:SW_HIDE);
                }
                else
                    ShowWindow(hc, SW_HIDE);
                
                for(j=0;j<8;j++)
                    EnableWindow(GetDlgItem(win,3001+j), IsNonNull(s) );
            }
            break;
        case 3009|(LBN_DBLCLK<<16):
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            if(i==-1) break;
            if(!sndinit) Initsound();
            if(!sndinit) break;
//          EnterCriticalSection(&cs_song);
            Stopsong();
            playedpatt=i;
            sounddoc=ed->ew.doc;
            playedsong=ed->ew.doc->songs[ed->sel_song];
            Playpatt();
//          LeaveCriticalSection(&cs_song);
            break;
        case 3009|(LBN_SELCHANGE<<16):
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            if(i==-1) break;
            doc=ed->ew.doc;
            j=ed->sel_song;
            ed->init=1;
            for(k=0;k<8;k++)
            {
                text_buf_ty buf;
                
                wsprintf(buf, "%04X", ed->ew.doc->songs[j]->tbl[i]->tbl[k]&65535);
                
                SetDlgItemText(win,3012+k, buf);
            }
            ed->init=0;
            break;
        case 3001:
        case 3002:
        case 3003:
        case 3004:
        case 3005:
        case 3006:
        case 3007:
        case 3008:
            i=wparam-3001;
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            Edittrack(ed->ew.doc,ed->ew.doc->songs[ed->sel_song]->tbl[j]->tbl[i]);
            break;
        case 3010:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            if(ed->sel_song==-1 || !ed->ew.doc->songs[ed->sel_song]) break;
            if(!sndinit) Initsound();
            if(sndinit) Playsong(ed->ew.doc,ed->sel_song);
            break;
        case 3011:
            if(!sndinit) {sounddoc=0;break;}
//          EnterCriticalSection(&cs_song);
            Stopsong();
//          LeaveCriticalSection(&cs_song);
            break;
        case 3012|(EN_CHANGE<<16):
        case 3013|(EN_CHANGE<<16):
        case 3014|(EN_CHANGE<<16):
        case 3015|(EN_CHANGE<<16):
        case 3016|(EN_CHANGE<<16):
        case 3017|(EN_CHANGE<<16):
        case 3018|(EN_CHANGE<<16):
        case 3019|(EN_CHANGE<<16):
            
            {
                text_buf_ty buf;
                
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            
            if(ed->init) break;
            
            GetWindowText((HWND)lparam, buf, sizeof(buf));
            k=strtol(buf, 0, 16);
            i=wparam-(3012|(EN_CHANGE<<16));
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            s=ed->ew.doc->songs[ed->sel_song];
            if(!s) break;
            s->tbl[j]->tbl[i]=k;
            ed->ew.doc->m_modf=1;
            }
            
            break;
        case 3020:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            s=ed->ew.doc->songs[ed->sel_song];
            if(!s) break;
            ed->ew.doc->sp_mark=s->tbl[j];
            break;
        case 3021:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=ed->sel_song;
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            
            s->numparts++;
            s->tbl = (SONGPART**) realloc(s->tbl, s->numparts << 2);
            
            if(j != -1)
                MoveMemory(s->tbl + j + 1,
                           s->tbl + j,
                           (s->numparts - j) << 2);
            else
                j = s->numparts - 1;
            (s->tbl[j]=doc->sp_mark)->inst++;
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3022:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=ed->sel_song;
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            s->numparts++;
            s->tbl = (SONGPART**) realloc(s->tbl,s->numparts<<2);
            sp=s->tbl[s->numparts - 1] = (SONGPART*) malloc(sizeof(SONGPART));
            for(k=0;k<8;k++) sp->tbl[k]=-1;
            sp->flag=s->flag&1;
            sp->inst=1;
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3023:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=ed->sel_song;
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            s->numparts--;
            sp=s->tbl[j];
            sp->inst--;
            
            if(!sp->inst)
                free(sp);
            
            sp = (SONGPART*) CopyMemory(s->tbl + j,
                                        s->tbl + j + 1,
                                        (s->numparts - j) << 2);
            
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3024:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(!s) break;
            NewSR(ed->ew.doc,((j==-1)?(s->flag&1):(s->tbl[j]->flag&1))?0:ed->ew.param+1);
            break;
        case 3025|(EN_CHANGE<<16):
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            if(ed->init) break;
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(s) s->lopst=GetDlgItemInt(win,3025,0,0);
            ed->ew.doc->m_modf=1;
            break;
        case 3026:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(s) {
                hc=GetDlgItem(win,3025);
                if((IsDlgButtonChecked(win,3026)==BST_CHECKED)) {
                    s->flag|=2;
                    ShowWindow(hc,SW_SHOW);
                } else {
                    s->flag&=-3;
                    ShowWindow(hc,SW_HIDE);
                }
                ed->ew.doc->m_modf=1;
            }
            break;
        case 3027:
            
            ed = (MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            
            i = ed->sel_song;
            
            if(i == -1)
                break;
            
            doc = ed->ew.doc;
            
            if(doc->songs[i])
            {
                MessageBox(framewnd,
                           "Please delete the existing song first.",
                           "Bad error happened",
                           MB_OK);
                
                break;
            }
            
            s = doc->songs[i] = (SONG*) malloc(sizeof(SONG));
            
            s->flag=0;
            s->inst=1;
            s->numparts=0;
            s->tbl=0;
            
            goto updsongs;
        
        case 3028:
            ed=(MUSEDIT*)GetWindowLongPtr(win,DWLP_USER);
            i=ed->sel_song;
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(s==playedsong) {
                if(sndinit) {
//                  EnterCriticalSection(&cs_song);
                    Stopsong();
//                  LeaveCriticalSection(&cs_song);
                } else sounddoc=0;
            }
            doc->songs[i]=0;
            if(s) {
                s->inst--;
                if(!s->inst) {
                    for(i=0;i<s->numparts;i++) {
                        sp=s->tbl[i];
                        sp->inst--;
                        if(!sp->inst) free(sp);
                    }
                    free(s);
                }
updsongs:
                ed->ew.doc->m_modf=1;
                ed->sel_song=-1;
                musdlgproc(win,WM_COMMAND,3000|(LBN_SELCHANGE<<16), (LPARAM) GetDlgItem(win,3000));
            } else MessageBox(framewnd,"There is no song","Bad error happened",MB_OK);
            break;
        }
    }
    return FALSE;
}

// =============================================================================
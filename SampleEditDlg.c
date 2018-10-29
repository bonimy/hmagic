
    #include "structs.h"
    #include "prototypes.h"

    #include "AudioLogic.h"
    #include "SampleEnum.h"

// =============================================================================

void
Loadeditinst(HWND             const win,
             SAMPEDIT const * const ed)
{
    text_buf_ty text_buf = { 0 };
    
    ZINST*zi;
    zi=ed->ew.doc->insts+ed->editinst;
    
    SetDlgItemInt(win, 3014, (zi->multhi << 8) + zi->multlo, 0);
    
    wsprintf(text_buf,
             "%04X",
             (zi->ad << 8) + zi->sr);
    
    SetDlgItemText(win, 3016, text_buf);
    
    wsprintf(text_buf,
             "%02X",
             zi->gain);
    
    SetDlgItemText(win, 3018, text_buf);
    
    SetDlgItemInt(win, 3020, zi->samp, 0);
}

// =============================================================================

BOOL CALLBACK
sampdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SAMPEDIT*ed;
    HWND hc;
    HGLOBAL hgl;
    char*b,*dat;
    int i = 0, j = 0, k = 0;
    
    ZWAVE*zw,*zw2;
    ZINST*zi;
    WAVEFORMATEX*wfx;
    
    const static int wavehdr[] =
    {
        0x46464952,0,0x45564157,0x20746d66,16,0x10001,
        11025,22050,0x100002,0x61746164
    };
    
    switch(msg) {
    case WM_INITDIALOG:
        
        SetWindowLong(win,DWL_USER,lparam);
        
        ed=(SAMPEDIT*)lparam;
        ed->dlg=win;
        
        if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
        
        ed->init=1;
        ed->editsamp=0;
        ed->editinst=0;
        ed->zoom=65536;
        ed->scroll=0;
        ed->flag=0;
        SetDlgItemInt(win, ID_Samp_SampleIndexEdit, 0, 0);
        
        SetWindowLongPtr(GetDlgItem(win, ID_Samp_Display),
                         GWLP_USERDATA,
                         (LONG_PTR) ed);
        
        SetDlgItemInt(win,3012,0,0);
        Loadeditinst(win,ed);
chgsamp:
        i=ed->editsamp;
        zw=ed->ew.doc->waves+i;
        ed->zw=zw;
updcopy:
        
        if(zw->copy != -1)
        {
            CheckDlgButton(win, ID_Samp_SampleIsCopyCheckBox, BST_CHECKED);
            
            ShowWindow(GetDlgItem(win, ID_Samp_SampleCopyOfIndexEdit),
                       SW_SHOW);
            
            SetDlgItemInt(win, ID_Samp_SampleCopyOfIndexEdit, zw->copy, 0);
            
            EnableWindow(GetDlgItem(win, ID_Samp_PasteFromClipboardButton), 0);
            EnableWindow(GetDlgItem(win,3008),0);
        }
        else
        {
            CheckDlgButton(win, ID_Samp_SampleIsCopyCheckBox, BST_UNCHECKED);
            
            ShowWindow(GetDlgItem(win, ID_Samp_SampleCopyOfIndexEdit),
                       SW_HIDE);
            
            EnableWindow(GetDlgItem(win, ID_Samp_PasteFromClipboardButton), 1);
            EnableWindow(GetDlgItem(win,3008),1);
        }
        
        SetDlgItemInt(win,3008,zw->end,0);
        SetDlgItemInt(win,3010,zw->lopst,0);
        
        if(zw->lflag)
        {
            CheckDlgButton(win,3009,BST_CHECKED);
            ShowWindow(GetDlgItem(win,3010),SW_SHOW);
        }
        else
        {
            CheckDlgButton(win,3009,BST_UNCHECKED);
            ShowWindow(GetDlgItem(win,3010),SW_HIDE);
        }
        
        ed->sell=0;
        ed->selr=0;
upddisp:
        
        ed->init=0;
        
        if(ed->sell>=zw->end) ed->sell=ed->selr=0;
        
        if(ed->selr>=zw->end) ed->selr=zw->end;
        
        hc = GetDlgItem(win, ID_Samp_Display);
        
        Updatesize(hc);
        InvalidateRect(hc,0,1);
        
        break;
    
    case WM_COMMAND:
        
        ed = (SAMPEDIT*)GetWindowLong(win,DWL_USER);
        
        if(ed->init)
            break;
        
        switch(wparam)
        {
        
        case ID_Samp_SampleIsCopyCheckBox:
            
            zw = ed->zw;
            
            if(zw->copy!=-1)
            {
                zw->copy=-1;
                zw->end=zw->lflag=0;
                
                zw->buf = (short*) calloc(1, sizeof(short));
                
                ShowWindow(GetDlgItem(win, ID_Samp_SampleCopyOfIndexEdit),
                           SW_HIDE);
            }
            else
            {
                j=ed->ew.doc->numwave;
                zw2=ed->ew.doc->waves;
                for(i=0;i<j;i++) {
                    if(zw2->copy==-1 && i!=ed->editsamp) goto chgcopy;
                    zw2++;
                }
                CheckDlgButton(win, ID_Samp_SampleIsCopyCheckBox, BST_UNCHECKED);
chgcopy:
                zw->copy=i;
                free(zw->buf);
                zw->end=zw2->end;
                zw->lopst=zw2->lopst;
                
                zw->buf = (int16_t*) calloc(zw->end + 1, sizeof(int16_t));
                
                i = (zw->end + 1) * sizeof(int16_t);
                
                memcpy(zw->buf, zw2->buf, i);
            }
            
            goto updcopy;
        
        case ID_Samp_CopyToClipboardButton:
            
            zw = ed->zw;
            
            i = (ed->selr - ed->sell) << 1;
            j = ed->sell;
            
            if(i == 0)
                i = zw->end << 1, j = 0;
            
            hgl=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,44+i);
            b=GlobalLock(hgl);
            
            memcpy(b, wavehdr, 40);
            
            *(int*)(b+4)=36+i;
            *(int*)(b+40)=i;
            
            memcpy(b+44,zw->buf+j,i);
            GlobalUnlock(hgl);
            OpenClipboard(0);
            EmptyClipboard();
            
            SetClipboardData(CF_WAVE,hgl);
            CloseClipboard();
            
            break;
        
        case ID_Samp_PasteFromClipboardButton:
            
            OpenClipboard(0);
            
            hgl = GetClipboardData(CF_WAVE);
            
            if(!hgl)
            {
                MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
                CloseClipboard();
                break;
            }
            
            b = GlobalLock(hgl);
            
            if( (*(int*)b!=0x46464952) || *(int*)(b+8)!=0x45564157)
            {
error:
                MessageBox(framewnd,"This is not a wave.","Bad error happened",MB_OK);
noclip:
                GlobalUnlock(hgl);
                CloseClipboard();
                break;
            }
            
            j = *(int*) (b + 4);
            b += 8;
            dat = 0;
            wfx = 0;
            
            for(i = 4; i < j;)
            {
                switch(*(int*)(b+i))
                {
                
                case 0x20746d66:
                    wfx = (WAVEFORMATEX*) (b + i + 8);
                    
                    break;
                
                case 0x61746164:
                    dat = b + i + 8;
                    
                    k = *(int*)(b + i + 4);
                    
                    if(wfx)
                        goto foundall;
                    
                    break;
                }
                
                i += 8 + *(int*) ( b + i + 4);
            }
            if((!wfx)||!dat) goto error;
foundall:
            if(wfx->wFormatTag!=1) {
                MessageBox(framewnd,"The wave is not PCM","Bad error happened",MB_OK);
                goto noclip;
            }
            if(wfx->nChannels!=1) {
                MessageBox(framewnd,"Only mono is allowed.","Bad error happened",MB_OK);
                goto noclip;
            }
            
            if(wfx->wBitsPerSample == 16)
                k >>= 1;
            
            // \task Fixing a bug around here when you paste beyond the range
            // of the current sample.
            zw = ed->zw;
            
            zw->end += k - (ed->selr - ed->sell);
            
            if(k > (ed->selr - ed->sell) )
            {
                // Resize the sample buffer if the size of the data being
                // pasted in exceeds the size of the currently selected
                // region.
                zw->buf = (short*) realloc(zw->buf, (zw->end + 1) << 1);
            }
            
            // Move the part of the sample that is to the right of the selection
            // region in such a way that there is just enough room to copy
            // the new data in.
            memmove(zw->buf + ed->sell + k,
                    zw->buf + ed->selr,
                    (zw->end - k - ed->sell) << 1);
            
            if(k < (ed->selr - ed->sell) )
            {
                // Shrink the sample buffer if the pasted in data is smaller
                // than the selection region.
                zw->buf = (short*) realloc(zw->buf, ( (zw->end + 1) << 1 ) );
            }
            
            if(zw->lopst >= ed->selr)
                zw->lopst += ed->sell + k - ed->selr;
            
            if(zw->lopst >= zw->end)
                zw->lflag = 0, zw->lopst = 0;
            
            if(zw->lflag)
                zw->buf[zw->end] = zw->buf[zw->lopst];
            
            ed->selr = ed->sell + k;
            
            if(wfx->wBitsPerSample == 16)
                memcpy(zw->buf+ed->sell,dat,k<<1);
            else
            {
                j = ed->sell;
                
                for(i = 0; i < k; i++)
                    zw->buf[j++] = ( (dat[i] - 128) << 8 );
            }
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            GlobalUnlock(hgl);
            CloseClipboard();
            Modifywaves(ed->ew.doc,ed->editsamp);
            ed->init=1;
            if(!zw->lflag) {
                CheckDlgButton(win,3009,BST_UNCHECKED);
                ShowWindow(GetDlgItem(win,3010),SW_HIDE);
            }
            SetDlgItemInt(win,3008,zw->end,0);
            SetDlgItemInt(win,3010,zw->lopst,0);
            goto upddisp;
        case 3009:
            zw=ed->zw;
            if(!zw->lflag) {
                zw->lflag=1;
                ShowWindow(GetDlgItem(win,3010),SW_SHOW);
                zw->buf[zw->end]=zw->buf[zw->lopst];
            } else {
                zw->lflag=0;
                ShowWindow(GetDlgItem(win,3010),SW_HIDE);
            }
            Modifywaves(ed->ew.doc,ed->editsamp);
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            break;
        case 3021:
            if(sounddev>=0x20000) {
                MessageBox(framewnd,"A wave device must be selected","Bad error happened",MB_OK);
                break;
            }
            if(sndinit) Stopsong();
            else Initsound();
            playedsong=0;
            sounddoc=ed->ew.doc;
            globvol=65535;
            zwaves->wnum=ed->editsamp;
            zwaves->vol1=zwaves->vol2=127;
            zwaves->sus=7;
            zwaves->rel=0;
            zwaves->atk=31;
            zwaves->dec=0;
            zwaves->envs=zwaves->envclk=zwaves->envclklo=zwaves->pos=0;
            zwaves->freq=1781;
            zwaves->pflag=1;
            break;
        case 3022:
            if(sndinit) Stopsong();
            break;

        case (EN_KILLFOCUS << 16) | ID_Samp_SampleIndexEdit:
            
            if(ed->flag & 1)
            {
                i=GetDlgItemInt(win, ID_Samp_SampleIndexEdit, 0, 0);
                
                if(i < 0)
                    i = 0;
                
                if(i >= ed->ew.doc->numwave)
                    i = ed->ew.doc->numwave-1;
                
                ed->flag &= -2;
                ed->init = 1;
                
                SetDlgItemInt(win, ID_Samp_SampleIndexEdit, i, 0);
                
                ed->editsamp = i;
                
                goto chgsamp;
            }
            
            break;
        
        case (EN_KILLFOCUS << 16) | ID_Samp_SampleCopyOfIndexEdit:
            
            if(ed->flag & 2)
            {
                ed->flag&=-3;
                zw=ed->zw;
                j=zw->copy;
                
                i=GetDlgItemInt(win, ID_Samp_SampleCopyOfIndexEdit, 0, 0);
                
                k=zw->end;
                
                if(i<0) i=0;
                if(i>=ed->ew.doc->numwave) i=ed->ew.doc->numwave-1;
                
                zw2=ed->ew.doc->waves+i;
                
                if(zw2->copy!=-1) i=j;
                
                ed->init=1;
                
                SetDlgItemInt(win, ID_Samp_SampleCopyOfIndexEdit, i, 0);
                
                ed->init=0;
                if(i==j) break;
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                goto chgcopy;
            }
            break;
        case 0x02000bc0:
            if(ed->flag&16) {
                ed->flag&=-17;
                zw=ed->zw;
                i=GetDlgItemInt(win,3008,0,0);
                k=zw->end;
                if(i<0) {
                    i=0;
chglen:
                    SetDlgItemInt(win,3008,i,0);
                    ed->init=0;
                    break;
                }
                if(i>65536) { i=65536; goto chglen; }
                b = realloc(zw->buf, (i + 1) << 1);
                if(!b) {
                    MessageBox(framewnd,"Not enough memory","Bad error happened",MB_OK);
                    i=k;
                    ed->init=1;
                    goto chglen;
                } else zw->buf=(short*)b;
                zw->end=i;
                for(j=k;j<i;j++) zw->buf[j]=0;
                if(zw->lopst!=-1) zw->buf[zw->end]=zw->buf[zw->lopst];
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                goto upddisp;
            }
            break;
        case 0x02000bc2:
            if(ed->flag&32) {
                ed->flag&=-33;
                zw=ed->zw;
                j=i=GetDlgItemInt(win,3010,0,0);
                if(i<0) i=0;
                else if(i>=zw->end) i=zw->end-1;
                if(i!=j) {
                    ed->init=1;
                    SetDlgItemInt(win,3010,i,0);
                    ed->init=0;
                }
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                zw->lopst=i;
                zw->buf[zw->end]=zw->buf[zw->lopst];
            }
            break;
        case 0x02000bc4:
            if(ed->flag&64) {
                ed->flag&=-65;
                i=GetDlgItemInt(win,3012,0,0);
                if(i<0) i=0;
                else if(i>=ed->ew.doc->numinst) i=ed->ew.doc->numinst-1;
                else goto nochginst;
                ed->init=1;
                SetDlgItemInt(win,3012,i,0);
                ed->init=0;
nochginst:
                ed->editinst=i;
                Loadeditinst(win,ed);
            }
            break;
        case 0x02000bc6:
        case 0x02000bc8:
        case 0x02000bca:
        case 0x02000bcc:
            if(ed->flag&128)
            {
                text_buf_ty text_buf = { 0 };
                
                ed->flag&=-129;
                zi=ed->ew.doc->insts+ed->editinst;
                i=GetDlgItemInt(win,3014,0,0);
                zi->multlo=i;
                zi->multhi=i>>8;
                GetDlgItemText(win,3016, text_buf, 5);
                i=strtol(text_buf, 0, 16);
                zi->sr=i;
                zi->ad=i>>8;
                
                GetDlgItemText(win, 3018, text_buf, 3);
                
                zi->gain = (uint8_t) strtol(text_buf, 0, 16);
                
                zi->samp=GetDlgItemInt(win,3020,0,0);
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
            }
            break;
        
        case (EN_CHANGE << 16) | ID_Samp_SampleIndexEdit:
            
            ed->flag |= 1;
            
            break;
        
        case (EN_CHANGE << 16) | ID_Samp_SampleCopyOfIndexEdit:
            
            ed->flag |= 2;
            
            break;
        
        case 0x03000bc0:
            
            ed->flag|=16;
            
            break;
        
        case 0x03000bc2:
            
            ed->flag|=32;
            
            break;
        
        case 0x03000bc4:
            
            ed->flag|=64;
            
            break;
        
        case 0x03000bc6:
        case 0x03000bc8:
        case 0x03000bca:
        case 0x03000bcc:
            ed->flag |= 128;
            break;
        }
    }
    return FALSE;
}
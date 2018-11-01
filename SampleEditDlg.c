
    #include "structs.h"
    #include "prototypes.h"

    #include "AudioLogic.h"
    #include "SampleEnum.h"
    
    // For copy and paste, currently.
    #include "SampleEditLogic.h"

// =============================================================================

void
Loadeditinst(HWND             const p_win,
             SAMPEDIT const * const ed)
{
    text_buf_ty text_buf = { 0 };
    
    ZINST*zi;
    zi=ed->ew.doc->insts+ed->editinst;
    
    SetDlgItemInt(p_win, 3014, (zi->multhi << 8) + zi->multlo, 0);
    
    wsprintf(text_buf,
             "%04X",
             (zi->ad << 8) + zi->sr);
    
    SetDlgItemText(p_win, 3016, text_buf);
    
    wsprintf(text_buf,
             "%02X",
             zi->gain);
    
    SetDlgItemText(p_win, 3018, text_buf);
    
    SetDlgItemInt(p_win, 3020, zi->samp, 0);
}

// =============================================================================

    static void
    SampleEditDlg_UpdateDisplay(SAMPEDIT * const p_ed,
                                HWND       const p_win)
    {
        ZWAVE const * const zw = p_ed->zw;
        
        HWND const disp = GetDlgItem(p_win, ID_Samp_Display);
        
        // -----------------------------
        
        p_ed->init = 0;
        
        if(p_ed->sell >= zw->end)
        {
            p_ed->sell = p_ed->selr = 0;
        }
        
        if(p_ed->selr >= zw->end)
        {
            p_ed->selr = zw->end;
        }
        
        Updatesize(disp);
        
        InvalidateRect(disp, 0, 1);
    }

// =============================================================================

    static void
    SampleEditDlg_UpdateCopy(SAMPEDIT * const p_ed,
                             HWND       const p_win)
    {
        ZWAVE const * const zw = p_ed->zw;
        
        HWND const copy_edit_ctl = GetDlgItem
        (
            p_win,
            ID_Samp_SampleCopyOfIndexEdit
        );
        
        // -----------------------------
        
        if(zw->copy != -1)
        {
            CheckDlgButton(p_win, ID_Samp_SampleIsCopyCheckBox, BST_CHECKED);
            
            ShowWindow(copy_edit_ctl, SW_SHOW);
            
            SetDlgItemInt(p_win, ID_Samp_SampleCopyOfIndexEdit, zw->copy, 0);
            
            EnableWindow(GetDlgItem(p_win, ID_Samp_PasteFromClipboardButton), 0);
            EnableWindow(GetDlgItem(p_win, ID_Samp_SampleLengthEdit),0);
        }
        else
        {
            CheckDlgButton(p_win, ID_Samp_SampleIsCopyCheckBox, BST_UNCHECKED);
            
            ShowWindow(copy_edit_ctl, SW_HIDE);
            
            EnableWindow(GetDlgItem(p_win, ID_Samp_PasteFromClipboardButton), 1);
            EnableWindow(GetDlgItem(p_win, ID_Samp_SampleLengthEdit), 1);
        }
        
        SetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, zw->end, 0);
        SetDlgItemInt(p_win, ID_Samp_LoopPointEdit,zw->lopst, 0);
        
        if(zw->lflag)
        {
            CheckDlgButton(p_win, ID_Samp_LoopCheckBox, BST_CHECKED);
            ShowWindow(GetDlgItem(p_win, ID_Samp_LoopPointEdit), SW_SHOW);
        }
        else
        {
            CheckDlgButton(p_win, ID_Samp_LoopCheckBox, BST_UNCHECKED);
            ShowWindow(GetDlgItem(p_win, ID_Samp_LoopPointEdit), SW_HIDE);
        }
        
        p_ed->sell = 0;
        p_ed->selr = 0;
        
        SampleEditDlg_UpdateDisplay(p_ed, p_win);
    }

// =============================================================================

extern BOOL CALLBACK
sampdlgproc(HWND p_win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SAMPEDIT*ed;
    char*b;
    int i = 0, j = 0, k = 0;
    
    ZWAVE*zw,*zw2;
    ZINST*zi;
    
    switch(msg) {
    case WM_INITDIALOG:
        
        SetWindowLong(p_win, DWL_USER, lparam);
        
        ed=(SAMPEDIT*)lparam;
        ed->dlg= p_win;
        
        if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
        
        ed->init=1;
        ed->editsamp=0;
        ed->editinst=0;
        ed->zoom=65536;
        ed->scroll=0;
        ed->flag=0;
        SetDlgItemInt(p_win, ID_Samp_SampleIndexEdit, 0, 0);
        
        SetWindowLongPtr(GetDlgItem(p_win, ID_Samp_Display),
                         GWLP_USERDATA,
                         (LONG_PTR) ed);
        
        SetDlgItemInt(p_win, 3012,0,0);
        Loadeditinst(p_win, ed);
chgsamp:
        
        i=ed->editsamp;
        zw=ed->ew.doc->waves+i;
        ed->zw=zw;
        
        SampleEditDlg_UpdateCopy(ed, p_win);
        
        break;
    
    case WM_COMMAND:
        
        ed = (SAMPEDIT*) GetWindowLong(p_win, DWL_USER);
        
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
                
                ShowWindow(GetDlgItem(p_win, ID_Samp_SampleCopyOfIndexEdit),
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
                CheckDlgButton(p_win, ID_Samp_SampleIsCopyCheckBox, BST_UNCHECKED);
chgcopy:
                zw->copy=i;
                free(zw->buf);
                zw->end=zw2->end;
                zw->lopst=zw2->lopst;
                
                zw->buf = (int16_t*) calloc(zw->end + 1, sizeof(int16_t));
                
                i = (zw->end + 1) * sizeof(int16_t);
                
                memcpy(zw->buf, zw2->buf, i);
            }
            
            SampleEditDlg_UpdateCopy(ed, p_win);

            break;
        
        case ID_Samp_CopyToClipboardButton:
            
            SampleEdit_CopyToClipboard(ed);
            
            break;
        
        case ID_Samp_PasteFromClipboardButton:
            
            if( SampleEdit_PasteFromClipboard(ed) == FALSE )
            {
                break;
            }
            
            if( ! ed->zw->lflag)
            {
                CheckDlgButton(p_win, ID_Samp_LoopCheckBox, BST_UNCHECKED);
                ShowWindow(GetDlgItem(p_win, ID_Samp_LoopPointEdit), SW_HIDE);
            }
            
            SetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, zw->end, 0);
            SetDlgItemInt(p_win, ID_Samp_LoopPointEdit, zw->lopst, 0);
            
            SampleEditDlg_UpdateDisplay(ed, p_win);
            
            break;
        
        case ID_Samp_LoopCheckBox:
            
            zw = ed->zw;
            
            if( ! zw->lflag )
            {
                zw->lflag = 1;
                
                ShowWindow(GetDlgItem(p_win, ID_Samp_LoopPointEdit), SW_SHOW);
                
                zw->buf[zw->end]=zw->buf[zw->lopst];
            }
            else
            {
                zw->lflag = 0;
                
                ShowWindow(GetDlgItem(p_win, ID_Samp_LoopPointEdit), SW_HIDE);
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
            
            if(sndinit)
                Stopsong();
            else
                Initsound();
            
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
            
            if(ed->flag & Samp_Index_Changed)
            {
                i = GetDlgItemInt(p_win, ID_Samp_SampleIndexEdit, 0, 0);
                
                if(i < 0)
                    i = 0;
                
                if(i >= ed->ew.doc->numwave)
                    i = ed->ew.doc->numwave-1;
                
                ed->flag &= ~(Samp_Index_Changed);
                ed->init = 1;
                
                SetDlgItemInt(p_win, ID_Samp_SampleIndexEdit, i, 0);
                
                ed->editsamp = i;
                
                goto chgsamp;
            }
            
            break;
        
        case (EN_KILLFOCUS << 16) | ID_Samp_SampleCopyOfIndexEdit:
            
            if(ed->flag & Samp_CopyIndex_Changed)
            {
                ed->flag &= ~(Samp_CopyIndex_Changed);
                
                zw = ed->zw;
                
                j = zw->copy;
                
                i = GetDlgItemInt(p_win, ID_Samp_SampleCopyOfIndexEdit, 0, 0);
                
                k=zw->end;
                
                if(i<0) i=0;
                if(i>=ed->ew.doc->numwave) i=ed->ew.doc->numwave-1;
                
                zw2=ed->ew.doc->waves+i;
                
                if(zw2->copy!=-1) i=j;
                
                ed->init=1;
                
                SetDlgItemInt(p_win, ID_Samp_SampleCopyOfIndexEdit, i, 0);
                
                ed->init=0;
                if(i==j) break;
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                goto chgcopy;
            }
            
            break;
        
        case (EN_KILLFOCUS << 16) | ID_Samp_SampleLengthEdit:
            
            if(ed->flag & 16)
            {
                ed->flag&=-17;
                zw=ed->zw;
                i=GetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, 0, 0);
                k=zw->end;
                if(i<0) {
                    i=0;
chglen:
                    SetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, i, 0);
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
                
                SampleEditDlg_UpdateDisplay(ed, p_win);
            }
            
            break;
        
        case 0x02000bc2:
            if(ed->flag&32) {
                ed->flag&=-33;
                zw=ed->zw;
                j=i=GetDlgItemInt(p_win, ID_Samp_LoopPointEdit, 0, 0);
                if(i<0) i=0;
                else if(i>=zw->end) i=zw->end-1;
                if(i!=j) {
                    ed->init=1;
                    SetDlgItemInt(p_win, ID_Samp_LoopPointEdit, i, 0);
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
                i=GetDlgItemInt(p_win, 3012, 0,0);
                if(i<0) i=0;
                else if(i>=ed->ew.doc->numinst) i=ed->ew.doc->numinst-1;
                else goto nochginst;
                ed->init=1;
                SetDlgItemInt(p_win, 3012, i, 0);
                ed->init=0;
nochginst:
                ed->editinst=i;
                Loadeditinst(p_win, ed);
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
                i=GetDlgItemInt(p_win, 3014, 0,0);
                zi->multlo=i;
                zi->multhi=i>>8;
                GetDlgItemText(p_win, 3016, text_buf, 5);
                i=strtol(text_buf, 0, 16);
                zi->sr=i;
                zi->ad=i>>8;
                
                GetDlgItemText(p_win, 3018, text_buf, 3);
                
                zi->gain = (uint8_t) strtol(text_buf, 0, 16);
                
                zi->samp=GetDlgItemInt(p_win, 3020,0,0);
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
            }
            break;
        
        case (EN_CHANGE << 16) | ID_Samp_SampleIndexEdit:
            
            ed->flag |= Samp_Index_Changed;
            
            break;
        
        case (EN_CHANGE << 16) | ID_Samp_SampleCopyOfIndexEdit:
            
            ed->flag |= Samp_CopyIndex_Changed;
            
            break;
        
        case (EN_CHANGE << 16) | ID_Samp_SampleLengthEdit:
            
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
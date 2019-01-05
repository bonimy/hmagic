
    #include "structs.h"
    #include "prototypes.h"

    #include "AudioLogic.h"
    #include "SampleEnum.h"
    
    // For copy and paste, currently.
    #include "SampleEditLogic.h"

// =============================================================================

SD_ENTRY samp_sd[]={
    {"STATIC","Edit:",0,48,60,20, ID_Samp_SampleIndexStatic, WS_VISIBLE|WS_CHILD,0,0},
    {"NumEdit","",64,48,40,20, ID_Samp_SampleIndexEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"SAMPEDIT","",0,96,0,0, ID_Samp_Display, WS_VISIBLE|WS_CHILD|WS_HSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Copy of:",0,72,60,20, ID_Samp_SampleIsCopyCheckBox, WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"NumEdit","",64,72,40,20, ID_Samp_SampleCopyOfIndexEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Copy",110,48,50,20, ID_Samp_CopyToClipboardButton, WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Paste",110,72,50,20, ID_Samp_PasteFromClipboardButton, WS_VISIBLE|WS_CHILD,0,0},
    {"STATIC","Length:",170,48,60,20, ID_Samp_LengthLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"NumEdit","",234,48,60,20, ID_Samp_SampleLengthEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Loop:",170,72,60,20, ID_Samp_LoopCheckBox, WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"NumEdit","",234,72,40,20, ID_Samp_LoopPointEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Ed.Inst:",0,0,40,20, ID_Samp_InstrumentLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",44,0,40,20, ID_Samp_InstrumentIndexEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Freq:",90,0,40,20, ID_Samp_InstrFrequencyLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",134,0,40,20, ID_Samp_InstrFrequencyEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","ADSR:",180,0,40,20, ID_Samp_InstrADSR_Label, WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",224,0,40,20, ID_Samp_InstrADSR_Edit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Gain:",0,24,40,20, ID_Samp_InstrGainLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",44,24,40,20, ID_Samp_InstrGainEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Samp:",90,24,40,20, ID_Samp_InstrSampleIndexLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",134,24,40,20, ID_Samp_InstrSampleIndex_Edit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Play",180,24,36,20, ID_Samp_PlayButton, WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Stop",220,24,36,20, ID_Samp_StopButton, WS_VISIBLE|WS_CHILD,0,0}
};

// =============================================================================

#define HM_EN_KILLFOCUS(id) ( (EN_KILLFOCUS << 16) | (id & 0xffff) )

// =============================================================================

#define HM_EN_CHANGE(id) ( (EN_CHANGE << 16) | (id & 0xffff) )

// =============================================================================

    static void
    LoadInstrument(HWND             const p_win,
                   SAMPEDIT const * const ed)
    {
        text_buf_ty text_buf = { 0 };
        
        ZINST const * const zi = ed->ew.doc->insts+ed->editinst;
        
        // -----------------------------
        
        SetDlgItemInt(p_win,
                      ID_Samp_InstrFrequencyEdit,
                      (zi->multhi << 8) + zi->multlo,
                      0);
        
        wsprintf(text_buf,
                 "%04X",
                 (zi->ad << 8) + zi->sr);
        
        SetDlgItemText(p_win, ID_Samp_InstrADSR_Edit, text_buf);
        
        wsprintf(text_buf,
                 "%02X",
                 zi->gain);
        
        SetDlgItemText(p_win, ID_Samp_InstrGainEdit, text_buf);
        
        SetDlgItemInt(p_win, ID_Samp_InstrSampleIndex_Edit, zi->samp, 0);
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
sampdlgproc(HWND p_win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    SAMPEDIT *ed;
    
    char *b;
    
    int i = 0, j = 0;
    int k = 0;
    
    ZWAVE *zw, *zw2;
    ZINST *zi;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLongPtr(p_win, DWLP_USER, lparam);
        
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
        
        SetDlgItemInt(p_win, ID_Samp_InstrumentIndexEdit, 0, 0);
        
        LoadInstrument(p_win, ed);
        
chgsamp:
        
        i=ed->editsamp;
        zw=ed->ew.doc->waves+i;
        ed->zw=zw;
        
        SampleEditDlg_UpdateCopy(ed, p_win);
        
        break;
    
    case WM_COMMAND:
        
        ed = (SAMPEDIT*) GetWindowLongPtr(p_win, DWLP_USER);
        
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
            
            zw = ed->zw;
            
            if( ! zw->lflag)
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
        
        case ID_Samp_PlayButton:
            
            if(sounddev >= 0x20000)
            {
                MessageBox
                (
                    framewnd,
                    "A wave device must be selected",
                    "Bad error happened",
                    MB_OK
                );
                
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
        
        case ID_Samp_StopButton:
            
            if(sndinit)
                Stopsong();
            
            break;

        case HM_EN_KILLFOCUS(ID_Samp_SampleIndexEdit):
            
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
        
        case HM_EN_KILLFOCUS(ID_Samp_SampleCopyOfIndexEdit):
            
            if(ed->flag & Samp_CopyIndex_Changed)
            {
                ed->flag &= ~(Samp_CopyIndex_Changed);
                
                zw = ed->zw;
                
                j = zw->copy;
                
                i = GetDlgItemInt(p_win, ID_Samp_SampleCopyOfIndexEdit, 0, 0);
                
                k=zw->end;
                
                if(i < 0)
                    i = 0;
                
                if(i >= ed->ew.doc->numwave)
                    i = ed->ew.doc->numwave-1;
                
                zw2 = (ed->ew.doc->waves + i);
                
                if(zw2->copy != -1)
                    i = j;
                
                ed->init = 1;
                
                SetDlgItemInt(p_win, ID_Samp_SampleCopyOfIndexEdit, i, 0);
                
                ed->init = 0;
                
                if(i == j)
                    break;
                
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                
                goto chgcopy;
            }
            
            break;
        
        case HM_EN_KILLFOCUS(ID_Samp_SampleLengthEdit):
            
            if(ed->flag & Samp_Length_Changed)
            {
                ed->flag &= ~(Samp_Length_Changed);
                
                zw = ed->zw;
                
                i = GetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, 0, 0);
                k = zw->end;
                
                if(i < 0)
                {
                    i = 0;
                    
                chglen:
                    
                    SetDlgItemInt(p_win, ID_Samp_SampleLengthEdit, i, 0);
                    
                    ed->init = 0;
                    
                    break;
                }
                
                if(i > 65536)
                {
                    i = 65536;
                    
                    goto chglen;
                }
                
                b = (char*) realloc(zw->buf, (i + 1) << 1);
                
                if( ! b )
                {
                    MessageBox
                    (
                        framewnd,
                        "Not enough memory",
                        "Bad error happened",
                        MB_OK
                    );
                    
                    i = k;
                    ed->init = 1;
                    
                    goto chglen;
                }
                else
                    zw->buf = (short*) b;
                
                zw->end=i;
                
                for(j = k; j < i; j++)
                    zw->buf[j] = 0;
                
                if(zw->lopst!=-1)
                    zw->buf[zw->end]=zw->buf[zw->lopst];
                
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                
                SampleEditDlg_UpdateDisplay(ed, p_win);
            }
            
            break;
        
        case HM_EN_KILLFOCUS(ID_Samp_LoopPointEdit):
            
            if(ed->flag & Samp_LoopPoint_Changed)
            {
                ed->flag &= ~(Samp_LoopPoint_Changed);
                
                zw = ed->zw;
                
                j = i = GetDlgItemInt(p_win,
                                      ID_Samp_LoopPointEdit,
                                      0,
                                      0);
                
                if(i < 0)
                    i = 0;
                else if(i >= zw->end)
                    i = zw->end - 1;
                
                if(i != j)
                {
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
        
        case HM_EN_KILLFOCUS(ID_Samp_InstrumentIndexEdit):
            
            if(ed->flag & Samp_InstrumentIndex_Changed)
            {
                ed->flag &= ~(Samp_InstrumentIndex_Changed);
                
                i=GetDlgItemInt(p_win, ID_Samp_InstrumentIndexEdit, 0,0);
                
                if(i<0)
                    i = 0;
                else if(i>=ed->ew.doc->numinst)
                    i = ed->ew.doc->numinst-1;
                else
                    goto nochginst;
                
                ed->init=1;
                SetDlgItemInt(p_win, ID_Samp_InstrumentIndexEdit, i, 0);
                ed->init=0;
nochginst:
                
                ed->editinst=i;
                
                LoadInstrument(p_win, ed);
            }
            
            break;
        
        case HM_EN_KILLFOCUS(ID_Samp_InstrFrequencyEdit):
        case HM_EN_KILLFOCUS(ID_Samp_InstrADSR_Edit):
        case HM_EN_KILLFOCUS(ID_Samp_InstrGainEdit):
        case HM_EN_KILLFOCUS(ID_Samp_InstrSampleIndex_Edit):
            
            if(ed->flag & Samp_InstrumentConfig_Changed)
            {
                text_buf_ty text_buf = { 0 };
                
                ed->flag &= ~(Samp_InstrumentConfig_Changed);
                
                zi = (ed->ew.doc->insts + ed->editinst);
                
                i = GetDlgItemInt(p_win, ID_Samp_InstrFrequencyEdit, 0,0);
                
                zi->multlo = i;
                zi->multhi = i >> 8;
                
                GetDlgItemText(p_win, ID_Samp_InstrADSR_Edit, text_buf, 5);
                
                i = strtol(text_buf, 0, 16);

                zi->sr = i;
                zi->ad = i >> 8;
                
                GetDlgItemText(p_win, ID_Samp_InstrGainEdit, text_buf, 3);
                
                zi->gain = (uint8_t) strtol(text_buf, 0, 16);
                
                zi->samp = GetDlgItemInt(p_win, ID_Samp_InstrSampleIndex_Edit, 0,0);
                
                ed->ew.doc->m_modf = 1;
                ed->ew.doc->w_modf = 1;
            }
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_SampleIndexEdit):
            
            ed->flag |= Samp_Index_Changed;
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_SampleCopyOfIndexEdit):
            
            ed->flag |= Samp_CopyIndex_Changed;
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_SampleLengthEdit):
            
            ed->flag |= Samp_Length_Changed;
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_LoopPointEdit):
            
            ed->flag |= Samp_LoopPoint_Changed;
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_InstrumentIndexEdit):
            
            ed->flag |= Samp_InstrumentIndex_Changed;
            
            break;
        
        case HM_EN_CHANGE(ID_Samp_InstrFrequencyEdit):
        case HM_EN_CHANGE(ID_Samp_InstrADSR_Edit):
        case HM_EN_CHANGE(ID_Samp_InstrGainEdit):
        case HM_EN_CHANGE(ID_Samp_InstrSampleIndex_Edit):
            
            ed->flag |= Samp_InstrumentConfig_Changed;
            
            break;
        }
    }
    
    return FALSE;
}

// =============================================================================

#include "structs.h"
#include "prototypes.h"

#include "HMagicLogic.h"

// =============================================================================

BOOL CALLBACK
editsprname(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j;
    static CONFIG * config = 0;
    SCROLLINFO si;
    
    if(msg==WM_HSCROLL)
    {
        text_buf_ty text_buf = { 0 };
        
        i=soundvol;
        switch(wparam&65535) {
        case SB_LEFT:
            i=0;
            break;
        case SB_RIGHT:
            i=256;
            break;
        case SB_LINELEFT:
            i--;
            break;
        case SB_LINERIGHT:
            i++;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        case SB_PAGELEFT:
            i-=16;
            break;
        case SB_PAGERIGHT:
            i+=16;
        }
        if(i<0) i=0;
        if(i>256) i=256;
        soundvol=i;
        config->flag|=4;
        volflag|=255;
        si.cbSize=sizeof(si);
        si.nPos=i;
        si.fMask=SIF_POS;
        SetScrollInfo((HWND)lparam,SB_CTL,&si,1);
soundvolchg:
        
        wsprintf(text_buf,
                 "%d%%",
                 soundvol * 100 / 256);
        
        SetDlgItemText(win, IDC_STATIC2, text_buf);
    }
    else if(msg == WM_COMMAND)
    {
        switch(wparam)
        {
        
        case IDC_EDIT1|(EN_CHANGE<<16):
        updname:
            
            i=GetDlgItemInt(win,IDC_EDIT1,0,0);
            if(config->sprsetedit) j=0x1b; else j=255;
            if(i>j || i<0) { SetDlgItemInt(win,IDC_EDIT1,j,0); break; }
            
            config->namechg=1;
            SetDlgItemText(win,IDC_EDIT2,sprname[i+config->sprsetedit]);
            config->namechg=0;
            
            break;
        
        case IDC_EDIT2|(EN_CHANGE<<16):
            
            if(config->namechg) break;
            i=GetDlgItemInt(win,IDC_EDIT1,0,0);
            config->flag|=1;
            Updatesprites();
            GetDlgItemText(win,IDC_EDIT2,sprname[i+config->sprsetedit],16);
            Updatesprites();
            
            break;
        
        case IDC_EDIT3|(EN_CHANGE<<16):
            
            i=GetDlgItemInt(win,IDC_EDIT3,0,0);
            if(i>24 || i<0) { SetDlgItemInt(win,IDC_EDIT3,24,0); break; }
            config->namechg=1;
            SetDlgItemInt(win,IDC_EDIT4,midi_inst[i]&255,0);
            SetDlgItemInt(win,IDC_EDIT5,midi_inst[i]>>8,0);
            SetDlgItemInt(win,IDC_EDIT6,(char)(midi_trans[i]),1);
            SetDlgItemInt(win,IDC_EDIT8,(char)(midi_trans[i]>>8),1);
            config->namechg=0;
            
            break;
        
        case IDC_EDIT4|(EN_CHANGE<<16):
        case IDC_EDIT5|(EN_CHANGE<<16):
        case IDC_EDIT6|(EN_CHANGE<<16):
        case IDC_EDIT8|(EN_CHANGE<<16):
            
            if(config->namechg) break;
            i=GetDlgItemInt(win,IDC_EDIT3,0,0);
            midi_inst[i]=GetDlgItemInt(win,IDC_EDIT4,0,0)+(GetDlgItemInt(win,IDC_EDIT5,0,0)<<8);
            midi_trans[i]=(GetDlgItemInt(win,IDC_EDIT6,0,1)&255)+(GetDlgItemInt(win,IDC_EDIT8,0,1)<<8);
            config->flag|=8;
            break;
        
        case IDC_EDIT9|(EN_CHANGE<<16):
            if(config->namechg) break;
            GetDlgItemText(win,IDC_EDIT9,asmpath,MAX_PATH);
            config->flag|=16;
            
            break;
        
        case IDC_RADIO1:
            config->sprsetedit=0;
            
            goto updname;
        
        case IDC_RADIO3:
            
            config->sprsetedit=256;
            
            goto updname;
        
        case IDCANCEL:
            
            if(config->flag & 1)
            {
                Updatesprites();
                memcpy(sprname,config->savespr,0x11c0);
                Updatesprites();
            }
            
            strcpy(asmpath,config->saveasmp);
            
            memcpy(midi_inst, config->inst, MIDI_ARR_BYTES);
            memcpy(midi_trans, config->trans, MIDI_ARR_BYTES);
            
            soundvol=config->soundvol;
            free(config);
            EndDialog(win,0);
            
            break;
        
        case IDOK:
            
            cfg_flag |= config->flag;
            
            if(config->flag)
                cfg_modf = 1;
            
            free(config);
            
            EndDialog(win,0);
            
            break;
        }
    }
    else if(msg == WM_INITDIALOG)
    {
        config=malloc(sizeof(CONFIG));
        config->sprsetedit=0;
        config->namechg=1;
        config->soundvol=soundvol;
        config->flag=0;
        memcpy(config->savespr,sprname,0x11c0);
        memcpy(config->inst,midi_inst,100);
        CheckDlgButton(win,IDC_RADIO1,BST_CHECKED);
        SetDlgItemInt(win,IDC_EDIT1,0,0);
        SetDlgItemInt(win,IDC_EDIT3,0,0);
        SetDlgItemText(win,IDC_EDIT9,asmpath);
        strcpy(config->saveasmp,asmpath);
        config->namechg=0;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;
        si.nPos=soundvol;
        si.nMin=0;
        si.nMax=256;
        si.nPage=16;
        SetScrollInfo(GetDlgItem(win,IDC_SCROLLBAR1),SB_CTL,&si,0);
        goto soundvolchg;
    }
    
    return FALSE;
}

// =============================================================================

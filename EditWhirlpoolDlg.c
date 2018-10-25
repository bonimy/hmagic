
    #include "structs.h"

    #include "OverworldEdit.h"

// =============================================================================

BOOL CALLBACK
editwhirl(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    unsigned char*rom;
    int i,j,k,l,m,n;
    HWND hc;
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        rom = oved->ew.doc->rom;
        
        i=oved->selobj;
        j=(oved->ew.param&7)<<9;
        k=(oved->ew.param&56)<<6;
        
        if(i > 8)
            SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x16ce6))[i],0);
        else
        {
            ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_HIDE);
        }
        SetDlgItemInt(win,IDC_EDIT2,((short*)(rom + 0x16b29))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT3,((short*)(rom + 0x16b4b))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT4,((short*)(rom + 0x16bb1))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT5,((short*)(rom + 0x16bd3))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT6,((short*)(rom + 0x16b6d))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT7,((short*)(rom + 0x16b8f))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT12,((short*)(rom + 0x16ae5))[i],0);
        SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x16bf5 + i],1);
        SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16c17 + i],1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_RADIO2:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),0);
            break;
        case IDC_RADIO4: case IDC_RADIO5:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),1);
            break;
        case IDC_RADIO6:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),0);
            break;
        case IDC_RADIO7: case IDC_RADIO8:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),1);
            break;
        case IDOK:
            hc=GetDlgItem(oved->dlg,3001);
            Overselchg(oved,hc);
            rom=oved->ew.doc->rom;
            i=oved->selobj;
            l=((short*)(rom + 0x16ae5))[i]=GetDlgItemInt(win,IDC_EDIT12,0,0);
            n=(rom[0x12884 + l]<<8)|0xff;
            j=(l&7)<<9;
            k=(l&56)<<6;
            if(l!=oved->ew.param) oved->selobj=-1;
            if(i>8) ((short*)(rom + 0x16ce6))[i]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ((short*)(rom + 0x16b29))[i]=(l=GetDlgItemInt(win,IDC_EDIT2,0,0)&n)+k;
            ((short*)(rom + 0x16b4b))[i]=(m=GetDlgItemInt(win,IDC_EDIT3,0,0)&n)+j;
            ((short*)(rom + 0x16b07))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
            ((short*)(rom + 0x16bb1))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
            ((short*)(rom + 0x16bd3))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
            ((short*)(rom + 0x16b6d))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
            ((short*)(rom + 0x16b8f))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
            rom[0x16bf5 + i] = GetDlgItemInt(win,IDC_EDIT13,0,1);
            rom[0x16c17 + i] = GetDlgItemInt(win,IDC_EDIT14,0,1);
            Overselchg(oved,hc);
            oved->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

// =============================================================================


    #include "structs.h"

    #include "OverworldEnum.h"
    #include "OverworldEdit.h"

// =============================================================================

BOOL CALLBACK
editexit(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    unsigned char*rom;
    int i,j,k,l,m,n,o;
    HWND hc;
    const static int radio_ids[6]={IDC_RADIO2,IDC_RADIO4,IDC_RADIO5,IDC_RADIO6,IDC_RADIO7,IDC_RADIO8};
    const static int door_ofs[2]={0x16367,0x16405};
    const static int edit_ids[6]={IDC_EDIT8,IDC_EDIT9,IDC_EDIT10,IDC_EDIT11};
    const static int hide_ids[20]={IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,IDC_STATIC6,
        IDC_EDIT15,IDC_EDIT22,IDC_EDIT23,IDC_EDIT24,IDC_EDIT25,
        IDC_STATIC7,IDC_STATIC8,IDC_STATIC9,IDC_STATIC10,IDC_STATIC11,
        IDC_EDIT26,IDC_EDIT27,IDC_EDIT28,IDC_EDIT29,IDC_EDIT30};
    
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        rom=oved->ew.doc->rom;
        i=oved->selobj;
        j=(oved->ew.param&7)<<9;
        k=(oved->ew.param&56)<<6;
        SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x15d8a))[i],0);
        SetDlgItemInt(win,IDC_EDIT2,((short*)(rom + 0x15f15))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT3,((short*)(rom + 0x15fb3))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT4,((short*)(rom + 0x1618d))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT5,((short*)(rom + 0x1622b))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT6,((short*)(rom + 0x16051))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT7,((short*)(rom + 0x160ef))[i]-j,1);
        for(j=0;j<2;j++) {
            l=((unsigned short*)(rom+door_ofs[j]))[i];
            if(l && l!=65535) {
                m=(l>>15)+1;
                SetDlgItemInt(win,edit_ids[j*2],(l&0x7e)>>1,0);
                SetDlgItemInt(win,edit_ids[j*2+1],(l&0x3f80)>>7,0);
            } else {
                m=0;
                EnableWindow(GetDlgItem(win,edit_ids[j*2]),0);
                EnableWindow(GetDlgItem(win,edit_ids[j*2+1]),0);
            }
            m+=3*j;
            CheckDlgButton(win,radio_ids[m],BST_CHECKED);
        }
        SetDlgItemInt(win,IDC_EDIT12,rom[0x15e28 + i], 0);
        SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x162c9 + i], 1);
        SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16318 + i], 1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT1|(EN_CHANGE<<16):
            i=GetDlgItemInt(win,IDC_EDIT1,0,0);
            rom=oved->ew.doc->rom;
            if(i>=0x180 && i<0x190) {
                j=SW_SHOW;
                SetDlgItemInt(win,IDC_EDIT15,rom[0x16681 + i] >> 1, 0);
                SetDlgItemInt(win,IDC_EDIT22,rom[0x16691 + i], 0);
                SetDlgItemInt(win,IDC_EDIT23,rom[0x166a1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT24,rom[0x166b1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT25,rom[0x166c1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT26,((short*)(rom + 0x163e1))[i], 0);
                SetDlgItemInt(win,IDC_EDIT27,((short*)(rom + 0x16401))[i], 0);
                SetDlgItemInt(win,IDC_EDIT28,((short*)(rom + 0x16421))[i], 0);
                SetDlgItemInt(win,IDC_EDIT29,((short*)(rom + 0x16441))[i], 0);
                SetDlgItemInt(win,IDC_EDIT30,((short*)(rom + 0x164e1))[i], 0);
            } else j=SW_HIDE;
            for(k=0;k<20;k++)
                ShowWindow(GetDlgItem(win,hide_ids[k]),j);
            break;
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
            
            hc = GetDlgItem(oved->dlg, SD_Over_Display);
            Overselchg(oved,hc);
            
            rom=oved->ew.doc->rom;
            i=oved->selobj;
            l = rom[0x15e28 + i] = GetDlgItemInt(win,IDC_EDIT12,0,0);
            if(l!=oved->ew.param) oved->selobj=-1;
            n=oved->mapsize?1023:511;
            j=(l&7)<<9;
            k=(l&56)<<6;
            ((short*)(rom + 0x15d8a))[i]=o=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ((short*)(rom + 0x15f15))[i]=(l=(GetDlgItemInt(win,IDC_EDIT2,0,0)&n))+k;
            ((short*)(rom + 0x15fb3))[i]=(m=(GetDlgItemInt(win,IDC_EDIT3,0,0)&n))+j;
            ((short*)(rom + 0x15e77))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
            ((short*)(rom + 0x1618d))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
            ((short*)(rom + 0x1622b))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
            ((short*)(rom + 0x16051))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
            ((short*)(rom + 0x160ef))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
            m=0;
            for(j=0;j<2;j++) {
                if(IsDlgButtonChecked(win,radio_ids[m])) l=0;
                else {
                    l=(GetDlgItemInt(win,edit_ids[j*2],0,0)<<1)+(GetDlgItemInt(win,edit_ids[j*2+1],0,0)<<7);
                    if(IsDlgButtonChecked(win,radio_ids[m+2])) l|=0x8000;
                }
                ((short*)(rom+door_ofs[j]))[i]=l;
                m+=3;
            }
            rom[0x162c9 + i] = GetDlgItemInt(win,IDC_EDIT13,0,1);
            rom[0x16318 + i] = GetDlgItemInt(win,IDC_EDIT14,0,1);
            if(o>=0x180 && o<0x190)
            {
                rom[0x16681 + o] = GetDlgItemInt(win,IDC_EDIT15,0,0)<<1;
                rom[0x16691 + o] = GetDlgItemInt(win,IDC_EDIT22,0,0);
                rom[0x166a1 + o] = GetDlgItemInt(win,IDC_EDIT23,0,0);
                rom[0x166b1 + o] = GetDlgItemInt(win,IDC_EDIT24,0,0);
                rom[0x166c1 + o] = GetDlgItemInt(win,IDC_EDIT25,0,0);
                
                ((short*)(rom + 0x163e1))[o]=GetDlgItemInt(win,IDC_EDIT26,0,0);
                ((short*)(rom + 0x16401))[o]=GetDlgItemInt(win,IDC_EDIT27,0,0);
                ((short*)(rom + 0x16421))[o]=GetDlgItemInt(win,IDC_EDIT28,0,0);
                ((short*)(rom + 0x16441))[o]=GetDlgItemInt(win,IDC_EDIT29,0,0);
                ((short*)(rom + 0x164e1))[o]=GetDlgItemInt(win,IDC_EDIT30,0,0);
            }
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

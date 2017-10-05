
    #include "structs.h"
    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "HMagicUtility.h"

// ==============================================================================

BOOL CALLBACK
editroomprop(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int i,l;
    HWND hc;
    static int hs;
    unsigned char*rom;
    const static char*ef_str[8]={
        "Nothing","01","Moving floor","Moving water","04","Red flashes","Light torch to see floor","Ganon room"
    };
    const static char * tag_str[64] = {
        "Nothing","NW Kill enemy to open","NE Kill enemy to open","SW Kill enemy to open","SE Kill enemy to open","W Kill enemy to open","E Kill enemy to open","N Kill enemy to open","S Kill enemy to open","Clear quadrant to open","Clear room to open",
        "NW Move block to open","NE Move block to open","SW Move block to open","SE Move block to open","W Move block to open","E Move block to open","N Move block to open",
        "S Move block to open","Move block to open","Pull lever to open","Clear level to open door","Switch opens door(Hold)","Switch opens door(Toggle)",
        "Turn off water","Turn on water","Water gate","Water twin","Secret wall (Right)","Secret wall (Left)","Crash","Crash",
        "Use switch to bomb wall","Holes(0)","Open chest for holes(0)","Holes(1)","Holes(2)","Kill enemy to clear level","SE Kill enemy to move block","Trigger activated chest",
        "Use lever to bomb wall","NW Kill enemy for chest","NE Kill enemy for chest","SW Kill enemies for chest","SE Kill enemy for chest","W Kill enemy for chest","E Kill enemy for chest","N Kill enemy for chest",
        "S Kill enemy for chest","Clear quadrant for chest","Clear room for chest","Light torches to open","Holes(3)","Holes(4)","Holes(5)","Holes(6)",
        "Agahnim's room","Holes(7)","Holes(8)","Open chest for holes(8)","Move block to get chest","Kill to open Ganon's door","Light torches to get chest","Kill boss again",
    };
    const static int warp_ids[17]={
        IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,
        IDC_STATIC6,IDC_STATIC7,IDC_STATIC8,IDC_EDIT4,
        IDC_EDIT6,IDC_EDIT7,IDC_EDIT15,IDC_EDIT16,
        IDC_EDIT17,IDC_EDIT18,IDC_EDIT19,IDC_EDIT20,
        IDC_EDIT21
    };
    const static char warp_flag[20]={
        0x07,0x07,0x07,0x0f,0x1f,0x3f,0x7f,0x07,
        0x07,0x0f,0x0f,0x1f,0x1f,0x3f,0x3f,0x7f,
        0x7f,0,0,0
    };
    
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        rom=dunged->ew.doc->rom;
        hc=GetDlgItem(win, IDC_COMBO1);
        for(i=0;i<8;i++) SendMessage(hc,CB_ADDSTRING,0,(long)ef_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[4],0);
        hc=GetDlgItem(win,IDC_COMBO2);
        for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[5],0);
        hc=GetDlgItem(win,IDC_COMBO3);
        for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[6],0);
        SendDlgItemMessage(win,IDC_BUTTON1,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[2]);
        SendDlgItemMessage(win,IDC_BUTTON3,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[3]);
        hs=dunged->hsize;
        SetDlgItemInt(win,IDC_EDIT6,dunged->hbuf[7]&3,0);       // Hole/warp plane
        SetDlgItemInt(win,IDC_EDIT15,(dunged->hbuf[7]>>2)&3,0); // Staircase plane 1
        SetDlgItemInt(win,IDC_EDIT17,(dunged->hbuf[7]>>4)&3,0); // Staircase plane 2
        SetDlgItemInt(win,IDC_EDIT19,(dunged->hbuf[7]>>6)&3,0);
        SetDlgItemInt(win,IDC_EDIT21,dunged->hbuf[8]&3,0);
        i=dunged->mapnum&0xff00;
        SetDlgItemInt(win,IDC_EDIT4,dunged->hbuf[9]+i,0);  // Hole/warp room
        SetDlgItemInt(win,IDC_EDIT7,dunged->hbuf[10]+i,0); // Staircase 1 room
        SetDlgItemInt(win,IDC_EDIT16,dunged->hbuf[11]+i,0); // Staircase 2 room
        SetDlgItemInt(win,IDC_EDIT18,dunged->hbuf[12]+i,0);
        SetDlgItemInt(win,IDC_EDIT20,dunged->hbuf[13]+i,0);
        
        SetDlgItemInt(win,
                      IDC_TELEPATH_EDIT,
                      ldle16b_i(rom + 0x3f61d, dunged->mapnum),
                      0);
        
        for(i=0;i<57;i++) if(((short*)(rom + 0x190c))[i]==dunged->mapnum) {
            CheckDlgButton(win,IDC_CHECK5,BST_CHECKED);
            break;
        }
        
    updbtn:
        
        l = 1 << (hs - 7);
        
        for(i=0;i<17;i++)
            ShowWindow(GetDlgItem(win,warp_ids[i]),(warp_flag[i]&l)?SW_HIDE:SW_SHOW);
        
        EnableWindow(GetDlgItem(win,IDC_BUTTON1),hs>7);
        EnableWindow(GetDlgItem(win,IDC_BUTTON3),hs<14);
        
        break;
    
    case WM_COMMAND:
        switch(wparam) {
        case IDC_BUTTON1:
            hs--;
            if(hs==9) hs=7;
            goto updbtn;
        case IDC_BUTTON3:
            hs++;
            if(hs==8) hs=10;
            goto updbtn;
        case IDC_CHECK5:
            rom=dunged->ew.doc->rom;
            if(IsDlgButtonChecked(win,IDC_CHECK5)) {
                for(i=0;i<57;i++)
                    if( is16b_neg1_i(rom + 0x190c, i) )
                    {
                        ((short*)(rom + 0x190c))[i]=dunged->mapnum;
                        break;
                    }
                
                if(i == 57)
                {
                    MessageBox(framewnd,"You can't add anymore.","Bad error happened",MB_OK);
                    CheckDlgButton(win,IDC_CHECK5,BST_UNCHECKED);
                }
            } else for(i=0;i<57;i++) if(((short*)(rom + 0x190c))[i]==dunged->mapnum) {
                ((short*)(rom + 0x190c))[i]=-1;
                break;
            }
            break;
        case IDOK:
            rom=dunged->ew.doc->rom;
            dunged->hsize = hs;
            dunged->hbuf[4] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO1,CB_GETCURSEL,0,0);
            dunged->hbuf[5] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO2,CB_GETCURSEL,0,0);
            dunged->hbuf[6] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO3,CB_GETCURSEL,0,0);
            dunged->hbuf[7]=(GetDlgItemInt(win,IDC_EDIT6,0,0)&3)|
            ((GetDlgItemInt(win,IDC_EDIT15,0,0)&3)<<2)|
            ((GetDlgItemInt(win,IDC_EDIT17,0,0)&3)<<4)|
            ((GetDlgItemInt(win,IDC_EDIT19,0,0)&3)<<6);
            dunged->hbuf[8]=GetDlgItemInt(win,IDC_EDIT21,0,0)&3;
            dunged->hbuf[9]=GetDlgItemInt(win,IDC_EDIT4,0,0);
            dunged->hbuf[10]=GetDlgItemInt(win,IDC_EDIT7,0,0);
            dunged->hbuf[11]=GetDlgItemInt(win,IDC_EDIT16,0,0);
            dunged->hbuf[12]=GetDlgItemInt(win,IDC_EDIT18,0,0);
            dunged->hbuf[13]=GetDlgItemInt(win,IDC_EDIT20,0,0);
            
            stle16b_i(rom + 0x3f61d,
                      dunged->mapnum,
                      GetDlgItemInt(win, IDC_TELEPATH_EDIT, 0, 0) );
            
            dunged->modf  = 1;
            dunged->hmodf = 1;
            
            dunged->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
        }
    }
    return FALSE;
}

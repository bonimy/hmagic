
    #include "structs.h"

    #include "Wrappers.h"

    #include "AudioLogic.h"

// =============================================================================

    static const char *
    amb_str[9] =
    {
        "Nothing",
        "Heavy rain",
        "Light rain",
        "Stop",
        "Earthquake",
        "Wind",
        "Flute",
        "Chime 1",
        "Chime 2"
    };

// ==============================================================================

extern BOOL
CALLBACK editovprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    HWND hc;
    
    int i,j,k;
    
    unsigned char * rom;
    
    int cb_ids[4] = {IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO7};
    int cb2_ids[4] = {IDC_COMBO4, IDC_COMBO5, IDC_COMBO6, IDC_COMBO8};
    int text_ids[4] = {IDC_STATIC2, IDC_STATIC3, IDC_STATIC4};
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        i = 0;
        
        if(oved->ew.param>=0x40)
        {
            for( ; i < 3; i++)
            {
                ShowWindow( GetDlgItem(win, cb_ids[i]), SW_HIDE);
                ShowWindow( GetDlgItem(win, cb2_ids[i]), SW_HIDE);
                ShowWindow( GetDlgItem(win, text_ids[i]), SW_HIDE);
            }
            
            ShowWindow( GetDlgItem(win, IDC_STATIC5), SW_HIDE);
        }
        
        rom = oved->ew.doc->rom;
        
        for( ; i < 4; i++)
        {
            k = rom[0x14303 + oved->ew.param + (i << 6)];
            
            if(k >= 16)
            {
                k += 16;
            }
            
            hc = GetDlgItem(win, cb_ids[i]);
            
            for(j = 0; j < 16; j += 1)
            {
                HM_ComboBox_AddString(hc, mus_str[j + 1]);
            }
            
            HM_ComboBox_SelectItem(hc, k & 15);
            
            hc = GetDlgItem(win, cb2_ids[i]);
            
            for(j = 0; j < 9; j++)
            {
                HM_ComboBox_AddString(hc, amb_str[j]);
            }
            
            HM_ComboBox_SelectItem(hc, k >> 5);
        }
        
        SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x3f51d))[oved->ew.param],0);
        
        break;
    
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case IDOK:
            
            rom = oved->ew.doc->rom;
            
            if(oved->ew.param >= 0x40)
                i = 3;
            else
                i = 0;
            
            for( ; i < 4; i++)
            {
                hc = GetDlgItem(win,cb_ids[i]);
                
                k = HM_ComboBox_GetSelectedItem(hc);
                
                hc = GetDlgItem(win,cb2_ids[i]);
                
                k |= HM_ComboBox_GetSelectedItem(hc) << 5;
                
                if(k >= 32)
                    k -= 16;
                
                rom[0x14303 + oved->ew.param + (i << 6)] = k;
            }
            
            ((short*)(rom + 0x3f51d))[oved->ew.param]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            
            oved->ew.doc->modf;
        
        case IDCANCEL:
            
            EndDialog(win,0);
        }
    }
    
    return 0;
}

    #include "structs.h"

// =============================================================================

extern BOOL CALLBACK
editbosslocs(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    static int num;
    static FDOC*doc;
    static unsigned char*rom;
    int i;
    switch(msg) {
    case WM_INITDIALOG:
        num=lparam;
        doc=activedoc;
        rom=doc->rom;
        SetDlgItemInt(win,IDC_EDIT1,i=((short*)(rom + 0x10954))[num],0);
        SetDlgItemInt(win,IDC_EDIT2,rom[0x792d + num]+(i&256),0);
        SetDlgItemInt(win,IDC_EDIT3,rom[0x7939 + num]+(i&256),0);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ((short*)(rom + 0x10954))[num]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            rom[0x792d + num]=GetDlgItemInt(win,IDC_EDIT2,0,0);
            rom[0x7939 + num]=GetDlgItemInt(win,IDC_EDIT3,0,0);
            doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
        }
    }
    return FALSE;
}

// =============================================================================
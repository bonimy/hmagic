
    #include "structs.h"

// =============================================================================

    extern BOOL CALLBACK
    AboutDlg(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
    {
        (void) lparam;
        
        if(msg == WM_COMMAND && wparam == IDCANCEL)
            EndDialog(win,0);
        
        return FALSE;
    }

// =============================================================================

    #include "structs.h"

// =============================================================================

BOOL CALLBACK
errorsproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    DWORD read_bytes = 0;
    
    int i;
    
    char *b;
    
    HANDLE h;
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        h = CreateFile("HMAGIC.ERR",GENERIC_READ,0,0,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
        
        if(h == INVALID_HANDLE_VALUE)
        {
            break;
        }
        
        i = GetFileSize(h, 0);
        
        b = (char*) malloc(i + 1);
        
        ReadFile(h, b, i, &read_bytes, 0);
        
        CloseHandle(h);
        
        b[i] = 0;
        
        SetDlgItemText(win,IDC_EDIT1,b);
        
        free(b);
        
        break;
    
    case WM_COMMAND:
        
        if(wparam == IDCANCEL)
            EndDialog(win, 0);
    }
    
    return 0;
}

// =============================================================================
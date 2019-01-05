
    #include "structs.h"
    #include "prototypes.h"

// =============================================================================

// for copying rooms or overworld areas.
BOOL CALLBACK
duproom(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i, // source room/map
        j, // destination room/map
        k, // switch for map type (dungeon or overworld)
        l; // max index for each respective type. e.g. we have at most 296 overworld maps.
    
    FDOC *doc;
    
    switch(msg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(win,DWLP_USER,lparam);
        
        // the function apparently
        // passes in the associated rom file's pointer.
        doc = (FDOC*) lparam;
        
        // Fill in the default button.
        // otherwise it comes up blank.
        CheckDlgButton(win,IDC_RADIO1,BST_CHECKED);
        
        break;
    
    case WM_COMMAND:
        switch(wparam)
        {
        
        case IDOK: // they picked the OK button
            
            doc = (FDOC*) GetWindowLongPtr(win,DWLP_USER);
            
            i = GetDlgItemInt(win,IDC_EDIT1,0,0); // get the source room/map number
            
            j = GetDlgItemInt(win,IDC_EDIT2,0,0); // get the destination room/map number
            
            k = IsDlgButtonChecked(win, IDC_RADIO1) == BST_CHECKED; //1 = overworld, 0 = dungeon.
            
            // if k = 1 (overworld), l = 160, else l = 296 (dungeon)
            l = k ? 160 : 296;
            
            // "l" looks a lot like the number 1, doesn't it?
            if(i < 0 || j < 0 || i >= l || j >= l || i == j)
            {
                MessageBox(framewnd,
                           "Please enter two different room numbers in the appropriate range.",
                           "Bad error happened",
                           MB_OK);
                
                break;
            }
            
            if(k)
            {
                //overworld
                Changesize(doc, 0x50000 + j, i);
                Changesize(doc, 0x500a0 + j, 160 + i);
            }
            else
                //dungeon
                Changesize(doc,0x50140 + j,320+i);
        
        case IDCANCEL:
            
            // kill the dialog, and do nothing.
            EndDialog(win,0);
        }
    }
    
    return 0;
}

// =============================================================================


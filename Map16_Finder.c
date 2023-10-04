
    #include "structs.h"

    #include "prototypes.h"

// =============================================================================

BOOL CALLBACK
findblks(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    DWORD read_bytes  = 0;
    DWORD write_bytes = 0;
    
    OVEREDIT*ed;
    HWND hc;
    HANDLE h;
    static OPENFILENAME ofn;
    static char blkname[MAX_PATH]="findblks.dat";
    
    int i,j;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(win,DWLP_USER,lparam);
        ed=(OVEREDIT*)lparam;
        ed->schpush=-1;
        ed->schtyped=0;
        hc=GetDlgItem(win,IDC_CUSTOM1);
        SetWindowLongPtr(hc,GWLP_USERDATA,lparam);
        Updatesize(hc);
        break;
    case WM_COMMAND:
        ed=(OVEREDIT*)GetWindowLongPtr(win,DWLP_USER);
        switch(wparam) {
        case IDC_BUTTON1:
            for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=255;
upddisp:
            InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
            break;
        case IDC_BUTTON2:
            for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON3:
            for(i=0;i<0x1d5;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON4:
            for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON5:
            for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=255;
            goto upddisp;
        case IDC_BUTTON6:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=blkname;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Load block filter";
            ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            
            if(!GetOpenFileName(&ofn))
                break;
            
            h = CreateFile(ofn.lpstrFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
            
            ReadFile(h, ed->selsch, 1408, &read_bytes, 0);
            CloseHandle(h);
            goto upddisp;
        case IDC_BUTTON7:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=blkname;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Save block filter";
            ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            if(!GetSaveFileName(&ofn)) break;
            h=CreateFile(ofn.lpstrFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,0);
            WriteFile(h,ed->selsch,1408, &write_bytes, 0);
            CloseHandle(h);
            break;
        case IDOK:
            
            j = 0;
            
            for(i=0;i<0xea8;i++)
            {
                if(ed->selsch[i>>3]&(1<<(i&7)))
                {
                    if(j==4)
                    {
                        MessageBox(framewnd,
                                   "Please check only up to 4 \"Yes\" boxes.",
                                   "Bad error happened",
                                   MB_OK);
showpos:
                        ed->schscroll = Handlescroll
                        (
                            GetDlgItem(win, IDC_CUSTOM1),
                            SB_THUMBPOSITION + ( ( i - (ed->schpage >> 1) ) << 16),
                            ed->schscroll,
                            ed->schpage,
                            SB_VERT,
                            0xea8,
                            16
                        );
                        
                        goto brk;
                    }
                    
                    if(ed->selsch[(i + 0xea8) >> 3] & (1 << (i & 7) ) )
                    {
                        MessageBox(framewnd,"You can't require and disallow a block at the same time.","Bad error happened",MB_OK);
                        goto showpos;
                    }
                    
                    ed->schyes[j++]=i;
                }
            }
            for(;j<4;j++) ed->schyes[j]=-1;
            EndDialog(win,1);
brk:
            break;
        case IDCANCEL:
            EndDialog(win,0);
        }
        return 1;
    }
    return 0;
}
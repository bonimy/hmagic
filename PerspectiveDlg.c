
    #include "structs.h"

    #include "GdiObjects.h"

    #include "prototypes.h"

// =============================================================================

    static void
    PerspectiveDlg_Init
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        int i;
        int j;
        
        text_buf_ty buf;
        
        HWND const hc = GetDlgItem(p_win, 3000);
        
        PERSPEDIT * const ed = (PERSPEDIT*) p_lp;
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, DWLP_USER, p_lp);
        
        ed->xrot = ed->yrot = ed->zrot=0;
        
        ed->objsel = 0;
        ed->tool = 0;
        ed->selpt = -1;
        ed->enlarge = 100;
        ed->newlen = 0;
        ed->newptp = -1;
        ed->modf = 0;
        
        memcpy(ed->buf, ed->ew.doc->rom + 0x4ff8c, 116);
        
        i = *(unsigned short*)(ed->buf + 10) - 0xff8c;
        j = ed->buf[7];
        
        for( ; j ; j--)
            i += ed->buf[i] + 2;
        
        ed->len=i;
        
        SetWindowLongPtr(hc, GWLP_USERDATA, p_lp);
        
        Updatesize(hc);
        
        SendDlgItemMessage(p_win, 3001, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) arrows_imgs[0]);
        SendDlgItemMessage(p_win, 3002, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) arrows_imgs[1]);
        
        wsprintf(buf, "Free: %d", 116 - ed->len);
        
        SetDlgItemText(p_win, 3003, buf);
        CheckDlgButton(p_win, 3004, BST_CHECKED);
    }

// =============================================================================

BOOL CALLBACK perspdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PERSPEDIT*ed;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        PerspectiveDlg_Init(win, lparam);
        
        break;
    
    case WM_COMMAND:
        ed=(PERSPEDIT*)GetWindowLongPtr(win,DWLP_USER);
        switch(wparam) {
        case 3001:
            ed->objsel=0;
            goto updsel;
        case 3002:
            ed->objsel=6;
            goto updsel;
        case 3004:
            ed->tool=0;
updsel:
            ed->newptp=-1;
            ed->newlen=0;
            ed->selpt=-1;
            InvalidateRect(GetDlgItem(win,3000),0,1);
            break;
        case 3005:
            ed->tool=1;
            goto updsel;
        case 3006:
            ed->tool=2;
            goto updsel;
        case 3007:
            ed->tool=3;
            goto updsel;
        }
    }
    return FALSE;
}

// =============================================================================
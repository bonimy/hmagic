
#include "structs.h"
#include "prototypes.h"

#include "HMagicLogic.h"

#include "AudioLogic.h"

// =============================================================================

BOOL CALLBACK
wavesetting(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        SetDlgItemInt(win,IDC_EDIT1,ws_freq,0);
        SetDlgItemInt(win,IDC_EDIT2,ws_bufs,0);
        SetDlgItemInt(win,IDC_EDIT3,ws_len,0);
        CheckDlgButton(win,IDC_CHECK1,ws_flags&1);
        CheckDlgButton(win,IDC_CHECK2,(ws_flags>>1)&1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ws_freq=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ws_bufs=GetDlgItemInt(win,IDC_EDIT2,0,0);
            ws_len=GetDlgItemInt(win,IDC_EDIT3,0,0);
            ws_flags=IsDlgButtonChecked(win,IDC_CHECK1)
                |(IsDlgButtonChecked(win,IDC_CHECK2)<<1);
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}
BOOL CALLBACK midisetting(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        SetDlgItemInt(win,IDC_EDIT1,ms_tim1,0);
        SetDlgItemInt(win,IDC_EDIT2,ms_tim2,0);
        SetDlgItemInt(win,IDC_EDIT3,ms_tim3,0);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ms_tim1=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ms_tim2=GetDlgItemInt(win,IDC_EDIT2,0,0);
            ms_tim3=GetDlgItemInt(win,IDC_EDIT3,0,0);
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

// =============================================================================


// =============================================================================

int
Soundsetting(HWND const win,
             int  const dev)
{
    if((dev>>16)==1)
    {
        return ShowDialog(hinstance,(LPSTR)IDD_DIALOG3,win,wavesetting,0);
    }
    else
        return ShowDialog(hinstance,(LPSTR)IDD_DIALOG13,win,midisetting,0);
}

// =============================================================================

BOOL CALLBACK
seldevproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,lp;
    TVINSERTSTRUCT tvi;
    TVITEM*itemstr;
    WAVEOUTCAPS woc;
    MIDIOUTCAPS moc;
    HWND hc;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(win,DWLP_USER,0);
        hc=GetDlgItem(win,IDC_TREE1);
        tvi.hParent=0;
        tvi.hInsertAfter=TVI_LAST;
        tvi.item.mask=TVIF_CHILDREN|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
        tvi.item.stateMask=TVIS_BOLD;
        tvi.item.state=0;
        tvi.item.lParam=0;
        tvi.item.pszText="Wave devices";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        tvi.item.pszText="Wave mapper";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x10001;
        SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        j=waveOutGetNumDevs();
        if(j>256) j=256;
        tvi.item.pszText=woc.szPname;
        for(i=0;i<j;i++) {
            if(waveOutGetDevCaps(i,&woc,sizeof(woc))) continue;
            tvi.item.lParam=0x10002 + i;
            SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        }
        tvi.hParent=0;
        tvi.item.pszText="Midi devices";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        tvi.item.pszText="Midi mapper";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x20000;
        SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        j=midiOutGetNumDevs();
        if(j>256) j=256;
        tvi.item.pszText=moc.szPname;
        for(i=0;i<j;i++) {
            if(midiOutGetDevCaps(i,&moc,sizeof(moc))) continue;
            tvi.item.lParam=0x20001 + i;
            SendMessage(hc,TVM_INSERTITEM,0, (LPARAM) &tvi);
        }
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            lp=GetWindowLongPtr(win,DWLP_USER);
            if(!lp) break;
            if(!Soundsetting(win,lp)) break;
            Exitsound();
            sounddev=lp;
            Initsound();
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
        break;
    case WM_NOTIFY:
        switch(wparam) {
        case IDC_TREE1:
            switch(((NMHDR*)lparam)->code) {
            case TVN_SELCHANGED:
                itemstr=&(((NMTREEVIEW*)lparam)->itemNew);
                SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_GETITEM, 0,(LPARAM) itemstr);
                SetWindowLongPtr(win,DWLP_USER,itemstr->lParam);
                break;
            }
        }
    }
    return FALSE;
}
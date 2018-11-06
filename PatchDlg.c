
    #include "structs.h"

    #include "prototypes.h"

    #include "PatchLogic.h"

// =============================================================================

BOOL CALLBACK patchdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PATCHLOAD*ed;
    ASMHACK*mod;
    OPENFILENAME ofn;
    FDOC*doc;
    HWND hc;
    int i,j;
    char patchname[MAX_PATH];
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(PATCHLOAD*)lparam;
        ed->dlg=win;
        doc=ed->ew.doc;
        mod=doc->modules;
        hc=GetDlgItem(win,3000);
        j=doc->nummod;
        for(i=0;i<j;i++,mod++)
            SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)(doc->modules[i].filename));
        break;
    case WM_COMMAND:
        ed=(PATCHLOAD*)GetWindowLong(win,DWL_USER);
        doc=ed->ew.doc;
        switch(wparam) {
        case 3002:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="FSNASM source files\0*.ASM\0FSNASM module files\0*.OBJ\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=patchname;
            *patchname=0;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Load patch";
            ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            if(!GetOpenFileName(&ofn)) break;
            doc->nummod++;
            doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
            mod=doc->modules+doc->nummod-1;
            mod->filename=_strdup(patchname);
            mod->flag=0;
            SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)patchname);
            doc->p_modf=1;
            break;
        case 3003:
            i=SendDlgItemMessage(win,3000,LB_GETCURSEL,0,0);
            if(i==-1) break;
            SendDlgItemMessage(win,3000,LB_DELETESTRING,0,(int)patchname);
            doc->nummod--;
            memcpy(doc->modules+i,doc->modules+i+1,(doc->nummod-i)*sizeof(ASMHACK));
            doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
            doc->p_modf=1;
            break;
        case 3004:
            Buildpatches(doc);
            break;
        }
    }
    return 0;
}

// =============================================================================
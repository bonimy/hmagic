
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "PatchEnum.h"
    #include "PatchLogic.h"

// =============================================================================

SD_ENTRY patch_sd[] =
{
    {
        WC_LISTBOX,
        "",
        0, 20, 100, 70,
        ID_Patch_ModulesListBox,
        (
            WS_VISIBLE | WS_CHILD | WS_VSCROLL
          | LBS_NOTIFY
        ),
        WS_EX_CLIENTEDGE,
        10
    },
    {"STATIC","Loaded modules:",0,0,0,0, ID_Patch_ModulesLabel, WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Add",80,20,0,20, ID_Patch_AddModuleButton, WS_VISIBLE|WS_CHILD,0,3},
    {"BUTTON","Remove",80,44,0,20, ID_Patch_RemoveModuleButton, WS_VISIBLE|WS_CHILD,0,3},
    {"BUTTON","Build",80,68,0,20, ID_Patch_BuildButton, WS_VISIBLE|WS_CHILD,0,3}
};

// =============================================================================

SUPERDLG patchdlg =
{
    "",
    PatchDlg,
    WS_CHILD | WS_VISIBLE,
    200,
    200,
    MACRO_ArrayLength(patch_sd),
    patch_sd
};

// =============================================================================

void
PatchDlg_Init
(
    HWND   const p_win,
    LPARAM const p_lp
)
{
    int i = 0;
    int j = 0;
    
    PATCHLOAD * const ed = (PATCHLOAD*) p_lp;
    
    FDOC * const doc = ed->ew.doc;
    
    ASMHACK const * const mod = doc->modules;
    
    // -----------------------------
    
    ed->dlg = p_win;
    
    j = doc->nummod;
    
    for(i = 0; i < j; i++)
    {
        SendDlgItemMessage(p_win,
                           ID_Patch_ModulesListBox,
                           LB_ADDSTRING,
                           0,
                           (LPARAM) mod[i].filename);
    }
}

// =============================================================================

/// Macros to help make file filters more human readable. name and ext
/// are expected to be strings.
#define FILTER(name, ext) name "\0" ext "\0"
#define FILTER_TERMINATE() "\0";

// =============================================================================

void
PatchDlg_AddModule(PATCHLOAD * const p_ed,
                   HWND        const p_win)
{
    FDOC * const doc = p_ed->ew.doc;
    
    ASMHACK * mod = doc->modules;
    
    char patchname[MAX_PATH] = { 0 };
    
    OPENFILENAME ofn;

    // -----------------------------
    
    ofn.lStructSize = sizeof(ofn);
    
    ofn.hwndOwner = p_win;
    ofn.hInstance = hinstance;
    
    ofn.lpstrFilter = FILTER("FSNASM source files", "*.ASM")
                      FILTER("FSNASM module files", "*.OBJ")
                      FILTER("All Files", "*.*")
                      FILTER_TERMINATE();
    
    ofn.lpstrCustomFilter=0;
    ofn.nFilterIndex=1;
    ofn.lpstrFile = patchname;
    
    patchname[0] = 0;
    
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle=0;
    ofn.lpstrInitialDir=0;
    
    ofn.lpstrTitle = "Load patch";
    
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    
    ofn.lpstrDefExt = 0;
    ofn.lpfnHook = 0;
    
    if( ! GetOpenFileName(&ofn) )
    {
        return;
    }
    
    doc->nummod++;
    
    doc->modules = (ASMHACK*) realloc(doc->modules,
                                      sizeof(ASMHACK) * doc->nummod);
    
    mod = (doc->modules + doc->nummod - 1);
    
    mod->filename = _strdup(patchname);
    
    mod->flag = 0;
    
    SendDlgItemMessage(p_win,
                       ID_Patch_ModulesListBox,
                       LB_ADDSTRING,
                       0,
                       (LPARAM) patchname);
    
    doc->p_modf=1;
}

// =============================================================================

void
PatchDlg_RemoveModule
(
    PATCHLOAD * const p_ed,
    HWND        const p_win
)
{
    FDOC * const doc = p_ed->ew.doc;
    
    int i = SendDlgItemMessage(p_win,
                               ID_Patch_ModulesListBox,
                               LB_GETCURSEL,
                               0,
                               0);
    
    // -----------------------------
    
    if(i == LB_ERR)
    {
        return;
    }
    
    SendDlgItemMessage(p_win,
                       ID_Patch_ModulesListBox,
                       LB_DELETESTRING,
                       i,
                       (LPARAM) 0);
    
    if(always)
    {
        CP2(ASMHACK) mod = (doc->modules + i);
        
        // -----------------------------
        
        if(mod->filename)
        {
            free(mod->filename);
            
            mod->filename = NULL;
            mod->flag     = 0;
        }
    }
        
    doc->nummod--;
    
    memcpy(doc->modules + i,
           doc->modules + i + 1,
           (doc->nummod - i) * sizeof(ASMHACK));
    
    doc->modules = (ASMHACK*) realloc(doc->modules,
                                      sizeof(ASMHACK) * doc->nummod);
    
    doc->p_modf = 1;
}

// =============================================================================

extern BOOL CALLBACK
PatchDlg(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PATCHLOAD*ed;
    
    FDOC * doc;
    
    // -----------------------------
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        PatchDlg_Init(win, lparam);
        
        break;
    
    case WM_COMMAND:
        
        ed = (PATCHLOAD*) GetWindowLong(win, DWL_USER);
        
        doc = ed->ew.doc;
        
        switch(wparam)
        {
        
        case ID_Patch_AddModuleButton:
            
            PatchDlg_AddModule(ed, win);
            
            break;
        
        case ID_Patch_RemoveModuleButton:
            
            PatchDlg_RemoveModule(ed, win);
            
            break;
        
        case ID_Patch_BuildButton:
            
            Patch_Build(doc);
            
            break;
        }
    }
    
    return 0;
}

// =============================================================================
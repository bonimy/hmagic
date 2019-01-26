
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "Wrappers.h"

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

    static void
    PatchDlg_OnInitDialog
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        int i = 0;
        int j = 0;
        
        CP2(PATCHLOAD) ed = (PATCHLOAD*) p_lp;
        
        CP2(FDOC) doc = ed->ew.doc;
        
        CP2C(ASMHACK) mod = doc->modules;
        
        HWND const listbox = GetDlgItem
        (
            p_win,
            ID_Patch_ModulesListBox
        );
        
        // -----------------------------
        
        ed->dlg = p_win;
        
        j = doc->nummod;
        
        for(i = 0; i < j; i++)
        {
            HM_ListBox_AddString
            (
                listbox,
                mod[i].filename
            );
        }
    }

// =============================================================================

    static void
    MultiSelectFileDialogHook_OnNotify
    (
        HWND   const p_dlg_win,
        LPARAM const p_lp
    )
    {
        CP2C(OFNOTIFY) notification = (OFNOTIFY*) p_lp;
        
        UINT const code = notification->hdr.code;
        
        char files_buf[MAX_PATH * 2] = { 0 };
        
        size_t files_len = 5;
        
        // -----------------------------
        
        switch(code)
        {
        
        case CDN_SELCHANGE:
            
            files_len = SendMessage
            (
                GetParent(p_dlg_win),
                CDM_GETFILEPATH,
                MAX_PATH * 2,
                (LPARAM) files_buf
            );
            
            files_len = SendMessage
            (
                GetParent(p_dlg_win),
                CDM_GETSPEC,
                MAX_PATH * 2,
                (LPARAM) files_buf
            );
            
            files_len = 4;
            
            break;
        }
    }

// =============================================================================

    static UINT_PTR CALLBACK
    MultiSelectFileDialogHook
    (
        HWND p_win,
        UINT p_msg,
        WPARAM p_wp,
        LPARAM p_lp
    )
    {
        UNREFERENCED_PARAMETER(p_wp);
        
        // -----------------------------
        
        switch(p_msg)
        {
            
        case WM_NOTIFY:
            
            MultiSelectFileDialogHook_OnNotify
            (
                p_win,
                p_lp
            );
            
            break;
        }
        
        return 0;
    }

// =============================================================================

/// Macros to help make file filters more human readable. name and ext
/// are expected to be strings.
#define FILTER(name, ext) name "\0" ext "\0"
#define FILTER_TERMINATE(x) "\0";

// =============================================================================

CP2C(char) patch_filter = FILTER("FSNASM source files", "*.ASM")
                          FILTER("FSNASM module files", "*.OBJ")
                          FILTER("All Files", "*.*")
                          FILTER_TERMINATE("");

// =============================================================================

    void
    PatchDlg_AddModule
    (
        PATCHLOAD * const p_ed,
        HWND        const p_win
    )
    {
        int i         = 0;
        int num_files = 0;
        
        FDOC * const doc = p_ed->ew.doc;
        
        ASMHACK * mod = doc->modules;
        
        char patchname[MAX_PATH] = { 0 };
        
        char patch_dir[MAX_PATH] = { 0 };
        
        HWND const listbox = GetDlgItem
        (
            p_win,
            ID_Patch_ModulesListBox
        );
        
        OPENFILENAME ofn = { 0 };
        
        // -----------------------------
        
        ofn.lStructSize = sizeof(ofn);
        
        ofn.hwndOwner = p_win;
        ofn.hInstance = hinstance;
        
        ofn.lpstrFilter = patch_filter;
        
        ofn.lpstrCustomFilter=0;
        ofn.nFilterIndex=1;
        
        ofn.lpstrFile = (LPSTR) calloc(1, MAX_PATH);
        
        patchname[0] = 0;
        
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrInitialDir = patch_dir;
        
        ofn.lpstrTitle = "Load patch";
        
        ofn.Flags =
        (
            OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
          | OFN_ALLOWMULTISELECT | OFN_EXPLORER
        );
        
        ofn.lpstrDefExt = 0;
        
        // Hook the dialog's window procedure.
        ofn.Flags |= OFN_ENABLEHOOK;
        ofn.lpfnHook = MultiSelectFileDialogHook;
        
        
        // \task Make a wrapper for this, especially now that we're 
        // multiselecting. Specifically, we need a file count and a
        // list of pointers to the file names in the reverse order
        // as presented in the structure, as this reflects the order
        // in which they were selected.
        
        MessageBox(p_win, "This code doesn't work chief", "Fix it", MB_OK);
        
        if( ! GetOpenFileName(&ofn) )
        {
            return;
        }
        
        for(i = 0; i < num_files; i += 1)
        {
            doc->nummod++;
        
            doc->modules = (ASMHACK*) realloc(doc->modules,
                                              sizeof(ASMHACK) * doc->nummod);
            
            mod = (doc->modules + doc->nummod - 1);
            
            mod->filename = _strdup(patchname);
            
            mod->flag = 0;
            
            HM_ListBox_AddString
            (
                listbox,
                patchname
            );
        }
        
        doc->p_modf = 1;
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
    
    HWND const listbox = GetDlgItem
    (
        p_win,
        ID_Patch_ModulesListBox
    );
    
    int const i = HM_ListBox_GetSelectedItem(listbox);
        
    // -----------------------------
    
    if( IsListBoxError(i) )
    {
        return;
    }
    
    HM_ListBox_DeleteString
    (
        listbox,
        i
    );
    
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
        
        SetWindowLongPtr(win, DWLP_USER, lparam);
        
        PatchDlg_OnInitDialog
        (
            win,
            lparam
        );
        
        break;
    
    case WM_COMMAND:
        
        ed = (PATCHLOAD*) GetWindowLongPtr(win, DWLP_USER);
        
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
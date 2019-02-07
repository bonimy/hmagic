
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "Wrappers.h"

    #include "HMagicUtility.h"

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
        FLG_SDCH_FOWH
    },
    {
        "STATIC",
        "Loaded modules:",
        0, 0,
        0, 0,
        ID_Patch_ModulesLabel,
        (WS_VISIBLE | WS_CHILD),
        0,
        FLG_SDCH_NONE
    },
    {
        "BUTTON",
        "Add",
        80, 20,
        0, 20,
        ID_Patch_AddModuleButton,
        (WS_VISIBLE | WS_CHILD),
        0,
        (FLG_SDCH_FOX | FLG_SDCH_FOW)
    },
    {
        "BUTTON",
        "Remove",
        80, 44,
        0, 20,
        ID_Patch_RemoveModuleButton,
        (WS_VISIBLE | WS_CHILD),
        0,
        (FLG_SDCH_FOX | FLG_SDCH_FOW)
    },
    {
        "BUTTON",
        "Build",
        80,
        68,
        0,
        20,
        ID_Patch_BuildButton,
        (WS_VISIBLE | WS_CHILD),
        0,
        (FLG_SDCH_FOX | FLG_SDCH_FOW)
    }
};

// =============================================================================

SUPERDLG patchdlg =
{
    "",
    PatchDlg,
    (WS_CHILD | WS_VISIBLE),
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

    extern int
    HM_FileDialog_GetSpec
    (
        HWND      const p_file_dlg,
        CP2(char)       p_buffer,
        size_t    const p_buffer_size
    )
    {
        int const spec_length = SendMessage
        (
            GetParent(p_file_dlg),
            CDM_GETSPEC,
            p_buffer_size,
            (LPARAM) p_buffer
        );
        
        // -----------------------------
        
        return spec_length;
    }

// =============================================================================

    extern int
    HM_FileDialog_GetFolder
    (
        HWND      const p_file_dlg,
        CP2(char)       p_buffer,
        size_t    const p_buffer_size
    )
    {
        int const folder_length = SendMessage
        (
            GetParent(p_file_dlg),
            CDM_GETFOLDERPATH,
            p_buffer_size,
            (LPARAM) p_buffer
        );
        
        // -----------------------------
        
        return folder_length;
    }

// =============================================================================

    static size_t
    HM_MultiSelectFileDlgHook_OnOK
    (
        HWND              const p_file_dlg,
        CP2(OPENFILENAME)       p_ofn
    )
    {
        char dummy_spec[MAX_PATH] = { 0 };
        char folder[MAX_PATH] = { 0 };
        
        int const folder_length = HM_FileDialog_GetFolder
        (
            p_file_dlg,
            p_ofn->lpstrFile,
            p_ofn->nMaxFile
        );
        
        BOOL const exceeded = (folder_length > p_ofn->nMaxFile);
        
        int const spec_length = HM_FileDialog_GetSpec
        (
            p_file_dlg,
            IsTrue(exceeded) ? folder : p_ofn->lpstrFile + folder_length,
            IsTrue(exceeded) ? MAX_PATH : (p_ofn->nMaxFile - folder_length)
        );
        
        int const total_length = (spec_length + folder_length);
        
        // -----------------------------
        
        if(total_length > p_ofn->nMaxFile)
        {
            size_t const old_size = p_ofn->nMaxFile;
            
            // -----------------------------
            
            p_ofn->nMaxFile = (total_length + MAX_PATH);
            
            p_ofn->lpstrFile = (LPTSTR) recalloc
            (
                p_ofn->lpstrFile,
                p_ofn->nMaxFile,
                old_size,
                sizeof(char)
            );
        }

        return p_ofn->nMaxFile;
    }

// =============================================================================

    static size_t
    HM_MultiSelectFileDlgHook_OnSelectionChange
    (
        HWND              const p_file_dlg,
        CP2(OPENFILENAME)       p_ofn
    )
    {
        char dummy_spec[MAX_PATH] = { 0 };
        char folder[MAX_PATH] = { 0 };
        
        int const spec_length = HM_FileDialog_GetSpec
        (
            p_file_dlg,
            dummy_spec,
            MAX_PATH
        );
        
        int const folder_length = HM_FileDialog_GetFolder
        (
            p_file_dlg,
            folder,
            MAX_PATH
        );
        
        int const total_length = (spec_length + folder_length);
        
        // -----------------------------
        
        if(total_length > p_ofn->nMaxFile)
        {
            size_t const old_size = p_ofn->nMaxFile;
            
            // -----------------------------
            
            p_ofn->nMaxFile = (total_length + MAX_PATH);
            
            p_ofn->lpstrFile = (LPTSTR) recalloc
            (
                p_ofn->lpstrFile,
                p_ofn->nMaxFile,
                old_size,
                sizeof(char)
            );
        }
        
        return p_ofn->nMaxFile;
    }

// =============================================================================

    static void
    MultiSelectFileDialogHook_OnNotify
    (
        HWND   const p_file_dlg,
        LPARAM const p_lp
    )
    {
        CP2C(OFNOTIFY) notification = (OFNOTIFY*) p_lp;
        
        CP2(OPENFILENAME) ofn = notification->lpOFN;
        
        UINT const code = notification->hdr.code;
        
        // -----------------------------
        
        switch(code)
        {

        default:
            
            break;
        
        case CDN_FILEOK:

            HM_MultiSelectFileDlgHook_OnOK
            (
                p_file_dlg,
                ofn
            );
            
            break;
        
        case CDN_FOLDERCHANGE:
            
            break;
        
        case CDN_SELCHANGE:
            
            HM_MultiSelectFileDlgHook_OnSelectionChange
            (
                p_file_dlg,
                ofn
            );

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

    extern INT_PTR
    HM_MultiSelectFileDialog_Open
    (
        HWND              const p_parent,
        CP2C(char)              p_title,
        CP2C(char)              p_filter,
        CP2C(char)              p_initial_dir,
        CP2(OPENFILENAME)       p_out_ofn
    )
    {
        OPENFILENAME ofn = { 0 };
        
        INT_PTR r = IDCANCEL;
        
        // -----------------------------

        ofn.lStructSize = sizeof(ofn);
        
        ofn.hwndOwner = p_parent;
        ofn.hInstance = hinstance;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.lpstrFilter = p_filter;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.nMaxCustFilter = 0;
        
        ofn.lpstrCustomFilter = NULL;
        
        ofn.nFilterIndex = 1;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.nMaxFile = MAX_PATH;
        
        ofn.lpstrFile = (LPSTR) calloc(1, ofn.nMaxFile);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.lpstrInitialDir = p_initial_dir;
        
        ofn.lpstrTitle = p_title;
        
        // Hook the dialog's window procedure.
        ofn.Flags =
        (
            OFN_FILEMUSTEXIST
          | OFN_HIDEREADONLY
          | OFN_ALLOWMULTISELECT
          | OFN_EXPLORER
          | OFN_ENABLEHOOK
        );
        
        ofn.lpstrDefExt = 0;
        
        ofn.lpfnHook = MultiSelectFileDialogHook;

        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        r = IsTrue( GetOpenFileName(&ofn) ) ? IDOK : IDCANCEL;
        
        p_out_ofn[0] = ofn;
        
        return r;
    }

// =============================================================================

    extern int
    HM_FileDlg_CountFiles
    (
        CP2C(OPENFILENAME) p_ofn
    )
    {
        int i = 0;

        // -----------------------------
        
        for(i = 0; i < p_ofn->nMaxFile; i += 1)
        {
            printf(p_ofn->lpstrFile[i]);
        }
        
        return 0;
    }

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
        
        INT_PTR ofn_result = IDCANCEL;
        
        OPENFILENAME ofn = { 0 };
        
        // -----------------------------
        
        // \task[high] Make a wrapper for this, especially now that we're 
        // multiselecting. Specifically, we need a file count and a
        // list of pointers to the file names in the reverse order
        // as presented in the structure, as this reflects the order
        // in which they were selected.
        
        HM_OK_MsgBox
        (
            p_win,
            "This code doesn't work chief",
            "Fix it"
        );
        
        ofn_result = HM_MultiSelectFileDialog_Open
        (
            p_win,
            "Load Patch",
            patch_filter,
            patch_dir,
            &ofn
        );
        
        if( Is(ofn_result, IDCANCEL) )
        {
            return;
        }
        
        HM_FileDlg_CountFiles(&ofn);
        
        for(i = 0; i < num_files; i += 1)
        {
            doc->nummod++;
        
            doc->modules = (ASMHACK*) realloc
            (
                doc->modules,
                sizeof(ASMHACK) * doc->nummod
            );
            
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
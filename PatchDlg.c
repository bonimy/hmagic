
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

    typedef
    struct
    {
        DWORD m_length;
        
        DWORD m_error;
        
    }
    HM_FileDlg_Result;

// =============================================================================

    typedef
    struct
    {
        char * m_buffer;
        
        size_t m_buffer_length;

        DWORD m_error;
    }
    HM_FileDlgProperty;

// =============================================================================

    typedef
    struct
    {
        HM_FileDlgProperty m_folder;
        HM_FileDlgProperty m_spec;
    }
    HM_FileDlgData;

// =============================================================================

    static void
    HM_FileDlgProperty_New
    (
        CP2(HM_FileDlgProperty)       p_property,
        size_t                  const p_buffer_length
    )
    {
        p_property->m_buffer = (char*) calloc
        (
            p_buffer_length,
            sizeof(char)
        );
        
        p_property->m_buffer_length = p_buffer_length;
        
        p_property->m_error = 0;
    }

// =============================================================================

    extern HM_FileDlgData *
    HM_FileDlgData_New
    (
        void
    )
    {
        CP2(HM_FileDlgData) data = (HM_FileDlgData*) calloc
        (
            1,
            sizeof(HM_FileDlgData)
        );
        
        // -----------------------------
        
        if( IsNonNull(data) )
        {
            HM_FileDlgProperty_New(&data->m_folder, MAX_PATH);
            HM_FileDlgProperty_New(&data->m_spec, 0x1000);
        }
        
        return data;
    }

// =============================================================================

    extern void
    HM_FileDlgData_Delete
    (
        CP2(HM_FileDlgData) p_data
    )
    {
        HM_FileDlgData const zeroed_data = { 0 };
        
        // -----------------------------
        
        if( IsNull(p_data) )
        {
            return;
        }
        
        if( IsNonNull(p_data->m_folder.m_buffer) )
        {
            free(p_data->m_folder.m_buffer);
        }

        if( IsNonNull(p_data->m_spec.m_buffer) )
        {
            free(p_data->m_spec.m_buffer);
        }
        
        p_data[0] = zeroed_data;
    }

// =============================================================================

    extern HM_FileDlg_Result
    HM_FileDlg_GetSpec
    (
        HWND                    const p_file_dlg,
        CP2(HM_FileDlgProperty)       p_property
    )
    {
        HM_FileDlg_Result r = { 0 };
        
        int const spec_length = SendMessage
        (
            GetParent(p_file_dlg),
            CDM_GETSPEC,
            p_property->m_buffer_length,
            (LPARAM) p_property->m_buffer
        );
        
        // -----------------------------
        
        if(spec_length < 0)
        {
            p_property->m_error = CommDlgExtendedError();
            r.m_length = 0;
        }
        else
        {
            r.m_length = spec_length;
        }
        
        return r;
    }

// =============================================================================

    HM_FileDlg_Result
    HM_FileDlg_GetFolder
    (
        HWND                    const p_file_dlg,
        CP2(HM_FileDlgProperty)       p_property
    )
    {
        HM_FileDlg_Result r = { 0 };
        
        int const folder_length = SendMessage
        (
            GetParent(p_file_dlg),
            CDM_GETFOLDERPATH,
            p_property->m_buffer_length,
            (LPARAM) p_property->m_buffer
        );
        
        // -----------------------------
        
        if(folder_length < 0)
        {
            p_property->m_error = CommDlgExtendedError();
            r.m_length = 0;
        }
        else
        {
            r.m_length = folder_length;
        }
        
        return r;
    }

// =============================================================================

    static HM_FileDlgData *
    HM_MultiSelectFileDlgHook_GetData
    (
        CP2(OPENFILENAME) p_ofn
    )
    {
        CP2(HM_FileDlgData) data = (HM_FileDlgData*) p_ofn->lCustData;
        
        // -----------------------------
        
        return data;
    }


// =============================================================================

    static size_t
    HM_MultiSelectFileDlgHook_OnOK
    (
        HWND              const p_file_dlg,
        CP2(OPENFILENAME)       p_ofn
    )
    {
        CP2(HM_FileDlgData) data = HM_MultiSelectFileDlgHook_GetData(p_ofn);
        
        HM_FileDlg_Result folder_r = HM_FileDlg_GetFolder
        (
            p_file_dlg,
            &data->m_folder
        );
        
        HM_FileDlg_Result spec_r = HM_FileDlg_GetSpec
        (
            p_file_dlg,
            &data->m_spec
        );
        
        DWORD const total_length = (folder_r.m_length + spec_r.m_length);
        
        // -----------------------------
        
        if(folder_r.m_length > data->m_folder.m_buffer_length)
        {
            data->m_folder.m_buffer = (char*) recalloc
            (
                data->m_folder.m_buffer,
                folder_r.m_length,
                data->m_folder.m_buffer_length,
                sizeof(char)
            );
            
            if(data->m_folder.m_buffer)
            {
                data->m_folder.m_buffer_length = folder_r.m_length;
            }
            else
            {
                goto error;
            }
            
            folder_r = HM_FileDlg_GetFolder
            (
                p_file_dlg,
                &data->m_folder
            );
            
            if(folder_r.m_error)
            {
                data->m_folder.m_error = folder_r.m_error;
                
                goto error;
            }
        }
        
        if(spec_r.m_length > data->m_spec.m_buffer_length)
        {
            data->m_spec.m_buffer = (char*) recalloc
            (
                data->m_spec.m_buffer,
                spec_r.m_length,
                data->m_spec.m_buffer_length,
                sizeof(char)
            );
            
            if(data->m_spec.m_buffer)
            {
                data->m_spec.m_buffer_length = spec_r.m_length;
            }
            else
            {
                goto error;
            }
            
            spec_r = HM_FileDlg_GetSpec
            (
                p_file_dlg,
                &data->m_spec
            );
            
            if(spec_r.m_error)
            {
                data->m_spec.m_error = spec_r.m_error;
                
                goto error;
            }
        }        
        
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

    error:
        
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
        CP2(HM_FileDlgData) data = HM_MultiSelectFileDlgHook_GetData(p_ofn);
        
        HM_FileDlg_Result const spec_r = HM_FileDlg_GetSpec
        (
            p_file_dlg,
            &data->m_spec
        );
        
        HM_FileDlg_Result const folder_r = HM_FileDlg_GetFolder
        (
            p_file_dlg,
            &data->m_folder
        );
        
        DWORD const total_length = (spec_r.m_length + folder_r.m_length);
        
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

    #define MACRO_Stringify_ImplA(x) #x
    #define MACRO_Stringify_ImplW(x) L#x

// =============================================================================

#if defined UNICODE
    #define MACRO_Stringify_Impl(x) MACRO_Stringify_ImplW(x)
#else
    #define MACRO_Stringify_Impl(x) MACRO_Stringify_ImplA(x)
#endif

// =============================================================================

    #define MACRO_Stringify(x) MACRO_Stringify_ImplA(x)

    #define MACRO_StringifyA(x) MACRO_Stringify_ImplA(x)
    #define MACRO_StringifyW(x) MACRO_Stringify_ImplW(x)

// =============================================================================

    /// Macros to help make file filters more human readable. name and ext
    /// are expected to be strings.
    #define FILTER_ENTRY(name, ext)name##\0##ext##\0
    #define FILTER_TERMINATE()\0\0

// =============================================================================

// \note The white space of this preprocessor symbol is sensitive to 
// refactoring. Keep each line pinned to the respective first column, otherwise
// the resulting filter string will end up with extraneous space characters,
// and if you're especially not careful, an extraneous blank filter.
#define patch_filter_pre \
FILTER_ENTRY(FSNASM source files,*.ASM)\
FILTER_ENTRY(FSNASM module files,*.OBJ)\
FILTER_ENTRY(All Files,*.*)\
FILTER_TERMINATE()\

// =============================================================================

char const patch_filter[] = MACRO_StringifyA(patch_filter_pre);

wchar_t const patch_filter_wide[] = MACRO_StringifyW(patch_filter_pre);

// =============================================================================

    extern INT_PTR
    HM_MultiSelectFileDialog_Open
    (
        HWND              const p_parent,
        CP2C(char)              p_title,
        CP2C(char)              p_filter,
        CP2C(char)              p_initial_dir,
        CP2(HM_FileDlgData)     p_data
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
        
        ofn.lpstrDefExt = 0;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.nMaxFile = MAX_PATH;
        
        ofn.lpstrFile = (LPSTR) calloc(1, ofn.nMaxFile);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.lpstrInitialDir = p_initial_dir;
        
        ofn.lpstrTitle = p_title;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        // Hook the dialog's window procedure.
        ofn.Flags =
        (
            OFN_FILEMUSTEXIST
          | OFN_HIDEREADONLY
          | OFN_ALLOWMULTISELECT
          | OFN_EXPLORER
          | OFN_ENABLEHOOK
        );
        
        ofn.lpfnHook = MultiSelectFileDialogHook;

        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ofn.lCustData = (LPARAM) p_data;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        r = IsTrue( GetOpenFileName(&ofn) ) ? IDOK : IDCANCEL;
        
        free(ofn.lpstrFile);
        
        return r;
    }

// =============================================================================

    extern int
    HM_FileDlg_CountFiles
    (
        CP2C(HM_FileDlgData) p_data
    )
    {
        DWORD i = 0;

        // -----------------------------
        
        for(i = 0; i < p_data->m_folder.m_buffer_length; i += 1)
        {
            ;
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
        
        CP2(HM_FileDlgData) data = HM_FileDlgData_New();
        
        // -----------------------------
        
        // \task[high] The wrappers we've written are okay but we need to
        // finish the job so that we can easily tell what to add to the
        // patch list control after they've been selected (if any) from the
        // file dialog. Since some of the interfaces have changed recently
        // let's be extra careful to tidy up any loose ends.
        
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
            data
        );
        
        if( Is(ofn_result, IDCANCEL) )
        {
            goto cleanup;
        }
        
        HM_FileDlg_CountFiles(data);
        
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
        
    cleanup:
        
        HM_FileDlgData_Delete(data);
        
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
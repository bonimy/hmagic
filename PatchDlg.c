
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

    uint32_t const FNV_offset_basis_32bit = 0x811c9dc5;
    uint32_t const FNV_prime_32bit        = 0x01000193;

// =============================================================================

    // \note Information on implementing this came from Wikipedia.
    // \task[med] Move to a more appropriate file and have a header file too.
    extern uint32_t
    FNV_1A_Hash
    (
        CP2C(void)       p_data,
        size_t     const p_length
    )
    {
        CP2C(char) data = ( CP2C(char) ) p_data;
        
        uint32_t hash = FNV_offset_basis_32bit;
        
        size_t i = 0;
        
        // -----------------------------
        
        for(i = 0 ; i < p_length; i += 1)
        {
            uint8_t const octet = (uint8_t) data[i];
            
            // -----------------------------
            
            hash ^= octet;
            hash *= FNV_prime_32bit;
        }
        
        return hash;
    }

// =============================================================================

    /**
        Convenience wrapper for hashing strings.
    */
    extern uint32_t
    FNV_1A_HashOfString
    (
        CP2C(char) p_string
    )
    {
        size_t const length = strlen(p_string);
        
        // -----------------------------
        
        return FNV_1A_Hash
        (
            p_string,
            length
        );
    }

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
        
        for(i = 0; i < j; i += 1)
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
        
        // Number of actual characters used in the buffer
        size_t m_actual_length;
        
        size_t m_buffer_length;
        
        DWORD m_error;
    }
    HM_FileDlgProperty;

// =============================================================================

    /**
        Object that contains a vector of strings all having the same hash.
    */
    typedef
    struct
    {
        size_t m_count;
        size_t m_capacity;
        
        P2C(char) * m_entries;
    }
    HashedStringSubtable;

// =============================================================================

    /**
        The object that maps the hashed value to a subtable.
    */
    typedef
    struct
    {
        uint32_t m_hash;
        
        HashedStringSubtable * m_subtable;
    }
    HashedStringEntry;

// =============================================================================

    typedef
    struct
    {
        size_t m_count;
        size_t m_capacity;
        
        HashedStringEntry * m_entries;
    }
    HashedStringTable;

// =============================================================================

    extern int
    HashedStringSubtable_Clear
    (
        CP2(HashedStringSubtable) p_subtable
    )
    {
        size_t i = 0;

        // -----------------------------
        
        for(i = 0; i < p_subtable->m_count; i += 1)
        {
            P2C(char) string = p_subtable->m_entries[i];
            
            // -----------------------------
            
            free( (void*) string);
            
            p_subtable->m_entries[i] = NULL;
        }
        
        p_subtable->m_count = 0;
        
        return 0;
    }

// =============================================================================

    extern BOOL
    HashedStringSubtable_AddString
    (
        CP2(HashedStringSubtable) p_subtable,
        CP2C(char)                p_string
    )
    {
        CP2(HashedStringSubtable) st = p_subtable;
        
        size_t i = 0;
        
        // -----------------------------
        
        for(i = 0; i < st->m_count; i += 1)
        {
            CP2C(char) other_string = st->m_entries[i];
            
            // -----------------------------
            
            if( IsZero( strcmp(p_string, other_string) ) )
            {
                // Didn't need to be added.
                return FALSE;
            }
        }
        
        if( IsZero(st->m_capacity) )
        {
            enum
            {
                NUM_DefaultCapacity = 4
            };
            
            st->m_entries = ( P2C(char) * ) calloc
            (
                NUM_DefaultCapacity,
                sizeof( P2C(char) )
            );
            
            if( IsNonNull(st->m_entries) )
            {
                st->m_capacity = NUM_DefaultCapacity;
                
                st->m_count = 0;
            }
        }

        if( Is(st->m_count, st->m_capacity) )
        {
            size_t new_capacity = (st->m_capacity * 2);
            
            // -----------------------------
            
            st->m_entries = ( P2C(char) * ) recalloc
            (
                (void*) st->m_entries,
                st->m_capacity * 2,
                st->m_capacity,
                sizeof( P2C(char) )
            );
            
            if( IsNull(st->m_entries) )
            {
                // \task[high] This needs attention. We need more complex
                // information sent back than just a boolean.
                return FALSE;
            }
            
            st->m_capacity = new_capacity;
        }

        if(always)
        {
            size_t const k = st->m_count;
            
            // -----------------------------
            
            st->m_entries[k] = hm_strdup(p_string);
            
            st->m_count += 1;
        }
        
        return TRUE;
    }

// =============================================================================

    extern BOOL
    HashedStringTable_Init
    (
        CP2(HashedStringTable)       p_table,
        size_t                 const p_capacity
    )
    {
        p_table->m_entries = (HashedStringEntry*) calloc
        (
            p_capacity,
            sizeof(HashedStringEntry)
        );
        
        if(p_table->m_entries)
        {
            p_table->m_capacity = p_capacity;
            
            p_table->m_count = 0;
            
            return TRUE;
        }
        
        return FALSE;
    }

// =============================================================================

    extern int
    HashedStringTable_Clear
    (
        CP2(HashedStringTable) p_table
    )
    {
        size_t i = 0;
        
        // -----------------------------
        
        for(i = 0; i < p_table->m_count; i += 1)
        {
            CP2(HashedStringEntry) entry = &p_table->m_entries[i];
            
            // -----------------------------
            
            if(entry->m_subtable)
            {
                CP2(HashedStringSubtable) subtable = entry->m_subtable;
                
                HashedStringSubtable_Clear(subtable);
            }
        }
        
        return 0;
    }

// =============================================================================

    extern BOOL
    HashedStringTable_AddString
    (
        CP2(HashedStringTable) p_table,
        CP2C(char)             p_string
    )
    {
        uint32_t const h = FNV_1A_HashOfString(p_string);
        
        size_t i = 0;

        CP2(HashedStringEntry) ents = p_table->m_entries;
        
        // -----------------------------
        
        for(i = 0; i < p_table->m_count; i += 1)
        {
            CP2(HashedStringEntry) entry = &ents[i];
            
            if( Is(entry->m_hash, h) )
            {
                // There's already at least one string in the table that
                // has this hash. We'll need to defer to that hash's subtable
                // to check whether the string is already there.

                HashedStringSubtable_AddString
                (
                    entry->m_subtable,
                    p_string
                );
                
                return FALSE;
            }
        }
        
        return TRUE;
    }

// =============================================================================

    typedef
    struct
    {
        HashedStringTable m_files;
        
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
        
        p_property->m_actual_length = 0;
        
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
            
            HM_FileDlgProperty_New
            (
                &data->m_folder,
#if defined NDEBUG
                MAX_PATH
#else
                // test just to make sure the allocator is well behaved.
                0x40
#endif

            );
            HM_FileDlgProperty_New(&data->m_spec, 0x100);
        }
        
        return data;
    }

// =============================================================================

    extern void
    HM_FileDlgData_ResetFileSpec
    (
        CP2(HM_FileDlgData) p_data
    )
    {
        // \task[high] Implement this routine. We need to delete an array
        // of files but keep the length of the number of pointers the
        // same (just delete any strdup'd like data)
        HashedStringTable_Clear
        (
            &p_data->m_files
        );
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
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        if( IsNonNull(p_data->m_folder.m_buffer) )
        {
            free(p_data->m_folder.m_buffer);
        }

        if( IsNonNull(p_data->m_spec.m_buffer) )
        {
            free(p_data->m_spec.m_buffer);
        }
        
        HM_FileDlgData_ResetFileSpec(p_data);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        p_data[0] = zeroed_data;
        
        free(p_data);
    }

// =============================================================================

    extern int
    HM_ModularRoundUp
    (
        unsigned const p_value,
        unsigned const p_modulus
    )
    {
        unsigned residue = 0;
        
        // -----------------------------
        
        if( Is(p_modulus, 0) )
        {
            return 0;
        }
        
        residue = ( (p_modulus - p_value) % (p_modulus) );
        
        return (p_value + residue);
    }

// =============================================================================

    extern int
    HM_FileDlg_GetSpec
    (
        HWND                    const p_file_dlg,
        CP2(HM_FileDlgProperty)       p_property
    )
    {
        HWND const parent_win = GetParent(p_file_dlg);
        
        int spec_length = SendMessage
        (
            parent_win,
            CDM_GETSPEC,
            p_property->m_buffer_length,
            (LPARAM) p_property->m_buffer
        );
        
        // -----------------------------
        
        // Windows counts the null terminator, but we don't
        // care about that.
        spec_length -= 1;
        
        // Assume no errors happened at first.
        p_property->m_error = 0;
        
        if(spec_length < 0)
        {
            p_property->m_error = CommDlgExtendedError();
        }
        else
        {
            DWORD spec_length_dword = spec_length;
            
            // -----------------------------
            
            if(spec_length_dword <= p_property->m_buffer_length)
            {
                // We got the whole thing in one call, no need to reallocate.

                p_property->m_actual_length = spec_length;
            }
            else
            {
                size_t const new_count = HM_ModularRoundUp
                (
                    spec_length,
                    0x100
                );
                
                // -----------------------------
                
                p_property->m_buffer = (char*) recalloc
                (
                    p_property->m_buffer,
                    new_count,
                    p_property->m_buffer_length,
                    sizeof(char)
                );
                
                if(p_property->m_buffer)
                {
                    p_property->m_buffer_length = new_count;
                    
                    spec_length = SendMessage
                    (
                        parent_win,
                        CDM_GETSPEC,
                        p_property->m_buffer_length,
                        (LPARAM) p_property->m_buffer
                    );
                    
                    if(spec_length < 0)
                    {
                        p_property->m_error = CommDlgExtendedError();
                    }
                    else
                    {
                        p_property->m_actual_length = spec_length;
                    }
                }
                else
                {
                    p_property->m_buffer_length = 0;
                }
            }
        }
        
        return spec_length;
    }

// =============================================================================

    extern int
    HM_FileDlg_GetFolder
    (
        HWND                    const p_file_dlg,
        CP2(HM_FileDlgProperty)       p_property
    )
    {
        HWND const parent_win = GetParent(p_file_dlg);
        
        int folder_length = SendMessage
        (
            parent_win,
            CDM_GETFOLDERPATH,
            p_property->m_buffer_length,
            (LPARAM) p_property->m_buffer
        );
        
        // -----------------------------
        
        if(folder_length < 0)
        {
            // There was an error retrieving the current folder.
            
            p_property->m_error = CommDlgExtendedError();
            
            p_property->m_actual_length = 0;
        }
        else
        {
            // Grabbed it, now check that we got the whole thing.
            
            DWORD const folder_length_dword = folder_length;
            
            // -----------------------------
            
            if(folder_length_dword <= p_property->m_buffer_length)
            {
                // It fits. We're done.
                
                p_property->m_actual_length = (folder_length - 1);
            }
            else
            {
                // We only grabbed the partial folder path because our buffer
                // wasn't large enough. Reallocate it so that it will fit and
                // request it again.
                
                size_t const new_count = HM_ModularRoundUp
                (
                    folder_length,
                    0x100
                );
                
                // -----------------------------
                
                p_property->m_buffer = (char*) recalloc
                (
                    p_property->m_buffer,
                    new_count,
                    p_property->m_buffer_length,
                    sizeof(char)
                );
                
                if(p_property->m_buffer)
                {
                    p_property->m_buffer_length = new_count;
                    
                    folder_length = SendMessage
                    (
                        parent_win,
                        CDM_GETFOLDERPATH,
                        p_property->m_buffer_length,
                        (LPARAM) p_property->m_buffer
                    );
                    
                    if(folder_length < 0)
                    {
                        p_property->m_error = CommDlgExtendedError();
                    }
                    else
                    {
                        p_property->m_actual_length = (folder_length - 1);
                    }
                }
                else
                {
                    p_property->m_buffer_length = 0;
                }
            }
        }
        
        return folder_length;
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

    // \task
    extern size_t
    hm_str_count_char
    (
        CP2C(char)       p_string,
        char       const p_character
    )
    {
        // horizontal code golf.
        char const c = p_character;
        
        size_t const length = strlen(p_string);
        
        size_t i     = 0;
        size_t count = 0;
        
        // -----------------------------
        
        for(i = 0; i < length; i += 1)
        {
            char const d = p_string[i];
            
            // -----------------------------
            
            if( Is(c, d) )
            {
                count += 1;
            }
        }
        
        return count;
    }

// =============================================================================

    extern int
    HM_MultiSelectFileDlgHook_ParseFileSpec
    (
        CP2(HM_FileDlgProperty) p_file_spec,
        CP2(HashedStringTable)  p_files
    )
    {
        char const quote_char = '\"';
        
        size_t const quote_count = hm_str_count_char
        (
            p_file_spec->m_buffer,
            quote_char
        );

        // -----------------------------
        
        if( IsOdd(quote_count) )
        {
            // Unbalanced quoting. something is very off.
            return -1;
        }
        else if( IsZero(quote_count) )
        {
            // There's only one filename in the buffer.
            
        }
        
        return 0;
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
        
        int const folder_length = HM_FileDlg_GetFolder
        (
            p_file_dlg,
            &data->m_folder
        );
        
        int const spec_length = HM_FileDlg_GetSpec
        (
            p_file_dlg,
            &data->m_spec
        );
        
        DWORD const total_length =
        (
            data->m_folder.m_actual_length
          + data->m_spec.m_actual_length
        );
        
        // -----------------------------
        
        if( (spec_length < 0) || (folder_length < 0) )
        {
            goto error;
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
        // \task[low] We used to do something here, but it needs to be
        // re-thought out. There is a bit of a nuance wherein if the user
        // deselects all files in the dialog, it still returns at least one
        // file. I wanted to finesse this a bit and check if the "spec"
        // buffer returned two consecutive times is identical, to make
        // it considered to be zero files selected when the OK button is hit.
        // However, I have a feeling that the system will fight us tooth and
        // nail on this.
        CP2(HM_FileDlgData) data = HM_MultiSelectFileDlgHook_GetData
        (
            p_ofn
        );
        
        int const spec_length = HM_FileDlg_GetSpec
        (
            p_file_dlg,
            &data->m_spec
        );
        
        // -----------------------------
        
        HM_MultiSelectFileDlgHook_ParseFileSpec
        (
            &data->m_spec,
            &data->m_files
        );
        
        return 0;
    }

// =============================================================================

    static void
    HM_MultiSelectFileDlgHook_OnFolderChange
    (
        HWND              const p_file_dlg,
        CP2(OPENFILENAME)       p_ofn
    )
    {
        CP2(HM_FileDlgData) data = HM_MultiSelectFileDlgHook_GetData
        (
            p_ofn
        );
        
        // -----------------------------

        int const folder_length = HM_FileDlg_GetFolder
        (
            p_file_dlg,
            &data->m_folder
        );
        
        HM_FileDlgData_ResetFileSpec
        (
            data
        );
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
            
            HM_MultiSelectFileDlgHook_OnFolderChange
            (
                p_file_dlg,
                ofn
            );
            
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

    extern BOOL
    HM_FileDlg_GetNextFile
    (
        CP2C(HM_FileDlgData)       p_data,
        size_t               const p_start_pos,
        CP2(size_t)                p_end_pos,
        CP2(size_t)                p_name_start,
        CP2(size_t)                p_name_end
    )
    {
        BOOL in_name = FALSE;
        
        char const null_char    = '\0';
        char const space        = ' ';
        char const double_quote = '\"';
        
        CP2C(HM_FileDlgProperty) spec = &p_data->m_spec;
        
        size_t i          = 0;
        size_t name_start = 0;
        size_t name_end   = 0;
        
        // -----------------------------
        
        for(i = p_start_pos; i < spec->m_actual_length; i += 1)
        {
            char const c = spec->m_buffer[i];
            
            // -------------------------
            
            if( Is(c, null_char) )
            {
                break;
            }
            if( Is(c, space) && IsFalse(in_name) )
            {
                continue;
            }
            else if( Is(c, double_quote) ) 
            {
                if( IsFalse(in_name) )
                {
                    // Opening quote of the file name.

                    in_name = TRUE;
                    
                    name_start = (i + 1);
                    name_end   = name_start;
                    
                    continue;
                }
                else
                {
                    // Closing quote of the file name.
                    
                    in_name = FALSE;
                    
                    name_end = i;
                    
                    p_end_pos[0] = (i + 1);
                    
                    break;
                }
            }
            else if( IsFalse(in_name) )
            {
                // Encountered a filename character but we're not in a name
                // boundary yet; this is bad!
                return FALSE;
            }
        }
        
        if( Is(name_start, name_end) )
        {
            return FALSE;
        }
        
        p_name_start[0] = name_start;
        p_name_end[0]   = name_end;
        
        return TRUE;
    }

// =============================================================================

    extern int
    HM_FileDlg_CountFiles
    (
        CP2C(HM_FileDlgData) p_data,
        CP2(size_t)          p_max_name_length
    )
    {
        size_t max_name_length = 0;
        size_t file_count      = 0;
        size_t start_pos       = 0;
        
        // -----------------------------
        
        while(always)
        {
            size_t end_pos    = 0;
            size_t name_start = 0;
            size_t name_end   = 0;
            
            // \task[high] While we're adding high priority tasks, we should
            // also work out how to reverse the ordering of the files, since
            // the file dialog api seems to treat it like a FIFO... who
            // knows why. Where exactly to handle this, not sure yet.
            
            // \task[high] This logic doesn't work if there's only one
            // file selected because... for some dumbass reason Win32 doesn't
            // quote the file in that case, but does when more than one is
            // selected.
            BOOL const found_name = HM_FileDlg_GetNextFile
            (
                p_data,
                start_pos,
                &end_pos,
                &name_start,
                &name_end
            );
            
            // -----------------------------
            
            // \task[high] This is not quite the right condition at the
            // moment, because the actual length counts null terminators
            // at the end as well as spaces. We need to have a way to
            // know that searching for paths ended "naturally"
            if
            (
                (start_pos < p_data->m_spec.m_actual_length)
             && (end_pos <= start_pos)
            )
            {
                // Encountered some funky looking data...
                break;
            }
            
            start_pos = end_pos;
            
            if(found_name)
            {
                size_t const name_length = (name_end - name_start);
                
                // -----------------------------
                
                file_count += 1;
                
                if(name_length > max_name_length)
                {
                    max_name_length = name_length;
                }
            }
            else
            {
                break;
            }
        }
        
        p_max_name_length[0] = max_name_length;
        
        return file_count;
    }

// =============================================================================

    // \task[med] Move to wrapper header
    extern int
    HM_ListBox_FindString
    (
        HWND const       p_listbox,
        size_t     const p_start_index,
        CP2C(char)       p_string
    )
    {
        int r = SendMessage
        (
            p_listbox,
            LB_FINDSTRING,
            p_start_index,
            (LPARAM) p_string
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    static void
    PatchDlg_AddFileName
    (
        CP2(FDOC)        p_doc,
        HWND       const p_listbox,
        CP2C(char)       p_file_name

    )
    {
        // Index of the slot for the newly added module (if it ultimately
        // ends up being added.)
        size_t const k = p_doc->nummod;
        
        int const string_index = HM_ListBox_FindString
        (
            p_listbox,
            0,
            p_file_name
        );
        
        // -----------------------------
        
        // If the file is to be added to the list box, we expect that it
        // will not be present. If it is not present, that is an "error", but
        // that's what we want to see if we are to proceed and add the asm
        // file as a module.
        if( IsNotListBoxError(string_index) )
        {
            return;
        }
        
        p_doc->nummod += 1;
        
        p_doc->modules = (ASMHACK*) realloc
        (
            p_doc->modules,
            sizeof(ASMHACK) * p_doc->nummod
        );
        
        if(always)
        {
            CP2(ASMHACK) mod = p_doc->modules;

            // -----------------------------
            
            mod[k].filename = _strdup(p_file_name);
            
            mod[k].flag = 0;
            
            HM_ListBox_AddString
            (
                p_listbox,
                p_file_name
            );
        }
    }

// =============================================================================

    extern int
    HM_FileDlg_AddFiles
    (
        CP2(FDOC)                  p_doc,
        HWND                 const p_listbox,
        CP2C(HM_FileDlgData)       p_data
    )
    {
        size_t max_name_length = 0;
        
        size_t const file_count = HM_FileDlg_CountFiles
        (
            p_data,
            &max_name_length
        );
        
        size_t start_pos = 0;
        
        char * file_name_buf = (char*) calloc
        (
            p_data->m_folder.m_actual_length + max_name_length + 1,
            sizeof(char)
        );
        
        size_t const insertion_offset = strlen(p_data->m_folder.m_buffer);
        
        // -----------------------------
        
        strcpy
        (
            file_name_buf,
            p_data->m_folder.m_buffer
        );
        
        if( IsZero(file_count) )
        {
            goto cleanup;
        }
        
        while(always)
        {
            size_t end_pos    = 0;
            size_t name_start = 0;
            size_t name_end   = 0;
            
            BOOL const found_name = HM_FileDlg_GetNextFile
            (
                p_data,
                start_pos,
                &end_pos,
                &name_start,
                &name_end
            );
            
            // -----------------------------
            
            if(end_pos <= start_pos)
            {
                // Something's wrong. This should monotonically increase.
                break;
            }
            
            if(found_name)
            {
                CP2(char) dest   = (file_name_buf + insertion_offset);
                P2C(char) source = (p_data->m_spec.m_buffer + name_start);
                
                size_t i = 0;
                
                size_t const name_length = (name_end - name_start);
                
                // -----------------------------
                
                start_pos = end_pos;
                
                for(i = 0; i < name_length; i += 1)
                {
                    dest[i] = source[i];
                }
                
                dest[i] = '\0';
                
                PatchDlg_AddFileName
                (
                    p_doc,
                    p_listbox,
                    file_name_buf
                );
            }
            else
            {
                break;
            }
        }
        
    cleanup:
        
        if(file_name_buf)
        {
            free(file_name_buf);
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
        FDOC * const doc = p_ed->ew.doc;
        
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
        
        HM_FileDlg_AddFiles
        (
            doc,
            listbox,
            data
        );
        
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
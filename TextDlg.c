
    #include <stdio.h>

    #include "structs.h"
    #include "prototypes.h"

    #include "Callbacks.h"

    #include "Wrappers.h"

    #include "HMagicUtility.h"

    #include "TextEnum.h"
    #include "TextLogic.h"

// =============================================================================

SD_ENTRY text_sd[] =
{
    {
        "LISTBOX",
        "",
        0, 0, 0, 100,
        ID_TextEntriesListControl,
        (
            WS_VISIBLE
          | WS_CHILD
          | LBS_NOTIFY
          | WS_VSCROLL
          | WS_HSCROLL
          | WS_CLIPCHILDREN
        ),
        WS_EX_CLIENTEDGE,
        FLG_SDCH_FOWH
    },
    {
        "EDIT",
        "",
        10, 95, 10, 40,
        ID_TextEditWindow,
        (
            WS_VISIBLE
          | WS_TABSTOP
          | WS_CHILD
          | WS_BORDER
          | WS_CLIPSIBLINGS
          | ES_MULTILINE
          | WS_VSCROLL
          | WS_HSCROLL
          | WS_CLIPCHILDREN
          | ES_AUTOVSCROLL
          | ES_AUTOHSCROLL
        ),
        WS_EX_CLIENTEDGE,
        (FLG_SDCH_FOWH | FLG_SDCH_FOY)
    },
    {
        "BUTTON",
        "Set",
        0, 25, 50, 0,
        ID_TextSetTextButton,
        (WS_VISIBLE | WS_TABSTOP | WS_CHILD),
        0,
        (FLG_SDCH_FOY | FLG_SDCH_FOH)
    },
    {
        "BUTTON",
        "Edit text",
        60, 25, 65, 0,
        ID_TextEditTextRadio,
        (WS_VISIBLE | WS_TABSTOP | WS_CHILD | BS_AUTORADIOBUTTON),
        0,
        (FLG_SDCH_FOY | FLG_SDCH_FOH)
    },
    {
        "BUTTON",
        "Edit dictionary",
        130, 25, 120, 0,
        ID_TextEditDictionaryRadio,
        (WS_VISIBLE | WS_TABSTOP | WS_CHILD | BS_AUTORADIOBUTTON),
        0,
        (FLG_SDCH_FOY | FLG_SDCH_FOH)
    }
};

// =============================================================================

    SUPERDLG textdlg =
    {
        "",
        TextDlg,
        WS_CHILD | WS_VISIBLE,
        600, 200,
        MACRO_ArrayLength(text_sd),
        text_sd
    };

// =============================================================================

    static void
    TextDlg_SetText
    (
        CP2(TEXTEDIT)       p_ed,
        HWND          const p_win
    )
    {
        HWND const listbox = GetDlgItem
        (
            p_win,
            ID_TextEntriesListControl
        );
        
        // Message index (if any is selected)
        int m_i = HM_ListBox_GetSelectedItem(listbox);
        
        FDOC * const doc = p_ed->ew.doc;
        
        uint8_t * const rom = doc->rom;
        
        CP2C(text_offsets_ty) to = &offsets.text;
        
        unsigned const dict_loc = to->dictionary;
        
        // -----------------------------
        
        if( IsNotListBoxError(m_i) )
        {
            // abbreviation of text edit window.
            HWND const ted_win = GetDlgItem(p_win, ID_TextEditWindow);
    
            // Add one for a null terminator, because the API call doesn't
            // take one into consideration, however the subsequent API to get
            // the text requires your self reported buffer size to leave room
            // for one, otherwise you will be missing a character when you
            // read it out.
            size_t const ted_len = ( GetWindowTextLength(ted_win) + 1 );
            
            ZTextMessage zmsg = { 0 };
            
            AString amsg = { 0 };
            
            // -----------------------------
            
            AString_InitSized(&amsg, ted_len + 1);
            
            // \task[med] This sequence has code smell, it would be nice if
            // we could directly initialize ascii text message from
            // a dialog control... maybe. That way we could get the
            // length correct ahead of time.
            GetWindowText
            (
                ted_win,
                amsg.m_text,
                amsg.m_len
            );
            
            Makezeldastring(doc, &amsg, &zmsg);
            
            // \task[med] Lack of a better criterion for now.
            if(zmsg.m_text != NULL)
            {
                char * text_buf = NULL;
                
                // -----------------------------
                
                if(p_ed->num)
                {
                    // Note that this logic assumes that the dictionary
                    // data directly follows its pointer table.
                    uint16_t j = ldle16b
                    (
                        rom
                      + rom_addr_split(to->bank, ldle16b(rom + dict_loc) - 2)
                    );
                    
                    // Dictionary entry base and bound. (Bound is one byte past
                    // the end of the dictionary entry.
                    int const de_base  = ldle16b_i(rom + dict_loc, m_i);
                    int const de_bound = ldle16b_i(rom + dict_loc, m_i + 1);
                    int const de_delta = (de_bound - de_base);
                    
                    // Code golf for the length in zchars of the revised
                    // dictionary entry.
                    int const m = zmsg.m_len;
                    
                    int const num_dict = 
                    (
                        rom_addr_split(to->bank, ldle16b(rom + dict_loc) )
                      - dict_loc
                    ) >> 1;
                    
                    // Index into dictionary entries.
                    int d_i = 0;
                    
                    // One past the point where the dictionary data would end
                    // if we hypothetically adjusted the dictionary the way
                    // the user is requesting. If the resulting
                    // pool of dictionary entries is too large for the amount
                    // that has been set aside in the original rom, we
                    // reject the edit.
                    unsigned const trial_bound = rom_addr_split
                    (
                        to->bank,
                        j + m + de_base - de_bound
                    );
                    
                    // -----------------------------
                    
                    if(trial_bound > to->dictionary_bound)
                    {
                        // \task[low] Really we should give the user
                        // more indication as to how much space they have
                        // to work with. Ideally, we'd let them have a 
                        // larger dictionary via an ASM hack.
                        MessageBeep(0);
                        
                        ZTextMessage_Free(&zmsg);
                        AString_Free(&amsg);
                        
                        return;
                    }
                    
                    memcpy(rom + rom_addr_split(to->bank, de_base + m),
                           rom + rom_addr_split(to->bank, de_bound),
                           j - de_bound);
                    
                    memcpy(rom + rom_addr_split(to->bank, de_base),
                           zmsg.m_text,
                           m);
                    
                    for(d_i = (m_i + 1); d_i < num_dict; d_i += 1)
                    {
                        addle16b_i(rom + dict_loc, d_i, (m - de_delta) );
                    }
                }
                else
                {
                    ZTextMessage_Free( &doc->text_bufz[m_i] );
                    
                    doc->text_bufz[m_i] = zmsg;
                }
                
                TextLogic_ZStringToAString
                (
                    doc,
                    &zmsg,
                    &amsg
                );
                
                asprintf
                (
                    &text_buf,
                    "%03d: %s",
                    m_i,
                    amsg.m_text
                );
                
                if(p_ed->num)
                {
                    ZTextMessage_Free(&zmsg);
                }
                
                HM_ListBox_DeleteString(listbox, m_i);
                
                HM_ListBox_InsertString(listbox, m_i, text_buf);
                
                HM_ListBox_SelectItem(listbox, m_i);
                
                free(text_buf);
            }
            else
            {
                int e_i = text_error - amsg.m_text;
                
                // -------------------------
                
                SendMessage(ted_win, EM_SETSEL, e_i, e_i);
                
                SetFocus(ted_win);
                
                return;
            }
            
            AString_Free(&amsg);
            
            doc->t_modf = 1;
        }
    }

// =============================================================================

    static void
    TextDlg_ListMonologueStrings
    (
        CP2C(FDOC)       p_doc,
        HWND       const p_listbox
    )
    {
        char * text_buf = NULL;
        
        size_t m_i = 0;
        
        AString asc_msg = { 0 };
        
        // -----------------------------
        
        for(m_i = 0; m_i < p_doc->t_number; m_i += 1)
        {
            size_t dummy_len = 0;
            
            int write_len = 0;
            
            // -----------------------------
            
            AString_Init(&asc_msg);
            
            TextLogic_ZStringToAString
            (
                p_doc,
                &p_doc->text_bufz[m_i],
                &asc_msg
            );
            
            dummy_len = strlen(asc_msg.m_text);
            
            write_len = asprintf
            (
                &text_buf,
                "%03d: %s",
                m_i,
                asc_msg.m_text
            );
            
            HM_ListBox_AddString
            (
                p_listbox,
                text_buf
            );
        }
        
        // -~-~-~-~-~-~-~-~-~-~-~-~-~-~-
        
        AString_Free(&asc_msg);
        
        if(text_buf)
        {
            free(text_buf);
        }
    }

// =============================================================================

    static void
    TextDlg_ListDictionaryStrings
    (
        FDOC const * const p_doc,
        HWND         const p_listbox
    )                             
    {
        char * text_buf = NULL;
        
        CP2(uint8_t) rom = p_doc->rom;
        
        int de_offset = 0;
        
        uint16_t de_base = 0;

        CP2C(text_offsets_ty) to = &offsets.text;
        
        AString amsg = { 0 };
        
        ZTextMessage zmsg = { 0 };
        
        // Index into dictionary entry table.
        size_t d_i = 0;
        
        // \task[high] This should be rewritten to make more sense and feel
        // less convoluted.
        // Number of dictionary entries.
        size_t num_des =
        (
            ldle16b(rom + to->dictionary)
          - ( cpuaddr(to->dictionary + 2) & 0xffff )
        ) >> 1;
        
        // -----------------------------
        
        ZTextMessage_Init(&zmsg);
        
        de_base = ldle16b(rom + to->dictionary);
        
        for(d_i = 0; d_i < num_des; d_i += 1)
        {
            uint16_t const de_bound = ldle16b_i(rom + to->dictionary,
                                                d_i + 1);
            
            // -----------------------------
            
            ZTextMessage_Empty(&zmsg);
            
            de_offset = rom_addr_split(to->bank, de_base);
            
            ZTextMessage_AppendStream(&zmsg,
                                      rom + de_offset,
                                      de_bound - de_base);
            
            TextLogic_ZStringToAString(p_doc, &zmsg, &amsg);
            
            asprintf(&text_buf, "%03d: %s", d_i, amsg.m_text);
            
            HM_ListBox_AddString(p_listbox, text_buf);
            
            de_base = de_bound;
        }
        
        if(text_buf)
        {
            free(text_buf);
            
            text_buf = NULL;
        }
        
        ZTextMessage_Free(&zmsg);
        AString_Free(&amsg);
    }

// =============================================================================

    static void
    TextDlg_ListStrings
    (
        CP2C(TEXTEDIT)       p_ed,
        HWND           const p_listbox
    )
    {
        CP2C(FDOC) doc = p_ed->ew.doc;
        
        int max_extent = 0;
        
        // -----------------------------
        
        if(p_ed->num)
        {
            TextDlg_ListDictionaryStrings(doc, p_listbox);
        }
        else
        {
            TextDlg_ListMonologueStrings(doc, p_listbox);
        }
        
        max_extent = HM_ListBox_CalcMaxTextExtent(p_listbox);

        // \task[med] We should adjust this dynamically as items are edited.
        // that might take some careful consideration as to what design t
        // employ.
        HM_ListBox_SetHorizontalExtent
        (
            p_listbox,
            max_extent + 50
        );
    }

// =============================================================================

    extern BOOL CALLBACK
    TextDlg
    (
        HWND   p_win,
        UINT   p_msg,
        WPARAM p_wp,
        LPARAM p_lp)
    {
        unsigned char *rom;
        
        int i, k, l;
        
        TEXTEDIT * ed = NULL;
        
        FDOC *doc;
        
        AString asc_msg = { 0 };
        
        HWND const text_edit = GetDlgItem(p_win, ID_TextEditWindow);
        
        HWND const listbox = GetDlgItem(p_win, ID_TextEntriesListControl);
        
        text_offsets_ty const * const to = &offsets.text;
        
        unsigned const dict_loc = to->dictionary;
        
        // -----------------------------
        
        switch(p_msg)
        {
        
        case WM_INITDIALOG:
            
            SetWindowLongPtr(p_win, DWLP_USER, p_lp);
            
            ed = (TEXTEDIT*) p_lp;
            
            CheckDlgButton(p_win, ID_TextEditTextRadio, BST_CHECKED);
            
            ed->dlg = p_win;
            ed->num = 0;
            
            doc = ed->ew.doc;
            
            if( ! doc->t_loaded)
            {
                LoadText(doc);
            }
            
            TextDlg_ListStrings(ed, listbox);
            
            break;
        
        case WM_SETFOCUS:
            
            SetFocus(text_edit);
            
            break;
        
        case WM_CLOSE:
            
            return 1;
        
        case WM_COMMAND:
            
            ed = (TEXTEDIT*) GetWindowLongPtr(p_win, DWLP_USER);
            
            if(!ed)
                break;
            
            doc = ed->ew.doc;
            
            rom = doc->rom;
            
            switch(p_wp)
            {
            
            case ID_TextEntriesListControl | (LBN_DBLCLK << 16):
                
                i = HM_ListBox_GetSelectedItem( (HWND) p_lp);
                
                if( IsNotListBoxError(i) )
                {
                    if(ed->num)
                    {
                        int dict_entry_offset = 0;
                        
                        ZTextMessage zmsg = { 0 };
                        
                        // -----------------------------
                        
                        l = ldle16b_i(rom + dict_loc, i);
                        k = ldle16b_i(rom + dict_loc, i + 1);
                        
                        dict_entry_offset = rom_addr_split(to->bank, l);
                        
                        ZTextMessage_AppendStream(&zmsg,
                                                  rom + dict_entry_offset,
                                                  (k - l) );
                        
                        TextLogic_ZStringToAString
                        (
                            doc,
                            &zmsg,
                            &asc_msg
                        );
                        
                        ZTextMessage_Free(&zmsg);
                    }
                    else
                    {
                        TextLogic_ZStringToAString
                        (
                            doc,
                            &doc->text_bufz[i],
                            &asc_msg
                        );
                    }
                    
                    SetDlgItemText(p_win, ID_TextEditWindow, asc_msg.m_text);
                }
                else
                {
                    SetDlgItemText(p_win, ID_TextEditWindow, 0);
                }
                
                break;
            
            case ID_TextSetTextButton:
                
                TextDlg_SetText(ed, p_win);
                
                break;
            
            case ID_TextEditTextRadio:
                
                ed->num = 0;
                
                HM_ListBox_ResetContent(listbox);
                
                TextDlg_ListStrings(ed, listbox);
    
                break;
            
            case ID_TextEditDictionaryRadio:
                
                ed->num = 1;
                
                HM_ListBox_ResetContent(listbox);
                
                TextDlg_ListStrings(ed, listbox);
                
                break;
            }
            
            break;
        }
        
        AString_Free(&asc_msg);
        
        return FALSE;
    }

// =============================================================================

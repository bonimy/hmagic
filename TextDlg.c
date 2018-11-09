
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
    {"LISTBOX","",0,0,0,70, 3000, WS_VISIBLE|WS_CHILD|LBS_NOTIFY|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"EDIT","",0,65,0,24, 3001, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|ES_MULTILINE|WS_VSCROLL|ES_AUTOVSCROLL,WS_EX_CLIENTEDGE,14},
    {"BUTTON","Set",0,20,50,0, 3002, WS_VISIBLE|WS_TABSTOP|WS_CHILD,0,12},
    {"BUTTON","Edit text",60,20,65,0, 3003, WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON,0,12},
    {"BUTTON","Edit dictionary",130,20,120,0, 3004, WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON,0,12}
};

// =============================================================================

SUPERDLG textdlg =
{
    "", TextDlg, WS_CHILD|WS_VISIBLE,600,200, 5, text_sd
};

// =============================================================================

static void
TextDlg_SetText(TEXTEDIT * const p_ed,
                HWND       const p_win)
{
    int i = SendDlgItemMessage(p_win,
                               ID_TextEntriesListControl,
                               LB_GETCURSEL,
                               0,
                               0);
    
    int j = 0;
    int k = 0;
    int l = 0;
    int m = 0;
    
    FDOC * const doc = p_ed->ew.doc;
    
    uint8_t * const rom = doc->rom;
    
    // -----------------------------
    
    if(i != -1)
    {
        HWND const hc = GetDlgItem(p_win, ID_TextEntriesListControl);
        
        // abbreviation of text edit window.
        HWND const ted_win = GetDlgItem(p_win, ID_TextEditWindow);

        // Add one for a null terminator, because the API call doesn't
        // take one into consideration, however the subsequent API to get
        // the text requires your self reported buffer size to leave room
        // for one, otherwise you will be missing a character when you
        // read it out.
        size_t const ted_len = ( GetWindowTextLength(ted_win) + 1);
        
        ZTextMessage zmsg = { 0 };
        
        AsciiTextMessage amsg = { 0 };
        
        // -----------------------------
        
        // \task This sequence has code smell, clean it up.
        amsg.m_len      = ted_len;
        amsg.m_capacity = ted_len + 100;
        amsg.m_text     = (char*) calloc(ted_len + 100, sizeof(char) );
        
        GetDlgItemText(p_win,
                       ID_TextEditWindow,
                       amsg.m_text,
                       amsg.m_len);
        
        Makezeldastring(doc, &amsg, &zmsg);
        
        // \task Lack of a better criterion for now.
        if(zmsg.m_text != NULL)
        {
            char * text_buf = NULL;
            
            // -----------------------------
            
            if(p_ed->num)
            {
                m = zmsg.m_len;
                
                j = *(unsigned short*) (rom + 0x77ffe + *(short*)(rom + 0x74703));
                k = ((unsigned short*) (rom + 0x74705))[i];
                l = ((unsigned short*) (rom + 0x74703))[i];
                
                if(j + m + l - k > 0xc8d9)
                {
                    MessageBeep(0);
                    
                    ZTextMessage_Free(&zmsg);
                    AsciiTextMessage_Free(&amsg);
                    
                    return;
                }
                
                memcpy(rom + 0x68000 + l + m, rom + 0x68000 + k, j - k);
                memcpy(rom + 0x68000 + l, zmsg.m_text, m);
                
                k -= l;
                
                l = ( ldle16b(rom + 0x74703) - 0xc703 ) >> 1;
                
                for(j = i + 1; j < l; j++)
                    ((short*) (rom + 0x74703))[j] += m - k;
            }
            else
            {
                ZTextMessage_Free( &doc->text_bufz[i] );
                
                doc->text_bufz[i] = zmsg;
            }
            
            Makeasciistring(doc, &zmsg, &amsg);
            
            asprintf(&text_buf,
                     "%03d: %s",
                     i,
                     amsg.m_text);
            
            if(p_ed->num)
            {
                ZTextMessage_Free(&zmsg);
            }
            
            SendMessage(hc, LB_DELETESTRING, i, 0);
            SendMessage(hc, LB_INSERTSTRING, i, (LPARAM) text_buf);
            SendMessage(hc, LB_SETCURSEL, i, 0);
            
            free(text_buf);
        }
        else
        {
            char * text_buf = NULL;
            
            // \task What the hell was this intended to do?
            i = text_error - text_buf;
            
            SendMessage(ted_win, EM_SETSEL, i, i);
            
            SetFocus(ted_win);
            
            return;
        }
        
        AsciiTextMessage_Free(&amsg);
        
        doc->t_modf = 1;
    }
}



// =============================================================================

BOOL CALLBACK
TextDlg(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    TEXTEDIT * ed;
    
    FDOC *doc;
    
    int i, j, k, l;
    
    unsigned char *rom;
    
    HWND hc;
    
    AsciiTextMessage asc_msg = { 0 } ;
                
    // -----------------------------
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLongPtr(win, DWLP_USER, lparam);
        
        ed = (TEXTEDIT*) lparam;
        
        CheckDlgButton(win, ID_TextEditTextRadio, BST_CHECKED);
        
        ed->dlg = win;
        ed->num = 0;
        
        doc = ed->ew.doc;
        
        if( ! doc->t_loaded)
        {
            LoadText(doc);
        }
        
    updstrings:
        
        hc = GetDlgItem(win, ID_TextEntriesListControl);
        
        if(ed->num)
        {
            char * text_buf = NULL;
            
            int dict_entry_offset = 0;
            
            ZTextMessage zmsg = { 0 };
            
            // -----------------------------
            
            ZTextMessage_Init(&zmsg);
            
            rom = doc->rom;
            
            j = ( ldle16b(rom + 0x74703) - 0xc705 ) >> 1;
            
            l = ldle16b(rom + 0x74703);
            
            for(i = 0; i < j; i++)
            {
                k = ldle16b_i(rom + 0x74705, i);
                
                dict_entry_offset = romaddr(0xe0000 + l);
                
                ZTextMessage_AppendStream(&zmsg,
                                          rom + dict_entry_offset,
                                          k - l);
                
                Makeasciistring(doc, &zmsg, &asc_msg);
                
                asprintf(&text_buf, "%03d: %s", i, asc_msg.m_text);
                
                SendMessage(hc, LB_ADDSTRING, 0, (LPARAM) text_buf);
                
                ZTextMessage_Free(&zmsg);
                
                free(text_buf);
                
                l = k;
            }
        }
        else
        {
            char * text_buf = NULL;
            
            // -----------------------------
            
            for(i = 0; i < doc->t_number; i++)
            {
                size_t dummy_len = 0;
                int    write_len = 0;
                
                AsciiTextMessage_Init(&asc_msg);
                
                Makeasciistring(doc, &doc->text_bufz[i], &asc_msg);
                
                dummy_len = strlen(asc_msg.m_text);
                
                write_len = asprintf(&text_buf,
                                     "%03d: %s",
                                     i,
                                     asc_msg.m_text);
                
                SendMessage(hc, LB_ADDSTRING, 0, (LPARAM) text_buf);
                
                free(text_buf);
            }
        }
        
        break;
    
    case WM_CLOSE:
        
        return 1;
    
    case WM_COMMAND:
        
        ed = (TEXTEDIT*) GetWindowLongPtr(win, DWLP_USER);
        
        if(!ed)
            break;
        
        doc = ed->ew.doc;
        
        rom = doc->rom;
        
        switch(wparam)
        {
        
        case ID_TextEntriesListControl | (LBN_DBLCLK << 16):
            
            i = SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            
            if(i != -1)
            {
                if(ed->num)
                {
                    int dict_entry_offset = 0;
                    
                    ZTextMessage zmsg;
                    
                    // -----------------------------
                    
                    l = ldle16b_i(rom + 0x74703, i);
                    k = ldle16b_i(rom + 0x74705, i);
                    
                    dict_entry_offset = romaddr(0xe0000 + l);
                    
                    zmsg.m_len = (k - l);
                    zmsg.m_text = (uint8_t*) calloc(1, zmsg.m_len + 1);
                    
                    memcpy(zmsg.m_text,
                           rom + dict_entry_offset,
                           zmsg.m_len);
                    
                    Makeasciistring(doc, &zmsg, &asc_msg);
                }
                else
                {
                    Makeasciistring(doc, &doc->text_bufz[i], &asc_msg);
                }
                
                SetDlgItemText(win, ID_TextEditWindow, asc_msg.m_text);
            }
            else
            {
                SetDlgItemText(win, ID_TextEditWindow, 0);
            }
            
            break;
        
        case ID_TextSetTextButton:
            
            TextDlg_SetText(ed, win);
            
            break;
        
        case ID_TextEditTextRadio:
            
            ed->num = 0;
            
            SendDlgItemMessage(win,
                               ID_TextEntriesListControl,
                               LB_RESETCONTENT,
                               0,
                               0);
            
            goto updstrings;
        
        case ID_TextEditDictionaryRadio:
            
            ed->num = 1;
            
            SendDlgItemMessage(win,
                               ID_TextEntriesListControl,
                               LB_RESETCONTENT,
                               0,
                               0);
            
            goto updstrings;
        }
        
        break;
    }
    
    AsciiTextMessage_Free(&asc_msg);
    
    return FALSE;
}

// =============================================================================

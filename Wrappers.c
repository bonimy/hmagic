
    #include <stdio.h>
    #include <stdarg.h>

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "HMagicUtility.h"

// =============================================================================

RECT
HM_GetWindowRect(HWND const p_win)
{
    RECT r;
    
    GetWindowRect(p_win, &r);
    
    return r;
}

// =============================================================================

RECT
HM_GetClientRect(HWND const p_win)
{
    RECT r;
    
    GetClientRect(p_win, &r);
    
    return r;
}

// =============================================================================

POINT
HM_ClientToScreen(HWND const p_win)
{
    POINT pt;
    
    ClientToScreen(p_win, &pt);
    
    return pt;
}

// =============================================================================

POINT
HM_GetWindowPos(HWND const p_win)
{
    RECT const r = HM_GetWindowRect(p_win);
    
    POINT const pt = { r.left, r.top };
     
    return pt;
}

// =============================================================================

POINT
HM_GetClientPos(HWND const p_win)
{
    RECT const r = HM_GetClientRect(p_win);
    
    POINT const pt = { r.left, r.top };
    
    return pt;
}

// =============================================================================

POINT
HM_PointClientToScreen(HWND  const p_win,
                       POINT const p_rel_pos)
{
    POINT screen_pos = HM_GetWindowPos(p_win);
    
    screen_pos.x += p_rel_pos.x;
    screen_pos.y += p_rel_pos.y;
    
    return screen_pos;
}

// =============================================================================

RECT
HM_GetDlgItemRect(HWND     const p_dlg,
                  unsigned const p_item_id)
{
    HWND const child_wnd = GetDlgItem(p_dlg, p_item_id);
    
    return HM_GetClientRect(child_wnd);
}

// =============================================================================

signed int
HM_GetSignedLoword(LPARAM p_ptr)
{
    return ( (int) (short) LOWORD(p_ptr) );
}

// =============================================================================

signed int
HM_GetSignedHiword(LPARAM p_ptr)
{
    return ( (int) (short) HIWORD(p_ptr) );
}

// =============================================================================

HM_MouseMoveData
HM_GetMouseMoveData(HWND   const p_win,
                    WPARAM const wparam,
                    LPARAM const lparam)
{
    unsigned const flags = wparam;
    
    HM_MouseMoveData d = { FALSE, 0, 0, {0, 0} };
    
    POINT const rel_pos =
    {
        HM_GetSignedLoword(lparam),
        HM_GetSignedHiword(lparam)
    };
    
    // The absolute screen coordinates of the Window itself.
    POINT const win_screen_pos = HM_GetWindowPos(p_win);
    
    // The absolute screen coordinates of the location indicated by the event
    // (obviously this will be inside of the window, so further to the right,
    // further down.)
    POINT const screen_pos =
    {
        rel_pos.x + win_screen_pos.x,
        rel_pos.y + win_screen_pos.y
    };
    
    d.m_rel_pos = rel_pos;
    
    d.m_screen_pos = screen_pos;
    
    d.m_control_down = (flags & MK_CONTROL);
    
    return d;
}

// =============================================================================

    HM_MouseWheelData
    HM_GetMouseWheelData
    (
        WPARAM const p_wp,
        LPARAM const p_lp
    )
    {
        HM_MouseWheelData d;
        
        d.m_distance = HM_GetSignedHiword(p_wp);
        
        d.m_flags = LOWORD(p_wp);
        
        d.m_shift_key   = truth(d.m_flags & MK_SHIFT);
        d.m_control_key = truth(d.m_flags & MK_CONTROL);
    
        d.m_alt_key     = (GetKeyState(VK_MENU) < 0 );
        
        d.m_screen_pos.x = HM_GetSignedLoword(p_lp);
        d.m_screen_pos.y = HM_GetSignedHiword(p_lp);
        
        return d;
    }

// =============================================================================

HM_MouseData
HM_GetMouseData(MSG const p_packed_msg)
{
    WPARAM const wp = p_packed_msg.wParam;
    LPARAM const lp = p_packed_msg.lParam;
    
    // Get client coordinates of the click.
    POINT const rel_pos =
    {
        HM_GetSignedLoword(lp),
        HM_GetSignedHiword(lp)
    };
    
    // The absolute screen coordinates of the Window itself.
    POINT const win_screen_pos = HM_GetWindowPos(p_packed_msg.hwnd);
    
    // The absolute screen coordinates of the location indicated by the event
    // (obviously this will be inside of the window, so further to the right,
    // further down.)
    POINT const screen_pos =
    {
        rel_pos.x + win_screen_pos.x,
        rel_pos.y + win_screen_pos.y
    };
    
    HM_MouseData d;
    
    // -----------------------------
    
    d.m_flags = (unsigned) wp;
    
    d.m_shift_key   = truth(d.m_flags & MK_SHIFT);
    d.m_control_key = truth(d.m_flags & MK_CONTROL);

    d.m_alt_key     = (GetKeyState(VK_MENU) < 0 );
    
    d.m_rel_pos = rel_pos;
    
    d.m_screen_pos = screen_pos;
    
    return d;
}

// =============================================================================

    extern HM_MdiActivateData
    HM_MDI_GetActivateData
    (
        WPARAM const p_wp,
        LPARAM const p_lp
    )
    {
        HM_MdiActivateData d;
        
        // -----------------------------
        
        d.m_deactivating = (HWND) p_wp;
        d.m_activating   = (HWND) p_lp;
        
        return d;
    }

// =============================================================================

    extern HWND
    HM_MDI_GetActiveChild
    (
        HWND const p_mdi_client_wnd
    )
    {
        HWND const h = (HWND) SendMessage
        (
            p_mdi_client_wnd,
            WM_MDIGETACTIVE,
            HM_NullWP(),
            HM_NullLP()
        );
        
        // -----------------------------
        
        return h;
    }

// =============================================================================

    extern LRESULT
    HM_MDI_ActivateChild
    (
        HWND const p_mdi_client_wnd,
        HWND const p_mdi_child_wnd
    )
    {
        // MDI child windows should return zero if they process this message.
        LRESULT r = SendMessage
        (
            p_mdi_client_wnd,
            WM_MDIACTIVATE,
            (WPARAM) p_mdi_child_wnd,
            HM_NullLP()
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    extern void
    HM_MDI_DestroyChild
    (
        HWND const p_mdi_client_wnd,
        HWND const p_mdi_child_wnd
    )
    {
        SendMessage
        (
            p_mdi_client_wnd,
            WM_MDIDESTROY,
            (WPARAM) p_mdi_child_wnd,
            0
        );
    }

// =============================================================================

    BOOL
    HM_DrawRectangle
    (
        HDC  const p_device_context,
        RECT const p_rect
    )
    {
        return Rectangle
        (
            p_device_context,
            p_rect.left,
            p_rect.top,
            p_rect.right,
            p_rect.bottom
        );
    }

// =============================================================================

    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win)
    {
        SCROLLINFO si = { 0 };
        
        // -----------------------------
        
        si.cbSize = sizeof(SCROLLINFO);
        
        si.fMask = SIF_ALL;
         
        GetScrollInfo(p_win, SB_VERT, &si);
        
        return si;
    }

// =============================================================================

    SCROLLINFO
    HM_GetHorizScrollInfo(HWND const p_win)
    {
        SCROLLINFO si = { 0 };
        
        // -----------------------------
        
        si.cbSize = sizeof(SCROLLINFO);
        
        si.fMask = SIF_ALL;
         
        GetScrollInfo(p_win, SB_HORZ, &si);
        
        return si;
    }

// =============================================================================

BOOL
HM_IsEmptyRect(RECT const p_rect)
{
    if
    (
        (p_rect.right == p_rect.left)
     && (p_rect.top   == p_rect.bottom)
    )
    {
        return TRUE;
    }

    return FALSE;
}

// =============================================================================

MSG
HM_PackMessage(HWND const p_win,
               UINT       p_msg_id,
               WPARAM     p_wp,
               LPARAM     p_lp)
{
    MSG msg;
    
    POINT CONST dummy = { 0, 0 };
    
    // -----------------------------
    
    msg.hwnd = p_win;
    msg.lParam = p_lp;
    msg.message = p_msg_id;
    msg.pt = dummy;
    msg.time = 0;
    msg.wParam = p_wp;
    
    return msg;
}

// =============================================================================

RGBQUAD
HM_MakeRgb(uint8_t const p_red,
           uint8_t const p_green,
           uint8_t const p_blue)
{
    RGBQUAD q;
    
    q.rgbRed      = p_red;
    q.rgbGreen    = p_green;
    q.rgbBlue     = p_blue;
    q.rgbReserved = 0;
    
    return q;
}

// =============================================================================

RGBQUAD
HM_RgbFrom5bpc(uint16_t const p_color)
{
    RGBQUAD q;

    q.rgbRed      = ( (p_color << 3) & 0xf8 );
    q.rgbGreen    = ( (p_color >> 2) & 0xf8 );
    q.rgbBlue     = ( (p_color >> 7) & 0xf8 );
    q.rgbReserved = 0;
    
    return q;
}

// =============================================================================

uint16_t
HM_ColRefTo5bpc(COLORREF const p_cr)
{
    uint16_t snes_color = 0;
    
    snes_color |= ( (p_cr >> 3) & 0x001f );
    snes_color |= ( (p_cr >> 6) & 0x03e0 );
    snes_color |= ( (p_cr >> 9) & 0x7c00 );
    
    return snes_color;
}

// =============================================================================

uint16_t
HM_ColQuadTo5bpc(RGBQUAD const p_cr)
{
    uint16_t snes_color = 0;
    
    snes_color |= ( (p_cr.rgbRed   >> 3) & 0x001f );
    snes_color |= ( (p_cr.rgbGreen << 2) & 0x03e0 );
    snes_color |= ( (p_cr.rgbBlue  << 7) & 0x7c00 );
    
    return snes_color;
}

// =============================================================================

COLORREF
HM_ColQuadToRef(RGBQUAD const p_quad)
{
    COLORREF cr = 0;
    
    cr |= ( p_quad.rgbBlue << 16) | (p_quad.rgbGreen <<  8) | (p_quad.rgbRed);
    
    return cr;
}

// =============================================================================

BOOL
HM_BinaryCheckDlgButton(HWND     const p_win,
                        unsigned const p_dlg_control,
                        BOOL     const p_is_checked)
{
    return CheckDlgButton(p_win,
                          p_dlg_control,
                          p_is_checked ? BST_CHECKED : BST_UNCHECKED);
}

// =============================================================================

HM_FileStat
HM_LoadFileContents(char const * const p_file_name)
{
    HM_FileStat fail_state = { FALSE, INVALID_HANDLE_VALUE, 0, NULL };
    
    HANDLE const h = CreateFile(p_file_name,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                0,
                                OPEN_EXISTING,
                                0,
                                0);
    
    BOOL const valid_file = (h != INVALID_HANDLE_VALUE);
    
    DWORD const file_size = (valid_file) ? GetFileSize(h, 0) : 0;
    
    char * const contents = (file_size) ? (char*) calloc(1, file_size) : 0;
    
    if(valid_file)
    {
        DWORD bytes_read = 0;
        
        // -----------------------------
        
        ReadFile(h, contents, file_size, &bytes_read, NULL);
        
        if(file_size == bytes_read)
        {
            HM_FileStat succ_state = { TRUE, h, file_size, contents };
            
            return succ_state;
        }
    }
    
    if(valid_file)
    {
        CloseHandle(h);
    }
    
    if(contents)
    {
        free(contents);
    }
    
    return fail_state;
}

// =============================================================================

void
HM_FreeFileStat(HM_FileStat * const p_s)
{
    if( ! p_s )
    {
        return;
    }
    
    if(p_s->m_contents)
    {
        free(p_s->m_contents);
        
        p_s->m_contents = NULL;
    }
    
    if(p_s->m_h != INVALID_HANDLE_VALUE)
    {
        CloseHandle(p_s->m_h);
        
        p_s->m_h = INVALID_HANDLE_VALUE;
    }
    
    p_s->m_valid_file = FALSE;
    p_s->m_file_size = 0;
}

// =============================================================================

    WPARAM
    HM_NullWP(void)
    {
        return (WPARAM) 0;
    }

// =============================================================================

    LPARAM
    HM_NullLP(void)
    {
        return (LPARAM) 0;
    }

// =============================================================================

    extern int
    HM_ComboBox_AddString
    (
        HWND       const p_combobox,
        CP2C(char)       p_string
    )
    {
        int const insertion_index = SendMessage
        (
            p_combobox,
            CB_ADDSTRING,
            HM_NullWP(),
            (WPARAM) p_string
        );
        
        // -----------------------------
        
        return insertion_index;
    }

// =============================================================================

    extern int
    HM_ComboBox_SelectItem
    (
        HWND const p_combobox,
        int  const p_item_index
    )
    {
        int const selection_index = SendMessage
        (
            p_combobox,
            CB_SETCURSEL,
            (WPARAM) p_item_index,
            HM_NullLP()
        );
        
        // -----------------------------
        
        return selection_index;
    }

// =============================================================================

    extern int
    HM_ComboBox_GetSelectedItem
    (
        HWND const p_combobox
    )
    {
        int const selected_index = SendMessage
        (
            p_combobox,
            CB_GETCURSEL,
            HM_NullLP(),
            HM_NullLP()
        );

        // -----------------------------
        
        return selected_index;
    }

// =============================================================================

    extern int
    HM_ListBox_GetCount
    (
        HWND const p_listbox
    )
    {
        int const item_count = SendMessage
        (
            p_listbox,
            LB_GETCOUNT,
            HM_NullWP(),
            HM_NullLP()
        );
        
        // -----------------------------
        
        return item_count;
    }

// =============================================================================

    extern int
    HM_ListBox_AddString
    (
        HWND    const p_listbox,
        LPCSTR  const p_string
    )
    {
        int index = SendMessage
        (
            p_listbox,
            LB_ADDSTRING,
            0,
            (LPARAM) p_string
        );
        
        // -----------------------------
        
        return index;
    }

// =============================================================================

    extern int
    HM_ListBox_SetItemData
    (
        HWND   const p_listbox,
        int    const p_item_index,
        LPARAM const p_data
    )
    {
        int r = SendMessage
        (
            p_listbox,
            LB_SETITEMDATA,
            (WPARAM) p_item_index,
            p_data
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    extern int
    HM_ListBox_GetSelectedItem
    (
        HWND const p_listbox
    )
    {
        int selected_item_index = SendMessage
        (
            p_listbox,
            LB_GETCURSEL,
            0,
            0
        );
        
        // -----------------------------
        
        return selected_item_index;
    }

// =============================================================================

    extern int
    HM_ListBox_SelectItem
    (
        HWND const p_listbox,
        int  const p_item_index
    )
    {
        int r = SendMessage
        (
            p_listbox,
            LB_SETCURSEL,
            p_item_index,
            0
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    extern int
    HM_ListBox_InsertString
    (
        HWND       const p_listbox,
        int        const p_insertion_index,
        CP2C(char)       p_string
    )
    {
        // The index at which the string was inserted.
        int const inserted_where = SendMessage
        (
            p_listbox,
            LB_INSERTSTRING,
            p_insertion_index,
            (LPARAM) p_string
        );
        
        // -----------------------------
        
        return inserted_where;
    }

// =============================================================================

    extern int
    HM_ListBox_DeleteString
    (
        HWND const p_listbox,
        int  const p_deletion_index
    )
    {
        int const remaining_string_count = SendMessage
        (
            p_listbox,
            LB_DELETESTRING,
            p_deletion_index,
            0
        );
        
        // -----------------------------
        
        return remaining_string_count;
    }

// =============================================================================

    extern int
    HM_ListBox_GetText
    (
        HWND      const p_listbox,
        int       const p_item_index,
        CP2(char)       p_buffer
    )
    {
        int const text_length = SendMessage
        (
            p_listbox,
            LB_GETTEXT,
            p_item_index,
            (LPARAM) p_buffer
        );
        
        // -----------------------------
        
        return text_length;
    }

// =============================================================================

    extern int
    HM_ListBox_GetTextLength
    (
        HWND      const p_listbox,
        int       const p_item_index
    )
    {
        int const text_length = SendMessage
        (
            p_listbox,
            LB_GETTEXTLEN,
            p_item_index,
            HM_NullLP()
        );
        
        // -----------------------------
        
        return text_length;
    }

// =============================================================================

    extern void
    HM_ListBox_ResetContent
    (
        HWND const p_listbox
    )
    {
        SendMessage
        (
            p_listbox,
            LB_RESETCONTENT,
            0,
            0
        );
    }

// =============================================================================

    extern int
    HM_ListBox_GetItemRect
    (
        HWND      const p_listbox,
        int       const p_item_index,
        CP2(RECT)       p_item_rect
    )
    {
        int r = SendMessage
        (
            p_listbox,
            LB_GETITEMRECT,
            p_item_index,
            (LPARAM) p_item_rect
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    extern void
    HM_ListBox_SetHorizontalExtent
    (
        HWND const p_listbox,
        int  const p_extent
    )
    {
        SendMessage
        (
            p_listbox,
            LB_SETHORIZONTALEXTENT,
            p_extent,
            HM_NullLP()
        );
    }

// =============================================================================
    
    extern int
    HM_ListBox_CalcMaxTextExtent
    (
        HWND const p_listbox
    )
    {
        size_t max_text_length = 0x400;
        
        char * text_buf = (char*) calloc(1, max_text_length);
        
        int i = 0;
        
        int const item_count = HM_ListBox_GetCount(p_listbox);
        
        int max_item_extent = 0;
        
        HFONT const font = (HFONT) SendMessage
        (p_listbox, WM_GETFONT, HM_NullWP(), HM_NullLP() );
        
        HDC const dc = GetDC(p_listbox);

        HGDIOBJ const old_font = SelectObject(dc, font);
        
        // -----------------------------
        
        for(i = 0; i < item_count; i += 1)
        {
            size_t text_length = HM_ListBox_GetTextLength(p_listbox, i);
            
            SIZE item_size = { 0 };
            
            RECT item_rect = { 0 };
            
            // -----------------------------
            
            if(text_length > max_text_length)
            {
                // Add some slop on the end so we shouldn't have to reallocate
                // too often.
                max_text_length = (text_length + 0x400);
                
                text_buf = (char*) realloc(text_buf, max_text_length);
            }
            
            HM_ListBox_GetText(p_listbox, i, text_buf);
            
            GetTextExtentPoint32(dc, text_buf, text_length, &item_size);
            
            HM_ListBox_GetItemRect
            (
                p_listbox,
                i,
                &item_rect
            );

            if(item_size.cx > max_item_extent)
            {
                max_item_extent = item_size.cx;
            }
        }
        
        if(text_buf)
        {
            free(text_buf);
        }
        
        SelectObject(dc, old_font);
        
        ReleaseDC(p_listbox, dc);
        
        return max_item_extent;
    }

// =============================================================================

    extern unsigned long
    HM_NumPadKeyDownFilter
    (
        MSG const p_packed_msg
    )
    {
        unsigned long const key_info = p_packed_msg.lParam;
        
        unsigned long const key = p_packed_msg.wParam;
        
        // -----------------------------
        
        if( ! (key_info & 0x1000000) )
        {
            // Allows the num pad directional keys to work regardless of 
            // whether numlock is on or not. (This block is entered if numlock
            // is off.)
            
            switch(key)
            {
               
            default:
                break;
            
            case VK_RIGHT:
                return VK_NUMPAD6;
            
            case VK_LEFT:
                return VK_NUMPAD4;
            
            case VK_DOWN:
                return VK_NUMPAD2;
            
            case VK_UP:
                return VK_NUMPAD8;
            }
        }
        
        return key;
    }

// =============================================================================

    extern void
    HM_OK_MsgBox
    (
        HWND       const p_win,
        CP2C(char)       p_prompt,
        CP2C(char)       p_title_bar
    )
    {
        MessageBox
        (
            p_win,
            p_prompt,
            p_title_bar,
            MB_OK
        );
    }

// =============================================================================

    extern BOOL
    HM_YesNo_MsgBox
    (
        HWND       const p_win,
        CP2C(char)       p_prompt,
        CP2C(char)       p_title_bar
    )
    {
        int response = MessageBox
        (
            p_win,
            p_prompt,
            p_title_bar,
            MB_YESNO
        );
        
        // -----------------------------
        
        return Is(response, IDYES);
    }

// =============================================================================

    extern void *
    hm_memdup
    (
        CP2C(void)       p_array,
        size_t     const p_length
    )
    {
        CP2(void) new_arr = calloc(1, p_length);
        
        // -----------------------------
        
        if(new_arr)
        {
            memcpy
            (
                new_arr,
                p_array,
                p_length
            );
        }
        
        return new_arr;
    }

// =============================================================================

    extern char *
    hm_strdup
    (
        CP2C(char) p_string
    )
    {
        size_t const length = strlen(p_string);
        
        char * dup_string = (char*) calloc
        (
            (length + 1),
            sizeof(char)
        );
        
        // -----------------------------

        if( IsNonNull(dup_string) )
        {
            strcpy(dup_string, p_string);
        }
        
        return dup_string;
    }

// =============================================================================

    extern char *
    hm_strndup
    (
        CP2C(char)    p_string,
        size_t  const p_length
    )
    {
        char * out = (char*) calloc
        (
            (p_length + 1),
            sizeof(char)
        );
        
        size_t i = 0;
        
        // -----------------------------
        
        for(i = 0; i < p_length; i += 1)
        {
            out[i] = p_string[i];
            
            if(p_string[i] == 0)
            {
                break;
            }
        }
        
        return out;
    }

// =============================================================================

    /**
        Returns TRUE if the second parameter is found at the location
        pointed to by the first parameter.
    */
    BOOL
    HM_CheckEmbeddedStr(void const * const p_buffer,
                        char const * const p_string)
    {
        char const * const buf = (char const *) p_buffer;
        
        size_t const len = strlen(p_string);
        
        int r = strncmp(buf, p_string, len);
        
        // -----------------------------
        
        // The standard C function above returns zero only in the case of an
        // exact match.
        return (r == 0);
    }

// =============================================================================

    BOOL
    HM_FileExists
    (
        CP2C(char)  p_filename,
        CP2(HANDLE) p_handle
    )
    {
        HANDLE h = CreateFile(p_filename,
                              GENERIC_READ,
                              0,
                              0,
                              OPEN_EXISTING,
                              0,
                              0);
        
        // -----------------------------
        
        p_handle[0] = h;
        
        return (h != INVALID_HANDLE_VALUE);
    }

// =============================================================================

    extern int
    vasprintf
    (
        CP2(char *)       p_buf_out,
        CP2C(char)        p_fmt,
        va_list     const p_var_args
    )
    {
        va_list ap1;
        
        size_t size;
        
        char * buf;
        
        int r = 0;
        
        // -----------------------------
        
        va_copy(ap1, p_var_args);
        
        size = vsnprintf(NULL, 0, p_fmt, ap1) + 1;
        
        va_end(ap1);
        
        buf = (char*) calloc(1, size);
        
        if( ! buf )
        {
            return -1;
        }
        
        r = vsnprintf
        (
            buf,
            size,
            p_fmt,
            p_var_args
        );
        
        // \note This is done after the formatting of the newly allocated
        // string, because if the original string was input to the new string
        // we'll end up referencing freed memory which is bad-mmkay.
        if(p_buf_out[0])
        {
            free(p_buf_out[0]);
            
            p_buf_out[0] = NULL;
        }
        
        (*p_buf_out) = buf;
        
        return r;
    }

// =============================================================================

    extern int
    asprintf
    (
        CP2(char *) p_buf_out,
        CP2C(char)  p_fmt,
        ...
    )
    {
        int r;
        
        // -----------------------------
        
        va_list var_args;
    
        va_start(var_args, p_fmt);
        
        r = vasprintf(p_buf_out, p_fmt, var_args);
        
        va_end(var_args);
    
        return r;
    }

// =============================================================================

    extern int
    vascatf
    (
        char       ** const p_buf_out,
        CP2C(char)          p_fmt,
        va_list       const p_var_args
    )
    {
        va_list ap1;
        
        size_t current_len    = 0;
        size_t additional_len = 0;
        size_t final_len      = 0;
        
        char * buf = NULL;
        
        // -----------------------------
        
        va_copy(ap1, p_var_args);
        
        additional_len = vsnprintf(NULL, 0, p_fmt, ap1) + 1;
        
        va_end(ap1);
        
        if( ! p_buf_out )
        {
            return -1;
        }
        
        if( p_buf_out[0] )
        {
            current_len = strlen(p_buf_out[0]);
            
            final_len = (current_len + additional_len + 1);
            
            buf = (char*) recalloc(p_buf_out[0],
                                   final_len,
                                   current_len + 1,
                                   sizeof(char) );
        }
        else
        {
            buf = (char*) calloc(1, additional_len);
            
            final_len = additional_len;
        }
        
        if( ! buf )
        {
            return -1;
        }
        
        p_buf_out[0] = buf;
        
        return vsnprintf(buf + current_len,
                         final_len,
                         p_fmt,
                         p_var_args);
    }

// =============================================================================

    extern int
    ascatf
    (
        char       ** const p_buf_out,
        CP2C(char)          p_fmt,
        ...
    )
    {
        int r;
        
        // -----------------------------
        
        va_list var_args;
        
        va_start(var_args, p_fmt);
        
        r = vascatf(p_buf_out, p_fmt, var_args);
        
        va_end(var_args);
        
        return r;
    }

// =============================================================================

#if 1

    static int
    IconResourceArray[] = { IDI_ICON1 };

    enum
    {
        NUM_AnimIcons = MACRO_ArrayLength(IconResourceArray)
    };

// =============================================================================

    #include "HMagicLogic.h"

// =============================================================================

    void
    AnimateIcon
    (
        HINSTANCE const hInstance,
        HWND      const hWnd, 
        DWORD     const dwMsgType,
        UINT      const p_icon_index
    )
    {
        LPCTSTR a = MAKEINTRESOURCE( IconResourceArray[p_icon_index] );
        
        HICON const h_icon = LoadIcon(hInstance, a);
        
        NOTIFYICONDATA IconData;
        
        // -----------------------------
        
        IconData.cbSize = sizeof(NOTIFYICONDATA);
        IconData.hIcon  = h_icon;
        IconData.hWnd   = hWnd;
        
        lstrcpyn(IconData.szTip,
                 "Animated Icons - Demo", 
                 strlen("Animated Icons - Demo") + 1);
        
        IconData.uCallbackMessage = HM_WM_1;
        
        IconData.uFlags = (NIF_MESSAGE | NIF_ICON | NIF_TIP);
        
        Shell_NotifyIcon(dwMsgType, &IconData);
        
        SendMessage(hWnd,
                    WM_SETICON,
                    (WPARAM) NULL,
                    (LPARAM) h_icon);
        
        if(h_icon)
        {
            DestroyIcon(h_icon);
        }
    }

// =============================================================================

#endif

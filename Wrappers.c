
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
    return ( (int)(short) LOWORD(p_ptr) );
}

// =============================================================================

signed int
HM_GetSignedHiword(LPARAM p_ptr)
{
    return ( (int)(short) HIWORD(p_ptr) );
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
HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp)
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

HM_MdiActivateData
HM_GetMdiActivateData(WPARAM const p_wp, LPARAM const p_lp)
{
    HM_MdiActivateData d;
    
    // -----------------------------
    
    d.m_deactivating = (HWND) p_wp;
    d.m_activating   = (HWND) p_lp;
    
    return d;
}

// =============================================================================

BOOL
HM_DrawRectangle(HDC  const p_device_context,
                 RECT const p_rect)
{
    return Rectangle(p_device_context,
                     p_rect.left,
                     p_rect.top,
                     p_rect.right,
                     p_rect.bottom);
}

// =============================================================================

SCROLLINFO
HM_GetVertScrollInfo(HWND const p_win)
{
    SCROLLINFO si = { 0 };
    
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

unsigned long
HM_NumPadKeyDownFilter(MSG const p_packed_msg)
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

void*
hm_memdup(void const * const p_arr,
          size_t             p_len)
{
    void * const new_arr = calloc(1, p_len);
    
    if(new_arr)
    {
        memcpy(new_arr, p_arr, p_len);
    }
    
    return new_arr;
}

// =============================================================================

char *
hm_strndup(char const * const p_str,
           size_t             p_len)
{
    char * out = (char*) calloc(p_len + 1, sizeof(char));
    
    size_t i = 0;
    
    for(i = 0; i < p_len; i += 1)
    {
        out[i] = p_str[i];
        
        if(p_str[i] == 0)
        {
            break;
        }
    }
    
    return out;
}

// =============================================================================



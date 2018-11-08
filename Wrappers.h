
#if ! defined HMAGIC_WRAPPERS_HEADER_GUARD

    #define HMAGIC_WRAPPERS_HEADER_GUARD

    #include "structs.h"

// =============================================================================

typedef
struct
{
    BOOL m_control_down;

    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseMoveData;

// =============================================================================

/// For use with mouse button down / up messages, click, hover messages, and 
typedef
struct
{
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;
    
    /// Is the ALT key down?
    BOOL m_alt_key;
    
    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseData;

// =============================================================================

typedef
struct
{
    signed int m_distance;
    
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;

    /// Is the ALT key down?
    BOOL m_alt_key;

    POINT m_screen_pos;

} HM_MouseWheelData;

// =============================================================================

typedef
struct
{
    HWND m_deactivating;
    HWND m_activating;
}
HM_MdiActivateData;

// =============================================================================

typedef
struct
{
    BOOL m_valid_file;
    
    HANDLE m_h;
    
    DWORD m_file_size;
    
    char * m_contents;
    
} HM_FileStat;

// =============================================================================

typedef
struct
{
    unsigned m_num_lines;
    
    char ** m_lines;
    
} HM_TextResource;

// =============================================================================

    RECT
    HM_GetWindowRect(HWND const p_win);

    RECT
    HM_GetClientRect(HWND const p_win);

    RECT
    HM_GetDlgItemRect(HWND     const p_dlg,
                      unsigned const p_item_id);
    
    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win);

    HM_MouseMoveData
    HM_GetMouseMoveData(HWND   const p_win,
                        WPARAM const wparam,
                        LPARAM const lparam);

    HM_MouseData
    HM_GetMouseData(MSG const p_packed_msg);

    HM_MouseWheelData
    HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp);

    HM_MdiActivateData
    HM_GetMdiActivateData(WPARAM const p_wp, LPARAM const p_lp);

    BOOL
    HM_DrawRectangle(HDC const p_device_context,
                     RECT const p_rect);

    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win);

    SCROLLINFO
    HM_GetHorizScrollInfo(HWND const p_win);

    BOOL
    HM_IsEmptyRect(RECT const p_rect);

    MSG
    HM_PackMessage(HWND const p_win,
                   UINT       p_msg_id,
                   WPARAM     p_wp,
                   LPARAM     p_lp);

    RGBQUAD
    HM_MakeRgb(uint8_t const p_red,
               uint8_t const p_green,
               uint8_t const p_blue);

    /// From "5 bits per channel"
    RGBQUAD
    HM_RgbFrom5bpc(uint16_t const p_color);

    /// COLORREF to "5 bits per channel" snes color.
    uint16_t
    HM_ColRefTo5bpc(COLORREF const p_cr);

    /// RGBQUAD structure to "5 bits per channel" snes color.
    uint16_t
    HM_ColQuadTo5bpc(RGBQUAD const p_cr);

    /// Converts RGBQUAD to COLORREF.
    COLORREF
    HM_ColQuadToRef(RGBQUAD const p_quad);

    BOOL
    HM_BinaryCheckDlgButton(HWND     const p_win,
                            unsigned const p_dlg_control,
                            BOOL     const p_is_checked);

    HM_FileStat
    HM_LoadFileContents(char const * const p_file_name);

    void
    HM_FreeFileStat(HM_FileStat * const p_s);

    WPARAM
    HM_NullWP(void);

    LPARAM
    HM_NullLP(void);

    unsigned long
    HM_NumPadKeyDownFilter(MSG const p_packed_msg);

    void*
    hm_memdup(void const * const p_arr,
              size_t             p_len);

    char *
    hm_strndup(char const * const p_str,
               size_t             p_len);

    BOOL
    HM_CheckEmbeddedStr(void const * const p_buffer,
                        char const * const p_string);

    int __stdcall
    askinteger(int max, char *caption, char *text);

#endif

#if ! defined HMAGIC_WRAPPERS_HEADER_GUARD

    #define HMAGIG_WRAPPERS_HEADER_GUARD

    #include "structs.h"

// =============================================================================

    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win);

    HM_MouseMoveData
    HM_GetMouseMoveData(HWND   const p_win,
                        WPARAM const wparam,
                        LPARAM const lparam);

    HM_MouseWheelData
    HM_GetMouseData(MSG const p_packed_msg);

    HM_MouseWheelData
    HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp);

    HM_MdiActivateData
    HM_GetMdiActivateData(WPARAM const p_wp, LPARAM const p_lp);

    BOOL
    HM_DrawRectangle(HDC const p_device_context,
                     RECT const p_rect);

    BOOL
    HM_IsEmptyRect(RECT const p_rect);

    MSG
    HM_PackMessage(HWND const p_win,
                   UINT       p_msg_id,
                   WPARAM     p_wp,
                   LPARAM     p_lp);

    int __stdcall askinteger(int max, char *caption, char *text);

#endif
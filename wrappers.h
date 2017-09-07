
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
    HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp);

    HM_MdiActivateData
    HM_GetMdiActivateData(WPARAM const p_wp, LPARAM const p_lp);

    BOOL
    HM_DrawRectangle(HDC const p_device_context,
                     RECT const p_rect);

    int __stdcall askinteger(int max, char *caption, char *text);

#endif
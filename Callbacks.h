
#if ! defined HMAGIC_CALLBACK_HEADER_GUARD

#define HMAGIC_CALLBACK_HEADER_GUARD

// =============================================================================

    #include "windows.h"

// =============================================================================

LRESULT CALLBACK
dpceditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

LRESULT CALLBACK
dungselproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

LRESULT CALLBACK
dungmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);


// =============================================================================

#endif
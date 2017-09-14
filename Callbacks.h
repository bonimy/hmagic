
#if ! defined HMAGIC_CALLBACK_HEADER_GUARD

#define HMAGIC_CALLBACK_HEADER_GUARD

// =============================================================================

    #include "windows.h"

// =============================================================================

// Dungeon-related dialog procedures.

BOOL CALLBACK
choosesprite(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

BOOL CALLBACK
choosedung(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

// Dungeon-related window procedures.

LRESULT CALLBACK
dpceditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

LRESULT CALLBACK
dungselproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

LRESULT CALLBACK
DungeonMapProc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);

// Super Dialog procedure
BOOL CALLBACK
dungdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam);



// =============================================================================

#endif
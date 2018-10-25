
#if ! defined HMAGIC_CALLBACK_HEADER_GUARD

#define HMAGIC_CALLBACK_HEADER_GUARD

// =============================================================================

    #include "windows.h"

// =============================================================================

#define HM_DeclareWndProc(name) \
LRESULT CALLBACK name(HWND p_win, UINT p_msg, WPARAM p_wp, LPARAM p_lp)

// =============================================================================

#define HM_DeclareDlgProc(name) \
BOOL CALLBACK name(HWND p_win, UINT p_msg, WPARAM p_wp, LPARAM p_lp)

// =============================================================================

#define HM_DeclareAudioProc(name) \
void CALLBACK name(UINT p_timer_id, UINT p_msg, DWORD p_inst, \
                   DWORD p_p1, DWORD p_p2)

// =============================================================================

#define HM_DeclareTimerProc(name) \
void CALLBACK name(HWND p_win, UINT p_msg, UINT p_timer_id, DWORD p_sys_time)

// =============================================================================

    HM_DeclareWndProc(frameproc);

    // Root document window procedure.
    HM_DeclareWndProc(superdlgproc);

    // Root document window procedure.
    HM_DeclareWndProc(docproc);

// =============================================================================

    // ... Super Dialog with the main tree control for the whole game.
    HM_DeclareDlgProc(z3dlgproc);

// =============================================================================

    // Dungeon-related procedures.
    
    // Dialogs
    HM_DeclareDlgProc(choosedung);
    HM_DeclareDlgProc(editroomprop);

    // Windows
    HM_DeclareWndProc(dpceditproc);
    HM_DeclareWndProc(dungselproc);
    HM_DeclareWndProc(DungeonMapProc);

    // Super Dialog procedure.
    HM_DeclareDlgProc(dungdlgproc);

    // Child frame window procedure.
    HM_DeclareWndProc(dungproc);

// =============================================================================

    // Overworld-related window procedures

    // Dialogs
    HM_DeclareDlgProc(editexit);
    HM_DeclareDlgProc(editwhirl);
    HM_DeclareDlgProc(editovprop);
    HM_DeclareDlgProc(findblks);

    // Windows
    HM_DeclareWndProc(blksel8proc);
    HM_DeclareWndProc(blkedit8proc);

    HM_DeclareWndProc(blk16search);
    HM_DeclareWndProc(blksel16proc);
    HM_DeclareWndProc(blkedit16proc);

    HM_DeclareWndProc(blksel32proc);
    HM_DeclareWndProc(blkedit32proc);

    HM_DeclareWndProc(overmapproc);

    // Super Dialog procedure.
    HM_DeclareDlgProc(overdlgproc);

    // Child frame window procedure.
    HM_DeclareWndProc(overproc);

// =============================================================================

    // Super Dialog procedure.
    HM_DeclareDlgProc(textdlgproc);

    // Text-related window procedures.
    HM_DeclareWndProc(texteditproc);

// =============================================================================

    // Audio-related window procedures.
    HM_DeclareWndProc(sampdispproc);

    HM_DeclareDlgProc(sampdlgproc);

    HM_DeclareWndProc(trackeditproc);
    HM_DeclareWndProc(trackerproc);

    HM_DeclareWndProc(musbankproc);

// =============================================================================

    // Palette-related procedures.
    HM_DeclareWndProc(palselproc);

    // Child frame window procedure.
    HM_DeclareWndProc(palproc);

// =============================================================================

    // World Map-related procedures.

    // Windows
    HM_DeclareWndProc(wmapdispproc);

    // Super Dialog Procedure.
    HM_DeclareDlgProc(wmapdlgproc);

    // World Map-related window procedures.
    HM_DeclareWndProc(wmapproc);

// =============================================================================

    // Level (aka Palace) Map-related window procedures.
    HM_DeclareWndProc(lmapdispproc);
    HM_DeclareWndProc(lmapblksproc);

    HM_DeclareWndProc(lmapproc);

// =============================================================================

    // Tilemap-related procedures.
    // Tilemap is in Hyrule Magic's nomenclature is a catchall for
    // title screen and other special screens like the player select / copy
    // screens. Tilemap is a much more general term
    
    HM_DeclareWndProc(tmapdispproc);
    
    HM_DeclareWndProc(tmapproc);

// =============================================================================

    // 3D object-related procedures
    HM_DeclareWndProc(perspdispproc);

    HM_DeclareWndProc(perspproc);

// =============================================================================

    // ASM-related procedures.
    HM_DeclareWndProc(patchproc);

// =============================================================================

    // Window procedures for dialogs that aren't custom classes.

    HM_DeclareDlgProc(choosesprite);

    HM_DeclareDlgProc(editsprname);

    HM_DeclareDlgProc(editbosslocs);

    HM_DeclareDlgProc(editvarious);

    HM_DeclareDlgProc(aboutfunc);
    
    HM_DeclareDlgProc(seldevproc);

    HM_DeclareDlgProc(duproom);
   
    HM_DeclareDlgProc(rompropdlg);

// =============================================================================

    HM_DeclareAudioProc(midifunc);

    HM_DeclareTimerProc(midifunc2);

// =============================================================================

#endif
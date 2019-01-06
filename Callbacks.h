
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

    /**
        Window procedure for the application's main frame window. (In fact
        in typical MDI contexts, these are called "MainFrame" windows.)
    */
    HM_DeclareWndProc(MDI_FrameWnd);

// =============================================================================

    // Root document window procedure.
    HM_DeclareWndProc(SuperDlg);

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
    HM_DeclareDlgProc(OverworldDlg);

    // Child frame window procedure.
    HM_DeclareWndProc(overproc);

// =============================================================================

    // Super Dialog procedure.
    HM_DeclareDlgProc(TextDlg);

    // Text-related window procedures.
    HM_DeclareWndProc(TextFrameProc);

// =============================================================================

    // Audio-related window procedures.
    HM_DeclareWndProc(SampleDisplayProc);

    HM_DeclareDlgProc(sampdlgproc);
    
    // Tracker Dialog procedure.
    HM_DeclareDlgProc(trackdlgproc);

    HM_DeclareWndProc(trackeditproc);
    HM_DeclareWndProc(trackerproc);

    HM_DeclareDlgProc(musdlgproc);

    HM_DeclareWndProc(AudioFrameProc);

// =============================================================================

    // Palette-related procedures.
    
    // Palette Selector for viewing CHR tilesets.
    HM_DeclareWndProc(PaletteSelector);

    // Child frame window procedure.
    HM_DeclareWndProc(PaletteFrameProc);

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

    HM_DeclareDlgProc(lmapdlgproc);
    
    HM_DeclareWndProc(PalaceMapFrame);

// =============================================================================

    // Tilemap-related procedures.
    // Tilemap is in Hyrule Magic's nomenclature is a catchall for
    // title screen and other special screens like the player select / copy
    // screens. Tilemap is a much more general term
    
    HM_DeclareWndProc(tmapdispproc);
    
    HM_DeclareDlgProc(tmapdlgproc);

    HM_DeclareWndProc(tmapproc);

// =============================================================================

    // 3D object-related procedures
    HM_DeclareWndProc(perspdispproc);

    HM_DeclareDlgProc(perspdlgproc);

    HM_DeclareWndProc(PerspectiveFrameProc);

// =============================================================================

    // CHR (graphics) related procedures
    HM_DeclareDlgProc(blockdlgproc);

// =============================================================================

    // ASM-related procedures.

    HM_DeclareDlgProc(PatchDlg);

    HM_DeclareWndProc(PatchFrameProc);

// =============================================================================

    // Window procedures for dialogs that aren't custom classes.

    HM_DeclareDlgProc(choosesprite);

    HM_DeclareDlgProc(editsprname);

    HM_DeclareDlgProc(editbosslocs);

    HM_DeclareDlgProc(editvarious);

    HM_DeclareDlgProc(AboutDlg);
    
    HM_DeclareDlgProc(seldevproc);

    HM_DeclareDlgProc(duproom);
   
    HM_DeclareDlgProc(rompropdlg);

    HM_DeclareDlgProc(errorsproc);

// =============================================================================

    HM_DeclareAudioProc(midifunc);

    HM_DeclareTimerProc(midifunc2);

// =============================================================================

#endif
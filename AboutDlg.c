
    #include "structs.h"

    #include "Wrappers.h"

    #include "tchar.h"

    // \task[med] Just test variables, get rid of eventually.
    int hover_count = 0;
    BOOL hover_latch = FALSE;
    HWND tip_win = NULL;

// =============================================================================

    enum
    {
        foo = TTTOOLINFOA_V1_SIZE,
        foo2 = sizeof(TOOLINFO)
    };

    static HWND
    CreateToolTip
    (
        int       toolID,
        HWND      hDlg,
        PTSTR     pszText,
        CP2(BOOL) p_success
    )
    {
        BOOL r = FALSE;
        
        // Get the window of the tool.
        HWND const hwndTool = GetDlgItem(hDlg, toolID);
        
        TOOLINFO tool_info = { 0 };
        
        // -----------------------------
        
        // Create the tooltip. g_hInst is the global instance handle.
        tip_win = CreateWindowEx
        (
            WS_EX_TOPMOST,
            TOOLTIPS_CLASS,
            NULL,
            WS_POPUP | TTS_ALWAYSTIP /* | TTS_BALLOON */ ,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            hDlg, NULL,
            hinstance,
            NULL
        );
        
        if(!toolID || !hDlg || !pszText)
        {
            return FALSE;
        }
        
        if( IsNull(hwndTool) || IsNull(tip_win) )
        {
           return (HWND)NULL;
        }
        
        SetWindowPos
        (
            tip_win,
            HWND_TOPMOST,
            0, 0,
            0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
        );
        
        // Associate the tooltip with the tool.
        tool_info.cbSize = sizeof(tool_info);
        tool_info.hwnd = hDlg;
        tool_info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        tool_info.uId = (UINT_PTR) hwndTool;
        tool_info.lpszText = pszText;
        
        r = SendMessage(tip_win, TTM_ADDTOOL, 0, (LPARAM) &tool_info);
        
        if(p_success)
        {
            p_success[0] = r;
        }
        
        SendMessage(tip_win, TTM_ACTIVATE, TRUE, 0);
        
        return tip_win;
    }

// =============================================================================

    static HWND
    AboutDlg_SetUpTooltip
    (
        HWND const p_win
    )
    {
        BOOL added = FALSE;
        
        return CreateToolTip
        (
            IDC_TEST_TOOLTIP,
            p_win,
            "A Test Tooltip",
            &added
        );
    }

// =============================================================================

    static BOOL
    AboutDlg_SetUpHoverTracking
    (
        HWND const p_win
    )
    {
        BOOL b = FALSE;
        
        TRACKMOUSEEVENT event_track = { 0 };
        
        // -----------------------------
        
        event_track.cbSize = sizeof(TRACKMOUSEEVENT);
        
        event_track.dwFlags = TME_HOVER | TME_LEAVE;
        
        event_track.hwndTrack = p_win;
        
        // 3 seconds
        event_track.dwHoverTime = 3000;
        
        b = TrackMouseEvent(&event_track);
        
        return b;
    }

// =============================================================================

    static BOOL
    AboutDlg_CancelHoverTracking
    (
        HWND const p_win
    )
    {
        BOOL b = FALSE;
        
        TRACKMOUSEEVENT event_track = { 0 };
        
        // -----------------------------
        
        event_track.cbSize = sizeof(TRACKMOUSEEVENT);
        
        event_track.dwFlags = TME_HOVER | TME_CANCEL | TME_LEAVE;
        
        event_track.hwndTrack = p_win;
        
        event_track.dwHoverTime = HOVER_DEFAULT;
        
        b = TrackMouseEvent(&event_track);
        
        return b;
    }

// =============================================================================

    extern BOOL CALLBACK
    AboutDlg
    (
        HWND   p_win,
        UINT   msg,
        WPARAM wparam,
        LPARAM lparam)
    {
        (void) lparam;
        
        if( Is(msg, WM_INITDIALOG) )
        {
            AboutDlg_SetUpHoverTracking(p_win);
            
            AboutDlg_SetUpTooltip(p_win);
            
            return TRUE;
        }
        
        switch(msg)
        {

        case WM_COMMAND:
            
            if( Is(wparam, IDCANCEL) )
            {
                EndDialog(p_win, 0);
            }
            
            break;
        
        case WM_MOUSEMOVE:
            
            if( IsFalse(hover_latch) ) 
            {
                char * hover_count_str = NULL;
                
                // -----------------------------
                
                asprintf(&hover_count_str, "%d", hover_count);
                
                SetDlgItemText(debug_window, IDC_STATIC2, hover_count_str);
                
                free(hover_count_str);
                
                AboutDlg_SetUpHoverTracking(p_win);
                
                hover_latch = TRUE;
            }
            
            break;
        
        case WM_MOUSELEAVE:
            
            AboutDlg_CancelHoverTracking(p_win);
            
            hover_latch = FALSE;
            
            break;
        
        case WM_LBUTTONDOWN:
        case WM_MOUSEHOVER:
            
#if 0
            HM_OK_MsgBox
            (
                p_win,
                _T("Stop hovering over me!"),
                _T("okay?")
            );
#endif
            SendMessage(tip_win, TTM_ACTIVATE, TRUE, 0);
            
            
            hover_count += 1;
            
            hover_latch = FALSE;
            
            break;
        
        default:
            
            // return DefWindowProc(p_win, msg, wparam, lparam);
            
            break;
        }
        
        return FALSE;
    }

// =============================================================================
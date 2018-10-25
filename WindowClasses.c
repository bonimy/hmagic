
#include "structs.h"

#include "GdiObjects.h"
#include "Callbacks.h"
#include "resource.h"

// =============================================================================

static WNDPROC DefaultEditProc;

// =============================================================================

HM_DeclareWndProc(NumEditProc)
{
    switch(p_msg)
    {
    
    default:
        
        break;
        
    case WM_CHAR:
        
        if(always)
        {
            enum { backspace_char = '\x08' };
            
            unsigned short c = (unsigned short) p_wp;
            
            // Only pass numeric and backspace characters on to the
            // standard edit window proc.
            if('0' <= c && c <= '9')
            {
                break;
            }
            else if(backspace_char == c)
            {
                break;
            }
            
            return 0;
        }
    }
    
    return DefaultEditProc(p_win, p_msg, p_wp, p_lp);
}

// =============================================================================

void
HM_RegisterClasses(HINSTANCE p_inst)
{
    WNDCLASS wc;
    
    wc.style=0;
    wc.lpfnWndProc=frameproc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=p_inst;
    wc.hIcon=LoadIcon(p_inst, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = normal_cursor = LoadCursor(0,IDC_ARROW);
    wc.hbrBackground=0;
    wc.lpszMenuName=0;
    wc.lpszClassName="ZEFRAME";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=docproc;
    wc.lpszClassName="ZEDOC";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=overproc;
    wc.lpszClassName="ZEOVER";
    RegisterClass(&wc);
    
    // dungeon dialogue editing.
    wc.lpfnWndProc=dungproc;
    wc.lpszClassName="ZEDUNGEON";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=musbankproc;
    wc.lpszClassName="MUSBANK";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=trackeditproc;
    wc.lpszClassName="TRACKEDIT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=perspproc;
    wc.lpszClassName="PERSPEDIT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=patchproc;
    wc.lpszClassName="PATCHLOAD";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=texteditproc;
    wc.lpszClassName="ZTXTEDIT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=wmapproc;
    wc.lpszClassName="WORLDMAP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=lmapproc;
    wc.lpszClassName="LEVELMAP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=tmapproc;
    wc.lpszClassName="TILEMAP";
    RegisterClass(&wc);
    
    wc.style=CS_DBLCLKS;
    wc.lpfnWndProc=blksel16proc;
    wc.lpszClassName="BLKSEL16";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=blkedit32proc;
    wc.lpszClassName="BLKEDIT32";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=blksel8proc;
    wc.lpszClassName="BLKSEL8";
    RegisterClass(&wc);
    
    wc.style=0;
    wc.lpfnWndProc=blkedit16proc;
    wc.lpszClassName="BLKEDIT16";
    RegisterClass(&wc);
    
    wc.hbrBackground=(HBRUSH)(COLOR_APPWORKSPACE+1);
    wc.lpfnWndProc=blkedit8proc;
    wc.lpszClassName="BLKEDIT8";
    RegisterClass(&wc);
    
    wc.style=CS_DBLCLKS;
    wc.lpfnWndProc=overmapproc;
    wc.lpszClassName="OVERWORLD";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=blksel32proc;
    wc.lpszClassName="BLKSEL32";
    RegisterClass(&wc);
    
    wc.hCursor=0;
    wc.lpfnWndProc=DungeonMapProc;
    wc.lpszClassName="DUNGEON";
    RegisterClass(&wc);
    
    wc.hCursor = normal_cursor;
    wc.lpfnWndProc=wmapdispproc;
    wc.lpszClassName="WMAPDISP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=dungselproc;
    wc.lpszClassName="DUNGSEL";
    RegisterClass(&wc);
    
    wc.style=0;
    wc.lpfnWndProc=tmapdispproc;
    wc.lpszClassName="TMAPDISP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=blk16search;
    wc.lpszClassName="SEARCH16";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=palselproc;
    wc.lpszClassName="PALSELECT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=lmapdispproc;
    wc.lpszClassName="LMAPDISP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=dpceditproc;
    wc.lpszClassName="DPCEDIT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=palproc;
    wc.lpszClassName="PALEDIT";
    RegisterClass(&wc);
    
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=lmapblksproc;
    wc.lpszClassName="LMAPBLKS";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=trackerproc;
    wc.lpszClassName="TRAX0R";
    wc.style=0;
    wc.hbrBackground=white_brush;
    RegisterClass(&wc);
    
    wc.hbrBackground=black_brush;
    wc.lpfnWndProc=sampdispproc;
    wc.lpszClassName="SAMPEDIT";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=perspdispproc;
    wc.lpszClassName="PERSPDISP";
    RegisterClass(&wc);
    
    wc.lpfnWndProc=superdlgproc;
    wc.lpszClassName="SUPERDLG";
    wc.hbrBackground=(HBRUSH)(wver?COLOR_WINDOW+1:COLOR_BTNFACE+1);
    wc.cbWndExtra=12;
    RegisterClass(&wc);
    
    // Subclass the standard edit control with a different window procedure
    // that only accepts numerical characters.
    if( GetClassInfo(p_inst, "Edit", &wc) )
    {
        DefaultEditProc = wc.lpfnWndProc;
        
        wc.lpszClassName = "NumEdit";
        wc.lpfnWndProc = NumEditProc;
        
        RegisterClass(&wc);
    }
}

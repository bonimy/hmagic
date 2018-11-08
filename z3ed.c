// Hyrule magic. (c) 2001,2002 Sephiroth3

// personal reference
// bookmarks:
// 1 - definition of the dungeon dialog structure
// 
// Abbreviations: lmap = level map i.e. the dungeon maps you see when you hit X.
// 

// \task A note to merge the changes in the standalone project file
// for Epoch 2 into the repo.

/**
    
    \task Would like to have the text of markers on the overworld map not be clipped
    by the background region (the grey area outside of which we paint)

    \task Remaining things Puzzledude claims are borked:

    -overworld items are bugged if you edit them and save, same for whirlpools
    -pretty much all options: empty all... (dungeons rooms, exits, entrances)
    simply repoint all pointers to an empty room rather than actually make
    empty space

    -choosing "remove all overworld exits" also removes special white dots which
    tells the game what screen to load in the ending sequence

    -choosing "remove all overworld exits" also removes the "entrance" from Ganon
    room to the Triforce shrine

    -global grid editing in the overworld makes crashes or forcefully puts in
    all entrances (yellow dots) for some reason
    (despite the fact the entrances are not inserted yet)

    -false dungeon room pointer calculation if too much data, thus can not
    handle space problems if more data (main problem of the program)
*/


/** 
    \task Epoch3 change list
    
    - Fixed infamous monologue storage problem that could randomly corrupt the
    contents of the following bank and cause strange problems later in the game.
    
    - The "Exit" button is now hidden when editing dungeon overlays and layouts.
    This was probably an oversight given that the button seems to have been added
    after the logic for hiding certain controls was developed.

    - The allocation of the editor structure for Link's graphics was
    too small, using the size of the EDITCOMMON structure. It has changed
    so that it allocates a block the size of OVEREDIT, because it was
    causing buffer overflows when the program tried to access members beyond
    the size of an EDITCOMMON.

    - Improved error handling in writing and reading of
    *.cfg files (checking for incorrect sizes or I/O errors)

    - Fixed bug in Dungeon item handling (red markers) that made it very
    difficult to select one on BG2 by clicking. Furthermore, even if one
    managed to select one by some other means, such as left and right arrow
    keys, the data would be mishandled and revert the BG setting of the item
    to BG1. The object details static text control paid attention to BG1 / BG2
    settings but the code involved in moving them did not.
 
*/

// For standard C file i/o
#include <stdio.h>

#include "structs.h"
#include "prototypes.h"

#include "Callbacks.h"
#include "Wrappers.h"

#include "HMagicEnum.h"
#include "HMagicLogic.h"
#include "HMagicUtility.h"

#include "OverworldEnum.h"
#include "OverworldEdit.h"

#include "MetatileLogic.h"

#include "DungeonEnum.h"
#include "DungeonLogic.h"

#include "PaletteEdit.h"

// For text enumerations and Load / Save text functions.
#include "TextEnum.h"
#include "TextLogic.h"

#include "AudioLogic.h"
#include "SampleEnum.h"

#include "TrackerLogic.h"

// \task Probably will move this once the worldmap super dialog procedure gets
// its own file.
#include "WorldMapLogic.h"

    #include "TileMapLogic.h"

    #include "PerspectiveLogic.h"

    #include "GraphicsLogic.h"
    
// =============================================================================

offsets_ty offsets =
{
    {
        0x2736a,
        0x88c1
    },
    {
        0x33333
    }
};

// =============================================================================

short dm_x;
short dm_k;

uint8_t const u8_neg1 = (uint8_t) (~0);

uint16_t const u16_neg1 = (uint16_t) (~0);

uint32_t const u32_neg1 = (uint32_t) (~0);

int const always = 1;

int mouse_x;
int mouse_y;

// =============================================================================

// windows version number
int wver;

int palmode=0;

int door_ofs = 0;
int issplit = 0;

char namebuf[MAX_PATH] = "";

char buffer[0x400];

unsigned char drawbuf[0x400];

uint16_t *dm_rd  = 0;
uint16_t *dm_wr  = 0;
uint16_t *dm_buf = 0;
uint16_t *dm_tmp = 0;

unsigned char dm_l, dm_dl;

// "current secret"?
// Doesn't actually appear to be used, except to calculate the size of the
// boxes for droppable items (the red dots).
char const * cur_sec;

unsigned char masktab[16] = {255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int palhalf[8] = {1,0,0,1,1,1,0,0};

void *firstgraph, *lastgraph;

const static int palt_sz[] = {96,256,128,48};
const static int palt_st[] = {32,0,0,208};

const short bg3blkofs[4] = {221,222,220,0};

RECT const empty_rect = { 0, 0, 0, 0 };

HPEN green_pen,
     null_pen,
     white_pen,
     blue_pen,
     black_pen,
     gray_pen = 0,
     tint_pen = 0;

HBRUSH black_brush,
       white_brush,
       green_brush,
       yellow_brush,
       purple_brush,
       red_brush,
       blue_brush,
       gray_brush,
       tint_br = 0;

HGDIOBJ trk_font;

HCURSOR normal_cursor,
        forbid_cursor,
        wait_cursor;

HCURSOR sizecsor[5];

HBITMAP arrows_imgs[4];

HANDLE shade_brush[8];

// The handle to the program
HINSTANCE hinstance;

HWND framewnd, clientwnd;

RGBQUAD darkcolor={80,136,144,0};

RGBQUAD const blackcolor={0,0,0,0};

BITMAPINFOHEADER zbmih={sizeof(BITMAPINFOHEADER),32,32,1,8,BI_RGB,0,0,0,256,0};

SDCREATE *firstdlg = 0, *lastdlg = 0;

FDOC *mark_doc = 0;
FDOC *firstdoc = 0, *lastdoc = 0, *activedoc = 0;

DUNGEDIT * dispwnd = 0;

DPCEDIT * dpce = 0;

OVEREDIT * oved = 0;

SSBLOCK **ssblt = 0;

char sprname[0x11c][16];

HM_TextResource area_names;

HDC objdc;

HBITMAP objbmp;

// =============================================================================

INT_PTR CALLBACK
status_proc(HWND   p_win,
            UINT   p_msg,
            WPARAM p_wp,
            LPARAM p_lp)
{
    // \task Perhaps make this a macro for Window procedures?
    (void) p_win, p_msg, p_wp, p_lp;
    
    switch(p_msg)
    {
         
    default:
        
        break;
    
    case WM_INITDIALOG:
        
        if(always)
        {
            if(p_lp)
            {
                SetWindowPos(p_win, HWND_TOPMOST,
                             0, 0, 0, 0,
                             SWP_NOSIZE | SWP_NOMOVE);
                
                SetDlgItemText(p_win, IDC_STATIC5, "MURP");
            }
            
        }
        
        // Set keyboard focus
        return TRUE;
    
    case WM_ACTIVATE:
        
        return FALSE;
    }
    
#if 0
    return DefWindowProc(p_win, p_msg, p_wp, p_lp);
#else
    return 0;
#endif
    
    
}

// =============================================================================

typedef void (*ParseAction)(HM_TextResource  * const p_tr,
                            size_t             const p_line,
                            char       const * const p_text,
                            size_t             const p_len);

// =============================================================================

void
CountLine(HM_TextResource  * const p_tr,
          size_t             const p_line,
          char       const * const p_text,
          size_t             const p_len)
{
    (void) p_text, p_line, p_len;
    
    p_tr->m_num_lines += 1;
}

// =============================================================================

void
CopyLine(HM_TextResource  * const p_tr,
         size_t             const p_line,
         char       const * const p_text,
         size_t             const p_len)
{
    p_tr->m_lines[p_line] = hm_strndup(p_text, p_len);
}

// =============================================================================

void
ParseTextData(char const      * const p_text,
              size_t            const p_text_len,
              HM_TextResource * const p_tr,
              ParseAction             p_action)
{
    size_t i         = 0;
    size_t num_lines = 0;
    
    for(i = 0 ; i < p_text_len; num_lines += 1)
    {
        //
        size_t j = strcspn(p_text + i, "\r\n");
        
        size_t const j_orig = j;
        
        // Reached the end of the data?
        if( (i + j) == p_text_len )
        {
            p_action(p_tr, num_lines, p_text + i, 0);
            
            break;
        }
        
        if( p_text[i + j] == '\r')
        {
            j += 1;
            
            if( (i + j) == p_text_len )
            {
                break;
            }
        }
        
        if( p_text[i + j] == '\n')
        {
            j += 1;
        }
        
        p_action(p_tr, num_lines, p_text + i, j_orig);
        
        i += j;
    }
}

// =============================================================================

HM_TextResource
LoadTextFileResource(unsigned p_res_id)
{
    HM_TextResource data = { 0 };
    
    HRSRC const res = FindResource(hinstance,
                                   MAKEINTRESOURCE(p_res_id),
                                   RT_RCDATA);
    
    // -----------------------------
    
    if(res)
    {
        DWORD res_size = SizeofResource(hinstance, res);
        
        HGLOBAL const res_handle = LoadResource(hinstance,
                                                res);
        
        // -----------------------------
        
        if(res_handle && res_size)
        {
            char const * const
            text_res = (char const *) LockResource(res_handle);

            ParseTextData(text_res,
                          res_size,
                          &data,
                          CountLine);
            
            data.m_lines = (char **)
            calloc(data.m_num_lines,
                   sizeof(char*));
            
            ParseTextData(text_res,
                          res_size,
                          &data,
                          CopyLine);

            
            UnlockResource(res_handle);
            
            FreeResource(res_handle);
        }
    }
    
    return data;
}

// =============================================================================

HWND debug_window = 0;
HWND debug_box = 0;

HWND
CreateNotificationWindow(HWND const p_parent)
{
    HWND const win = CreateDialogParam
    (
        hinstance,
        MAKEINTRESOURCE(IDD_DEBUG_DLG),
        p_parent,
        status_proc,
        (LPARAM) 5
    );
    
    if(win)
    {
        unsigned const screen_width = GetSystemMetrics(SM_CXSCREEN);
        
        RECT const r = HM_GetWindowRect(win);
        
        SetWindowPos(win,
                     HWND_NOTOPMOST,
                     screen_width - (r.right - r.left),
                     0,
                     0, 0, SWP_NOSIZE);
        
        ShowWindow(win, SW_SHOW);
        
        SetActiveWindow(p_parent);
    }
    
    return win;
}

// =============================================================================

LRESULT CALLBACK
HM_NotifyBoxProc(HWND p_win, UINT p_msg, WPARAM p_wp, LPARAM p_lp)
{
    switch(p_msg)
    {
    
    default:

        break;
        
    case WM_PAINT:
        
        {
            RECT rc = HM_GetClientRect(p_win);
            
            PAINTSTRUCT ps;
            
            HDC const dc = BeginPaint(p_win, &ps);
            
            FrameRect(dc, &rc, blue_brush);
            
            EndPaint(p_win, &ps);
            
            return 0;
        }
    
    case WM_MOUSEACTIVATE:
        
        return MA_NOACTIVATE;
    }
    
    return DefWindowProc(p_win, p_msg, p_wp, p_lp);
}

// =============================================================================

HWND
CreateNotificationBox(HWND const p_parent)
{
    WNDCLASSEX wc;
    
    RECT const parent_rect = HM_GetWindowRect(p_parent);
    
    {
        wc.cbSize = sizeof(WNDCLASSEX);
        
        wc.style = 0;
        
        wc.lpfnWndProc = HM_NotifyBoxProc;
        
        wc.cbClsExtra = 0;
    
        wc.cbWndExtra = 0;
        
        wc.hInstance = hinstance;
        
        wc.hIcon = NULL;
        
        wc.hCursor = normal_cursor;
        
        wc.hbrBackground = (HBRUSH) (COLOR_HIGHLIGHTTEXT);
        
        wc.lpszMenuName = NULL;
        
        wc.lpszClassName = "HMAGIC_NOTIFY_BOX";
        
        wc.hIconSm = NULL;
    }
    
    if( RegisterClassEx(&wc) )
    {
        HWND const w = CreateWindowEx
        (
            WS_EX_LAYERED | WS_EX_TRANSPARENT,
            "HMAGIC_NOTIFY_BOX",
            NULL,
            WS_POPUP,
            parent_rect.right - 220,
            parent_rect.bottom - 60,
            200,
            40,
            p_parent,
            NULL,
            hinstance,
            NULL
        );
        
        if(w)
        {
            SetLayeredWindowAttributes(w,
                                       0,
                                       0x20,
                                       LWA_ALPHA);
            
            ShowWindow(w, SW_SHOW);
            
            SetActiveWindow(p_parent);
        }
        
        return w;
    }
    else
    {
        char text_buf[0x100];
        
        DWORD i = GetLastError();
        
        sprintf(text_buf, "Error code: %u", i);
        
        MessageBox(p_parent, text_buf, NULL, MB_OK);
    }
    
    return NULL;
}

// =============================================================================

// \task Dummied out except for staging1 for now.

#if 0

//Port and related functions******************************************

//CopyHeader#**************************************************

int CopyHeader( FDOC *src, FDOC *port, int a, int b )
{
    // we are passed in a dungeon editing window, and a map number (room number)

    int i1, // lower limit for the first header offset.
        i2, // lower limit for the second header offset.
        j, // counter variable for looping through all dungeon rooms.
        k1, //size of the first header
        k2, // size of the second header
        m; // upper limit for the header offset.
    
    unsigned char *src_rom = src->rom;
    unsigned char *port_rom = port->rom;
    
    int const src_headerAddrLocal = src->headerLocation;
    int const port_headerAddrLocal = port->headerLocation;
    
    char src_hbuf[14];
    
    // copy the buffer and get its size
    i1 = ( (short*) ( src_rom + src_headerAddrLocal) )[a];
    
    // by default the size is 14
    k1 = 14;
    
    // sort through all the other header offsets
    for(j = 0; j < 0x140; j++)
    {
    
        // m gives the upper limit for the header.
        // if is less than 14 bytes from i.
        m = ( (short*) (src_rom + src_headerAddrLocal))[j];
    
        if( (m > i1) && (m < i1 + 14) )
        {
            // determine the size of the header
            k1 = m - i1; 
            
            break;
        }
    }
    
    // copy 14 bytes from the i offset.
    memcpy(src_hbuf, src_rom + 0x28000 + i1, 14);
    
    
    //--------------------------------------------
    // begin work on the second rom
    i2 = ( (short*) ( port_rom + port_headerAddrLocal) )[b];
    
    // by default the size is 14
    k2 = 14;
    
    // sort through all the other header offsets
    for(j = 0; j < 0x140; j++)
    {
        
        // m gives the upper limit for the header.
        // if is less than 14 bytes from i.
        m = ( (short*) (port_rom + port_headerAddrLocal))[j];
        
        if( (m > i2) && (m < i2 + 14) )
        {
            // determine the size of the header
            k2 = m - i2;
            
            break;
        }
    }

    if(k1 > k2)
        memcpy(port_rom + 0x28000 + i2, src_hbuf, k2);
    else
        memcpy(port_rom + 0x28000 + i2, src_hbuf, k1);

    return 1;
}

//CopyHeader***************************************************

//CopyTiles********************************

void CopyTiles(FDOC *src, FDOC *port, int a, int b)
{

    int i, // will be used to generate each buffer size.
        j,
        l;

    unsigned char *buf1,
                  *buf2,
                  *selector;

    unsigned char *src_rom = src->rom,
                  *port_rom = port->rom;
    
    unsigned short k; // used as a temporary value holder of values from the buffers.
    
    //designed to mimick a DUNGEDIT struct.
    short chkofs[6];
    int selobj, selchk;

    short src_length = 0;
    short port_length = 0;
        
    // get the base address for this room.
    buf1 = src_rom + romaddr( *(int*) (src_rom + 0xf8000 + a * 3));
    buf2 = port_rom + romaddr( *(int*) (port_rom + 0xf8000 + b * 3));

    if(buf1 == buf2)
    {
        MessageBox(framewnd,"Source and target locations are the same","Bad error happened",MB_OK); 
        goto sameobj; // this is the same room, no need to copy the same room in the same rom, eh?
    }

    selector = buf1;

loopstart:

    selobj = 0;
    selchk = 0;

    for(j = 0, i = 2; j < 6; j++)
    {

        chkofs[j] = i;
        
        for(;;) 
        {
            k = *(unsigned short*)(selector + i);
            
            if(k == 0xffff) // code indicating to stop.
                break;
            
            if(k == 0xfff0) // code indicating to do this loop...
            {
                j++;
                
                chkofs[j] = i + 2;
                
                for(;;) 
                {
                    i += 2;
                    
                    k = *(unsigned short*)(selector + i);
                    
                    if(k == 0xffff) 
                        goto end;
                    
                    if(!selobj)
                        selobj = i,
                        selchk = j;

                }
            } 
            else 
                i += 3;
            
            if(!selobj) // if there is no selected object, pick one.
            {
                selobj = i-3,
                selchk = j;
            }       
            
        }
        
        j++;
        
        chkofs[j] = i+2;
end:
        i += 2;
    }
    
    // now we know where the room data separates into special subsections.
    
    // AND by 0xFFFFFFFE. Makes j into an even number.
    j = selchk & -2; 
        
    if(selector == buf1)
    {
        src_length = i; // size of the buffer.
        selector = buf2;

        goto loopstart;
    }
    else
    {
        port_length = i;
    }

    if(src_length <= port_length)
    {
        memcpy(buf2, buf1, src_length);

        
    }
    else
    {
        // do error message
        MessageBox(framewnd,"source room too large","Bad error happened",MB_OK);

    }

    //--------------------------------

sameobj:

    //begin the second transfer
    buf1 = src_rom + 0x50000 + ((short*) (src_rom + src->dungspr))[a];
    buf2 = port_rom + 0x50000 + ((short*) (port_rom + port->dungspr))[b];
    
    if(buf1 == buf2)
    {
        MessageBox(framewnd,"Source and target locations are the same","Bad error happened",MB_OK); 
        goto samesprites;
    }

    selector = buf1;

loopstart2:

    // go up by three until we hit an 0xFF
    for(i = 1; ;i += 3)
    {
        if(selector[i] == 0xff)
            break;
    }
        
    if(selector == buf1)
    {   
        src_length = i;
        selector = buf2;
                
        goto loopstart2;
    }
    else
        port_length = i;

    if(src_length <= port_length)
    {
        memcpy(buf2, buf1, src_length);
    }
    else
    {
        MessageBox(framewnd,"lack of room for sprites","Bad error happened",MB_OK); 
        
    }

    //-------------------------------

samesprites:

    //begin the third transfer
    buf1 = src_rom + 0x10000 + ((short*) (src_rom + 0xdb69))[a];
    buf2 = port_rom + 0x10000 + ((short*) (port_rom + 0xdb69))[b];

    if(buf1 == buf2)
    {
        MessageBox(framewnd,"Source and target locations are the same","Bad error happened",MB_OK); 
        goto samesound;
    }

    selector = buf1;

loopstart3:

    for(i = 0 ; ; i += 3)
    {
        if(*(short*)(selector + i) == -1) 
            break;
    }

    if(selector == buf1)
    {
        src_length = i;
        selector = buf2;
        
        goto loopstart3;
    }
    else
        port_length = i;

    if(src_length <= port_length)
    {
        memcpy(buf2, buf1, src_length);
    }
    else
    {
        MessageBox(framewnd,"lack of room for sound","Bad error happened",MB_OK);   
        
    }
    
    //----------------------------------------
    
samesound:

    //begin fourth transfer
    buf1 = src_rom + offsets.dungeon.torches;
    buf2 = port_rom + offsets.dungeon.torches;

    if(buf1 == buf2)
    {
        MessageBox(framewnd,"Source and target locations are the same","Bad error happened",MB_OK); 
        goto sametreasure;
    }

    // find the data for the first rom.
    l = ldle16b(src_rom + offsets.dungeon.torch_count);
    
    for(i = 0; ; i += 2)
    {
        // if i exceeds l, we copy nothing.
        if(i >= l)
        {
            src_length = 0;
                        
            break;
        }
        
        // preserve the index with j, b/c i may about to be altered.
        j = i;
        
        for( ; ; )
        {
            i += 2;
        
            // go until we hit 0xFFFF in the rom
            if(*(short*)(buf1 + i) == -1)
                break;
        }
        
        // if the map index matches that given offset, we copy to the buffer.
        if(*(short*)(buf1 + j) == a)
        {
            src_length = i - j;
            buf1 = buf1 + j;
            
            break;
        }
    }

    //do it for the port rom now.

    l = ldle16b(port_rom + offsets.dungeon.torch_count);
    
    for(i = 0; ; i += 2)
    {
        // if i exceeds l, we copy nothing.
        if(i >= l)
        {
            port_length = 0;
                        
            break;
        }
        
        // preserve the index with j, b/c i may about to be altered.
        j = i;
        
        for( ; ; )
        {
            i += 2;
        
            // go until we hit 0xFFFF in the rom
            if(*(short*)(buf2 + i) == -1)
                break;
        }
        
        // if the map index matches that given offset, we copy to the buffer.
        if(*(short*)(buf2 + j) == a)
        {
            port_length = i - j;
            buf2 = buf2 + j;

            break;
        }
    }

    if(src_length <= port_length)
    {
        //don't want to copy blank data, do we?
        if(src_length)
            memcpy(buf2, buf1, src_length);
    
        //memcpy(ed->tbuf = malloc(ed->tsize = i - j),buf + j,i-j);
    }
    else
    {
        // do error message
        MessageBox(framewnd,"not enough room for treasure","Bad error happened",MB_OK); 
        
    }
    
    //----------------------------------------
    


sametreasure:


    return;
    // end of routine
}

//CopyTiles*****************************

//hextodec has been imported from SNESDisasm to parse hex numbers

int hextodec(char *input, int length)
{
    int result = 0;
    int value;
    int ceiling = length - 1;
    int power16 = 16;

    int j = ceiling;

    for( ;  j >= 0; j--)
    {
        if(input[j] >= 'A' && input[j] <= 'F')
        {
            value = input[j] - 'F';
            value += 15;
        }
        else
        {
            value = input[j] - '9';
            value += 9;
        }

        if(j == ceiling)
        {
            result += value;
            continue;
        }

        result += (value * power16);
        power16 *= 16;
    }
    
    return result;
}


//GetArgument is imported from SNESDisasm to parse hex numbers.

int GetArgument(HWND win, int boxNumber, int *value)
{
    int i;
    
    char hextemp[256];
    
    if(!GetDlgItemText(win, boxNumber, hextemp, 256))
        return 0; // tell the calling routine that it didn't work.
    
    for(i = 0; hextemp[i] != 0; i++)
    {
        int j = hextemp[i];

        if(j >= 'A' && j <= 'F')
            continue;

        if(j >= '0' && j <= '9')
            continue;
        else
        {
            MessageBox(win, "Edit box contains non-hex characters", "you naughty boy", MB_OK);
            
            return 0;
        }
    }

    *value = hextodec(hextemp, i);

    return 1;   
}

//RelocateHeaders***************************

void RelocateHeaders(HWND win, FDOC *doc, int dest)
{
    char headerBuffer[14];
    uint8_t * const rom = doc->rom;
    
    int romHeaderLoc = doc->headerLocation;
    int cpuHeaderLoc = cpuaddr(romHeaderLoc);
    
    int bank = cpuHeaderLoc & 0xFF0000;
    int absolute;
    int j;
    
    int romHeaderDest = dest;
    int cpuHeaderDest = cpuaddr(romHeaderDest);
    
    short pointerEntry = (cpuHeaderDest & 0xFFFF) + 0x250;
    
    char bleh1 = 0xBF;
    char bleh2 = 0xA9;
    
    if( (rom[0xB5DC] != bleh1) || (rom[0xB5E6] != bleh2))
    {
        MessageBox(win, "Your rom's code has been modified\nMoving the headers would be risky", "sorry", MB_OK);
        
        return;
    }
    
    //compare the banks of the destination address and the end of the table
    //and see if it will cross a bank boundary.
    
    if( (char) (cpuHeaderDest >> 16) !=  (char) ((cpuaddr(dest + 0x1280)) >> 16)  )
    {
        MessageBox(win, "Moving the headers would cause them to cross a bank boundary", "that's bad", MB_OK); 
        
        return;
    }

    rom[0xB5E7] = (char) (cpuHeaderDest >> 16);

    doc->headerLocation = romHeaderDest;

    rom[0xB5DD] = (char) (cpuHeaderDest);
    rom[0xB5DE] = (char) (cpuHeaderDest >> 8);
    rom[0xB5DF] = (char) (cpuHeaderDest >> 16);

    for(j = 0; j < 0x250; j += 2, pointerEntry += 14)
    {
        absolute = (  *(int*) (rom + romHeaderLoc + j)  ) & 0xFFFF;
        absolute += bank;
        absolute = romaddr(absolute);
        
        memcpy(headerBuffer, rom + absolute, 0x0E);
        
        stle16b(rom + romHeaderDest + j, pointerEntry);
        
        absolute = ((int) pointerEntry) & 0xFFFF;
        absolute += (cpuHeaderDest & 0xFF0000);
        absolute = romaddr(absolute);
        
        memcpy(rom + absolute, headerBuffer, 14);
    }

}

//RelocateHeaders***************************

//MoveHeadersProc***************************

BOOL CALLBACK moveHeadersProc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int headLoc;
    int headDest = 0;

    char offset[7]; // will contain the hex address for the header offset.

    FDOC *currentdoc = activedoc;

    (void) lparam;
    
    if(!currentdoc)
        return FALSE;
    
    headLoc = currentdoc->headerLocation;

    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        itoa(currentdoc->headerLocation, offset, 16);
        
        strupr(offset);
        
        SetDlgItemText(win, IDC_OFFSET, offset);
        
        break;
    
    case WM_COMMAND:
    
        switch(wparam)
        {

        case IDC_MOVEDATA:

            if(!GetArgument(win, IDC_EDITDATA, &headDest))
                break;
            
            if(currentdoc->fsize < (headDest + 0x1280))
            {   
                MessageBox(win, "Not enough room to move header table", "error", MB_OK);
                
                break;
            }

            if(MessageBox(win, "Are you sure you want to move the header data here?", "risk of corruption", MB_YESNO) == IDNO)
                break;

            RelocateHeaders(win, currentdoc, headDest);

            itoa(currentdoc->headerLocation, offset, 16);
            strupr(offset);

            SetDlgItemText(win, IDC_OFFSET, offset);

            break;

        case IDCANCEL:
    
            EndDialog(win,0); // kill the dialogue, and do nothing.

            break;

        }

        break;

    }


    return FALSE;
}


//MoveHeadersProc***************************

//port#***************************************************************

BOOL CALLBACK port(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i, // source room/map
        j; // destination room/map
    
    char istr[30], jstr[30];
    
    switch(msg) 
    {
    case WM_INITDIALOG:
        SetWindowLong( win, DWL_USER, lparam);
        
        srcdoc = firstdoc;
    
        portdoc = lastdoc;
                
        SetDlgItemText(win, IDC_STATICROM1, srcdoc->filename);
        SetDlgItemText(win, IDC_STATICROM2, portdoc->filename);
        SetDlgItemInt(win, IDC_EDIT_SRC, 0, 0);
        SetDlgItemInt(win, IDC_EDIT_DEST, 0, 0);

        CheckDlgButton(win, IDC_CHECK_HED, BST_CHECKED);
        CheckDlgButton(win, IDC_CHECK_TIL, BST_CHECKED);
        CheckDlgButton(win, IDC_CHECK_SPR, BST_CHECKED);
        CheckDlgButton(win, IDC_CHECK_OBJ, BST_CHECKED);

        break;
    
    case WM_COMMAND:
        switch(wparam) 
        {
        // handlers for the bottom row of buttons. controls the destination document
        case IDC_BUTTON50:
            if((portdoc->next))
            {
                portdoc = portdoc->next;
                SetDlgItemText(win, IDC_STATICROM2, portdoc->filename);

                break;
            
            }
            else if(firstdoc)
            {
                portdoc = firstdoc;
                SetDlgItemText(win, IDC_STATICROM2, portdoc->filename);
                
                break;
            }
            else
            {                           
                SetDlgItemText(win, IDC_STATICROM2, "error");
            }   


            break;
        
        case IDC_BUTTON51:
            if((portdoc->prev))
            {
                portdoc = portdoc->prev;
                SetDlgItemText(win, IDC_STATICROM2, portdoc->filename);

                break;
            
            }
            else if(lastdoc)
            {
                portdoc = lastdoc;
                SetDlgItemText(win, IDC_STATICROM2, portdoc->filename);
                
                break;
            }
            else
            {                           
                SetDlgItemText(win, IDC_STATICROM2, "error");
            }   


            break;

        //handlers for the top row of buttons - the source document.
        case IDC_BUTTON53:
            if((srcdoc->next))
            {
                srcdoc = srcdoc->next;
                SetDlgItemText(win, IDC_STATICROM1, srcdoc->filename);

                break;
            
            }
            else if(firstdoc)
            {
                srcdoc = firstdoc;
                SetDlgItemText(win, IDC_STATICROM1, srcdoc->filename);
                
                break;
            }
            else
            {                           
                SetDlgItemText(win, IDC_STATICROM1, "error");
            }   


            break;
        
        case IDC_BUTTON52:
            if(srcdoc->prev)
            {
                srcdoc = srcdoc->prev;
                SetDlgItemText(win, IDC_STATICROM1, srcdoc->filename);

                break;
            
            }
            else if(lastdoc)
            {
                srcdoc = lastdoc;
                SetDlgItemText(win, IDC_STATICROM1, srcdoc->filename);
                
                break;
            }
            else
            {                           
                SetDlgItemText(win, IDC_STATICROM1, "error");
            }


            break;

        //what to do if they click OK (start porting)
        case IDOK:
            
            if(!srcdoc)
                break;

            if(!portdoc)
                break;

            i = GetDlgItemInt(win, IDC_EDIT_SRC, 0, 0);
            j = GetDlgItemInt(win, IDC_EDIT_DEST, 0, 0);
    
            i &= 0x1FF;
            j &= 0x1FF;

            while(i > 295)
                i -= 295;

            while(j > 295)
                j -= 295;

            SetDlgItemInt(win, IDC_EDIT_SRC, i, 0);
            SetDlgItemInt(win, IDC_EDIT_DEST, j, 0);

            itoa(i, istr, 10);
            itoa(j, jstr, 10);
    
            SetDlgItemText(win, IDC_TEST, (LPCTSTR) istr);

            if(IsDlgButtonChecked(win, IDC_CHECK_HED) == BST_CHECKED)
            {
                
                CopyHeader(srcdoc,portdoc,i,j);
            }

            if(IsDlgButtonChecked(win, IDC_CHECK_SPR) == BST_CHECKED)
            {
                //CopySprites()
            }

            if(IsDlgButtonChecked(win, IDC_CHECK_TIL) == BST_CHECKED)
            {
                CopyTiles(srcdoc,portdoc,i,j);
            }

            if(IsDlgButtonChecked(win, IDC_CHECK_OBJ) == BST_CHECKED)
            {
                //CopyObjects()
            }

            break;
            
        case IDCANCEL:
            
            // kill the dialog, and do nothing.
            EndDialog(win,0);
            
            break;
        }
    }
    
    return 0;
}

//Port********************************************************

//Port and related functions**********************************

#endif

// =============================================================================

void
Updatesize(HWND win)
{
    RECT rc = HM_GetClientRect(win);
    
    // the '0' is SIZE_RESTORED.
    SendMessage(win, WM_SIZE, 0, (rc.bottom << 16) + rc.right);
}

// =============================================================================

/*void Dlgtopoint(int bw,int bh,RECT*rc)
{
    rc->left=rc->left*bw>>2;
    rc->right=rc->right*bw>>2;
    rc->top=rc->top*bh>>3;
    rc->bottom=rc->bottom*bh>>3;
}*/

// A homegrown version of MFC's DoModal(). Granted, code like this surely
// existed long before MFC, so this code probably predates it.
// On the other hand, most of this code doesn't run unless the program
// is executing on the win32s platform.
int ShowDialog(HINSTANCE hinst,LPSTR id,HWND owner,DLGPROC dlgproc, LPARAM param)
{
    HGLOBAL hgl;
    
    HRSRC rc;
    
    HANDLE font;
    
    HWND hc;
    HWND win;
    
    unsigned short *p, *q;
    int x, y, bw, bh, i, j, k, l;
    
    MSG msg;
    
    TEXTMETRIC tm;
    RECT rect;
    
    DLGTEMPLATE *dt;
    DLGITEMTEMPLATE *dit;
    
    const static char *cls_def[] = {"BUTTON","EDIT","STATIC","LISTBOX","SCROLLBAR","COMBOBOX"};
    
    if(!wver)
        return DialogBoxParam(hinst, id, owner, dlgproc, param);
    
    rc = FindResource(hinst, id, RT_DIALOG);
    
    if(!rc)
    {
        MessageBox(framewnd,"Can't load dialog","Bad error happened",MB_OK);
        
        return -1;
    }
    
    hgl = LoadResource(hinst, rc);
    p = LockResource(hgl);
    j = 0;
    
    if(p[1] == 0xffff)
        p += 4, j = 1, k = ((int*) p)[0], l = ((int*) p)[1];
    else
        k = ((int*) p)[1], l = ((int*) p)[0];
    
    dt = (DLGTEMPLATE*) p;
    
    p += 11;
    l |= DS_3DLOOK;
    
    if(l & DS_MODALFRAME)
        k |= WS_EX_DLGMODALFRAME;
    
    WideCharToMultiByte(CP_ACP, 0, p, -1, buffer, 512, 0, 0);
    
    while(GetWindowLong(owner, GWL_STYLE) & WS_CHILD)
        owner = GetParent(owner);
    
    win = CreateWindowEx(k, (LPSTR) 32770, buffer, l & ~WS_VISIBLE, 0, 0, 100, 100, owner, 0, hinst, 0);
    SetWindowLong(win,DWL_DLGPROC,(int)dlgproc);
    p+=1+lstrlenW(p);
    
    if(l & DS_SETFONT)
    {
        HDC const dc = GetDC(win);
        
        HGDIOBJ oldobj;
        
        x = -*p * GetDeviceCaps(dc, LOGPIXELSY) / 72;
        
        if(j)
        {
            WideCharToMultiByte(CP_ACP,0,p+3,-1,buffer,512,0,0);
            font=CreateFont(x,0,0,0,p[1],p[2],0,0,0,0,0,0,0,buffer);
            p+=4+lstrlenW(p+3);
        }
        else
        {
            WideCharToMultiByte(CP_ACP,0,p+1,-1,buffer,512,0,0);
            font=CreateFont(x,0,0,0,0,0,0,0,0,0,0,0,0,buffer);
            p+=2+lstrlenW(p+1);
        }
        
        SendMessage(win,WM_SETFONT,(int)font,1);
        
        oldobj = SelectObject(dc, font);
        
        GetTextMetrics(dc, &tm);
        
        SelectObject(dc, oldobj);
        
        ReleaseDC(win, dc);
        
        SetWindowLong(win, 12, tm.tmAveCharWidth + (tm.tmHeight << 16) + 1);
    }
    else
    {
        font=0;
        bw=GetDialogBaseUnits();
        SetWindowLong(win,12,bw);
        bh=bw>>16;
        bw&=65535;
    }
    rect.left=dt->x;
    rect.top=dt->y;
    rect.right=dt->cx;
    rect.bottom=dt->cy;
    MapDialogRect(win,&rect);
    rect.right+=rect.left;
    rect.bottom+=rect.top;
    AdjustWindowRect(&rect,l,0);
    rect.right-=rect.left;
    rect.bottom-=rect.top;
    
    if(l & DS_CENTER)
    {
        rect.left = ( GetSystemMetrics(SM_CXSCREEN) - rect.right  ) >> 1;
        rect.top  = ( GetSystemMetrics(SM_CYSCREEN) - rect.bottom ) >> 1;
    }
    
    SetWindowPos(win,0,rect.left,rect.top,rect.right,rect.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
    
    q = p;
    
    for(i=dt->cdit;i;i--)
    {
        HWND child_win = NULL;
        
        HMENU child_id;
        
        p++;
        
        p = (unsigned short*) (((int)p)&0xfffffffc);
        
        if(j)
            p+=2,
            k=((int*)p)[0],
            l=((int*)p)[1];
        else
            k=((int*)p)[1],
            l=((int*)p)[0];
        
        dit = (DLGITEMTEMPLATE*) p;
        
        p+=9;
        
        child_id = (HMENU) (0 | dit->id);
        
        if(j)
            p++;
        
        if(*p==0xffff)
        {
            bw=p[1]-128;
            
            x = (int) cls_def[bw];
            p += 2;
            
            if(bw==1)
                k|=WS_EX_CLIENTEDGE;
        }
        else
        {
            WideCharToMultiByte(CP_ACP,0,p,-1,buffer,256,0,0);
            
            x=(int)buffer,p+=1+lstrlenW(p);
            bw=6;
        }
        
        if(*p==0xffff)
        {
            y = p[1];
            
            p += 2;
        }
        else
        {
            WideCharToMultiByte(CP_ACP,0,p,-1,buffer+256,256,0,0);
            y=(int)buffer+256,p+=1+lstrlenW(p);
        }
        
        rect.left   = dit->x;
        rect.top    = dit->y;
        rect.right  = dit->cx;
        rect.bottom = dit->cy;
        
        MapDialogRect(win,&rect);
        bh=l&31;
        
        if(bw==2 && (bh==SS_ICON))
            bh=y,y=(int)"";
        else
            bh=0;
        
        child_win = CreateWindowEx(k,
                                   (LPSTR)x,
                                   (LPSTR)y,
                                   l,
                                   rect.left,
                                   rect.top,
                                   rect.right,
                                   rect.bottom,
                                   win,
                                   child_id,
                                   hinst,
                                   j ? p + 1 : p);
        
        SendMessage(child_win,
                    WM_SETFONT,
                    (int) font,
                    1);
        
        if(bh)
        {
            HANDLE icon = LoadImage(hinst, (LPSTR) bh, IMAGE_ICON, 0, 0, 0);
            
            if(icon)
                SendMessage(child_win, STM_SETICON, (WPARAM) icon, 0);
        }
        
        p += (*p >> 1) + 1;
    }
    
    hc = GetNextDlgTabItem(win,0,0);
    
    if( SendMessage(win, WM_INITDIALOG, (int) hc, param) )
    {
        hc = GetNextDlgTabItem(win, 0, 0);
        
        SetFocus(hc);
    }
    
    ShowWindow(win, SW_SHOW);
    
    x = EnableWindow(owner, 0);
    
    while((!GetWindowWord(win,18)) && GetMessage(&msg,0,0,0))
    {
        if(!IsDialogMessage(win,&msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    y=GetWindowWord(win,22);
    p=q;
    
    for(i=dt->cdit;i;i--)
    {
        p++;
        
        p = (unsigned short*) (((int)p)&0xfffffffc);
        
        if(j) p+=2,l=((int*)p)[1]; else l=((int*)p)[0];
        
        dit = (DLGITEMTEMPLATE*) p;
        
        p += 9;
        
        if(j)
            p++;
        
        if(*p==0xffff)
        {
            bw=p[1]-128;
            p+=2;
        }
        else
        {
            bw=6;
            p+=1+lstrlenW(p);
        }
        
        if(*p==0xffff)
            p+=2;
        else
            p+=1+lstrlenW(p);
        
        bh=l&31;
        if(bw==2 && bh==SS_ICON)
        {
            HGDIOBJ icon =
            (HGDIOBJ) SendMessage(GetDlgItem(win, dit->id),
                                  STM_GETICON,
                                  0,
                                  0);
            
            if(icon)
                DeleteObject(icon);
        }
        
        p+=(*p>>1)+1;
    }
    
    DestroyWindow(win);
    EnableWindow(owner,!x);
    return y;
}

//#define ShowDialog DialogBoxParam

HWND
Editwin(FDOC       * const doc,
        char const * const wclass,
        char const * const title,
        LPARAM       const param,
        int          const size)
{
    char buf[1024];
    HWND hc;
    MDICREATESTRUCT mdic;
    
    EDITWIN * const a = (EDITWIN*) calloc(1, size);
    
    wsprintf(buf, "%s - %s", doc->filename, title);
    a->doc=doc;
    a->param=param;
    mdic.szClass=wclass;
    mdic.szTitle=buf;
    mdic.hOwner=hinstance;
    mdic.x=mdic.y=mdic.cx=mdic.cy=CW_USEDEFAULT;
    mdic.style=WS_SYSMENU|WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    mdic.lParam=(long)a;
    hc=(HWND)SendMessage(clientwnd,WM_MDICREATE,0,(long)&mdic);
    
    if(hc)
    {
        SendMessage(clientwnd, WM_MDIACTIVATE, (long)hc, 0);
        SendMessage(clientwnd, WM_MDIREFRESHMENU, 0, 0);
    }
    else
    {
        MessageBox(framewnd, "Edit window creation failed!", NULL, MB_OK);
    }
    
    return hc;
}


// =============================================================================

int
Changesize(FDOC * const doc,
           int          num,
           int          size)
{
    unsigned char *rom = doc->rom; // the calling rom matches the one we're using.
    int *blah;
    
    int addr = 0,
        i    = 0, // counter variable
        j    = doc->fsize, // get the file's size
        k    = 0,
        m    = 0,
        n    = 0,
        o    = 0,
        p    = 0,
        q    = 0,
        r    = 0,
        s    = 0,
        v    = 0,
        x    = 0,
        max  = 0;
    
    int base[40] = {0x58000,
                    0x60000,
                    0xf8780,
                    0x50000,
                    0x1eba0,
                    0x26cf9,
                    doc->tmend3,
                    0x75a9c}; // these are the starting points to where data can be written.
    
    static int limit[40] = {0x5fe70,// <-- known end of some code.
                            0x64118,
                            0x100000, // an obvious upper bound
                            0x53730,
                            0x20000,
                            0x271de,
                            0x66d7a,
                            0x75d40}; // these are the limits to which data can be written.
    
    int lastaddr[40] = { doc->mapend2,
                         doc->mapend,
                         doc->roomend,
                         doc->roomend2,
                         doc->roomend3,
                         doc->ovlend,  // overlay end
                         doc->tmend1,  // tile map end
                         doc->tmend2}; // various other pointers...
    
    static char move_flag[40]; // ???
    
    char w[40];
    
    static int trans_st[39],trans_end[39];
    
    int l[768],
        t[0x140],
        u[768];
    
    for(i = 0; i < 320; i++)
        // offset the rom by 0x1794d, add in a multiple of three.
        // my guess is we're loading long pointers from the rom (24 bit)
        l[i] = romaddr(*(int*) (rom + 0x1794d + (i * 3) ));
    
    for(i = 0; i < 320; i++)
    {
        // load more long pointers from 0xF8000
        l[i + 320] = m = romaddr( *(int*) (rom + 0xf8000 + i*3) );

        // get some long pointers from 0xF83C0, and subtract off the other offsets above.
        // maybe a measure of length of some sort.
        t[i] = romaddr( *(int*) (rom + 0xf83c0 + i * 3) ) - m;
    }
    
    // Another pointer...
    l[640] = romaddr( *(int*) (rom + 0x882d));
    
    for(i = 0; i < 19; i++)
        l[i + 641] = romaddr( *(int*) (rom + 0x26cc0 + i*3));
    
    for(i = 0; i < 8; i++)
        // here we use that l[640] pointer as an offset.
        l[i + 660] = romaddr(*(int*) (rom + l[640] + i*3));
    
    // obtain another pointer... note rom[0x9C25] is going to be the bank of the long pointer.
    l[668] = romaddr( *(unsigned short*) (rom + 0x9c2a) + (rom[0x9c25] << 16 ) );
    
    for(i = 0; i < 7; i++)
        //and the beat goes on... note we have a high byte, low byte, and a bank.
        l[669 + i] = romaddr(rom[0x137d + i] + (rom[0x1386 + i] << 8) + (rom[0x138f + i] << 16));
    
    // max = 676 plus the number of segments in the FDOC.
    max = 676 + doc->numseg;
    
    for(i = 676; i < max; i++)
        // gimme a set of pointers. presumably 92 or so pointers.
        //segs has dimension 92 usually.
        l[i] = doc->segs[i - 676];
    
    // 0 if no expansion has taken place.
    base[8] = doc->mapexp;
    
    for(i = 0; i < 32; i++)
        limit[i + 8] = 0x108000 + (i << 15);
    
    // limit[8] = 0x108000, limit[9] = 0x110000, limit[10] = 0x118000...
    // limit[38] = 0x1F8000, limit[39] = 0x200000
    
    // if the maps have been expanded...
    if(doc->mapexp >= 0x100080)
    {
        for(i = 0; i < 31; i++)
        {
            lastaddr[i + 8] = ( (int*) (rom + 0x100004) )[i];
            base[i + 9] = (i << 15) + 0x108000;
        }
        
        // basically, take this value as the bigger of the two.
        lastaddr[39] = (doc->fsize > 0x1f8000) ? doc->fsize : 0x1f8000;
    }
    else // else expand the rom.
    {
        base[8] = 0x100080;
        
        for(i=1;i<32;i++)
            base[i + 8] = limit[i + 8] - 0x8000;
        
        for(i = 0; i < 32; i++)
            lastaddr[i + 8] = base[i + 8];
    }
    
    for(i = 0; i < 40; i++)
        move_flag[i] = 1;
    
    // in the case of dungeon copying, it's 50140 + j, where j is the room index.
    x = num;
    
    // truncate down to 16 bit.
    num &= 65535;
    
    if(x & 0x80000) // no for dungeon or overworld copying.
    {
        for(i = 0; i < 40; i++)
            if(limit[i] - lastaddr[i] >= size)
            {
                j = addr = lastaddr[i];
                v = i;
                
                goto oknewblk;
            }
        
        goto nospace;
    }
    
    if(num == 4097) // no again.
    {
        v = 8;
        addr = 0x100080;
        j = doc->mapexp;
        size = 0;
        m = (doc->fsize >> 5) - 24;
        
        if(m < 40)
        {
            for(i = m + 1; i < 40; i++)
                lastaddr[i] = base[i];
            
            lastaddr[m]=doc->fsize;
        }
        
        for(i = 8; i < m; i++)
            lastaddr[i] = 0x200000;
        
        for(i = 0; i < max; i++)
        {
            m = u[i];
            
            if(m < 9)
                continue;
            
            if(l[i] < lastaddr[i - 1])
                lastaddr[i - 1] = l[i];
        }
    }
    else if(num == 4096) // no again.
    {
        addr = 0x65d6d;
        size -= addr;
        
        if(size > 0x100d)
        {
            MessageBox(framewnd,"Not enough room for menu template","Bad error happened",MB_OK);
            return 0;
        }
        
        v = 6;
        j = base[6];
        base[6] = 0x65d6d;
    }
    else // yep for dungeons, overworld
    {
        // the overworld or dungeon index. e.g. 0x145 for room 6
        // addr gives the rom offset for it. l[num] gives the base address for
        // this, I believe.
        addr = l[num];
        
        for(v = 0; v < 40; v++)
        {
            if( addr >= base[v] && addr < lastaddr[v] )
                break;
        }
        
        if(v == 40) // couldn't find anywhere to use
        {
            wsprintf(buffer,"Cannot save, unknown location %06X",addr);
            MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
            
            return 0;
        }
        
        j = lastaddr[v]; // otherwise, get the index of the last address for the room.
    }
oknewblk:
    
    for(i = 0; i < max; i++) // i < about 676 + 92
    {
        m = l[i];
        
        for(k = 0; k < 40; k++)
        {
            if(m >= base[k] && m < lastaddr[k])
                break; // if you find somewhere it fits...
        }
        
        // if(k==9) {
        //     wsprintf(buffer,"Warning, unknown address %06X found, index %03X",addr,i);
        //     MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
        // }
        
        u[i] = k; // u[i] is the index at which that occurs.
        
        if( (k == v) && (m > addr) && (m < j) )
            j = m; // truncate the ending address further back, no need to waste.
    }
    
    if(x & 0x80000)
    {
        u[num] = v;
        l[num] = addr;
    }
    
    if(x & 0x40000)
    {
        l[num] = l[size]; // in the case of dungeons, size is the source room index.
        size = 0; // hence, the rom address for the target room will be the same as the source's.
    }

    if(x & 0x10000)
    {
        for(i = 0; i < max; i++)
        {
            // handle repetitions of this data.
            if( (i != num) && (l[i] == l[num]) )
            {
                if(x & 0x20000) // in the dungeon case, no.
                {
                    if(num < 320) // if it's an overworld room.
                        wsprintf(buffer,"The data for area %02X is reused. Modify only this area?",num);
                    else
                        wsprintf(buffer,"The data for room %d is reused. Modify only this room?",num-320);
                    
                    // handle the result of the dialog.
                    if( MessageBox(framewnd, buffer, "Hyrule Magic", MB_YESNO) == IDNO)
                        goto nosplit;
                }
                
                // in the case of 0x50000 types, nothing happens in the above.
                // we're going to split the data off and put it in a unique sement of memory.
                j = addr;
                issplit = 1;
                
                break;
            }
        }
    }

nosplit:
    
    addr += size; // if a 0x50000 type, size is zero by the above.
    r = addr; // the target rom address of the room.
    s = j;
    
    n = addr - j;
    
    if(!n) // if they map to the same room, don't change anything.
        goto nochange;
    
    // otherwise,

    if( (num < 4096) && (v) )
    {
        q = addr - base[v];
        p = lastaddr[v - 1];
        k = limit[v - 1] - p;
        
        if(q <= k)
        {
            u[num]--;
            o = 0;
            
            for(i = 0; i < max; i++)
            {
                if(u[i] == v)
                {
                    m = l[i] - base[v];
                    
                    if(m <= k && m > o)
                        o = m;
                }
            }
            
            if(!o)
                o = j - base[v];
            
            for(i = 0; i < max; i++)
            {
                if(u[i] == v)
                {
                    m = l[i] - base[v];
                    
                    if(m <= q)
                        l[i] = m + p,
                        u[i]--;
                    else if(m < o)
                        l[i] += p + size - j;
                }
            }
            
            memcpy(rom + p, rom + base[v], q);
            
            l[num] = p;
            p += q;
            k -= q;
            
            memcpy(rom + p, rom + j, o + base[v] - j);
            
            lastaddr[v - 1] = p + o + base[v] - j;
            addr = base[v];
            j = addr + o;
            n = -o;
        }
    }
    
    for(q = v; ; q++)
    {
        if(!addr)
            break;
        
        k = lastaddr[q];
        
        if(num == 4097)
        {
            if(n > 32768)
            {
                MessageBox(framewnd,
                           "This ROM is corrupt.",
                           "Bad error happened",
                           MB_OK);
                
                return 0;
            }
            
            n = base[q] - lastaddr[q - 1];
        }
        else if(!n)
            break;
        
        if(n > limit[q] - base[q])
        {
            if(q == 39)
                goto nospace;
            
            trans_end[q] = trans_end[q - 1];
            trans_end[q - 1] = trans_st[q] = trans_st[q - 1];
            j = base[q + 1];
            addr = j + n;
            w[q] = w[q - 1];
            
            continue;
        }
        
        for(i = 0; i < max; i++)
        {
            m = l[i];
            
            if(i != num && u[i] == q && m >= j && m < k)
                l[i] = m + n;
        }
        
        if(q >= 8)
        {
            if(!doc->mapexp)
                k = doc->mapexp = 0x100080;
            
            k += n;
            
            if(k <= doc->mapexp)
            {
                k=0;
                doc->mapexp = 0;
                doc->fsize = 0x100000;
                move_flag[v] = 0;
                
                goto newsize;
            }
            else if(q == 39 || lastaddr[q + 1] == base[q + 1])
            {
                if(always)
                {
                    doc->fsize = k;
                    
                newsize:
                    
                    rom = (uint8_t*) realloc(doc->rom, doc->fsize);
                }
            }
            
            base[8] = doc->mapexp;
            
            if(q == 39)
            {
                lastaddr[q] = k;
                p = k = k - n;
                if(k > 0x200000)
                {
nospace:
                    
                    MessageBox(framewnd, n == 4097 ? "This ROM is too large." : "Not enough space.", "Bad error happened", MB_OK);
                    return 0;
                }
                
                break;
            }
        }
        else
            k += n;
        
        if(k > limit[q])
        {
            o = k;
            p = base[q];
            
            for(i = 0; i < max; i++)
            {
                m = l[i];
                if(u[i] == q && m > limit[q] && m < o)
                    o = m;
            }
            
            for(i = 0; i < max; i++)
            {
                m = l[i];
                
                if(u[i] == q && m > p && m < o)
                    p = m;
            }
            
            if(p == base[q])
            {
                MessageBox(framewnd,"Internal error","Bad error happened",MB_OK);
                return 0;
            }
            
            lastaddr[q] = p;
            p -= n,k -= n;
            
            if(q == v && r > p)
                move_flag[q] = 0;
        }
        else
        {
            if(addr <= k)
                memmove(rom + addr, rom + j, k - addr), move_flag[q] = 0;
            
            if( (q < 39) && base[q + 1] != lastaddr[q + 1] )
            {
                p = 0;
                
                for(i = 0; i < max; i++)
                {
                    m = l[i];
                    
                    if(u[i] == q + 1 && m - base[q + 1] < limit[q] - k && m - base[q + 1] > p)
                        p = m - base[q + 1];
                }
                
                if(p)
                {
                    for(i = 0; i < max; i++)
                    {
                        m = l[i];
                        
                        if(u[i] == q + 1 && m - base[q + 1] < p)
                            l[i] = m - base[q + 1] + k, u[i]--;
                    }
                    
                    for(i = 0; i < max; i++)
                    {
                        if(u[i] == q + 1)
                            l[i] -= p;
                    }
                    
                    lastaddr[q + 1] -= p;
                    memcpy(rom + k, rom + base[q + 1], p);
                    memcpy(rom + base[q + 1], rom + base[q + 1] + p, lastaddr[q + 1] - base[q + 1]);
                    
                    k += p;
                }
            }
            
            lastaddr[q] = p = k;
        }
        
        trans_st[q] = p;
        trans_end[q] = k;
        w[q] = (char) q;
        n = k - p;
        j = base[q + 1];
        addr = j + n;
    }
    
    for(q--; q >= v; q--)
    {
        p = trans_st[q];
        k = trans_end[q];
        
        if(p == k)
            continue;
        
        if(lastaddr[q+1] > base[q+1] && move_flag[q+1])
            memmove(rom + base[q+1] + k - p, rom + base[q+1], lastaddr[q+1] - base[q+1] - k + p);
        
        memmove(rom + base[q + 1], rom + p, k - p);
        
        for(i = 0; i < max; i++)
        {
            m = l[i];
            
            if(u[i] == w[q] && m >= lastaddr[u[i]])
                l[i] = m + base[q + 1] - lastaddr[u[i]], u[i] = q + 1;
        }
    }
    doc->mapend2 = lastaddr[0];
    doc->mapend = lastaddr[1];
    doc->roomend = lastaddr[2];
    doc->roomend2 = lastaddr[3];
    doc->roomend3 = lastaddr[4];
    doc->ovlend = lastaddr[5];
    doc->tmend1 = lastaddr[6];
    doc->tmend2 = lastaddr[7];
    
    if(num==4096)
        doc->tmend3 = base[6] = r;
    
    if(doc->mapexp)
    {
        for(i = 0; i < 31; i++)
        {
            // 0x100004 in rom is used as the offset for the end of HM's 
            // expanded data. Useful to know.
            ((int*) (rom + 0x100004))[i] = lastaddr[i+8];
        }
    }
    
    if(move_flag[v]) memmove(rom + r,rom + s, p - s);
    
    doc->rom = rom;
    
    for(i = 0; i < max; i++)
    {
        m = l[i];
        q = u[i];
        
        if(q == 40)
            continue;
        
        if(m < base[q] || m > lastaddr[q] || (m > limit[q]))
        {
            wsprintf(buffer,"Internal error, block %d is outside range %d",i,q);
            MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
        }
    }
nochange:
    
    for(i = 0; i < 320; i++)
    {
        blah = (int*) (rom + 0x1794d + i*3);
        *blah &= 0xff000000;
        *blah |= cpuaddr(l[i]);
    }
    
    *(int*) (rom + 0x882d) = cpuaddr(l[640]) + 0x85000000;
    *(int*) (rom + 0x8827) = cpuaddr(l[640]) + 0x85000001;
    
    j = cpuaddr(l[668]);
    
    rom[0x9c25] = (uint8_t) (j >> 16);
    stle16b(rom + 0x9c2a, (j & 0xffff) );
    
    rom[0xcbad] = (uint8_t) (j >> 16);
    stle16b(rom + 0xcba2, (j & 0xffff) );
    
    for(i = 0; i < 19; i++)
    {
        blah = (int*) (rom + 0x26cc0 + i*3);
        
        *blah &= 0xff000000;
        *blah |= cpuaddr(l[i + 641]);
    }
    
    for(i = 0; i < 8; i++)
    {
        blah = (int*) (rom + l[640] + i*3);
        *blah &= 0xff000000;
        *blah |= cpuaddr(l[i+660]);
    }
    
    for(i = 0; i < 7; i++)
    {
        if(!(rom[0x1386 + i] & 128))
            continue;
        
        j = cpuaddr(l[i + 669]);
        rom[0x137d + i] = (uint8_t) (j >>  0);
        rom[0x1386 + i] = (uint8_t) (j >>  8);
        rom[0x138f + i] = (uint8_t) (j >> 16);
    }
    
    for(i = 676; i < max; i++)
    {
        if( l[i] != doc->segs[i-676] )
            doc->p_modf = 1;
        
        doc->segs[i - 676] = l[i];
    }
    
    if(num < 4096)
    {
        j = l[num];
        
        for(i = 0; i < 320; i++)
        {
            if(l[i + 320] == j)
                t[i] = door_ofs;
        }
    }
    
    for(i = 0; i < 320; i++)
    {
        blah = (int*) (rom + 0xf8000 + i*3);
        *blah &= 0xff000000;
        *blah |= cpuaddr(l[i+320]);
        blah = (int*) (rom + 0xf83c0 + i*3);
        *blah &= 0xff000000;
        *blah |= cpuaddr(l[i + 320] + t[i]);
    }
    
    if(num == 4097)
        return doc->mapexp;
    
    if(num == 4096)
        return doc->tmend3;
    
    if(l[num] >= doc->fsize)
        return 0;
    
    return l[num];
}

//Changesize********************************************

//GetBlocks#*************************************

void Getblocks(FDOC *doc, int b)
{
    unsigned char *rom;
    
    int a;
    
    ZBLOCKS *zbl = doc->blks + b;
    
    unsigned char *buf, *buf2;
    
    if(b > 0xe1 || b < 0)
        return;
    
    zbl->count++;
    
    if(zbl->count > 1)
        return;
    
    rom = doc->rom;
    
    // Check for specific tilesets that need speical handling.
    if(b == 0xe1)
    {
        buf = Unsnes(rom + 0x80000, 0x7000);
    }
    else if(b == 0xe0)
    {
        buf2 = GetBG3GFX(rom + 0x70000, 0x2000);
        buf = Unsnes(buf2, 0x2000);
        free(buf2);
    }
    else if(b == 0xdf)
    {
        buf = malloc(0x4000);
        memcpy(buf, rom + 0xc4000, 0x4000);
    }
    else
    {
        a = romaddr
        (
            (rom[0x4f80 + b] << 16)
          + (rom[0x505f + b] << 8)
          + rom[0x513e + b]
        );
        
        if(b >= 0x73 && b < 0x7f)
        {
            buf2 = Make4bpp(rom + a, 0x800);
            buf = Unsnes(buf2, 0x800);
        }
        else
        {
            buf = Uncompress(rom + a, 0, 0);
            
            if(b >= 220 || (b >= 113 && b < 115))
            {
                buf2 = GetBG3GFX(buf, 0x1000);
                
                free(buf);
                
                buf = Unsnes(buf2, 0x1000);
            }
            else
            {
                buf2 = Make4bpp(buf, 0x800);
                free(buf);
                
                buf = Unsnes(buf2, 0x800);
            }
        }
        
        free(buf2);
    }
    
    zbl->buf = buf;
    zbl->modf = 0;
}

//GetBlocks*****************************************

//Releaseblks#***************************************

void
Releaseblks(FDOC * const doc,
            int          b)
{
    ZBLOCKS *zbl = doc->blks + b;
    
    unsigned char *buf, *rom, *b2;
    
    int a,c,d,e,f;
    
    if(b > 225 || b < 0)
        return;
    
    zbl->count--;
    
    if(!zbl->count)
    {
        if(zbl->modf)
        {
            rom = doc->rom;
            
            if(b == 225)
            {
                b2 = Makesnes(zbl->buf, 0xe000);
                
                memcpy(rom + 0x80000, b2, 0x7000);
                free(b2);
                doc->modf = 1;
            }
            else if(b == 224)
            {
                b2 = Make2bpp(zbl->buf, 0x2000);
                memcpy(rom + 0x70000, b2, 0x1000);
                free(b2);
                doc->modf = 1;
            }
            else if(b == 223)
            {
                memcpy(rom + 0xc4000, zbl->buf, 0x4000);
                doc->modf = 1;
            }
            else
            {
                a = romaddr((rom[0x4f80 + b] << 16) + (rom[0x505f + b] << 8) + rom[0x513e + b]);
                
                if(b >= 0x73 && b < 0x7f)
                {
                    buf = Make3bpp(zbl->buf, 0x1000);
                    memcpy(rom + a, buf, 0x600);
                    doc->modf = 1;
                }
                else
                {
                    if( (b >= 220) || (b >= 113 && b < 115) )
                    {
                        buf = Make2bpp(zbl->buf, 0x1000);
                        b2 = Compress(buf, 0x800, &c, 0);
                    }
                    else
                    {
                        buf = Make3bpp(zbl->buf, 0x1000);
                        b2 = Compress(buf, 0x600, &c, 0);
                    }
                    
                    free(buf);
                    f = doc->gfxend;
                    
                    for(d = 0; d < 223; d++)
                    {
                        e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
                        if(e < f && e > a)
                            f = e;
                    }
                    
                    if(doc->gfxend - f + a + c > 0xc4000)
                    {
                        free(b2);
                        wsprintf(buffer, "Not enough space for blockset %d", b);
                        MessageBox(framewnd, buffer, "Bad error happened", MB_OK);
                        
                        goto done;
                    }
                    
                    for(d = 0; d < 223; d++)
                    {
                        e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
                        
                        if(e > a)
                        {
                            e = cpuaddr(e + a - f + c);
                            
                            rom[0x4f80 + d] = ( (e >> 16) & 0xff );
                            rom[0x505f + d] = ( (e >>  8) & 0xff );
                            rom[0x513e + d] = ( (e >>  0) & 0xff );
                        }
                    }
                    
                    memmove(doc->rom + a + c, doc->rom + f, doc->gfxend - f);
                    doc->gfxend += a - f + c;
                    memcpy(doc->rom + a, b2, c);
                    free(b2);
                    doc->modf = 1;
                }
            }
        }
done:
        free(zbl->buf);
        zbl->buf = 0;
    }
}

//Releaseblks*****************************************

//LoadPal#**********************************

void Loadpal(void *ed, unsigned char *rom, int start, int ofs, int len, int pals)
{
    int i;
    int j;
    int k;
    int l;
    
    short *a = (short*) (rom + romaddr(start));
    
    if( ( (DUNGEDIT*) ed)->hpal)
    {
        PALETTEENTRY pe[16];
        
        for(i = 0; i < pals; ++i)
        {
            for(j = 0; j < len; ++j)
            {
                l = *(a++);
                
                pe[j].peRed   = (BYTE) ((l & 0x1f) << 3);
                pe[j].peGreen = (BYTE) ((l & 0x3e0) >> 2);
                pe[j].peBlue  = (BYTE) ((l & 0x7c00) >> 7);
                pe[j].peFlags = 0;
            }
            
            SetPaletteEntries(( (DUNGEDIT*) ed)->hpal, (ofs + *(short*) (( (DUNGEDIT*) ed)->pal)) & 255, len, pe);
            
            ofs += 16;
        }
    }
    else
    {
        RGBQUAD *pal = ( (DUNGEDIT*) ed)->pal;
        
        for(i = 0; i < pals; ++i)
        {
            k = ofs;
            
            for(j = 0; j < len; ++j)
            {
                l = *(a++);
                
                pal[k] = HM_RgbFrom5bpc(l);
                
                ++k;
            }
            
            ofs += 16;
        }
    }
}

// =============================================================================

int
Savesprites(FDOC          * const doc,
            int                   num,
            unsigned char *       buf,
            int                   size)
{
    int i,k,l,m,n;
    int adr[0x288];
    unsigned char*rom=doc->rom;
    if(size) size++;
    for(i=0;i<0x160;i++)
        adr[i]=((short*)(rom + 0x4c881))[i] + 0x50000;
    k=doc->dungspr - 0x2c0;
    for(;i<0x288;i++)
        adr[i]=((short*)(rom+k))[i] + 0x50000;
    if(num&65536) {
        num&=65535;
        l=adr[num];
        for(i=0;i<0x288;i++) if(adr[i]==l) {
            adr[num]=0x4cb41;
            goto nochg;
        }
        size=0;
    } else l=adr[num];
    if(l==0x4cb41 || l==doc->sprend-2) m=l=((num>=0x160)?(doc->dungspr + 0x300):0x4cb42); else {
        m=(num>=0x160)?doc->sprend:doc->dungspr;
        for(i=0;i<0x288;i++) {
            n=adr[i];
            if(n<m && n>l) m=n;
        }
    }
    if(size==2 && !buf[0]) size=0;
    if(doc->sprend+l-m+size>0x4ec9f) {
        MessageBox(framewnd,"Not enough space for sprites","Bad error happened",MB_OK);
        return 1;
    }
    if(size+l != m) {
        memmove(rom+l+size,rom+m,doc->sprend-m);
        doc->sprend+=size-m+l;
        for(i=0;i<0x288;i++) if(adr[i]>=m) adr[i]+=size-m+l;
    }
    if(!size) adr[num]=((num>=0x160)?(doc->sprend-2):0x4cb41); else {
        memcpy(rom+l,buf,size-1);
        rom[l+size-1]=0xff;
        adr[num]=l;
    }
    if(doc->dungspr>=l)
        doc->dungspr+=size-m+l;
nochg:
    for(i=0;i<0x160;i++)
        ((short*)(rom + 0x4c881))[i]=adr[i];
    k=doc->dungspr - 0x2c0;
    for(;i<0x288;i++)
        ((short*)(rom+k))[i]=adr[i];
    return 0;
}

/*
#pragma code_seg("seg2")
#pragma data_seg("seg3")
BOOL (WINAPI*postmsgfunc)(HWND,UINT,WPARAM,LPARAM);
void CALLBACK testfunc(UINT timerid,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
    postmsgfunc(framewnd,4100,0,0);
}
#pragma code_seg()
#pragma data_seg()*/

// =============================================================================

HACCEL actab;

extern void
ProcessMessage(MSG * msg)
{
    RECT rc;
    SDCREATE *sdc;
    if(!TranslateMDISysAccel(clientwnd,msg)) {
        if(!TranslateAccelerator(framewnd,actab,msg)) {
            if(msg->message==WM_MOUSEMOVE) {
                GetWindowRect(msg->hwnd,&rc);
                mouse_x=LOWORD(msg->lParam)+rc.left;
                mouse_y=HIWORD(msg->lParam)+rc.top;     
            }
            for(sdc=firstdlg;sdc;sdc=sdc->next)
                if(IsDialogMessage(sdc->win,msg)) break;
            if(!sdc) {
                TranslateMessage(msg);
                DispatchMessage(msg);
            }
        }
    }
}

// =============================================================================

int Handlescroll(HWND win,int wparam,int sc,int page,int scdir,int size,int size2)
{
    SCROLLINFO si;
    int i=sc;
    
    int dx = 0;
    int dy = 0;
    
    switch(wparam&65535) {
    case SB_BOTTOM:
        i=size-page;
        break;
    case SB_TOP:
        i=0;
        break;
    case SB_LINEDOWN:
        i++;
        break;
    case SB_LINEUP:
        i--;
        break;
    case SB_PAGEDOWN:
        i+=page;
        break;
    case SB_PAGEUP:
        i-=page;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        i=wparam>>16;
        break;
    }
    
    if(i>size-page)
        i=size-page;
    
    if(i<0)
        i=0;
    
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    si.nPos = i;
    
    SetScrollInfo(win,scdir,&si,1);
    
    if(scdir == SB_VERT)
    {
        dy = (sc - i) * size2;
    }
    else
    {
        dx = (sc - i) * size2;
    }
    
    ScrollWindowEx(win, dx, dy, 0, 0, 0, 0, SW_INVALIDATE | SW_ERASE);
    
    return i;
}

char* sec_str[]={
    "Hole",
    "Warp",
    "Staircase",
    "Bombable",
    "Switch",
    "8A",
    "8C",
    "8E"
};

char const *
Getsecretstring(uint8_t const * const rom,
                int             const i)
{
    int a;
    
    if(i >= 128)
        return sec_str[ (i & 15) >> 1 ];
    else if(i == 4)
        return "Random";
    else if(i == 0)
        a = 0;
    else
        a = rom[0x301f3 + i];
    
    return a ? sprname[a] : "Nothing";
}

// =============================================================================

void
Drawblock(OVEREDIT const * const ed,
          int const x,
          int const y,
          int const n,
          int const t)
{
    register unsigned char *b1 = 0,
                           *b2 = 0,
                           *b3 = 0,
                           *b4 = 0;
    
    int a,
        b,
        c;
    
    // which chr (character / tile) to use.
    int d = (n & 0x03ff);
    
    unsigned e;
    
    const static char f[14] = {1,1,0,0,0,0,0,0,1,1,1,0,1,1};
    
    int col;
    int mask,tmask;
    
    b2 = (drawbuf + 992 + x - (y << 5));
    
    *(char*) &col = *(((char*)&col) + 1) = *(((char*)&col) + 2) = *(((char*)&col) + 3) = ((n & 0x1c00) >> 6);
    
    if((t & 24) == 24)
    {
        // \task Used with only with Link's graphics dialog,
        // seemingly. Need a name(s) for these bits.
        
        b3 = ed->ew.doc->blks[225].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + (n << 6);
        mask = 0xf0f0f0f;
        col = 0;
        b4 = b1 + 0xe000;
    }
    else if(t & 16)
    {
        // \task 2bpp graphics? Not sure.
        // Used with the dungeon map screen (not the maps themselves)
        // Need a name for this bit.
        
        if(d >= 0x180)
            goto noblock;
        
        b3 = ed->ew.doc->blks[bg3blkofs[d >> 7]].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + ((d & 0x7f) << 6);
        mask = 0x3030303;
        col >>= 2;
        
        if(n & 0x4000)
            b1 += 0x4000;
        
        b4 = b1 + 0x2000;
    }
    else if(t & 8)
    {
        // \task Used with a lot of the tilemap screens, title screen in particular.
        // Need a name for this bit.
        
        if(d >= 0x100)
            goto noblock;
        
        b3 = ed->ew.doc->blks[224].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + (d << 6);
        mask = 0x3030303;
        col >>= 2;
        
        if(n & 0x4000)
            b1 += 0x8000;
        
        b4 = b1 + 0x4000;
    }
    else if(d >= 0x200)
    {
        if(d < 0x240)
            goto noblock;
        else if(d < 0x280)
        {
            b3 = ed->ew.doc->blks[ed->blocksets[10]].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + ((d - 0x240) << 6);
            mask = 0xf0f0f0f;
        }
        else if(d < 0x300)
        {
            b3 = ed->ew.doc->blks[0x79 + ( (d - 0x280) >> 6)].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + (((d - 0x280) & 63) << 6);
            mask = 0x7070707;
        }
        else
        {
            e = ed->blocksets[(d >> 6) - 1];
            b3 = ed->ew.doc->blks[e].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + ((d & 0x3f) << 6);
            
            if(e >= 0xc5 && e < 0xd3)
                mask = f[e - 0xc5] ? 0xf0f0f0f : 0x7070707;
            else
                mask = 0x7070707;
        }
        
        if(n & 0x4000)
            b1 += 0x2000;
        
        b4 = b1 + 0x1000;
    }
    else if(ed->gfxtmp == 0xff)
    {
        mask = -1;
        col = 0;
        b1 = ed->ew.doc->blks[223].buf + (n << 6);
    }
    else
    {
        if(ed->gfxtmp >= 0x20)
            mask = palhalf[(n & 0x1c0) >> 6] ? 0xf0f0f0f : 0x7070707;
        else
            mask = (n & 0x100) ? 0x7070707 : 0xf0f0f0f;
        
        if(ed->anim != 3)
        {
            if(ed->gfxtmp >= 0x20)
            {
                e = d - 0x1c0;
                
                if(e < 0x20u)
                    d = (ed->anim << 5) + e + 0x200;
            }
            else
            {
                e = d - 0x1b0;
                if(e<0x20u) {
                    d=(ed->anim<<4)+e + 0x200;
                    if(e>=0x10) d+=0x30;
                }
            }
        }
        
        if(n & 0xff0000)
            b3 = ed->ew.doc->blks[(n >> 16) - 1].buf;
        else
            b3 = ed->ew.doc->blks[ed->blocksets[d >> 6]].buf;
        
        if(!b3)
        {
noblock:
            
            if(t & 1)
                return;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = 0;
                ((int*) b2)[1] = 0;
                
                b2 -= 32;
            }
            return;
        }
        
        b1 = b3 + ((d & 0x3f) << 6);
        
        if(n & 0x4000)
            b1 += 0x2000;
        
        b4 = b1 + 0x1000;
    }
    
    if(t & 4)
        col += 0x80808080;
    
    switch(t & 3)
    {
    
    case 0:
        
        switch(n & 0x8000)
        {
        
        case 0:
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2     = ((*(int*) b1)     & mask) + col;
                ((int*) b2)[1] = ((((int*) b1)[1]) & mask) + col;
                
                b2 -= 32;
                b1 += 8;
            }
            
            break;
        
        case 0x8000:
            
            b2 -= 224;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2     = ((*(int*) b1)     & mask) + col;
                ((int*) b2)[1] = ((((int*) b1)[1]) & mask) + col;
                b2 += 32;
                b1 += 8;
            }
            
            break;
        }
        
        break;
    
    case 1:
        
        switch(n & 0x8000)
        {
        case 0:
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0];
                c = ((int*) b4)[1];
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c;
                
                b2 -= 32;
                b1 += 8;
                b4 += 8;
            }
            
            break;
        
        case 0x8000:
            
            b2 -= 224;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0];
                c = ((int*) b4)[1];
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c;
                
                b2 += 32;
                b1 += 8;
                b4 += 8;
            }
            
            break;
        }
        
        break;
    
    case 2:
        
        switch(n & 0x8000)
        {
        
        case 0:
            
            tmask = 0xff00ff;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = (((*(int*) b1) & mask) + col) & tmask;
                ((int*) b2)[1] = (((((int*) b1)[1]) & mask) + col) & tmask;
                
                b2 -= 32;
                b1 += 8;
                tmask ^= -1;
            }
            
            break;
        
        case 0x8000:
            
            b2 -= 224;
            
            tmask = 0xff00ff00;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = (((*(int*) b1) & mask) + col) & tmask;
                ((int*) b2)[1] = (((((int*) b1)[1]) & mask) + col) & tmask;
                
                b2 += 32;
                b1 += 8;
                
                tmask ^= -1;
            }
            
            break;
        }
        
        break;
    
    case 3:
        
        switch(n & 0x8000)
        {
        case 0:
            
            tmask = 0xff00ff;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0] | ~tmask;
                c = ((int*) b4)[1] | ~tmask;
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b & tmask;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c & tmask;
                
                b2 -= 32;
                b1 += 8;
                b4 += 8;
                
                tmask ^= -1;
            }
            
            break;
        
        case 0x8000:
            
            tmask = 0xff00ff00;
            
            b2 -= 224;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0] | ~tmask;
                c = ((int*) b4)[1] | ~tmask;
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b & tmask;
                ((int*) b2)[1] |= (((*(int*) (b1 + 4)) & mask) + col) & ~c & tmask;
                
                b2 += 32;
                b1 += 8;
                b4 += 8;
                
                tmask ^= -1;
            }
            
            break;
        }
        
        break;
    }
}

//Drawblock*********************************

void Paintblocks(RECT*rc,HDC hdc,int x,int y,DUNGEDIT*ed)
{
    int a,
        b,
        c,
        d;
    
    if(gbtnt)
    {
        // \note Doesn't seem to be accessible normally as there is currently no
        // menu entry to toggle the above flag.
        
        c = 0, d = 0;
        
        if(x < rc->left)
            c = rc->left - x, x = rc->left;
        
        if(y < rc->top)
            d = rc->top - y, y = rc->top;
        
        a = 32 - c;
        b = 32 - d;
        
        if(x + a > rc->right)
            a = rc->right - x;
        
        if(y + b>rc->bottom)
            b = rc->bottom - y;
        
        if(a < 1 || b < 1 || c > 31 || d > 31)
            return;
        
        SetDIBitsToDevice(hdc,
                          x, y, a, b, 0, 0, 0, 32,
                          drawbuf + c + ( (d + b - 32) << 5),
                          (BITMAPINFO*) &(ed->bmih),
                          DIB_RGB_COLORS);
    }
    else
    {
        SetDIBitsToDevice(hdc,
                          x, y,
                          32, 32,
                          0, 0, 0, 32,
                          drawbuf,
                          (BITMAPINFO*)&(ed->bmih),
                          ed->hpal ? DIB_PAL_COLORS : DIB_RGB_COLORS);
    }
}

// =============================================================================

void
DrawDungeonMap8(DUNGEDIT const * const p_ed,
                int              const p_x,
                int              const p_y,
                int              const p_offset,
                uint16_t const * const p_buf)
{
    int const r = p_x << 3;
    int const s = p_y >> 3;
    int const t = p_offset + p_x + p_y;
    
    int u = 0;
    
    // Fake overworld editor. Type punning going on here.
    OVEREDIT const * const fake_oed = (OVEREDIT*) p_ed;
    
    // -----------------------------
    
    if(p_ed->layering & 2)
    {
        // only used by special tilemap modes (title screen)
        
        Drawblock(fake_oed, r, s, p_buf[t + 0x2000], 8);
        
        u = 1;
    }
    
    switch(p_ed->layering >> 5)
    {
    
    case 0:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            Drawblock(fake_oed, r, s, p_buf[t], u);
        }
            
        break;
    
    case 1:
    case 5:
    case 6:
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            Drawblock(fake_oed, r, s, p_buf[t + 0x1000], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            Drawblock(fake_oed,r,s, p_buf[t],u);
        }
        
        break;
    
    case 2:
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            Drawblock(fake_oed, r, s, p_buf[t + 0x1000], u + 2);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            Drawblock(fake_oed, r, s, p_buf[t], u);
        }
        
        break;
    
    case 3:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            Drawblock(fake_oed, r, s, p_buf[t], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            Drawblock(fake_oed, r, s, p_buf[t + 0x1000], u);
        }
        
        break;
    
    case 4:
    case 7:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            Drawblock(fake_oed, r, s, p_buf[t], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            Drawblock(fake_oed, r, s, p_buf[t + 0x1000], u + 2);
        }
        
        break;
    }
    
    if(p_ed->layering & 4)
    {
        Drawblock(fake_oed, r, s, p_buf[t + 0x2000], 17);
    }
}

// =============================================================================

void
DrawDungeon32x32(DUNGEDIT const * const p_ed,
                 int              const i,
                 int              const j,
                 int              const n,
                 int              const o,
                 uint16_t const * const p_buf)
{
    // 
    int const m = ( (i + n) >> 3)
                + ( (j + o) << 3);
    
    int p = 0;
    
    // -----------------------------
    
    for(p = 0; p < 4; p++)
    {
        int q = 0;
        
        for(q = 0; q < 256; q += 64)
        {
            DrawDungeonMap8(p_ed,
                            p,
                            q,
                            m,
                            p_buf);
        }
    }
}

// =============================================================================

void
Paintdungeon(DUNGEDIT * const ed,
             HDC hdc,
             RECT *rc,
             int x,int y,
             int k,int l,
             int n,int o,
             unsigned short const *buf)
{
    // loop variable that represents the y coordinate in the tilemap.
    int j = 0;
    
    // Seems to be nonzero if one of the backgrounds is disabled or
    // full of blackness.
    int v;
    
    HGDIOBJ oldobj = 0;
    
    // -----------------------------
    
    if
    (
        ( ! (ed->disp & SD_DungShowBothBGs))
     || ( ( ! (ed->disp & SD_DungShowBG1) ) && ! (ed->layering >> 5) )
    )
    {
        oldobj = SelectObject(hdc, black_brush);
        
        v = 1;
    }
    else
    {
        v = 0;
    }
    
    for(j = y; j < l; j += 32)
    {
        // loop variable that represents the current x coordinate in the tilemap.
        // (aligned to a 32 pixel grid though).
        int i = x;
        
        // -----------------------------
        
        for( ; i < k; i += 32)
        {
            if(v)
            {
                Rectangle(hdc, i, j, i + 32, j + 32);
            }
            else
            {
                DrawDungeon32x32(ed, i, j, n, o, buf);
                
                Paintblocks(rc, hdc, i, j, ed);
            }
        }
    }
    
    if(oldobj)
    {
        SelectObject(hdc, oldobj);
    }
}

// =============================================================================

void
Setpalette(HWND const win, HPALETTE const pal)
{
    HDC const hdc = GetDC(win);
    
    HPALETTE const oldpal = SelectPalette(hdc, pal, 0);
    
    RealizePalette(hdc);
    
    SelectPalette(hdc, oldpal, 1);
    
    ReleaseDC(win, hdc);
    
    return;
}

void Updateblk8sel(BLOCKSEL8 *ed, int num)
{
    int i=num+(ed->scroll<<4);
    int j,k,l,m;
    RECT rc;
    rc.left=0;
    rc.right=8;
    rc.top=0;
    rc.bottom=8;
    if(wver) i^=0x8000;
    Drawblock(ed->ed,0,24,i^ed->flags,((i&0x200)>>7)+ed->dfl);
    j=(i&15)*ed->w>>4;
    k=(i&240)*ed->h>>8;
    l=(((i&15)+1)*ed->w>>4)-j;
    m=(((i&240)+16)*ed->h>>8)-k;
    StretchDIBits(ed->bufdc,j,k,l,m,0,0,wver?33:8,wver ? -8 : 8,drawbuf,(BITMAPINFO*)&(ed->ed->bmih),ed->ed->hpal?DIB_PAL_COLORS:DIB_RGB_COLORS,SRCCOPY);
    l--;
    m--;
//  MoveToEx(ed->bufdc,j+l,k,0);
//  LineTo(ed->bufdc,j+l,k+m);
//  LineTo(ed->bufdc,j-1,k+m);
}

DUNGEDIT*dunged;

void InitBlksel8(HWND hc,BLOCKSEL8*bs,HPALETTE hpal,HDC hdc)
{
    int i;
    
    RECT rc = HM_GetClientRect(hc);
    
    HPALETTE oldpal;
    
    bs->sel=0;
    bs->scroll=0;
    bs->flags=0;
    bs->dfl=0;
    bs->w=rc.right;
    bs->h=rc.bottom;
    bs->bufdc=CreateCompatibleDC(hdc);
    bs->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
    
    SelectObject(bs->bufdc,bs->bufbmp);
    SelectObject(bs->bufdc,white_pen);
    SelectObject(bs->bufdc,black_brush);
    oldpal=SelectPalette(objdc,hpal,1);
    SelectPalette(bs->bufdc,hpal,1);
    Rectangle(bs->bufdc,0,0,bs->w,bs->h);
    
    for(i=0;i<256;i++) Updateblk8sel(bs,i);
    
    SelectPalette(objdc,oldpal,1);
    SelectPalette(bs->bufdc,oldpal,1);
    
    SetWindowLong(hc, GWLP_USERDATA, (LONG_PTR) bs);
    
    Updatesize(hc);
}

// =============================================================================

void Changeblk8sel(HWND win,BLOCKSEL8*ed)
{
    RECT rc2;
    int i=ed->sel-(ed->scroll<<4);
    if(i>=0 && i<256) {
        rc2.left=(i&15)*ed->w>>4;
        rc2.right=rc2.left+(ed->w>>4);
        rc2.top=(i&240)*ed->h>>8;
        rc2.bottom=rc2.top+(ed->h>>4);
        InvalidateRect(win,&rc2,0);
    }
}

// =============================================================================

BOOL CALLBACK choosesprite(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    HWND hc;
    int i,j,k;
    switch(msg) {
    case WM_INITDIALOG:
        hc=GetDlgItem(win,IDC_LIST1);
        j=(lparam&512)?0x11c:256;
        for(i=0;i<j;i++) {
            SendMessage(hc,LB_SETITEMDATA,k=SendMessage(hc,LB_ADDSTRING,0,(long)sprname[i]),i);
            if(i==(lparam&511)) SendMessage(hc,LB_SETCURSEL,k,0);
        }
        SendMessage(hc,LB_SETTOPINDEX,SendMessage(hc,LB_GETCURSEL,0,0),0);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            hc=GetDlgItem(win,IDC_LIST1);
            EndDialog(win,SendMessage(hc,LB_GETITEMDATA,SendMessage(hc,LB_GETCURSEL,0,0),0));
            break;
        case IDCANCEL:
            EndDialog(win,-1);
            break;
        }
    }
    return FALSE;
}

BOOL CALLBACK getnumber(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j;
    if(msg==WM_INITDIALOG) {
        SetWindowLong(win,DWL_USER,lparam);
        SetWindowText(win,((LPSTR*)lparam)[1]);
        SetDlgItemText(win,IDC_STATIC2,((LPSTR*)lparam)[2]);
        SetFocus(GetDlgItem(win,IDC_EDIT1));
    } else if(msg==WM_COMMAND) if(wparam==IDOK) {
        i=GetDlgItemInt(win,IDC_EDIT1,0,0);
        j=*(int*)GetWindowLong(win,DWL_USER);
        if(i<0 || i>j) {
            wsprintf(buffer,"Please enter a number between 0 and %d",j);
            MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
            return FALSE;
        }
        EndDialog(win,i);
    } else if(wparam==IDCANCEL) EndDialog(win,-1);
    return FALSE;
}

int __stdcall askinteger(int max,char*caption,char*text)
{
    (void) caption, text;
    
    return ShowDialog(hinstance,(LPSTR)IDD_DIALOG4,framewnd,getnumber,(int)&max);
}

// =============================================================================

void
Setpalmode(DUNGEDIT *ed)
{
    int i,
        j,
        k,
        t = ed->paltype;
    
    struct
    {
        short blah1, blah2;
        int pal[256];
    } palstr;
    
    palstr.blah1 = 0x300;
    palstr.blah2 = palt_sz[t];
    
    k = palt_st[t];
    
    for(i = 0; i < palstr.blah2; i++)
    {
        j = *(int*)(ed->pal + i + k);
        palstr.pal[i] = ((j & 0xff) << 16) | ( ( j & 0xff0000) >> 16) | (j & 0xff00);
    }
    
    ed->hpal = CreatePalette((LOGPALETTE*)&palstr);
    
    for(i = 0; i < 256; i++)
        ((short*)(&(ed->pal)))[i] = (i - k) & 255;
}

// =============================================================================

void
Setfullmode(DUNGEDIT *ed)
{
    int pal[256];
    
    int i,
        j,
        k = ed->paltype,
        l;
    
    l = i = palt_st[k];
    j = palt_sz[k];
    
    GetPaletteEntries(ed->hpal, i, j, (PALETTEENTRY*) pal);
    
    for( ; i < j; i++)
    {
        k = pal[i];
        
        *(int*) (ed->pal + i + l) = (k >> 16) | ((k & 0xff) << 16) | (k & 0xff0000);
    }
    
    DeleteObject(ed->hpal);
    
    ed->hpal = 0;
}

//Setfullmode*******************************

//Addgraphwin#*******************************

void Addgraphwin(DUNGEDIT *ed, int t)
{
    ed->prevgraph = lastgraph;
    
    if(lastgraph)
        ((DUNGEDIT*) lastgraph)->nextgraph = ed;
    else
        firstgraph = ed;
    
    lastgraph = ed;
    
    ed->nextgraph = 0;
    ed->paltype = t;
    
    if(palmode)
        Setpalmode(ed);
}

//Addgraphwin********************************

void Delgraphwin(DUNGEDIT*ed)
{
    if(ed->hpal)
        DeleteObject(ed->hpal);
    
    if(dispwnd == ed)
        dispwnd = ed->prevgraph;
    
    if(ed->prevgraph)
        ((DUNGEDIT*)ed->prevgraph)->nextgraph = ed->nextgraph;
    else
        firstgraph = ed->nextgraph;
    
    if(ed->nextgraph)
        ((DUNGEDIT*)ed->nextgraph)->prevgraph = ed->prevgraph;
    else
        lastgraph = ed->prevgraph;
    
    if(!dispwnd)
        dispwnd = lastgraph;
}

void
Updpal(void*ed)
{
    if(ed == dispwnd && ( (DUNGEDIT*) ed)->hpal)
        Setpalette(framewnd, ( (DUNGEDIT*) ed)->hpal);
    else
        SendMessage(( (DUNGEDIT*) ed)->dlg, 4002, 0, 0);
}

void Setdispwin(DUNGEDIT *ed)
{
    if(ed != dispwnd)
    {
        dispwnd = ed;
        Setpalette(framewnd, ed->hpal);
    }
}

// =============================================================================

void
LoadOverlays(FDOC * const doc)
{
    int i,k,l,m,bd,bs,o,p;
    
    unsigned short *b;
    unsigned char *rom = doc->rom;
    
    if(doc->o_loaded)
        return;
    
    b = malloc(1024);
    bd = 129;
    bs = 512;
    
    *(int*)doc->o_enb = *(int*)(doc->o_enb+4)
                      = *(int*)(doc->o_enb+8)
                      = *(int*)(doc->o_enb+12)
                      = 0;
    
    for(m = 0; m < 128; m++)
    {
        i=0x78000 + ((short*)(rom + 0x77664))[m];
        b[m]=bd;
        
        if(i >= doc->oolend)
            continue;
        
        for(o = 0; o < m; o++)
        {
            if(0x78000 + ((short*) (rom + 0x77664))[o] == i)
            {
                b[m] = b[o];
                
                goto okovl;
            }
        }
        
        k = -1;
        p = -1;
        
        for(;;)
        {
            if(bd > bs - 4)
            {
                bs += 512;
                b = realloc(b, bs << 1);
            }
            
            switch( rom[i++] )
            {
            case 0x1a:
                k++;
                
                break;
            
            case 0x4c:
                i = 0x78000 + *(short*) (rom + i);
                
                break;
            
            case 0x60:
                goto okovl;
            
            case 0x8d:
                l = *(short*) (rom + i);
                i += 2;
settile:
                
                if(k != p)
                {
                    if(p >= 0)
                        b[bd++] = 0xffff;
                    
                    p = k;
                    b[bd++] = k;
                }
                
                switch(l >> 13)
                {
                
                case 1:
                    b[bd++] = (l & 8191) >> 1;
                    
                    break;
                
                case 2:
                    b[bd++] = ((l & 2047) >> 1) | 4096;
                    
                    break;
                
                default:
                    goto badovl;
                }
                
                break;
            
            case 0x9d:
                l = *(short*)(rom+i)+o;
                i += 2;
                
                goto settile;
            
            case 0x8f:
                l = *(short*)(rom+i);
                
                if(rom[i + 2] != 126)
                    goto badovl;
                
                i += 3;
                
                goto settile;
            
            case 0x9f:
                l = *(short*) (rom + i) + o;
                
                if(rom[i + 2] != 126)
                    goto badovl;
                
                i += 3;
                
                goto settile;
            
            case 0xa2:
                o = *(short*)(rom+i);
                i += 2;
                
                break;
            
            case 0xa9:
                k = *(short*) (rom+i);
                i += 2;
                
                break;
            
            case 0xea:
                break;
            
            default:
badovl:
                free(b);
                doc->o_loaded=2;
                return;
            }
            
            if(bd > b[m] + 0x2800)
                goto badovl;
        }
        
        goto badovl;
okovl:
        b[bd++] = 0xfffe;
        doc->o_enb[m >> 3] |= 1 << (m & 7);
    }
    
    b = realloc(b, bd << 1);
    
    doc->ovlbuf = b;
    b[128] = bd;
    doc->o_loaded = 1;
}

// =============================================================================

void
SaveOverlays(FDOC * const doc)
{
    int i, // counter variable
        j = 0,
        k, // another counter variable
        l,
        m,
        n,
        o,
        p,
        q = 0,
        r = 0;
    
    unsigned short *b = doc->ovlbuf;
    unsigned char *rom = doc->rom;
    
    char buf[4096];
    
    // if the overlays have not been loaded, exit the routine. 
    if(doc->o_loaded != 1)
        return;
    
    m = 0;
    o = 0;
    
    for(i = 0; i < 128; i++)
    {
        for(k = 0; k < i; k++)
        {
            // if b[k] is the same as the last one in the list (b[i]), copy it in the rom
            if(b[k] == b[i])
            {
                ( (short*) ( rom + 0x77664 ) )[i] = ((short*) ( rom + 0x77664 ))[k];
                
                // skip to the next overlay. i.e. increment i.
                goto nextovl;
            }
        }
        
        // if none of them match, take i and make k into the value of b[i].
        k = b[i];
        
        ( (short*) ( rom + 0x77664 ) )[i] = j + 0xf764;
        
        if( !( doc->o_enb[i >> 3] & (1 << (i & 7) ) ))
            // beats me. seems like loop around if certain overlays are not enabled.
            continue;
        
        // convert m to the next lowest even integer. 
        if(b[k] == 0xfffe && m)
        {
            ((short*) ( rom + 0x77664 ))[i] = m + 0xf864;
            
            continue;
        }
        
        p = 0;
        
        k = b[i];
        
        n = 0xfffe;
        
        for(;;)
        {
            l = b[k++];
            
            if(l >= 0xfffe)
                break;
            
            if(l == 0x918)
                q = j;
            
            if(l - n == 1)
            {
                if(n == p + 0x918)
                    p++;
                else
                    p = 0;
                
                buf[j++] = 0x1a;
            }
            else
            {
                p = 0;
                
                if(l - n == 2)
                {
                    *(short*) (buf + j) = 0x1a1a;
                    j += 2;
                }
                else
                {
                    buf[j] = 0xa9;
                    
                    *(short*) (buf + j + 1) = l;
                    
                    j += 3;
                }
            }
            
            n = l;
            
            for(;;)
            {
                if(j >= 0x89c)
                {
error:
                    MessageBox(framewnd,"Not enough room for overlays","Bad error happened",MB_OK);
                    return;
                }
                
                l = b[k++];
                
                if(l >= 0xfffe)
                    break;
                
                if(!p)
                    r = l;
                else if( map16ofs[p] != l - r)
                    p = 0;
                
                buf[j] = 0x8d;
                
                *(short*) (buf + j + 1) = (l << 1) + 0x2000;
                j += 3;
            }
            
            if(l == 0xfffe)
                break;
        }
        
        if(p == 3)
        {
            j = q;
            buf[j] = 0xa2;
            
            *(short*) (buf + j + 1) = r << 1;
            
            if(o)
            {
                buf[j + 3] = 0x4c;
                
                *(short*) (buf + j + 4) = o + 0xf764;
                j += 6;
            }
            else
            {
                j += 3;
                o = j;
                
                *(int*) (buf + j) = 0x9d0918a9;
                *(int*) (buf + j + 4) = 0x9d1a2000;
                *(int*) (buf + j + 8) = 0x9d1a2002;
                *(int*) (buf + j + 12) = 0x9d1a2080;
                *(int*) (buf + j + 16) = 0x602082;
                
                j += 19;
            }
        }
        else
            buf[j++] = 0x60;
        
        if(!m)
            m = j;
        
        if(j >= 0x89c)
            goto error;
        
nextovl:;
    }
    
    memcpy(rom + 0x77764, buf, j);
    
    doc->oolend = j + 0x77764;
    doc->o_modf = 0;
}

// =============================================================================

void Unloadovl(FDOC *doc)
{
    // if the overlays are loaded, free the overlay buffer.
    if(doc->o_loaded == 1)
        free(doc->ovlbuf);
    
    // the overlays are regarded as not being loaded. 
    doc->o_loaded = 0;
}

// =============================================================================

int
loadoverovl(FDOC *doc, uint16_t *buf, int mapnum)
{
    unsigned short*b;
    int i,k,l;
    LoadOverlays(doc);
    if(doc->o_loaded!=1) return 0;
    b=doc->ovlbuf;
    memset(buf,-1,0x2800);
    if(!(doc->o_enb[mapnum>>3]&(1<<(mapnum&7)))) return 0;
    i=b[mapnum];
    for(;;) {
        k=b[i++];
        if(k>=0xfffe) break;
        for(;;) {
            l=b[i++];
            if(l>=0xfffe) break;
            buf[l]=k;
        }
        if(l==0xfffe) break;
    }
    return 1;
}

// =============================================================================

void
Changeblk16sel(HWND win, BLOCKSEL16*ed)
{
    RECT rc = HM_GetClientRect(win);
    RECT rc2;
    
    int i=ed->sel-(ed->scroll<<4);
    
    rc2.left=(rc.right>>1)-64+((i&7)<<4);
    rc2.right=rc2.left+16;
    rc2.top=(i&0xfff8)<<1;
    rc2.bottom=rc2.top+16;
    InvalidateRect(win,&rc2,0);
}

void SetBS16(BLOCKSEL16*bs,int i,HWND hc)
{
    int j;
    if(i<0) i=0;
    if(i>0xea7) i=0xea7;
    j=bs->scroll<<4;
    if(i<j) j=i;
    if(i>=j+(bs->page<<4)) j=i-(bs->page<<4)+16;
    SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
    Changeblk16sel(hc,bs);
    bs->sel=i;
    Changeblk16sel(hc,bs);
}

int Editblocks(OVEREDIT *ed, int num, HWND win)
{
    int i;
    
    // First slot is for the editor pointer, second is for the blockset to
    // edit.
    LPARAM x[2];
    
    // -----------------------------
    
    // \task Pointer problem on 64-bit (and just generally)
    x[0] = (LPARAM) ed;
    
    if(num == 17)
    {
        num = askinteger(219,
                         "Edit blocks",
                         "Which blocks?");
        
        if(num == -1)
            return 0;
        
        Getblocks(ed->ew.doc, num);
        
        num += 32;
    }
    
    x[1] = num;
    
    i = ShowDialog(hinstance,(LPSTR)IDD_DIALOG16,win,blockdlgproc, (LPARAM) x);
    
    if(num >= 32 && num < 256)
        Releaseblks(ed->ew.doc, num - 32);
    
    return i;
}

// =============================================================================

TEXTMETRIC textmetric;

// =============================================================================

// \note More re-entrant version of another function in that it
// doesn't write using a global text buffer.
void
PaintSprName(HDC p_dc,
             int x,
             int y,
             RECT const * const p_clip,
             char const * const p_name)
{
    size_t len = strlen(p_name);
    
    // -----------------------------
    
    // Probably not strictly necessary, but the program doesn't make a point
    // to set it anywhere more general.
    SetTextAlign(p_dc, TA_LEFT | TA_TOP);
    
    SetTextColor(p_dc, 0);
    
    ExtTextOut(p_dc, x + 1, y + 1, ETO_CLIPPED, p_clip, p_name, len, NULL);
    ExtTextOut(p_dc, x - 1, y - 1, ETO_CLIPPED, p_clip, p_name, len, NULL);
    ExtTextOut(p_dc, x + 1, y - 1, ETO_CLIPPED, p_clip, p_name, len, NULL);
    ExtTextOut(p_dc, x - 1, y + 1, ETO_CLIPPED, p_clip, p_name, len, NULL);
    
#if 0
    SetTextColor(p_dc, 0xffbf3f);
#else
    SetTextColor(p_dc, 0xfefefe);
#endif
    
    ExtTextOut(p_dc,x, y, ETO_CLIPPED, p_clip, p_name, len, NULL);
}

// =============================================================================

void
Paintspr(HDC const p_dc,
         int const p_x,
         int const p_y,
         int const p_hscroll,
         int const p_vscroll,
         size_t const p_window_size)
{
    size_t const len = strlen(buffer);
    
    size_t final_len = (signed) len;
    
    // -----------------------------
    
    // \task Small nitpick, but "secrets" drawn at the far right
    // edge of the screen can leave a half circle artifact.
    // also the very bottom of the screen. This is owing to their
    // being 8 pixel aligned. Certainly fixable but will have to look
    // a bit in depth.
    
    // Probably not strictly necessary, but the program doesn't make a point
    // to set it anywhere more general.
    SetTextAlign(p_dc, TA_LEFT | TA_TOP);
    
    if( (len * textmetric.tmAveCharWidth) + p_x > (p_window_size - p_hscroll) )
        final_len = (p_window_size - p_hscroll - p_x) / textmetric.tmAveCharWidth;
    
    {
        char text_buf[0x100];
        
        sprintf(text_buf, "x = %d, y = %d, n = %d, o = %d, w = %d",
                p_x, p_y, p_hscroll, p_vscroll, p_window_size);
        
        SetDlgItemText(debug_window, IDC_STATIC2, text_buf);
    }
    
    if(final_len > len)
    {
        final_len = len;
    }
    
    SetTextColor(p_dc, 0);
    
    TextOut(p_dc, p_x + 1, p_y + 1, buffer, final_len);
    TextOut(p_dc, p_x - 1, p_y - 1, buffer, final_len);
    TextOut(p_dc, p_x + 1, p_y - 1, buffer, final_len);
    TextOut(p_dc, p_x - 1, p_y + 1, buffer, final_len);
    
#if 0
    SetTextColor(p_dc, 0xffbf3f);
#else
    SetTextColor(p_dc, 0xfefefe);
#endif
    
    TextOut(p_dc, p_x, p_y, buffer, final_len);
}

void Savepersp(PERSPEDIT*ed)
{
    memcpy(ed->ew.doc->rom + 0x4ff8c,ed->buf,ed->len);
    ed->ew.doc->modf=1;
}

void Savedungmap(LMAPEDIT*ed)
{
    int i,j,k,l,m,n,o,p;
    unsigned char*rom=ed->ew.doc->rom;
    m=*(short*)(rom + 0x56640) + 0x58000;
    o=ed->ew.doc->dmend;
    if(ed->ew.param<13) i=((short*)(rom + 0x57605))[ed->ew.param+1] + 0x58000;
    else i=m;
    j=((short*)(rom + 0x57605))[ed->ew.param] + 0x58000;
    if(ed->ew.param<13) k=((short*)(rom+m))[ed->ew.param+1] + 0x58000;
    else k=o;
    n=(ed->basements+ed->floors)*25;
    l=((short*)(rom+m))[ed->ew.param] + 0x58000;
    if(j+n-i+l+ed->len-k+o>0x57ce0) {
        MessageBox(framewnd,"Not enough space","Bad error happened",MB_OK);
        return;
    }
    memmove(rom+j+n,rom+i,o-i);
    memcpy(rom+j,ed->rbuf,n);
    for(p=ed->ew.param+1;p<14;p++)
        ((short*)(rom + 0x57605))[p]+=j+n-i;
    o+=j+n-i;
    k+=j+n-i;
    l+=j+n-i;
    m+=j+n-i;
    memmove(rom+l+ed->len,rom+k,o-k);
    memcpy(rom+l,ed->buf,ed->len);
    for(p=0;p<14;p++)
        ((short*)(rom+m))[p]+=j+n-i;
    for(p=ed->ew.param+1;p<14;p++)
        ((short*)(rom+m))[p]+=l+ed->len-k;
    o+=l+ed->len-k;
    *(short*)(rom + 0x56640)=m - 0x8000;
    ed->ew.doc->dmend=o;
    rom[0x56196 + ed->ew.param]=ed->level;
    ((short*)(rom + 0x56807))[ed->ew.param]=ed->bossroom;
    ((short*)(rom + 0x56e79))[ed->ew.param]=(ed->bossroom==15)?-1:(ed->bosspos/25-ed->basements);
    ((short*)(rom + 0x56e5d))[ed->ew.param]=ed->bossofs;
    ((short*)(rom + 0x575d9))[ed->ew.param]=ed->basements+(ed->floors<<4)+(ed->bg<<8);
    ed->ew.doc->modf=1;
}

// =============================================================================

void Drawdot(HDC hdc,RECT*rc,int q,int n,int o)
{
    rc->right=rc->left+16;
    rc->bottom=rc->top+16;
    if(rc->right>q-n) rc->right=q-n;
    if(rc->bottom>q-o) rc->bottom=q-o;
    Ellipse(hdc,rc->left,rc->top,rc->right,rc->bottom);
}


// \note Unused. Looks like this was for tile properties.
char*terrain_str[]={
    "Land",
    "Wall",
    "Obstacle",
    "03",
    "Grass",
    "05",
    "06",
    "07",
    "Water",
    "Puddle",
    "0A",
    "0B",
    "0C",
    "Painful",
    "Slippery",
    "Ice",
    "NW wall",
    "NE wall",
    "SW wall",
    "SE wall",
    "14",
    "15",
    "16",
    "17",
    "NW inner wall",
    "NE inner wall",
    "SW inner wall",
    "SE inner wall",
    "1C",
    "Stairs1D",
    "Stairs1E",
    "Stairs1F",
    "Hole",
    "21",
    "Stairs",
    "Stairs23",
    "23",
    "24",
    "25",
    "26",
    "Sticky",
    "Jump N",
    "Jump S",
    "Jump W",
    "Jump E",
    "Jump NW",
    "Jump SW",
    "Jump NE",
    "Jump SE",
    "30",
    "31",
    "32",
    "33",
    "34",
    "35",
    "36",
    "37",
    "38",
    "39",
    "3A",
    "3B",
    "3C",
    "3D",
    "3E",
    "3F",
    "Grass",
    "41",
    "42",
    "Flamerod",
    "Shock",
    "45",
    "46",
    "47",
    "Diggable",
    "49",
    "4A",
    "Warp",
};

// =============================================================================

SD_ENTRY mus_sd[]={
    {"LISTBOX","",0,0,100,60,3000,WS_VISIBLE|WS_CHILD|LBS_NOTIFY|WS_VSCROLL,WS_EX_CLIENTEDGE,8},
    {"LISTBOX","",115,0,100,60,3009,WS_VISIBLE|WS_CHILD|LBS_NOTIFY|WS_VSCROLL,WS_EX_CLIENTEDGE,8},
    {"BUTTON","1",230,5,20,20,3001,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","2",230,30,20,20,3002,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","3",230,55,20,20,3003,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","4",230,80,20,20,3004,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","5",230,105,20,20,3005,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","6",230,130,20,20,3006,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","7",230,155,20,20,3007,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","8",230,180,20,20,3008,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Loop",217,205,43,20,3026,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Play",0,50,60,20,3010,WS_VISIBLE|WS_CHILD,0,12},
    {"BUTTON","Stop",70,50,60,20,3011,WS_VISIBLE|WS_CHILD,0,12},
    {"EDIT","",260,5,40,20,3012,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,30,40,20,3013,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,55,40,20,3014,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,80,40,20,3015,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,105,40,20,3016,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,130,40,20,3017,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,155,40,20,3018,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"EDIT","",260,180,40,20,3019,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Copy",310,5,45,20,3020,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Paste",360,5,45,20,3021,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","NewPart",310,30,45,20,3022,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","DelPart",360,30,45,20,3023,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","New Tr.",310,55,45,20,3024,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","N.Song",360,55,45,20,3027,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","DelSong",310,80,45,20,3028,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",260,205,40,20,3025,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0}
};

SD_ENTRY track_sd[]={
    {"TRAX0R","",0,0,0,60,3000,WS_VISIBLE|WS_CHILD|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"EDIT","",40,24,40,4,3001,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Bank",0,24,35,4,3002,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12}
};

SD_ENTRY wmap_sd[]={
    {"WMAPDISP","",0,50,266,0,3000,WS_VISIBLE|WS_CHILD|WS_HSCROLL|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"BLKSEL8","",256,0,0,256,3001,WS_VISIBLE|WS_CHILD,WS_EX_CLIENTEDGE,3},
    {"BUTTON","Draw",0,0,60,20,3002,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_GROUP,0,0},
    {"BUTTON","Select",64,0,60,20,3003,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"BUTTON","Rectangle",128,0,60,20,3004,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"BUTTON","Move",192,0,60,20,3006,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"BUTTON","Undo",256,0,60,20,3007,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Layout",256,24,60,20,3008,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"COMBOBOX","",0,24,150,100,3005,WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL,0,0}
};
SD_ENTRY lmap_sd[]={
    {"STATIC","Level #:",0,24,60,20,3003,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",64,24,60,20,3002,WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"BUTTON","",128,0,20,20,3006,WS_VISIBLE|WS_CHILD|BS_BITMAP,0,0},
    {"BUTTON","",128,24,20,20,3007,WS_VISIBLE|WS_CHILD|BS_BITMAP,0,0},
    {"BUTTON","Mountain",0,48,76,20,3008,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Water",80,48,60,20,3009,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"LMAPDISP","",0,72,116,100,3010,WS_VISIBLE|WS_CHILD,WS_EX_CLIENTEDGE,0},
    {"LMAPBLKS","",120,72,0,0,3004,WS_VISIBLE|WS_CHILD|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Grid",160,0,60,20,3005,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Graphic",160,24,60,20,3011,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|WS_GROUP,0,0},
    {"BUTTON","Room",160,48,60,20,3012,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Del floor",0,180,80,20,3013,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Insert above",0,205,80,20,3016,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Insert below",0,230,80,20,3017,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Boss",0,0,60,20,3000,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0}
};
SD_ENTRY tmap_sd[]={
    {"TMAPDISP","",0,70,276,0,3000,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,10},
    {"BLKSEL8","",272,0,0,256,3001,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_VSCROLL,WS_EX_CLIENTEDGE,3},
    {"BUTTON","BG1",0,0,40,16,3002,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|WS_GROUP|WS_TABSTOP,0,0},
    {"BUTTON","BG2",0,20,40,16,3003,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|WS_TABSTOP,0,0},
    {"BUTTON","BG3",0,40,40,16,3006,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|WS_TABSTOP,0,0},
    {"BUTTON","Move",44,0,40,16,3004,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_GROUP|WS_TABSTOP,0,0},
    {"BUTTON","Draw",44,20,40,16,3005,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_TABSTOP,0,0},
    {"STATIC","Pal:",266,32,226,12,3007,WS_VISIBLE|WS_CHILD|WS_TABSTOP,0,15},
    {"EDIT","",216,32,196,12,3008,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,15},
    {"BUTTON","X flip",190,32,145,12,3009,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_TABSTOP,0,15},
    {"BUTTON","Y flip",140,32,95,12,3010,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_TABSTOP,0,15},
    {"BUTTON","In front",90,32,30,12,3011,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_TABSTOP,0,15},
    {"BUTTON","BG1",98,0,40,16,3012,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_TABSTOP,0,0},
    {"BUTTON","BG2",98,20,40,16,3013,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_TABSTOP,0,0}
};
SD_ENTRY persp_sd[]={
    {"PERSPDISP","",0,48,0,20,3000,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,10},
    {"BUTTON","",0,0,20,20,3001,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",24,0,20,20,3002,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"STATIC","",0,24,40,20,3003,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Move",50,0,60,20,3004,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_GROUP,0,0},
    {"BUTTON","Add points",50,24,60,20,3005,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"BUTTON","Add faces",114,0,60,20,3006,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0},
    {"BUTTON","Del faces",114,24,60,20,3007,WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE,0,0}
};

SD_ENTRY text_sd[]={
    {"LISTBOX","",0,0,0,70, 3000, WS_VISIBLE|WS_CHILD|LBS_NOTIFY|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"EDIT","",0,65,0,24, 3001, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|ES_MULTILINE|WS_VSCROLL|ES_AUTOVSCROLL,WS_EX_CLIENTEDGE,14},
    {"BUTTON","Set",0,20,50,0, 3002, WS_VISIBLE|WS_TABSTOP|WS_CHILD,0,12},
    {"BUTTON","Edit text",60,20,65,0, 3003, WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON,0,12},
    {"BUTTON","Edit dictionary",130,20,120,0, 3004, WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON,0,12}
};

// =============================================================================

SUPERDLG sampdlg={
    "",sampdlgproc,WS_CHILD|WS_VISIBLE,300,100,23,samp_sd
};

SUPERDLG textdlg={
    "",textdlgproc,WS_CHILD|WS_VISIBLE,600,200, 5, text_sd
};

SUPERDLG overdlg={
    "",overdlgproc,WS_CHILD|WS_VISIBLE,560,140, SD_OverNumControls, over_sd
};

SUPERDLG trackdlg = {
    "",trackdlgproc,WS_CHILD|WS_VISIBLE,100,100,3,track_sd
};

SUPERDLG musdlg={
    "",musdlgproc,WS_CHILD|WS_VISIBLE,400,300,29,mus_sd
};

SUPERDLG wmapdlg={
    "",wmapdlgproc,WS_CHILD|WS_VISIBLE,560,100,9,wmap_sd
};

SUPERDLG lmapdlg={
    "",lmapdlgproc,WS_CHILD|WS_VISIBLE,280,280,15,lmap_sd
};

SUPERDLG tmapdlg={
    "",tmapdlgproc,WS_CHILD|WS_VISIBLE,560,140,14,tmap_sd
};

SUPERDLG perspdlg={
    "",perspdlgproc,WS_CHILD|WS_VISIBLE,200,200,8,persp_sd
};

// =============================================================================

BOOL
Loadsprnames(char   const * const p_buffer,
             size_t         const p_size)
{
    int i;
    
    size_t j        = 0;
    size_t name_len = 0;
    
    // -----------------------------
    
    for(i = 0; i < 0x11c; i++)
    {
        if(j >= p_size)
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "Truncated sprite names section",
                       "Bad error happened",
                       MB_OK);
            
            return FALSE;
        }
        
        name_len = p_buffer[j];
        
        if(name_len > 15)
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "Sprite name too long",
                       "Bad error happened",
                       MB_OK);
            
            return FALSE;
        }
        else if(name_len == 0)
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "Zero length sprite name",
                       "Bad error happened",
                       MB_OK);
            
            return FALSE;
        }
        
        j += 1;
        
        if( (j + name_len) > p_size )
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "\nTruncated sprite names section",
                       "Bad error happened",
                       MB_OK);
            
            // indicate that we had a problem.
            return FALSE;
        }
        
        memcpy(sprname[i],
               p_buffer + j,
               name_len);
        
        // Null terminate.
        sprname[i][name_len] = 0;
        
        j += name_len;
    }
    
    return TRUE;
}

// =============================================================================

BOOL
ValidateAltSpriteNamesFile(HM_FileStat const * const p_s)
{
    unsigned i = 0;
    
    char const * b = p_s->m_contents;
    
    // -----------------------------
    
    if(p_s->m_file_size < (256 * 9) )
    {
        return FALSE;
    }
    
    // The purpose of this loop is to check that all characters in the file
    // are printable / displayable. It also verifies that each name
    // actually contains a null terminator. This limits the names to
    // 8 characters for a well-formed file that passes validation.
    for
    (
        i = 0;
        i < 256;
        i += 1, b += 9
    )
    {
        BOOL found_null = FALSE;
        
        unsigned j = 0;
        
        // -----------------------------
        
        for(j = 0; j < 9; j += 1)
        {
            if(b[j] == '\0')
            {
                found_null = TRUE;
                
                continue;
            }
            
            if( ! isprint( b[j] ) )
            {
                return FALSE;
            }
        }
        
        if( ! found_null )
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

// =============================================================================

// Creates an example alternate spr names file that should fail validation
// because it has nonprintable characters (even though they wouldn't be
// picked up by the copying mechanism, this is a bit of an overzealous
// check, but it's really hypothetical anyway as we've not seen one of these
// alternate format sprite names files out in the wild. The program also
// doesn't seem to have any leftover code to generate one of its own, too.
void
CreateBadAltSprNamesFile(void)
{
    FILE *f = fopen("altsprname_good.DAT", "wb");
    
    int i = 0;
    
    for(i = 0; i < 256; i += 1)
    {
        char text_buf[0x100] = { 0 };
        
        sprintf(text_buf, "spr%02x", i);
        
        text_buf[6] = 0x01;
        text_buf[7] = 0x02;
        text_buf[8] = 0x03;
        
        fwrite(text_buf, 1, 9, f);
    }
    
    fclose(f);
}

// =============================================================================

// Creates an example alternate spr names file that should pass validation.
void
CreateGoodAltSprNamesFile(void)
{
    FILE *f = fopen("altsprname_good.DAT", "wb");
    
    int i = 0;
    
    for(i = 0; i < 256; i += 1)
    {
        char text_buf[0x100] = { 0 };
        
        sprintf(text_buf, "spr%02x", i);
        
        text_buf[8] = 0x00;
        
        fwrite(text_buf, 1, 9, f);
    }
    
    fclose(f);
}

// =============================================================================

// \note Alternative method of loading sprite names
// Slightly different format, probably dates to an earlier time in the
// development of Hyrule Magic. In practice limits each sprite name to 8
// characters. Overlord sprites are coerced into using rather uninformative
// names in this scheme too.
BOOL
LoadAltSpriteNamesFile(HM_FileStat const * const p_s)
{
     char const * b = p_s->m_contents;
     
     int i = 0;
     
     // -----------------------------
     
     if( ValidateAltSpriteNamesFile(p_s) == FALSE )
     {
         return FALSE;
     }
     
     for(i = 0; i < 256; i++, b += 9)
     {
         strcpy(sprname[i], b);
     }
     
     // Give overlord sprites generic as hell names.
     for(i = 0; i < 28; i++)
     {
         sprintf(sprname[i + 256], "S%02X", i);
     }
     
     return TRUE;
}

// =============================================================================

BOOL
LoadStdSpriteNamesFile(HM_FileStat const * const p_s)
{
    if(p_s->m_file_size < 5)
    {
        return FALSE;
    }
    else
    {
        DWORD const size_alleged = ldle32b((uint8_t *) p_s->m_contents + 1);
        
        // Claims to have more data than we read out.
        if( size_alleged > (p_s->m_file_size - 5) )
        {
            return FALSE;
        }
        else
        {
            return Loadsprnames(&p_s->m_contents[5],
                                size_alleged);
        }
    }
}

// =============================================================================

BOOL
LoadSpriteNamesFile(void)
{
    BOOL result = FALSE;
    
    HM_FileStat s = HM_LoadFileContents("sprname.dat");
    
    // -----------------------------
    
    if( s.m_valid_file )
    {
        if(s.m_contents[0])
        {
            result = LoadAltSpriteNamesFile(&s);
        }
        else
        {
            result = LoadStdSpriteNamesFile(&s);
        }
    }
    
    HM_FreeFileStat(&s);
        
    return result;
}

// =============================================================================

int WINAPI WinMain(HINSTANCE hinst,HINSTANCE pinst,LPSTR cmdline,int cmdshow)
{
    HMENU hmenu;
    
    HANDLE h,h2;
    
    MSG msg;
    
    // device context handle
    HDC hdc;
    
    int i, // i = windows version
        j,
        k,
        l;
    
    unsigned char *b, *b2;
    
    (void) pinst, cmdline;
    
    i=GetVersion();
    
    // Appears to be a check to see if we're running Win32s
    // (32-bit runtime for Windows 3.1)
    // There's probably someone in a remote bunker somewhere that is... idk.
    wver = ( (i < 0) && ( (i & 255) == 3) );
    
    // both are globals
    black_brush=GetStockObject(BLACK_BRUSH);
    white_brush=GetStockObject(WHITE_BRUSH);
    
    InitCommonControls();
    
    // set the global instance variable to this program.
    hinstance=hinst;
    
    HM_RegisterClasses(hinst);
    
    //  postmsgfunc=&PostMessage;
    //  wsprintf(buffer,"%08X",hinstance);
    //  MessageBox(framewnd,buffer,"blah",MB_OK);
    
    hmenu = LoadMenu(hinst,MAKEINTRESOURCE(IDR_MENU1));
    
    // load the one menu we have :)
    
    // file is the first menu.
    filemenu = GetSubMenu(hmenu,0);
    
    framewnd=CreateWindowEx(WS_EX_LEFT,
                            "ZEFRAME",
                            "Hyrule Magic",
                            WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            hmenu,
                            hinst,
                            0);
    
    ShowWindow(framewnd, cmdshow);
    
    actab=LoadAccelerators(hinstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
    
    green_brush = CreateSolidBrush(0xff00);
    purple_brush = CreateSolidBrush(0xff00ff);
    yellow_brush = CreateSolidBrush(0xffff);
    red_brush = CreateSolidBrush(0xff);
    blue_brush = CreateSolidBrush(0xff0000);
    gray_brush = CreateSolidBrush(0x608060);
    
    green_pen = CreatePen(PS_SOLID,0,0xff00);
    
    null_pen  = (HPEN) GetStockObject(NULL_PEN);
    white_pen = (HPEN) GetStockObject(WHITE_PEN);
    
    blue_pen  = (HPEN) CreatePen(PS_SOLID, 0, 0xff0000);
    
    black_pen = (HPEN) GetStockObject(BLACK_PEN);
    
    for(i=0;i<8;i++)
        shade_brush[i]=CreateSolidBrush(i * 0x1f1f1f);
    
    forbid_cursor=LoadCursor(0,IDC_NO);
    
    arrows_imgs[0]=LoadBitmap(0,(LPSTR)OBM_LFARROW);
    arrows_imgs[1]=LoadBitmap(0,(LPSTR)OBM_RGARROW);
    arrows_imgs[2]=LoadBitmap(0,(LPSTR)OBM_UPARROW);
    arrows_imgs[3]=LoadBitmap(0,(LPSTR)OBM_DNARROW);
    
    sizecsor[0] = LoadCursor(0,IDC_SIZENESW);
    sizecsor[1] = LoadCursor(0,IDC_SIZENS);
    sizecsor[2] = LoadCursor(0,IDC_SIZENWSE);
    sizecsor[3] = LoadCursor(0,IDC_SIZEWE);
    sizecsor[4] = normal_cursor;
    
    wait_cursor = LoadCursor(0, IDC_WAIT);
    
#if 0
    trk_font = GetStockObject(ANSI_FIXED_FONT);
#else
    trk_font = CreateFont(16, 0,
                          0, 0,
                          0,
                          FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET,
                          OUT_OUTLINE_PRECIS,
                          CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY,
#if 0
                          | ANTIALIASED_QUALITY,
#endif
                          FIXED_PITCH, "Consolas");
#endif

    hdc=GetDC(framewnd);
    
    if(GetDeviceCaps(hdc,RASTERCAPS)&RC_PALETTE)
        palmode=1;
    
    objdc=CreateCompatibleDC(hdc);
    objbmp=CreateCompatibleBitmap(hdc,512,512);
    SelectObject(objdc,objbmp);
    SelectObject(objdc,trk_font);
    GetTextMetrics(objdc,&textmetric);
    ReleaseDC(framewnd,hdc);
    
    for(i = 0; i < 256; i++)
    {
        cost[i] = (short) ( cos(i * 0.0245436926) * 16384.0 );
    }
    
    l = 0;
    
    strcpy(asmpath, "FSNASM");
    
    h = CreateFile("HMAGIC.CFG",
                   GENERIC_READ,
                   FILE_SHARE_READ,
                   0,
                   OPEN_EXISTING,
                   0,
                   0);
    
    for(j=0;j<4;j++)
        mrulist[j]=0;
    
    if(h != INVALID_HANDLE_VALUE)
    {
        // Loop until there are no configuration sections left to read.
        for( ; ; )
        {
            // Descriptor for a configuration section.
            unsigned char cfg_desc[5];
            
            unsigned code = 0;
            
            size_t section_size = 0;
            
            DWORD desc_size = 0;
            
            // -----------------------------
            
            ReadFile(h, cfg_desc, 5, &desc_size, 0);
            
            if(desc_size < 5)
                break;
            
            code = cfg_desc[0];
            
            // Number of bytes that are supposed to be in this configuration
            // section.
            section_size = ldle32b(cfg_desc + 1);
            
            if(section_size == 0)
            {
                // A section with no data is of no use and probably indicates
                // malformed config file. Abort further reading of it.
                break;
            }
            
            // Tells us which type of setting to load.
            // Don't load another section of the same type if one has already
            // successfully loaded.
            if( (code < 5) && ( (l & (1 << code) ) == 0) )
            {
                // Number of bytes able to be read from the section.
                DWORD n = 0;
                
                BOOL spr_loaded = FALSE;
                
                unsigned char *section_data = 0;
                
                // -----------------------------
                
                cfg_flag |= (1 << code);
                
                section_data = (unsigned char*) calloc(1, section_size);
                
                ReadFile(h,
                         section_data,
                         section_size,
                         &n, 0);
                
                if(n < section_size)
                {
                    // There was a mismatch in the number of bytes purported to
                    // be in the section and what we could actually read out.
                    goto wrapup;
                }
                
                switch(code) 
                {
                
                case 0:
                    
                    // Use an array of sprite names that has been directly
                    // inlined in the config file.
                    spr_loaded = Loadsprnames((char const *) section_data,
                                              section_size);
                    
                    if(spr_loaded)
                    {
                        l |= CFG_SPRNAMES_LOADED;
                    }
                    
                    break;
                
                case 1:
                    
                    // Looks like a list of MRU entries.
                    
                    // Get number of MRU entries.
                    i = section_data[0];
                    
                    if(i > 4)
                        i = 4;
                    
                    k = 1;
                    
                    for(j = 0; j < i; j++)
                    {
                        size_t len = 0;
                        
                        // -------------------------
                        
                        if( (section_size - (k + 1) ) < 2)
                        {
                            // Making sure there's enough data left for
                            // a path length and a path.
                            break;
                        }
                        
                        len = section_data[k];
                        
                        k += 1;
                        
                        if( len > (section_size - k) )
                        {
                            // Making sure there's enough data left in the
                            // section to provide the alleged path size.
                            break;
                        }
                        
                        // Size (in bytes) is pascal style and preceeds
                        // the string / path.
                        mrulist[j] =
                        (char*) calloc(1,
                                       (len + 1) );
                        
                        memcpy(mrulist[j],
                               section_data + k,
                               len);
                        
                        // Null terminate the path.
                        mrulist[j][len]=0;
                        
                        k += len;
                    }
                    
                    l |= CFG_MRU_LOADED;
                    
                    break;
                
                case 2:
                    
                    if(section_size >= 2)
                    {
                        soundvol = ldle16b(section_data);
                        
                        l |= CFG_SNDVOL_LOADED;
                    }
                    
                    break;
                
                case 3:
                    
                    // Custom midi configuration of some sort.
                    
                    if(section_size >= (MIDI_ARR_BYTES * 2) )
                    {
                        size_t i = 0;
                        
                        for(i = 0; i < MIDI_ARR_WORDS; i += 1)
                        {
                            midi_inst[i] = ldle16b_i(section_data, i);
                            
                            midi_trans[i] = ldle16b_i(section_data,
                                                      i + MIDI_ARR_WORDS);
                        }
                        
                        l |= CFG_MIDI_LOADED;
                    }
                    
                    break;
                
                case 4:
                    
                    // Custom path to FNASM assembler.
                    
                    if(section_size < MAX_PATH)
                    {
                        memcpy(asmpath,
                               section_data,
                               section_size);
                        
                        asmpath[section_size] = 0;
                        
                        l |= CFG_ASM_LOADED;
                    }
                }
                
            wrapup:
                
                free(section_data);
            }
            else
            {
                // Unrecognized configuration section type, so advance by the
                // reported length of the section.
                // (acting in good faith, basically.)
                // I guess the idea was make it future proof to a certain
                // extent so that config files from future versions could be
                // used without issue..
                SetFilePointer(h, section_size, 0, 1);
            }
        }
        
        CloseHandle(h);
        
        cfg_flag = l;
    }
    
    UpdMRU();
    
    if( ! (l & CFG_SPRNAMES_LOADED) )
    {
        if( LoadSpriteNamesFile() == FALSE )
        {
            HRSRC const
            spr_names_resource = FindResource(0,
                                              MAKEINTRESOURCE(IDR_SPRNAMES),
                                              RT_RCDATA);
            
            HGLOBAL const spr_names_handle = LoadResource(0,
                                                          spr_names_resource);
            
            size_t const spr_names_size = SizeofResource(0,
                                                         spr_names_resource);
            
            // -----------------------------
            
            if(spr_names_size > 5)
            {
                char const * const
                spr_names = (char const *) LockResource(spr_names_handle);
                
                Loadsprnames(spr_names + 5,
                             spr_names_size - 5);
            }
            
            // Even though these do nothing... why the hell not.
            UnlockResource(spr_names_handle);
            
            FreeResource(spr_names_resource);
        }
    }
    
    entrance_names = LoadTextFileResource(IDR_ENTRANCE_NAMES);
    area_names = LoadTextFileResource(IDR_AREA_NAMES);
    
    GetCurrentDirectory(MAX_PATH, currdir);
    
    debug_window = CreateNotificationWindow(framewnd);
    debug_box    = CreateNotificationBox(framewnd);
    
    while(GetMessage(&msg,0,0,0))
    {
        if(debug_box && (msg.hwnd == debug_box) )
        {
            if(msg.message == WM_LBUTTONDOWN && ! always)
            {
                HWND const below = GetWindow(msg.hwnd, GW_HWNDNEXT);
                
                char class_name[0x200];
                
                GetClassName(below, class_name, 0x200);
                
                msg.hwnd = below;
                
                DispatchMessage(&msg);
                
                continue;
            }
        }
        
        if(msg.message == WM_MOUSEWHEEL)
        {
            POINT pt;
            GetCursorPos(&pt);
            
            msg.hwnd = WindowFromPoint(pt);
            
            DispatchMessage(&msg);
            
            continue;
        }
        
        ProcessMessage(&msg);
    }
    
    if(cfg_modf)
    {
        DWORD write_bytes = 0;
        
        // Descriptor for the configuration section.
        uint8_t desc[5];
        
        SetCurrentDirectory(currdir);
        h=CreateFile("HMAGIC.CFG",GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
        
        if(cfg_flag & CFG_SPRNAMES_LOADED)
        {
            j=0;
            
            for(i = 0; i < 0x11c; i++)
                j += strlen(sprname[i]) + 1;
            
            b2 = b = (unsigned char*) malloc(j + 5);
            *b2=0;
            
            stle32b(b2 + 1, j);
            
            b2 += 5;
            
            for(i=0;i<0x11c;i++)
            {
                k=strlen(sprname[i]);
                (*b2) = (unsigned char) k;
                b2++;
                memcpy(b2,sprname[i],k);
                b2+=k;
            }
            
            // \note Writes a sprite names file if you hold shift while
            // saving. Okay then...
            if(GetKeyState(VK_SHIFT) & 128)
            {
                h2 = CreateFile("SPRNAME.DAT",GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
                
                WriteFile(h2, b, j + 5, &write_bytes, 0);
                
                CloseHandle(h2);
            }
            
            WriteFile(h, b, j + 5, &write_bytes, 0);
            free(b);
        }
        
        if(cfg_flag & CFG_MRU_LOADED)
        {
            j = 0;
            
            // Calculate total length required for an MRU section.
            for(i = 0; i < 4; i++)
            {
                if( !mrulist[i] )
                    break;
                
                j += strlen(mrulist[i]) + 1;
            }
            
            b2 = b = (unsigned char*) malloc(j + 6);
            
            b2[0] = 1;
            
            stle32b(b2 + 1, j + 1);
            
            // Write how many MRU entries follow. Note that this could be
            // zero, in which case the overall written out section is only
            // 6 bytes long.
            b2[5] = (unsigned char) i;
            
            b2 += 6;
            
            for(i = 0; i < 4; i++)
            {
                if(!mrulist[i])
                    break;
                
                k = strlen(mrulist[i]);
                
                (*b2) = (unsigned char) k;
                
                b2++;
                
                memcpy(b2, mrulist[i], k);
                
                b2 += k;
            }
            
            WriteFile(h, b, j + 6, &write_bytes, 0);
            
            free(b);
        }
        
        if(cfg_flag & CFG_SNDVOL_LOADED)
        {
            uint8_t section[2];
            
            // -----------------------------
            
            desc[0] = 2;
            
            stle32b(desc + 1, 2);
            
            stle16b(section, soundvol);
            
            WriteFile(h, desc, 5, &write_bytes, 0);
            WriteFile(h, section, 2, &write_bytes, 0);
        }
        
        if(cfg_flag & CFG_MIDI_LOADED)
        {
            int i = 0;
            
            uint8_t entry[2];
            
            // -----------------------------
            
            desc[0] = 3;
            
            stle32b(desc + 1, 100);
            
            WriteFile(h, desc, 5, &write_bytes, 0);
            
            for(i = 0; i < MIDI_ARR_WORDS; i += 1)
            {
                stle16b(entry, midi_inst[i]);
                
                WriteFile(h, entry, 2, &write_bytes, 0);
            }
            
            for(i = 0; i < MIDI_ARR_WORDS; i += 1)
            {
                stle16b(entry, midi_trans[i]);
                
                WriteFile(h, entry, 2, &write_bytes, 0);
            }
        }
        
        if(cfg_flag & CFG_ASM_LOADED)
        {
            desc[0] = 4;
            
            stle32b(desc + 1, strlen(asmpath) );
            
            WriteFile(h, desc, 5, &write_bytes, 0);
            WriteFile(h, asmpath, strlen(asmpath), &write_bytes, 0);
        }
        
        CloseHandle(h);
    }
    
    Exitsound();
    
    DeleteObject(arrows_imgs[0]);
    DeleteObject(arrows_imgs[1]);
    DeleteObject(arrows_imgs[2]);
    DeleteObject(arrows_imgs[3]);
    
    DeleteObject(objdc);
    DeleteObject(objbmp);
    
    DeleteObject(green_brush);
    DeleteObject(purple_brush);
    DeleteObject(yellow_brush);
    DeleteObject(red_brush);
    DeleteObject(blue_brush);
    DeleteObject(gray_brush);
    DeleteObject(green_pen);
    DeleteObject(blue_pen);
    
    for(i=0;i<8;i++)
        DeleteObject(shade_brush[i]);
    
    return 0;
}

// =============================================================================
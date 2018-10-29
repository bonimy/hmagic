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

    \task Sample editor crashes if you paste in a region that is past the
    end of the sample.

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

#include "DungeonEnum.h"
#include "DungeonLogic.h"

#include "PaletteEdit.h"

// For text enumerations and Load / Save text functions.
#include "TextEnum.h"
#include "TextLogic.h"

#include "AudioLogic.h"
#include "SampleEnum.h"

// \task Probably will move this once the worldmap super dialog procedure gets
// its own file.
#include "WorldMapLogic.h"

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
int mark_sr;
int mark_start,mark_end,mark_first,mark_last;

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

static unsigned char masktab[16] = {255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int palhalf[8] = {1,0,0,1,1,1,0,0};

void *firstgraph, *lastgraph;

const static int palt_sz[] = {96,256,128,48};
const static int palt_st[] = {32,0,0,208};

const short bg3blkofs[4] = {221,222,220,0};

RECT const empty_rect = { 0, 0, 0, 0 };

char *note_str[12] = {"C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B "};

char cmd_str[32][9]={
    "Instr.  ",
    "Pan     ",
    "Pansweep",
    "Vibrato ",
    "Vib.Off ",
    "Glob.Vol",
    "GV ramp ",
    "Speed   ",
    "Spd ramp",
    "G. Trans",
    "Transpos",
    "Tremolo ",
    "Trm.Off ",
    "Volume  ",
    "Vol.Ramp",
    "CallLoop",
    "Cmd F0  ",
    "PSlideTo",
    "PSlideFr",
    "Cmd F3  ",
    "Finetune",
    "Cmd F5  ",
    "Cmd F6  ",
    "Cmd F7  ",
    "Echo    ",
    "PSlide  ",
    "Cmd FA  ",
    "Cmd FB  ",
    "Cmd FC  ",
    "Cmd FD  ",
    "Cmd FE  ",
    "Cmd FF  "
};

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
    
    dt = (void*) p;
    
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

uint8_t *
Compress(uint8_t const * const src,
         int             const oldsize,
         int           * const size,
         int             const flag)
{
    unsigned char *b2 = (unsigned char*) malloc(0x1000); // allocate a 2^12 sized buffer
    
    int i, j, k, l, m = 0,
        n,
        o = 0,
        bd = 0, p, q = 0,r;
    
    for(i = 0; i < oldsize; )
    {
        l=src[i]; // grab a char from the buffer.
        
        k=0;
        
        r=!!q; // r = the same logical value (0 or 1) as q, but not the same value necesarily.
        
        for(j = 0; j < i - 1; j++)
        {
            if(src[j] == l)
            {
                m = oldsize - j;
                
                for(n = 0; n < m; n++)
                    if(src[n+j]!=src[n+i])
                        break;
                
                if(n > k)
                    k=n,o=j;
            }
        }
        
        for(n = i + 1; n < oldsize; n++)
        {
            if(src[n] != l)
            {
                // look for chars identical to the first one. 
                // stop if we can't find one.
                // n will reach i+k+1 for some k >= 0.
                
                break;
            }
        }
        
        n -= i; // offset back by i. i.e. n = k+1 as above.
        
        if(n > 1 + r)
            p=1;
        else
        {
            m=src[i+1];
            
            for(n=i+2;n<oldsize;n++)
            {
                if(src[n]!=l)
                    break;
                
                n++;
                
                if(src[n]!=m)
                    break;
            }
            
            n-=i;
            
            if(n>2+r)
                p=2;
            else
            {
                m=oldsize-i;
                
                for(n=1;n<m;n++)
                    if(src[i+n]!=l+n)
                        break;
                
                if(n>1+r)
                    p=3;
                else
                    p=0;
            }
        }
        
        if(k>3+r && k>n+(p&1))
            p=4,n=k;
        
        if(!p)
            q++,i++;
        else
        {
            if(q)
            {
                q--;
                
                if(q>31)
                {
                    b2[bd++] = (unsigned char) ( 224 + (q >> 8) );
                }
                
                b2[bd++] = (unsigned char) q;
                q++;
                
                memcpy(b2+bd,src+i-q,q);
                
                bd += q;
                q = 0;
            }
            
            i+=n;
            n--;
            
            if(n>31)
            {
                b2[bd++] = (unsigned char) ( 224 + (n >> 8) + (p << 2) );
                b2[bd++] = (unsigned char) n;
            }
            else
                b2[bd++] = (unsigned char) ( (p << 5) + n );
            
            switch(p)
            {
            
            case 1: case 3:
                b2[bd++] = (unsigned char) l;
                break;
            
            case 2:
                b2[bd++] = (unsigned char) l;
                b2[bd++] = (unsigned char) m;
                
                break;
            
            case 4:
                if(flag)
                {
                    b2[bd++] = (unsigned char) (o >> 8);
                    b2[bd++] = (unsigned char) o;
                }
                else
                {
                    b2[bd++] = (unsigned char) o;
                    b2[bd++] = (unsigned char) (o >> 8);
                }
            }
            
            continue;
        }
    }
    
    if(q)
    {
        q--;
        
        if(q>31)
        {
            b2[bd++] = (unsigned char) ( 224 + (q >> 8) );
        }
        
        b2[bd++] = (unsigned char) q;
        q++;
        
        memcpy(b2 + bd,
               src + i - q,
               q);
        
        bd += q;
    }
    
    b2[bd++]=255;
    b2 = (unsigned char*) realloc(b2, bd);
    *size=bd;
    
    return b2;
}

// =============================================================================

uint8_t *
Uncompress(uint8_t const *       src,
           int           * const size,
           int             const p_big_endian)
{
    unsigned char *b2 = (unsigned char*) malloc(1024);
    
    int bd = 0, bs = 1024;
    
    unsigned char a;
    unsigned char b;
    unsigned short c, d;
    
    for( ; ; )
    {
        // retrieve a uchar from the buffer.
        a = *(src++);
        
        // end the decompression routine if we encounter 0xff.
        if(a == 0xff)
            break;
        
        // examine the top 3 bits of a.
        b = (a >> 5);
        
        if(b == 7) // i.e. 0b 111
        {
            // get bits 0b 0001 1100
            b = ( (a >> 2) & 7 );
            
            // get bits 0b 0000 0011, multiply by 256, OR with the next byte.
            c  = ( (a & 0x0003) << 8 );
            c |= *(src++);
        }
        else
            // or get bits 0b 0001 1111
            c = (uint16_t) (a & 31);
        
        c++;
        
        if( (bd + c) > (bs - 512) )
        {
            // need to increase the buffer size.
            bs += 1024;
            b2 = (uint8_t*) realloc(b2,bs);
        }
        
        // 7 was handled, here we handle other decompression codes.
        
        switch(b)
        {
        
        case 0: // 0b 000
            
            // raw copy
            
            // copy info from the src buffer to our new buffer,
            // at offset bd (which we'll be increasing;
            memcpy(b2+bd,src,c);
            
            // increment the src pointer accordingly.
            src += c;
            
            bd += c;
            
            break;
        
        case 1: // 0b 001
            
            // rle copy
            
            // make c duplicates of one byte, inc the src pointer.
            memset(b2+bd,*(src++),c);
            
            // increase the b2 offset.
            bd += c;
            
            break;
        
        case 2: // 0b 010
            
            // rle 16-bit alternating copy
            
            d = ldle16b(src);
            
            src += 2;
            
            while(c > 1)
            {
                // copy that 16-bit number c/2 times into the b2 buffer.
                stle16b(b2 + bd, d);
                
                bd += 2;
                c -= 2; // hence c/2
            }
            
            if(c) // if there's a remainder of c/2, this handles it.
                b2[bd++] = (char) d;
            
            break;
        
        case 3: // 0b 011
            
            // incrementing copy
            
            // get the current src byte.
            a = *(src++);
            
            while(c--)
            {
                // increment that byte and copy to b2 in c iterations.
                // e.g. a = 4, b2 will have 4,5,6,7,8... written to it.
                b2[bd++] = a++;
            }
            
            break;
        
        default: // 0b 100, 101, 110
            
            // lz copy
            
            if(p_big_endian)
            {
                d = (*src << 8) + src[1];
            }
            else
            {
                d = ldle16b(src);
            }
            
            while(c--)
            {
                // copy from a different location in the buffer.
                b2[bd++] = b2[d++];
            }
            
            src += 2;
        }
    }
    
    b2 = (unsigned char*) realloc(b2,bd);
    
    if(size)
        (*size) = bd;
    
    // return the unsigned char* buffer b2, which contains the uncompressed data.
    return b2;
}

//Uncompress*********************************

//Make4bpp*********************************

unsigned char* Make4bpp(unsigned char *buf,int size)
{
    short *b1 = malloc(size);
    short *a;
    
    unsigned char *b;
    short *c;
    
    int d,e;
    
    // looks at the uchar buffer in terms of shorts.
    a = (short*) buf;
    
    //cast again on (a+8). 
    //Note that this is 16 bytes ahead, not 8.
    //this is because it was incremented 8 shorts, then typecast
    //as char. 8 shorts = 16 bytes.
    b = (unsigned char*) (a + 8);
    
    c = b1;
    
    // e is the size divided by 32.
    for(e = size >> 5; e; e--)
    {
        for(d = 8; d; d--)
        {
            // looks complicated. OR's the lower bytes of *a and *b, shifts them left 8 bits.
            // (*a | *b) = temp. AND by 0xFF00, so we have only bits 8-15 being occupied.
            // next, OR with *a, so we get *a's upper and lower bytes mapped in again.
            // Also, OR in *b
            
            *c = *a;
            c[8] = ( ( ( *a | ( ( *a | *b) << 8) ) & 0xff00 ) | *b );
            
            a++; // inc by 1 short
            b++; // inc by 1 char
            c++; // inc by 1 short
        }
        
        a = (short*) b;
        b += 16;
        c += 8;
    }
    
    return (unsigned char*) b1;
}

//Make4bpp*********************************

unsigned char* Makesnes(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size << 1);
    
    int a, e = 0;
    
    unsigned char h, c, d, f[4];
    
    short g;
    
    for(a = size >> 5; a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 16; h <<= 1, g++)
            {
                c = ( ( b[7] & h ) >> g );
                
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                
                f[g]=c; // this is the algorithm that turns a bitmap on it side, so it seems.
            }
            
            b += 8;
            
            buf[e+d+d]=f[0];
            
            buf[e+d+d+1]=f[1];
            
            buf[e+d+d+16]=f[2];
            
            buf[e+d+d+17]=f[3];
        }
        
        e+=32;
    }
    
    return buf;
}

unsigned char* Make3bpp(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size * 3 >> 3);
    int a, e = 0;
    unsigned char h,c,d,f[3];
    short g;
    
    for(a = (size >> 6); a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 8; h <<= 1, g++)
            {
                c = ((b[7] & h) >> g);
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                f[g] = c;
            }
            
            b += 8;
            buf[e + d + d] = f[0];
            buf[e + d + d + 1] = f[1];
            buf[e + d + 16] = f[2];
        }
        
        e += 24;
    }
    
    return buf;
}

unsigned char* Make2bpp(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size >> 1);
    int a, e = 0;
    unsigned char h, c, d, f[2];
    short g;
    
    for(a = size >> 5; a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 4; h <<= 1, g++)
            {
                c = (( b[7] & h ) >> g);
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                f[g] = c;
            }
            
            b += 8;
            buf[e + d + d] = f[0];
            buf[e + d + d + 1] = f[1];
        }
        
        e+=16;
    }
    
    return buf;
}

// Think this converts a bitplaned tile to a bitmap
unsigned char * Unsnes(unsigned char * const buf, int size)
{
    int i,
        j,
        k = 0,
        l,
        m,
        n = size << 1,
        o = (size << 2) + 7;
    
    unsigned char * const b2 = (unsigned char*) malloc(size << 3);
    
    // -----------------------------
    
    for(j = 0; j < size; j += 16)
    {
        for(m = 8; m; m--, j += 2)
        {
            for(i = 7, l = 0x80; l; )
            {
                b2[k^o] = b2[k] =
                (
                    (
                        ( buf[j] & l )
                      + ( (buf[j + 1]  & l) << 1 )
                      + ( (buf[j + 16] & l) << 2 )
                      + ( (buf[j + 17] & l) << 3 )
                    ) >> i
                );
                
                b2[(k^o) + n] = b2[k + n] = masktab[ b2[k] ];
                k++;
                l >>= 1;
                i--;
            }
        }
    }
    
    return b2;
}

//Changesize#*****************************************

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

//GetBG3GFX*************************************

unsigned char* GetBG3GFX(unsigned char *buf, int size)
{
    int c,d;
    
    unsigned char *buf2 = malloc(size);
    
    size >>= 1;
    
    for(d = 0; d < size; d += 16)
    {
        for(c = 0; c < 16; c++)
            buf2[d+d+c]=buf[d+c];
        
        for(c = 0; c < 16; c++)
        {
            // interesting, it's interlaced with a row of zeroes every
            //other line.
            buf2[d+d+c+16]=0;
        }
    }
    
    return buf2;
}

//GetBG3GFX**************************************

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

// =============================================================================

const char *mus_str[]={
    "Same",
    "None",
    "Title",
    "World map",
    "Beginning",
    "Rabbit",
    "Forest",
    "Intro",
    "Town",
    "Warp",
    "Dark world",
    "Master swd",
    "File select",
    "Soldier",
    "Mountain",
    "Shop",
    "Fanfare",
    "Castle",
    "Palace",
    "Cave",
    "Clear",
    "Church",
    "Boss",
    "Dungeon",
    "Psychic",
    "Secret way",
    "Rescue",
    "Crystal",
    "Fountain",
    "Pyramid",
    "Kill Agah",
    "Ganon room",
    "Last boss",
    "Triforce",
    "Ending",
    "Staff",
    "Stop",
    "Fade out",
    "Lower vol",
    "Normal vol"
};

static int sbank_ofs[] = { 0xc8000, 0, 0xd8000, 0};

char op_len[32]=
    {1,1,2,3,0,1,2,1,2,1,1,3,0,1,2,3,1,3,3,0,1,3,0,3,3,3,1,2,0,0,0,0};

short spcbank;

unsigned short spclen;

unsigned char * Getspcaddr(FDOC *doc, unsigned short addr, short bank)
{
    unsigned char *rom;
    unsigned short a,b;
    spcbank = bank + 1;

again:
    rom = doc->rom + sbank_ofs[spcbank];
    
    for(;;)
    {
        a = *(unsigned short*) rom;
        
        if(!a)
        {
            if(spcbank)
            {
                spcbank = 0;
                
                goto again;
            }
            else
                return 0;
        }
        
        b = *(unsigned short*) (rom + 2);
        rom += 4;
        
        if(addr >= b && addr - b < a)
        {
            spclen = a;
            
            return rom + addr - b;
        }
        
        rom += a;
    }
}

short lastsr;
short ss_lasttime;

short Getblocktime(FDOC *doc, short num, short prevtime)
{
    SCMD *sc = doc->scmd, *sc2;
    
    int i = -1, j = 0, k = 0, l, m = 0, n = prevtime;
    l = num;
    
    if(l == -1)
        return 0;
    
    for(;;)
    {
        if(sc[l].flag&4)
        {
            j = sc[l].tim;
            m = sc[l].tim2;
            k=1;
        }
        
        if(!k)
            i = l;
        
        if(sc[l].flag & 1)
            n = sc[l].b1;
        
        l = sc[l].next;
        
        if(l == -1)
        {
            if(!k)
            {
                m = 0;
                j = 0;
            }
            
            break;
        }
    }
    
    if(i!=-1) for(;;) {
        if(i==-1) {
            MessageBox(framewnd,"Really bad error","Bad error happened",MB_OK);
            doc->m_modf=1;
            return 0;
        }
        sc2=sc+i;
        if(sc2->cmd==0xef)
        {
            k=*(short*)&(sc2->p1);
            if(k>=doc->m_size) {MessageBox(framewnd,"Invalid music address","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
            if(sc2->flag&1) {
                j+=Getblocktime(doc,k,0)*sc2->p3;
                if(ss_lasttime) {
                    j+=ss_lasttime*m;
                    j+=sc[k].tim2*sc2->b1;
                    j+=(sc2->p3-1)*sc[k].tim2*ss_lasttime;
                } else {
                    j+=sc2->b1*(m+sc[k].tim2*sc2->p3);
                }
                m=0;
            }
            else
            {
                j+=Getblocktime(doc,k,0)*sc2->p3;
                j+=ss_lasttime*m;
                if(ss_lasttime) j+=(sc2->p3-1)*ss_lasttime*sc[k].tim2,m=sc[k].tim2;
                else m+=sc[k].tim2*sc2->p3;
            }
        }
        else
        {
            if(sc2->cmd<0xe0) m++;
            if(sc2->flag&1) {j+=m*sc[i].b1; m=0; }
        }
        sc2->tim=j;
        sc2->tim2=m;
        sc2->flag|=4;
        
        if(i==num)
            break;
        
        i=sc2->prev;
    }
    ss_lasttime=n;
    return sc[num].tim+prevtime*sc[num].tim2;
}
short Loadscmd(FDOC*doc,unsigned short addr,short bank,int t)
{
    int b = 0,
        c = 0,
        d = 0,
        e = 0,
        f = 0,
        g = 0,
        h = 0,
        i = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0;
    
    unsigned char j = 0,
                  k = 0;
    
    unsigned char *a = 0;
    SRANGE*sr;
    SCMD*sc=doc->scmd,*sc2;
    if(!addr) return -1;
    
    a = Getspcaddr(doc,addr,bank);
    d = spcbank;
    if(!a) {
        MessageBox(framewnd,"Address not found when loading track","Bad error happened",MB_OK);
        return -1;
    }
    sr=doc->sr;
    e=doc->srnum;
    f=0x10000;
    for(c=0;c<e;c++) {
        if(sr[c].bank==d) if(sr[c].start>addr) {if(sr[c].start<f) f=sr[c].start; n=c;}
        else if(sr[c].end>addr) {
            for(f=sr[c].first;f!=-1;f=sc[f].next) {
                if(sc[f].flag&4) m=sc[f].tim,o=sc[f].tim2;
                if(sc[f].addr==addr) {
                    sc[f].tim=m;
                    sc[f].tim2=o;
                    lastsr=c;
                    return f;
                }
                if(sc[f].flag&1) k=sc[f].b1;
                if(sc[f].cmd<0xca) if(k) m-=k; else o--;
            }
            MessageBox(framewnd,"Misaligned music pointer","Bad error happened",MB_OK);
            return -1;
        }
    }
    c=n;
    i=h=doc->m_free;
    a-=addr;
    m=0;
    k=0;
    o=0;
    for(g=addr;g<f;) {
        sc2=sc+i;
        if(sc2->next==-1) {
            l=doc->m_size;
            sc=doc->scmd=realloc(sc,sizeof(SCMD)*(doc->m_size+=1024));
            sc2=sc+i;
            n=l+1023;
            while(l<n) sc[l].next=l+1,l++;
            sc[l].next=-1;
            n-=1023;
            while(l>n) sc[l].prev=l-1,l--;
            sc[l].prev=i;
            sc2->next=l;
        }
        sc2->addr=g;
        b=a[g];
        if(!b) break;
        if(m>=t && b!=0xf9) break;
        g++;
        j=0;
        if(b<128) {
            j=1;
            k=sc2->b1=b;
            b=a[g++];
            if(b<128) j=3,sc2->b2=b,b=a[g++];
        }
        if(b<0xe0) if(k) m+=k; else o++;
        sc2->cmd=b;
        sc2->flag=j;
        if(b>=0xe0) {
            b-=0xe0;
            if(op_len[b]) sc2->p1=a[g++];
            if(op_len[b]>1) sc2->p2=a[g++];
            if(op_len[b]>2) sc2->p3=a[g++];
            if(b==15) {
                doc->m_free=sc2->next;
                sc[sc2->next].prev=-1;
                l = Loadscmd(doc,*(short*)(&(sc2->p1)),bank,t-m);
                sc=doc->scmd;
                sc2=sc+i;
                *(short*)(&(sc2->p1))=l;
                sc2->next=doc->m_free;
                sc[sc2->next].prev=i;
                Getblocktime(doc,l,0);
                if(sc[l].flag&4) m+=(sc[l].tim+sc[l].tim2*k)*sc2->p3; else {i=sc2->next;break;}
                if(doc->sr[lastsr].endtime) k=doc->sr[lastsr].endtime;
            }
        }
        i=sc2->next;
    }
    sc[h].tim=m;sc[h].tim2=o;sc[h].flag|=4;
    if(f==g && m<t) {
        l=sc[i].prev;
        lastsr=c;
        sc[sr[lastsr].first].prev=l;
        l=sc[l].next=sr[lastsr].first;
        if(sc[l].flag&4) sc[h].tim=sc[l].tim+m+sc[l].tim2*k,sc[h].flag|=4;
        sr[lastsr].first=h;
        sr[lastsr].start=addr;
        sr[lastsr].inst++;
    } else {
        if(doc->srsize==doc->srnum) doc->sr=realloc(doc->sr,(doc->srsize+=16)*sizeof(SRANGE));
        lastsr=doc->srnum;
        sr=doc->sr+(doc->srnum++);
        sr->start=addr;
        sr->end=g;
        sr->first=h;
        sr->endtime=k;
        sr->inst=1;
        sr->editor=0;
        sr->bank=d;
        sc[sc[i].prev].next=-1;
    }
    sc[i].prev=-1;
    doc->m_free=i;
    return h;
}
char fil1[4]={0,15,61,115};
char fil2[4]={0,4,5,6};
char fil3[4]={0,0,15,13};

int ss_num,ss_size;
unsigned short ss_next=0;
SSBLOCK * Allocspcblock(int len,int bank)
{
    SSBLOCK*sbl;
    if(!len) {
        MessageBox(framewnd,"Warning zero length block allocated","Bad error happened",MB_OK);
    }
    if(ss_num==ss_size) {
        ss_size+=512;
        ssblt=realloc(ssblt,ss_size<<2);
    }
    ssblt[ss_num]=sbl=malloc(sizeof(SSBLOCK));
    ss_num++;
    sbl->start=ss_next;
    sbl->len=len;
    sbl->buf=malloc(len);
    sbl->relocs=malloc(32);
    sbl->relsz=16;
    sbl->relnum=0;
    sbl->bank=bank&7;
    sbl->flag=bank>>3;
    ss_next+=len;
    return sbl;
}
void Addspcreloc(SSBLOCK*sbl,short addr)
{
    sbl->relocs[sbl->relnum++]=addr;
    if(sbl->relnum==sbl->relsz) {
        sbl->relsz+=16;
        sbl->relocs=realloc(sbl->relocs,sbl->relsz<<1);
    }
}
short Savescmd(FDOC*doc,short num,short songtime,short endtr)
{
    SCMD*sc=doc->scmd,*sc2;
    SRANGE*sr=doc->sr;
    SSBLOCK*sbl;
    unsigned char*b;
    int i = num,
        j = 0,
        k = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0,
        p = 0;
    
    if(i==-1) return 0;
    if(i>=doc->m_size) {MessageBox(framewnd,"Error.","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
    if(sc[i].flag&8) return sc[i].addr;
    for(;;) {
        j=sc[i].prev;
        if(j==-1) break;
        i=j;
    }
    for(j=0;j<doc->srnum;j++) {
        if(sr[j].first==i) {
            l=Getblocktime(doc,i,0);
            m=i;
            for(;;) {
                if(m==-1) break;
                k++;
                sc2=sc+m;
                if(sc2->flag&1) k++,n=sc2->b1;
                if(sc2->flag&2) k++;
                if(sc2->cmd>=0xe0) k+=op_len[sc2->cmd - 0xe0];
                m=sc2->next;
            }
            songtime-=l;
            if(songtime>0) {
                l=(songtime+126)/127;
                if(songtime%l) l+=2;
                l++;
                if(n && !songtime%n) {
                    p=songtime/n;
                    if(p<l) l=p;
                } else p=-1;
                k+=l;
            }
            k++;
            sbl=Allocspcblock(k,sr[j].bank|((!endtr)<<3)|16);
            b=sbl->buf;
            for(;;) {
                if(i==-1) break;
                sc2=sc+i;
                sc2->addr=b-sbl->buf+sbl->start;
                sc2->flag|=8;
                if(sc2->flag&1) *(b++)=sc2->b1;
                if(sc2->flag&2) *(b++)=sc2->b2;
                *(b++)=sc2->cmd;
                if(sc2->cmd>=0xe0) {
                    o=op_len[sc2->cmd - 0xe0];
                    if(sc2->cmd==0xef) {
                        *(short*)b=Savescmd(doc,*(short*)&(sc2->p1),0,1);
                        if(b) Addspcreloc(sbl,b-sbl->buf);
                        b[2]=sc2->p3;
                        b+=3;
                    } else {
                        if(o) *(b++)=sc2->p1;
                        if(o>1) *(b++)=sc2->p2;
                        if(o>2) *(b++)=sc2->p3;
                    }
                }
                i=sc2->next;
            }
            if(songtime>0) {
                if(l!=p) {
                    l=(songtime+126)/127;
                    if(songtime%l) n=127; else n=songtime/l;
                    *(b++)=n;
                }
                
                for( ; songtime >= n; songtime -= n)
                    *(b++) = 0xc9;
                
                if(songtime)
                {
                    *(b++) = (uint8_t) songtime;
                    *(b++) = 0xc9;
                }
            }
            *(b++)=0;
            return sc[num].addr;
        }
    }
    wsprintf(buffer,"Address %04X not found",num);
    MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
    doc->m_modf=1;
    return 0;
}
int Writespcdata(FDOC*doc,void*buf,int len,int addr,int spc,int limit)
{
    unsigned char*rom=doc->rom;
    if(!len) return addr;
    if(((addr+len+4)&0x7fff)>0x7ffb) {
        if(addr+5>limit) goto error;
        *(int*)(rom+addr)=0x00010140;
        rom[addr+4]=0xff;
        addr+=5;
    }
    if(addr+len+4>limit) {
error:
        MessageBox(framewnd,"Not enough space for sound data","Bad error happened",MB_OK);
        doc->m_modf=1;
        return 0xc8000;
    }
    *(short*)(rom+addr)=len;
    *(short*)(rom+addr+2)=spc;
    memcpy(rom+addr+4,buf,len);
    return addr+len+4;
}

// =============================================================================

void
Modifywaves(FDOC * const doc,
            int    const es)
{
    int i,j;
    ZWAVE*zw=doc->waves,*zw2=zw+es;
    j=doc->numwave;
    for(i=0;i<j;i++) {
        if(zw->copy==es) {
            if(zw->end!=zw2->end)
            {
                zw->end = zw2->end;
                
                zw->buf = (short*) realloc(zw->buf,
                                           zw->end << 1);
            }
            memcpy(zw->buf,zw2->buf,zw->end<<1);
            if(zw->lopst>=zw->end) zw->lflag=0;
            if(zw->lflag) zw->buf[zw->end]=zw->buf[zw->lopst];
        }
        zw++;
    }
}

void Savesongs(FDOC *doc)
{
    int i,j,k,l=0,m,n,o,p,q,r,t,u,v,w,a,e,f,g;
    unsigned short bank_next[4],bank_lwr[4];
    short *c, *d;
    unsigned char *rom, *b;
    
    SONG *s;
    SCMD *sc;
    SONGPART *sp;
    SSBLOCK *stbl, *sptbl, *trtbl, *pstbl;
    ZWAVE *zw, *zw2;
    ZINST *zi;
    SAMPEDIT *sed;
    
    short wtbl[128],x[16],y[18];
    unsigned char z[64];
    
    ss_num = 0;
    ss_size = 512;
    ss_next = 0;
    ssblt = malloc(512 * sizeof(SSBLOCK));
    
    // if the music has not been modified, return. 
    if(! (doc->m_modf) )
        return;
    
    // set it so the music has not been modified. (reset the status)
    doc->m_modf = 0;
    rom = doc->rom;
    
    SetCursor(wait_cursor);
    
    for(i = 0; i < 3; i++)
    {
        k = doc->numsong[i];
        
        for(j = 0; j < k; j++)
        {
            s = doc->songs[l++];
            
            if(!s)
                continue;
            
            s->flag &= -5;
            
            for(m = 0; m < s->numparts; m++)
            {
                sp = s->tbl[m];
                sp->flag &= -3;
            }
        }
    }
    
    j = doc->m_size;
    sc = doc->scmd;
    
    for(i = 0; i < j; i++)
    {
        sc->flag &= -13;
        sc++;
    }
    
    l = 0;
    
    for(i = 0; i < 3; i++)
    {
        k = doc->numsong[i];
        stbl = Allocspcblock(k<<1,i+1);
        
        for(j = 0; j < k; j++)
        {
            s = doc->songs[l++];
            
            if(!s)
            {
                ( (short*) (stbl->buf) )[j] = 0;
                
                continue;
            }
            
            if(s->flag & 4)
                goto alreadysaved;
            
            sptbl = Allocspcblock(((s->numparts+1)<<1)+(s->flag&2),(s->flag&1)?0:(i+1));
            
            for(m = 0; m < s->numparts; m++)
            {
                sp = s->tbl[m];
                
                if(sp->flag&2)
                    goto spsaved;
                
                trtbl = Allocspcblock(16,(sp->flag&1)?0:(i+1));
                
                p = 0;
                
                for(n = 0; n < 8; n++)
                {
                    o = Getblocktime(doc,sp->tbl[n],0);
                    
                    if(o > p)
                        p = o;
                }
                
                q = 1;
                
                for(n = 0; n < 8; n++)
                {
                    stle16b_i(trtbl->buf,
                              n,
                              Savescmd(doc, sp->tbl[n], p, q) );
                    
                    if( ldle16b_i(trtbl->buf, n) )
                        Addspcreloc(trtbl,n<<1),q=0;
                }
                
                sp->addr = trtbl->start;
                sp->flag |= 2;
spsaved:
                ( (short*) (sptbl->buf) )[m] = sp->addr;
                
                Addspcreloc(sptbl,m<<1);
            }
            
            if(s->flag&2)
            {
                ( (short*) (sptbl->buf) )[m++] = 255;
                ( (short*) (sptbl->buf) )[m] = sptbl->start + (s->lopst << 1);
                
                Addspcreloc(sptbl,m<<1);
            }
            else
                ( (short*) (sptbl->buf) )[m++] = 0;
            
            s->addr = sptbl->start;
            s->flag |= 4;
alreadysaved:
            ( (short*) (stbl->buf) )[j] = s->addr;
            
            Addspcreloc(stbl, j << 1);
        }
    }
    
    if(doc->w_modf)
    {
        b = (uint8_t*) malloc(0xc000);
        j = 0;
        
        zw = doc->waves;
        
        if(doc->mbanks[3])
            sed = (SAMPEDIT*)GetWindowLong(doc->mbanks[3],GWL_USERDATA);
        else
            sed = 0;
        
        for(i = 0; i < doc->numwave; i++, zw++)
        {
            if(zw->copy != -1)
                continue;
            
            wtbl[i << 1] = j + 0x4000;
            
            if(zw->lflag)
            {
                l = zw->end-zw->lopst;
                
                if(l&15)
                {
                    k = (l << 15) / ((l + 15) & -16);
                    p = (zw->end << 15)/k;
                    c = malloc(p << 1);
                    n = 0;
                    d = zw->buf;
                    
                    for(m = 0;;)
                    {
                        c[n++] = (d[m >> 15] * ( (m & 32767)^32767) + d[(m>>15)+1]*(m&32767))/32767;
                        
                        m += k;
                        
                        if(n >= p)
                            break;
                    }
                    
                    zw->lopst = (zw->lopst << 15) / k;
                    zw->end = p;
                    zw->buf = (short*) realloc(zw->buf, (zw->end + 1) << 1);
                    memcpy(zw->buf,c,zw->end<<1);
                    free(c);
                    zw->buf[zw->end]=zw->buf[zw->lopst];
                    zw2=doc->waves;
                    
                    for(m=0;m<doc->numwave;m++,zw2++)
                        if(zw2->copy==i)
                            zw2->lopst=zw2->lopst<<15/k;
                    
                    zi=doc->insts;
                    
                    for(m=0;m<doc->numinst;m++)
                    {
                        n=zi->samp;
                        
                        if(n>=doc->numwave)
                            continue;
                        
                        if(n==i || doc->waves[n].copy==i)
                        {
                            o=(zi->multhi<<8)+zi->multlo;
                            o=(o<<15)/k;
                            zi->multlo=o;
                            zi->multhi=o>>8;
                            
                            if(sed && sed->editinst==m)
                            {
                                sed->init=1;
                                SetDlgItemInt(sed->dlg,3014,o,0);
                                sed->init=0;
                            }
                        }
                        
                        zi++;
                    }
                    
                    Modifywaves(doc,i);
                }
        }
        k=(-zw->end)&15;
        d=zw->buf;
        n=0;
        wtbl[ (i << 1) + 1] = ( (zw->lopst + k) >> 4) * 9 + wtbl[i << 1];
        y[0]=y[1]=0;
        u=4;
        for(;;) {
            for(o=0;o<16;o++) {
                if(k) k--,x[o]=0;
                else x[o]=d[n++];
            }
            p=0x7fffffff;
            a=0;
            for(t=0;t<4;t++) {
                r=0;
                for(o=0;o<16;o++) {
                    l=x[o];
                    y[o+2]=l;
                    l+=(y[o]*fil3[t]>>4)-(y[o+1]*fil1[t]>>fil2[t]);
                    if(l>r) r=l;
                    else if(-l>r) r=-l;
                }
                r<<=1;
                if(t) m=14; else m=15;
                for(q=0;q<12;q++,m+=m) if(m>=r) break;
tryagain:
                if(q && (q<12 || m == r))
                    v = (1 << (q - 1) ) - 1;
                else
                    v = 0;
                m=0;
                for(o=0;o<16;o++) {
                    l=(y[o+1]*fil1[t]>>fil2[t])-(y[o]*fil3[t]>>4);
                    w=x[o];
                    r = (w - l + v) >> q;
                    if((r+8)&0xfff0) {q++; a-=o; goto tryagain;}
                    z[a++]=r;
                    l=(r<<q)+l;
                    y[o+2]=l;
                    l-=w;
                    m+=l*l;
                }
                if(u==4) { u=0,e=q,f=y[16],g=y[17]; break; }
                if(m<p) p=m,u=t,e=q,f=y[16],g=y[17];
            }
            m=(e<<4)|(u<<2);
            if(n==zw->end) m|=1;
            if(zw->lflag) m|=2;
            b[j++]=m;
            m=0;
            a=u<<4;
            for(o=0;o<16;o++) {
                m|=z[a++]&15;
                if(o&1) b[j++]=m,m=0; else m<<=4;
            }
            if(n==zw->end) break;
            y[0]=f;
            y[1]=g;
        }
    }
    if(sed) {
        SetDlgItemInt(sed->dlg,3008,sed->zw->end,0);
        SetDlgItemInt(sed->dlg,3010,sed->zw->lopst,0);
        InvalidateRect(GetDlgItem(sed->dlg,3002),0,1);
    }
    zw=doc->waves;
    for(i=0;i<doc->numwave;i++,zw++) if(zw->copy!=-1) {
            wtbl[i<<1]=wtbl[zw->copy<<1];
            wtbl[(i<<1)+1]=(zw->lopst>>4)*9+wtbl[i<<1];
        }
    m=Writespcdata(doc,wtbl,doc->numwave<<2,0xc8000,0x3c00,0xd74fc);
    m=Writespcdata(doc,b,j,m,0x4000,0xd74fc);
    free(b);
    m=Writespcdata(doc,doc->insts,doc->numinst*6,m,0x3d00,0xd74fc);
    m=Writespcdata(doc,doc->snddat1,doc->sndlen1,m,0x800,0xd74fc);
    m=Writespcdata(doc,doc->snddat2,doc->sndlen2,m,0x17c0,0xd74fc);
    m=Writespcdata(doc,doc->sndinsts,doc->numsndinst*9,m,0x3e00,0xd74fc);
    doc->m_ofs=m;
    } else m=doc->m_ofs;
    bank_next[0]=0x2880;
    bank_next[1]=0xd000;
    bank_next[2]=0xd000;
    bank_next[3]=0xd000;
    bank_lwr[0]=0x2880;
    for(k=0;k<4;k++) {
        pstbl=0;
        for(i=0;i<ss_num;i++) {
            stbl=ssblt[i];
            if(stbl->bank!=k) continue;
            j=bank_next[k];
            if(j+stbl->len>0xffc0) { if(k==3) j=0x2880; else j=bank_next[0]; bank_lwr[k]=j; pstbl=0; }
            if(j+stbl->len>0x3c00 && j<0xd000) {
                SetCursor(normal_cursor);
                wsprintf(buffer,"Not enough space for music bank %d",k);
                MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
                doc->m_modf=1;
                return;
            }
            if(pstbl && (pstbl->flag&1) && (stbl->flag&2)) j--,pstbl->len--;
            stbl->addr=j;
            pstbl=stbl;
            bank_next[k]=j+stbl->len;
        }
    }
    for(i=0;i<ss_num;i++) {
        stbl=ssblt[i];
        for(j=stbl->relnum-1;j>=0;j--) {
            k=*(unsigned short*)(stbl->buf+stbl->relocs[j]);
            for(l=0;l<ss_num;l++) {
                sptbl=ssblt[l];
                if(sptbl->start<=k && sptbl->len>k-sptbl->start) goto noerror;
            }
            MessageBox(framewnd,"Internal error","Bad error happened",MB_OK);
            doc->m_modf=1;
            return;
noerror:
            if(((!sptbl->bank) && stbl->bank<3)||(sptbl->bank==stbl->bank)) {
                *(unsigned short*)(stbl->buf+stbl->relocs[j])=sptbl->addr+k-sptbl->start;
            } else {
                wsprintf(buffer,"An address outside the bank was referenced",sptbl->bank,stbl->bank);
                MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
                doc->m_modf=1;
                return;
            }
        }
    }
    l=m;
    for(k=0;k<4;k++) {
        switch(k) {
        case 1:
            rom[0x914]=l;
            rom[0x918]=(l>>8)|128;
            rom[0x91c]=l>>15;
            break;
        case 2:
            l=0xd8000;
            break;
        case 3:
            l=m;
            rom[0x932]=l;
            rom[0x936]=(l>>8)|128;
            rom[0x93a]=l>>15;
            break;
        }
        for(o=0;o<2;o++) {
            n=l+4;
            for(i=0;i<ss_num;i++) {
                stbl=ssblt[i];
                if(!stbl) continue;
                if((stbl->addr<0xd000)^o) continue;
                if(stbl->bank!=k) continue;
                if(n+stbl->len>((k==2)?0xdb7fc:0xd74fc)) {
                    MessageBox(framewnd,"Not enough space for music","Bad error happened",MB_OK);
                    doc->m_modf=1;
                    return;
                }
                memcpy(rom+n,stbl->buf,stbl->len);
                n+=stbl->len;
                free(stbl->relocs);
                free(stbl->buf);
                free(stbl);
                ssblt[i]=0;
            }
            if(n>l+4) {
                *(short*)(rom+l)=n-l-4;
                *(short*)(rom+l+2)=o?bank_lwr[k]:0xd000;
                l=n;
            }
        }
        *(short*)(rom+l)=0;
        *(short*)(rom+l+2)=0x800;
        if(k==1) m=l+4;
    }
    free(ssblt);
    SetCursor(normal_cursor);
}

void
Loadsongs(FDOC * const doc)
{
    unsigned char*b,*c,*d;
    short*e;
    SONG*s,*s2;
    SONGPART*sp;
    SCMD*sc;
    ZWAVE*zw;
    int i,j,k,l=0,m,n,o,p,q,r,t,u;
    int range,filter;
    sc=doc->scmd=malloc(1024*sizeof(SCMD));
    doc->m_free=0;
    doc->m_size=1024;
    doc->srnum=0;
    doc->srsize=0;
    doc->sr=0;
    doc->sp_mark=0;
    b=doc->rom;
    sbank_ofs[1]=(b[0x91c]<<15)+((b[0x918]&127)<<8)+b[0x914];
    sbank_ofs[3]=(b[0x93a]<<15)+((b[0x936]&127)<<8)+b[0x932];
    for(i=0;i<1024;i++) sc[i].next=i+1,sc[i].prev=i-1;
    sc[1023].next=-1;
    for(i=0;i<3;i++) {
        
        b = Getspcaddr(doc,0xd000,i);
        
        for(j = 0; ; j++)
        {
            if((r=((unsigned short*)b)[j])>=0xd000)
            {
                r = (r - 0xd000) >> 1;
                break;
            }
        }
        
        doc->numsong[i]=r;
        for(j=0;j<r;j++) {
            k=((unsigned short*)b)[j];
            if(!k) doc->songs[l]=0;
            else
            {
                c = Getspcaddr(doc,k,i);
                
                if(!spcbank)
                    m = 0;
                else
                    m = l - j;
                
                for( ; m < l; m++)
                    if(doc->songs[m] && doc->songs[m]->addr == k)
                    {
                        (doc->songs[l]=doc->songs[m])->inst++;
                        
                        break;
                    }
                
                if(m == l)
                {
                    doc->songs[l] = s = malloc(sizeof(SONG));
                    s->inst=1;
                    s->addr=k;
                    s->flag=!spcbank;
                    
                    for(m=0;;m++)
                        if((n=((unsigned short*)c)[m])<256) break;
                    
                    if(n > 0)
                        s->flag |= 2,
                        s->lopst = ( ((unsigned short*)c)[m + 1] - k ) >> 1;
                    
                    s->numparts=m;
                    s->tbl=malloc(4*m);
                    for(m=0;m<s->numparts;m++) {
                        k=((unsigned short*)c)[m];
                        d=Getspcaddr(doc,k,i);
                        if(!spcbank) n=0; else n=l-j;
                        for(;n<l;n++) {
                            s2=doc->songs[n];
                            if(s2) for(o=0;o<s2->numparts;o++)
                                if(s2->tbl[o]->addr==k) {
                                    (s->tbl[m]=s2->tbl[o])->inst++;
                                    goto foundpart;
                                }
                        }
                        for(o=0;o<m;o++) if(s->tbl[o]->addr==k) {
                            (s->tbl[m]=s->tbl[o])->inst++;
                            goto foundpart;
                        }
                        sp=s->tbl[m]=malloc(sizeof(SONGPART));
                        sp->flag=!spcbank;
                        sp->inst=1;
                        sp->addr=k;
                        p=50000;
                        for(o=0;o<8;o++) {
                            q=sp->tbl[o]=Loadscmd(doc,((unsigned short*)d)[o],i,p);
                            sc=doc->scmd+q;
                            if((sc->flag&4) && sc->tim<p) p=sc->tim;
                        }
foundpart:;
                    }
                }
            }
            l++;
        }
    }
    b=Getspcaddr(doc,0x800,0);
    doc->snddat1=malloc(spclen);
    doc->sndlen1=spclen;
    memcpy(doc->snddat1,b,spclen);
    b=Getspcaddr(doc,0x17c0,0);
    doc->snddat2=malloc(spclen);
    doc->sndlen2=spclen;
    memcpy(doc->snddat2,b,spclen);
    b=Getspcaddr(doc,0x3d00,0);
    doc->insts=malloc(spclen);
    memcpy(doc->insts,b,spclen);
    doc->numinst=spclen/6;
    b=Getspcaddr(doc,0x3e00,0);
    doc->m_ofs=b-doc->rom+spclen;
    doc->sndinsts=malloc(spclen);
    memcpy(doc->sndinsts,b,spclen);
    doc->numsndinst=spclen/9;
    b=Getspcaddr(doc,0x3c00,0);
    zw=doc->waves=malloc(sizeof(ZWAVE)*(spclen>>2));
    p=spclen>>1;
    
    for(i=0;i<p;i+=2)
    {
        j=((unsigned short*)b)[i];
        
        if(j==65535) break;
        
        for(k=0;k<i;k+=2)
        {
            if(((unsigned short*)b)[k]==j)
            {
                zw->copy = (short) (k >> 1);
                goto foundwave;
            }
        }
        
        zw->copy=-1;
        
foundwave:
        
        d = Getspcaddr(doc,j,0);
        e = malloc(2048);
        
        k = 0;
        l = 1024;
        u = t = 0;
        
        for(;;) {
            m=*(d++);
            range=(m>>4)+8;
            filter=(m&12)>>2;
            for(n=0;n<8;n++) {
                o=(*d)>>4;
                if(o>7) o-=16;
                o<<=range;
                if(filter) o+=(t*fil1[filter]>>fil2[filter])-((u&-256)*fil3[filter]>>4);
                if(o>0x7fffff) o=0x7fffff;
                if(o < -0x800000) o = -0x800000;
                u=o;
// \code             if(t>0x7fffff) t=0x7fffff;
// \code              if(t < -0x800000) t=-0x800000;
                e[k++]=o>>8;
                o=*(d++)&15;
                if(o>7) o-=16;
                o<<=range;
                if(filter) o+=(u*fil1[filter]>>fil2[filter])-((t&-256)*fil3[filter]>>4);
                if(o>0x7fffff) o=0x7fffff;
                if(o < -0x800000) o = -0x800000;
                t=o;
// \code             if(u>0x7fffff) u=0x7fffff;
// \code             if(u < -0x800000) u= -0x800000;
                e[k++]=o>>8;
            }
            if(m&1) {zw->lflag=(m&2)>>1; break;}
            if(k==l) {l+=1024;e=realloc(e,l<<1);}
        }
        e = zw->buf = realloc(e, (k + 1) << 1);
        zw->lopst=(((unsigned short*)b)[i+1]-j)*16/9;
        if(zw->lflag) e[k]=e[zw->lopst]; else e[k]=0;
        zw->end=k;
        zw++;
    }
    doc->numwave=i>>1;
    doc->m_loaded=1;
    doc->w_modf=0;
}
void Edittrack(FDOC*doc,short i)
{
    int j,k,l;
    SRANGE*sr=doc->sr;
    SCMD*sc;
    k=doc->srnum;
    sc=doc->scmd;
    if(i==-1) return;
    if(i>=doc->m_size) {wsprintf(buffer,"Invalid address: %04X",i);goto error;}
    for(;;) {if((j=sc[i].prev)!=-1) i=j; else break;}
    for(l=0;l<k;l++) if(sr->first==i) break; else sr++;
    if(l==k) {
        wsprintf(buffer,"Not found: %04X",i);
error:
        MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
        return;
    }
    if(sr->editor) SendMessage(clientwnd,WM_MDIACTIVATE,(int)(sr->editor),0);
    else Editwin(doc,"TRACKEDIT","Song editor",l+(i<<16),sizeof(TRACKEDIT));
}
int sndinit=0;

FDOC * sounddoc=0;

int exitsound=0;

//CRITICAL_SECTION cs_song;

// =============================================================================

void
Playpatt(void)
{
    int i;
    ZCHANNEL*zch=zchans;
    SONGPART*sp;
    sp=playedsong->tbl[playedpatt];
    for(i=0;i<8;i++) zch->playing=1,zch->tim=1,zch->loop=0,(zch++)->pos=sp->tbl[i];
}

// =============================================================================

void
midinoteoff(ZCHANNEL * const zch)
{
    if(zch->midnote!=0xff)
    {
        HMIDIOUT const hmo = (HMIDIOUT) hwo;
        
        midiOutShortMsg(hmo,
                        (0x80 + zch->midch) | (zch->midnote << 8) | 0x400000);
        
        if(zch->midnote & 0xff00)
            midiOutShortMsg(hmo,
                            (0x80 + zch->midch) | (zch->midnote & 0xff00) | 0x400000);
    }
    
    zch->midnote = 0xff;
}

// =============================================================================

void
Stopsong(void)
{
    int i;
    ZMIXWAVE*zw=zwaves;
    sounddoc=0;
    if(sounddev<0x20000)
    for(i=0;i<8;i++) (zw++)->pflag=0;
    else for(i=0;i<8;i++) midinoteoff(zchans+i);
}

// =============================================================================

void CALLBACK
midifunc(UINT timerid,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
    (void) timerid, msg, inst, p1, p2;
    
    if(exitsound) {
//      if(exitsound==1) {
//          timeKillEvent(midi_timer);
//          exitsound=2;
//          SetEvent(wave_end);
//      }
        return;
    }
    Updatesong();
}

// =============================================================================

void CALLBACK
midifunc2(HWND win,UINT msg,UINT timerid,DWORD systime)
{
    (void) win, msg, timerid, systime;
    
    if(exitsound)
        return;
    
    Updatesong();
}

// =============================================================================

int CALLBACK
soundproc(HWAVEOUT bah,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
    int r;
    
    (void) bah, inst, p2;
    
    switch(msg) {
    case WOM_DONE:
        r=((WAVEHDR*)p1)->dwUser;
        if(exitsound) {
            if(exitsound==1) {
                exitsound=2;
                if(wave_end) SetEvent(wave_end);
            }
            return 0;
        }
        wcurbuf=r;
        if(r==wcurbuf) {
            while(wbufs[wcurbuf].dwFlags&WHDR_DONE) {
                Mixbuffer();
                wbufs[wcurbuf].dwFlags&=~WHDR_DONE;
                waveOutWrite(hwo,wbufs+wcurbuf,sizeof(WAVEHDR));
                wcurbuf=wcurbuf+1;
                if(wcurbuf==wnumbufs) wcurbuf=0;
            }
        }
    }
    
    return 0;
}

int CALLBACK soundfunc(LPVOID param)
{
    MSG msg;
    
    (void) param;
    
    while(GetMessage(&msg,0,0,0)) {
        switch(msg.message) {
        case MM_WOM_DONE:
            soundproc(0,WOM_DONE,0,msg.lParam,0);
            break;
        }
    }
    return 0;
}

void
Exitsound(void)
{
    int n;
    
    WAVEHDR*wh;
    
    if(!sndinit) return;
    exitsound=1;
    
    if(sounddev < 0x20000)
    {
        SetCursor(wait_cursor);
        if(soundthr) {
            WaitForSingleObject(wave_end,INFINITE);
        } else {
            while(WaitForSingleObject(wave_end,50)==WAIT_TIMEOUT);
        }
        SetCursor(normal_cursor);
        waveOutReset(hwo);
        wh=wbufs;
        for(n=0;n<wnumbufs;n++,wh++) waveOutUnprepareHeader(hwo,wh,sizeof(WAVEHDR));
        waveOutClose(hwo);
        CloseHandle(wave_end);
        free(wbufs);
        free(wdata);
        free(mixbuf);
    }
    else
    {
        if(wver)
        {
            KillTimer(framewnd,midi_timer);
        }
        else
        {
            timeKillEvent(midi_timer);
            timeEndPeriod(miditimeres);
        }
        
        for(n = 0; n < 8; n++)
            midinoteoff(zchans+n);
        
        midiOutClose((HMIDIOUT) hwo);
    }
    
    // DeleteCriticalSection(&cs_song);
    exitsound=0;
    sndinit=0;
    soundthr=0;
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

short AllocScmd(FDOC*doc)
{
    int i=doc->m_free;
    int j,k;
    SCMD*sc;
    if(i==-1) {
        j=doc->m_size;
        doc->m_size+=1024;
        sc=doc->scmd=realloc(doc->scmd,doc->m_size*sizeof(SCMD));
        k=1023;
        while(k--) sc[j].next=j+1,j++;
        sc[j].next=-1;
        k=1023;
        while(k--) sc[j].prev=j-1,j--;
        sc[j].prev=-1;
        i=j;
    } else sc=doc->scmd;
    doc->m_free=sc[doc->m_free].next;
    if(doc->m_free!=-1) sc[doc->m_free].prev=-1;
    return i;
}

// =============================================================================

void NewSR(FDOC*doc,int bank)
{
    SCMD*sc;
    SRANGE*sr;
    if(doc->srnum==doc->srsize) {
        doc->srsize+=16;
        doc->sr=realloc(doc->sr,doc->srsize*sizeof(SRANGE));
    }
    sr=doc->sr+doc->srnum;
    doc->srnum++;
    sr->first=AllocScmd(doc);
    sr->bank=bank;
    sr->editor=0;
    sc=doc->scmd+sr->first;
    sc->prev=-1;
    sc->next=-1;
    sc->cmd=128;
    sc->flag=0;
    Edittrack(doc,sr->first);
}

// =============================================================================

void
Removepatches(FDOC * const doc)
{
    int j,k;
    PATCH*p;
    j=doc->numpatch;
    p=doc->patches;
    for(k=0;k<j;k++) {
        memcpy(doc->rom+p->addr,p->pv,p->len);
        free(p->pv);
        p++;
    }
    for(k=0;k<doc->numseg;k++) {
        Changesize(doc,676+k,0);
    }
    doc->numpatch=0;
    free(doc->patches);
    doc->patches=0;
    doc->numseg=0;
}
HACCEL actab;
void ProcessMessage(MSG*msg)
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

BOOL CALLBACK
errorsproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    DWORD read_bytes = 0;
    
    int i;
    
    char *b;
    
    HANDLE h;
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        h = CreateFile("HMAGIC.ERR",GENERIC_READ,0,0,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
        
        if((int)h==-1)
            break;
        
        i = GetFileSize(h, 0);
        
        b = (char*) malloc(i + 1);
        
        ReadFile(h, b, i, &read_bytes, 0);
        
        CloseHandle(h);
        
        b[i] = 0;
        
        SetDlgItemText(win,IDC_EDIT1,b);
        
        free(b);
        
        break;
    
    case WM_COMMAND:
        
        if(wparam == IDCANCEL)
            EndDialog(win, 0);
    }
    
    return 0;
}

// =============================================================================

BOOL
FileTimeEarlier(FILETIME const p_left,
                FILETIME const p_right)
{
    if(p_left.dwHighDateTime < p_right.dwHighDateTime)
    {
        return TRUE;
    }
    else if(p_left.dwHighDateTime == p_right.dwHighDateTime)
    {
        if(p_left.dwLowDateTime < p_right.dwLowDateTime)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

// =============================================================================

int
Buildpatches(FDOC * const doc)
{
    DWORD q = 0;
    
    int i, j, k, l, r, s;
    
    ASMHACK*mod;
    MSG msg;
    PATCH*p;
    HANDLE h,h2,h3[2];
    FILETIME tim,tim2;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO sti;
    PROCESS_INFORMATION pinfo;
    char*m,*t;
    int*n,*o=0;
    j=doc->nummod;
    mod=doc->modules;
    Removepatches(doc);
    
    if(!j)
    {
        
    nomod:
        
        MessageBox(framewnd,"No modules are loaded","Bad error happened",MB_OK);
        return 0;
    }
    
    k = wsprintf(buffer,"\"%s\"",asmpath);
    
    l = 0;
    
    for(i = 0; i < j; i++, mod++)
    {
        if(mod->flag&1) continue;
        l++;
        t=buffer+k;
        k+=wsprintf(buffer+k," \"%s\"",mod->filename);
        m=strrchr(t,'.');
        if(!m) continue;
        if(strrchr(t,'\\')>m) continue;
        if(_stricmp(m,".asm")) continue;
        h=CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
        if(h==(HANDLE)-1) {
            wsprintf(buffer,"Unable to open %s",mod->filename);
            MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
            return 1;
        }
        GetFileTime(h,0,0,&tim);
        CloseHandle(h);
        *(int*)m='jbo.';
        h=CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
        if(h==(HANDLE)-1) goto buildnew;
        GetFileTime(h,0,0,&tim2);
        CloseHandle(h);
        
#if 0
        __asm
        {
            mov eax,tim.dwLowDateTime
            mov edx,tim.dwHighDateTime
            sub eax,tim2.dwLowDateTime
            sbb edx,tim2.dwHighDateTime
            jb nonew
        }
#else
        
        // \task Check that this actually works.
        if( FileTimeEarlier(tim, tim2) )
        {
            goto nonew;
        }
        
#endif
        
buildnew:
        *(int*)m='msa.';
nonew:;
    }
    if(!l) goto nomod;
    sa.nLength=12;
    sa.lpSecurityDescriptor=0;
    sa.bInheritHandle=1;
    h=CreateFileMapping((HANDLE)-1,&sa,PAGE_READWRITE,0,4096,0);
    if(!h) {
nomem:
        MessageBox(framewnd,"Not enough memory.","Bad error happened",MB_OK);
        return 1;
    }
    n=MapViewOfFile(h,FILE_MAP_WRITE,0,0,0);
    if(!n) {
        CloseHandle(h);
        goto nomem;
    }
    wsprintf(buffer+k," -l -h $%X -o HMTEMP.DAT",h);
    h2=CreateFile("HMAGIC.ERR",GENERIC_WRITE,0,&sa,CREATE_ALWAYS,0,0);
    sti.cb=sizeof(sti);
    sti.lpReserved=0;
    sti.lpDesktop=0;
    sti.lpTitle=0;
    sti.dwFlags=STARTF_USESTDHANDLES;
    sti.cbReserved2=0;
    sti.lpReserved2=0;
    sti.hStdInput=(HANDLE)-1;
    sti.hStdOutput=h2;
    sti.hStdError=h2;
    n[0]=(int)CreateEvent(&sa,0,0,0);
    n[1]=(int)CreateEvent(&sa,0,0,0);
    if(!CreateProcess(0,buffer,0,0,1,DETACHED_PROCESS,0,0,&sti,&pinfo)) {
        CloseHandle(h2);
        UnmapViewOfFile(n);
        CloseHandle(h);
        wsprintf(buffer,"Unable to start %s",asmpath);
        MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
        return 1;
    }
    CloseHandle(h2);
    j=0;
    k=0;
    l=0;
    h3[0]=(void*)(n[0]);
    h3[1]=pinfo.hProcess;
    for(;;) {
        switch(MsgWaitForMultipleObjects(2,h3,0,INFINITE,QS_ALLEVENTS)) {
        case WAIT_OBJECT_0:
            switch(n[2]) {
            case 0:
                if(n[3]>=3) goto error;
                if(doc->numseg==92) {
                    MessageBox(framewnd,"Too many segments","Bad error happened",MB_OK);
                    break;
                }
                doc->numseg++;
                doc->segs[k]=0;
                i=Changesize(doc, 0x802a4 + k, n[4]);
                n[3]=cpuaddr(i);
                k++;
                if(!i) goto error;
                goto regseg;
            case 1:
                if(doc->patches) goto error;
                i=n[6];
                if(n[3]<3) {
                    i=romaddr(i);
                    if(i>=0x100000) {error:n[2]=1;break;}
                    l++;
regseg:
                    if( !(j & 255))
                        o = realloc(o, (j + 512) << 2);
                    o[j]=i;
                    o[j+1]=n[4];
                    o[j+2]=n[5];
                    o[j+3]=n[2];
                    j+=4;
                }
                n[2]=0;
                break;
            case 2:
                doc->patches=malloc(l*sizeof(PATCH));
                for(i=0;i<j;i++) {
                    doc->patches[i].len=0;
                    doc->patches[i].addr=0;
                    doc->patches[i].pv=0;
                }
                break;
            default:
                n[2]=1;
            }
            SetEvent((HANDLE)n[1]);
            break;
        case WAIT_OBJECT_0+1:
            goto done;
        case WAIT_OBJECT_0+2:
            while(PeekMessage(&msg,0,0,0,PM_REMOVE)) {
                if(msg.message==WM_QUIT) break;
                ProcessMessage(&msg);
            }
        }
    }
done:
    GetExitCodeProcess(pinfo.hProcess,&q);
    if(q) {
        DialogBoxParam(hinstance,(LPSTR)IDD_DIALOG23,framewnd,errorsproc,0);
errors:
        Removepatches(doc);
    }
    else
    {
        DWORD read_bytes = 0;
        
        DeleteFile("HMAGIC.ERR");
        h2=CreateFile("HMTEMP.DAT",GENERIC_READ,0,0,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
        if(h2==(HANDLE)-1) {
            MessageBox(framewnd,"Unable to apply patch","Bad error happened",MB_OK);
            q=1;
            goto errors;
        }
        p=doc->patches=malloc(l*sizeof(PATCH));
        doc->numpatch=l;
        s=j;
        j>>=2;
        for(r=0;r<j;r++)
        {
            for(i=0;i<s;i+=4)
                if(o[i+2]==r)
                    break;
            
            if(o[i+3])
            {
                p->addr=o[i];
                p->len=o[i+1];
                p->pv=malloc(p->len);
                memcpy(p->pv,doc->rom+p->addr,p->len);
                p++;
            }
            
            ReadFile(h2,doc->rom+o[i],o[i+1], &read_bytes, 0);
        }
        
        CloseHandle(h2);
    }
    
    free(o);
    
    CloseHandle(pinfo.hThread);
    CloseHandle(pinfo.hProcess);
    
    UnmapViewOfFile(n);
    CloseHandle(h);
    
    if(!q)
    {
        GetSystemTimeAsFileTime(&(doc->lastbuild));
        doc->modf=1;
        doc->p_modf=0;
    }
    return q;
}
BOOL CALLBACK patchdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PATCHLOAD*ed;
    ASMHACK*mod;
    OPENFILENAME ofn;
    FDOC*doc;
    HWND hc;
    int i,j;
    char patchname[MAX_PATH];
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(PATCHLOAD*)lparam;
        ed->dlg=win;
        doc=ed->ew.doc;
        mod=doc->modules;
        hc=GetDlgItem(win,3000);
        j=doc->nummod;
        for(i=0;i<j;i++,mod++)
            SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)(doc->modules[i].filename));
        break;
    case WM_COMMAND:
        ed=(PATCHLOAD*)GetWindowLong(win,DWL_USER);
        doc=ed->ew.doc;
        switch(wparam) {
        case 3002:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="FSNASM source files\0*.ASM\0FSNASM module files\0*.OBJ\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=patchname;
            *patchname=0;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Load patch";
            ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            if(!GetOpenFileName(&ofn)) break;
            doc->nummod++;
            doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
            mod=doc->modules+doc->nummod-1;
            mod->filename=_strdup(patchname);
            mod->flag=0;
            SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)patchname);
            doc->p_modf=1;
            break;
        case 3003:
            i=SendDlgItemMessage(win,3000,LB_GETCURSEL,0,0);
            if(i==-1) break;
            SendDlgItemMessage(win,3000,LB_DELETESTRING,0,(int)patchname);
            doc->nummod--;
            memcpy(doc->modules+i,doc->modules+i+1,(doc->nummod-i)*sizeof(ASMHACK));
            doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
            doc->p_modf=1;
            break;
        case 3004:
            Buildpatches(doc);
            break;
        }
    }
    return 0;
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

char mus_min[3]={0,15,31};
char mus_max[3]={15,31,34};

BOOL CALLBACK musdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k;
    HWND hc;
    MUSEDIT*ed;
    FDOC*doc;
    SONG*s;
    SONGPART*sp;
    char const * st;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(MUSEDIT*)lparam;
        ed->dlg=win;
        ed->sel_song=-1;
        if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
        hc=GetDlgItem(win,3000);
        j=ed->ew.doc->numsong[ed->ew.param];
        for(i=0;i<j;i++) {
            if(i<mus_min[ed->ew.param] || i>=mus_max[ed->ew.param])
                st = buffer, wsprintf(buffer, "Song %d", i + 1);
            else
                st = mus_str[i + 2];
            
            SendMessage(hc,LB_ADDSTRING,0,(long)st);
        }
        break;
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case 3000|(LBN_SELCHANGE<<16):
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            doc=ed->ew.doc;
            j=ed->ew.param;
            while(j--) i+=doc->numsong[j];
            if(ed->sel_song!=i) {
                ed->sel_song=i;
songchg:
                hc=GetDlgItem(win,3009);
                SendMessage(hc,LB_RESETCONTENT,0,0);
                s=doc->songs[i];
                if(s) {
                    ShowWindow(hc,SW_SHOW);
                    for(j=0;j<s->numparts;j++) {
                        wsprintf(buffer,"Part %d",j);
                        SendMessage(hc,LB_ADDSTRING,0,(long)buffer);
                    }
                    SetDlgItemInt(win,3025,s->lopst,0);
                    CheckDlgButton(win,3026,(s->flag&2)?BST_CHECKED:BST_UNCHECKED);
                    ShowWindow(GetDlgItem(win,3025),(s->flag&2)?SW_SHOW:SW_HIDE);
                } else ShowWindow(hc,SW_HIDE);
                for(j=0;j<8;j++) EnableWindow(GetDlgItem(win,3001+j),(int)s);
            }
            break;
        case 3009|(LBN_DBLCLK<<16):
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            if(i==-1) break;
            if(!sndinit) Initsound();
            if(!sndinit) break;
//          EnterCriticalSection(&cs_song);
            Stopsong();
            playedpatt=i;
            sounddoc=ed->ew.doc;
            playedsong=ed->ew.doc->songs[ed->sel_song];
            Playpatt();
//          LeaveCriticalSection(&cs_song);
            break;
        case 3009|(LBN_SELCHANGE<<16):
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            if(i==-1) break;
            doc=ed->ew.doc;
            j=ed->sel_song;
            ed->init=1;
            for(k=0;k<8;k++) {
                wsprintf(buffer,"%04X",ed->ew.doc->songs[j]->tbl[i]->tbl[k]&65535);
                SetDlgItemText(win,3012+k,buffer);
            }
            ed->init=0;
            break;
        case 3001:
        case 3002:
        case 3003:
        case 3004:
        case 3005:
        case 3006:
        case 3007:
        case 3008:
            i=wparam-3001;
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            Edittrack(ed->ew.doc,ed->ew.doc->songs[ed->sel_song]->tbl[j]->tbl[i]);
            break;
        case 3010:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            if(ed->sel_song==-1 || !ed->ew.doc->songs[ed->sel_song]) break;
            if(!sndinit) Initsound();
            if(sndinit) Playsong(ed->ew.doc,ed->sel_song);
            break;
        case 3011:
            if(!sndinit) {sounddoc=0;break;}
//          EnterCriticalSection(&cs_song);
            Stopsong();
//          LeaveCriticalSection(&cs_song);
            break;
        case 3012|(EN_CHANGE<<16):
        case 3013|(EN_CHANGE<<16):
        case 3014|(EN_CHANGE<<16):
        case 3015|(EN_CHANGE<<16):
        case 3016|(EN_CHANGE<<16):
        case 3017|(EN_CHANGE<<16):
        case 3018|(EN_CHANGE<<16):
        case 3019|(EN_CHANGE<<16):
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            if(ed->init) break;
            GetWindowText((HWND)lparam,buffer,sizeof(buffer));
            k=strtol(buffer,0,16);
            i=wparam-(3012|(EN_CHANGE<<16));
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            s=ed->ew.doc->songs[ed->sel_song];
            if(!s) break;
            s->tbl[j]->tbl[i]=k;
            ed->ew.doc->m_modf=1;
            break;
        case 3020:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || ed->sel_song==-1) break;
            s=ed->ew.doc->songs[ed->sel_song];
            if(!s) break;
            ed->ew.doc->sp_mark=s->tbl[j];
            break;
        case 3021:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=ed->sel_song;
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            
            s->numparts++;
            s->tbl = (SONGPART**) realloc(s->tbl, s->numparts << 2);
            
            if(j != -1)
                MoveMemory(s->tbl + j + 1,
                           s->tbl + j,
                           (s->numparts - j) << 2);
            else
                j = s->numparts - 1;
            (s->tbl[j]=doc->sp_mark)->inst++;
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3022:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=ed->sel_song;
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            s->numparts++;
            s->tbl = (SONGPART**) realloc(s->tbl,s->numparts<<2);
            sp=s->tbl[s->numparts - 1] = (SONGPART*) malloc(sizeof(SONGPART));
            for(k=0;k<8;k++) sp->tbl[k]=-1;
            sp->flag=s->flag&1;
            sp->inst=1;
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3023:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=ed->sel_song;
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            if(j==-1 || i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(!s) break;
            s->numparts--;
            sp=s->tbl[j];
            sp->inst--;
            
            if(!sp->inst)
                free(sp);
            
            sp = (SONGPART*) CopyMemory(s->tbl + j,
                                        s->tbl + j + 1,
                                        (s->numparts - j) << 2);
            
            ed->ew.doc->m_modf=1;
            goto songchg;
        case 3024:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(!s) break;
            NewSR(ed->ew.doc,((j==-1)?(s->flag&1):(s->tbl[j]->flag&1))?0:ed->ew.param+1);
            break;
        case 3025|(EN_CHANGE<<16):
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            if(ed->init) break;
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(s) s->lopst=GetDlgItemInt(win,3025,0,0);
            ed->ew.doc->m_modf=1;
            break;
        case 3026:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=ed->sel_song;
            if(i==-1) break;
            s=ed->ew.doc->songs[i];
            if(s) {
                hc=GetDlgItem(win,3025);
                if((IsDlgButtonChecked(win,3026)==BST_CHECKED)) {
                    s->flag|=2;
                    ShowWindow(hc,SW_SHOW);
                } else {
                    s->flag&=-3;
                    ShowWindow(hc,SW_HIDE);
                }
                ed->ew.doc->m_modf=1;
            }
            break;
        case 3027:
            
            ed = (MUSEDIT*)GetWindowLong(win,DWL_USER);
            
            i = ed->sel_song;
            
            if(i == -1)
                break;
            
            doc = ed->ew.doc;
            
            if(doc->songs[i])
            {
                MessageBox(framewnd,
                           "Please delete the existing song first.",
                           "Bad error happened",
                           MB_OK);
                
                break;
            }
            
            s = doc->songs[i] = (SONG*) malloc(sizeof(SONG));
            
            s->flag=0;
            s->inst=1;
            s->numparts=0;
            s->tbl=0;
            
            goto updsongs;
        
        case 3028:
            ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
            i=ed->sel_song;
            if(i==-1) break;
            doc=ed->ew.doc;
            s=doc->songs[i];
            if(s==playedsong) {
                if(sndinit) {
//                  EnterCriticalSection(&cs_song);
                    Stopsong();
//                  LeaveCriticalSection(&cs_song);
                } else sounddoc=0;
            }
            doc->songs[i]=0;
            if(s) {
                s->inst--;
                if(!s->inst) {
                    for(i=0;i<s->numparts;i++) {
                        sp=s->tbl[i];
                        sp->inst--;
                        if(!sp->inst) free(sp);
                    }
                    free(s);
                }
updsongs:
                ed->ew.doc->m_modf=1;
                ed->sel_song=-1;
                musdlgproc(win,WM_COMMAND,3000|(LBN_SELCHANGE<<16),(long)GetDlgItem(win,3000));
            } else MessageBox(framewnd,"There is no song","Bad error happened",MB_OK);
            break;
        }
    }
    return FALSE;
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

const char * amb_str[9]={
    "Nothing",
    "Heavy rain",
    "Light rain",
    "Stop",
    "Earthquake",
    "Wind",
    "Flute",
    "Chime 1",
    "Chime 2"
};

// ==============================================================================

BOOL CALLBACK editovprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    HWND hc;
    
    int i,j,k;
    
    unsigned char * rom;
    
    int cb_ids[4] = {IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_COMBO7};
    int cb2_ids[4] = {IDC_COMBO4, IDC_COMBO5, IDC_COMBO6, IDC_COMBO8};
    int text_ids[4] = {IDC_STATIC2, IDC_STATIC3, IDC_STATIC4};
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        i = 0;
        
        if(oved->ew.param>=0x40)
        {
            for( ; i < 3; i++)
            {
                ShowWindow( GetDlgItem(win, cb_ids[i]), SW_HIDE);
                ShowWindow( GetDlgItem(win, cb2_ids[i]), SW_HIDE);
                ShowWindow( GetDlgItem(win, text_ids[i]), SW_HIDE);
            }
            
            ShowWindow( GetDlgItem(win, IDC_STATIC5), SW_HIDE);
        }
        
        rom = oved->ew.doc->rom;
        
        for( ; i < 4; i++)
        {
            k = rom[0x14303 + oved->ew.param + (i << 6)];
            
            if(k >= 16)
                k += 16;
            
            hc = GetDlgItem(win, cb_ids[i]);
            
            for(j = 0; j < 16; j++)
                SendMessage(hc,CB_ADDSTRING,0,(long)mus_str[j+1]);
            
            SendMessage(hc,CB_SETCURSEL,k&15,0);
            
            hc = GetDlgItem(win, cb2_ids[i]);
            
            for(j = 0; j < 9; j++)
                SendMessage(hc,CB_ADDSTRING,0,(long)amb_str[j]);
            
            SendMessage(hc,CB_SETCURSEL,k>>5,0);
        }
        
        SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x3f51d))[oved->ew.param],0);
        
        break;
    
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case IDOK:
            
            rom = oved->ew.doc->rom;
            
            if(oved->ew.param >= 0x40)
                i = 3;
            else
                i = 0;
            
            for( ; i < 4; i++)
            {
                hc=GetDlgItem(win,cb_ids[i]);
                k=SendMessage(hc,CB_GETCURSEL,0,0);
                
                hc=GetDlgItem(win,cb2_ids[i]);
                k|=SendMessage(hc,CB_GETCURSEL,0,0)<<5;
                
                if(k >= 32)
                    k -= 16;
                
                rom[0x14303 + oved->ew.param + (i << 6)] = k;
            }
            
            ((short*)(rom + 0x3f51d))[oved->ew.param]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            
            oved->ew.doc->modf;
        
        case IDCANCEL:
            
            EndDialog(win,0);
        }
    }
    
    return 0;
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

static char const * screen_text[] = {
    "Title screen",
    "Naming screen",
    "Player select",
    "Copy screen",
    "Erase screen",
    "Map screen",
    "Load list",
    "Load screen",
    "Erase list",
    "Copy list",
    "Destination list"
};

char const * level_str[] =
{
    "None",
    "Church",
    "Castle",
    "East",
    "Desert",
    "Agahnim",
    "Water",
    "Dark",
    "Mud",
    "Wood",
    "Ice",
    "Tower",
    "Town",
    "Mountain",
    "Agahnim 2"
};

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

void Paintfloor(LMAPEDIT * const ed)
{
    int i,j,k,l,m,n,o,p,q;
    
    uint16_t * const nbuf = ed->nbuf;
    
    uint8_t const * const rom = ed->ew.doc->rom;
    
    m=ed->ew.param;
    
    for(i=0x6f;i<0x1ef;i+=32)
        nbuf[i+1] = nbuf[i] = 0xf00;
    
    if(ed->curfloor>=0) {
        nbuf[0x1af]=((short*)(rom + 0x564e9))[ed->curfloor]&0xefff;
        nbuf[0x1b0]=0xf1d;
    } else {
        nbuf[0x1af]=0xf1c;
        nbuf[0x1b0]=((short*)(rom + 0x564e9))[ed->curfloor^-1]&0xefff;
    }
    for(i=0;i<4;i++) {
        nbuf[((short*)(rom + 0x56431))[i]>>1]=((short*)(rom + 0x56429))[i]&0xefff;
    }
    for(j=0;j<2;j++) {
        k=((short*)(rom + 0x5643d))[j]>>1;
        for(i=0;i<10;i++) {
            nbuf[(k+i)&0x7ff]=((short*)(rom + 0x56439))[j]&0xefff;
        }
    }
    for(j=0;j<2;j++) {
        k=((short*)(rom + 0x56445))[j]>>1;
        for(i=0;i<0x140;i+=32) {
            nbuf[(k+i)&0x7ff]=((short*)(rom + 0x56441))[j]&0xefff;
        }
    }
    k=(ed->basements+ed->curfloor)*25;
    n=0x92;
    for(o=0;o<25;o+=5) {
        for(i=0;i<5;i++) {
            l=ed->rbuf[k];
            if(l==15) j=0x51; else {
                q=0;
                for(p=0;;p++) {
                    j=ed->rbuf[p];
                    if(j!=15) if(j==l) break; else q++;
                }
                j=ed->buf[q];
            }
            j<<=2;
            l=((short*)(rom + 0x57009))[j];
            nbuf[n]=l;
            l=((short*)(rom + 0x5700b))[j];
            nbuf[n+1]=l;
            l=((short*)(rom + 0x5700d))[j];
            nbuf[n+32]=l;
            l=((short*)(rom + 0x5700f))[j];
            nbuf[n+33]=l;
            n+=2;
            k++;
        }
        n+=54;
    }
}

void Updtmap(TMAPEDIT*ed)
{
    int i,j,k,l;
    unsigned char nb[0x6000];
    unsigned char*b=ed->buf;
    for(i=0;i<0x2000;i++) ((short*)nb)[i]=ed->back1;
    for(;i<0x3000;i++) ((short*)nb)[i]=ed->back2;
    for(;;) {
        j=(*b<<8)+b[1];
        if(j>=32768) break;
        if(j>=0x6000) j-=0x4000;
        j+=j;
        k=((b[2]&63)<<8)+b[3]+1;
        l=b[2];
        b+=4;
        if(l&64) k++;
        k>>=1;
        if(l&128) {
            if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=64; b+=2; }
            else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=64;
        } else {
            if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=2; b+=2; }
            else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=2;
        }
    }
    for(i=0;i<0x6000;i+=64) {
        b=(unsigned char*)(ed->nbuf+((i&0x7000)>>1)+(i&0x7c0)+((i&0x800)>>6));
        for(j=0;j<64;j+=2) *(short*)(b+j)=*(short*)(nb+i+j);
    }
}
void Gettmapsize(unsigned char*b,RECT*rc,int*s,int*t,int*u)
{
    int k,l,m,n,o,p,q,r;
    k=(*b<<8)+b[1];
    m=b[2];
    l=((m&63)<<8)+b[3]+1;
    n=(k&31)+((k&1024)>>5);
    o=((k&992)>>5)+((k&14336)>>6);
    if(!s) s=t=u=&r;
    if(m&64) l++,*s=6; else *s=l+4;
    *t=k;
    *u=l;
    l>>=1;
    
    if(m & 128)
        p = n + 1, q = o + l;
    else
        p = n + l, q = o + 1;
    
    r = ( (p - 1) >> 5) - (n >> 5);
    if(r) q+=r,n-=(n&31),p=n+32;
    rc->left=n<<3;
    rc->top=o<<3;
    rc->right=p<<3;
    rc->bottom=q<<3;
}
short cost[256];
int rptx,rpty,rptz;
void Get3dpt(PERSPEDIT*ed,int x,int y,int z,POINT*pt)
{
    int cx,cy,cz,sx,sy,sz,a,b,c,d;
    cx=cost[ed->xrot];
    cy=cost[ed->yrot];
    cz=cost[ed->zrot];
    sx=cost[(ed->xrot+192)&255];
    sy=cost[(ed->yrot+192)&255];
    sz=cost[(ed->zrot+192)&255];
    
    a = (cy * x - sy * z) >> 14; // A=X1
    b = (cy * z + sy * x) >> 14; // B=Z1
    c = (cx * b - sx * y) >> 14; // C=Z2
    b = (cx * y + sx * b) >> 14; // B=Y2
    d = (cz * a - sz * b) >> 14; // D=X3
    a = (cz * b + sz * a) >> 14; // A=Y3
    
    c+=250;
//  a=cx*x-sx*y>>14;
//  b=cx*y+sx*x>>14;
//  c=cz*z-sz*b>>14;
//  b=cz*b+sz*z>>14;
//  d=cy*a-sy*c>>14;
//  a=(cy*c+sy*a>>14)+250;
    pt->x=d*ed->scalex/c+(ed->width>>1);
    pt->y=a*ed->scaley/c+(ed->height>>1);
    rptx=d;
    rpty=a;
    rptz=c;
}
/*
void Perspselchg(PERSPEDIT*ed,HWND win)
{
    RECT rc;
    POINT pt;
    int j;
    if(ed->selpt==-1) return;
    j=*(unsigned short*)(ed->buf+2+ed->objsel)- 0xff8c + 3*ed->selpt;
    Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],&pt);
    rc.left=pt.x-3;
    rc.right=pt.x+3;
    rc.top=pt.y-3;
    rc.bottom=pt.y+3;
    InvalidateRect(win,&rc,1);
    rc.left=0;
    rc.right=200;
    rc.top=0;
    rc.bottom=24;
    InvalidateRect(win,&rc,1);
}
*/
BOOL CALLBACK perspdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    HWND hc;
    PERSPEDIT*ed;
    int i,j;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(PERSPEDIT*)lparam;
        ed->xrot=ed->yrot=ed->zrot=0;
        ed->objsel=0;
        ed->tool=0;
        ed->selpt=-1;
        ed->enlarge=100;
        ed->newlen=0;
        ed->newptp=-1;
        ed->modf=0;
        memcpy(ed->buf,ed->ew.doc->rom + 0x4ff8c,116);
        i=*(unsigned short*)(ed->buf+10) - 0xff8c;
        j=ed->buf[7];
        for(;j;j--) i+=ed->buf[i]+2;
        ed->len=i;
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,lparam);
        Updatesize(hc);
        SendDlgItemMessage(win,3001,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[0]);
        SendDlgItemMessage(win,3002,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[1]);
        wsprintf(buffer,"Free: %d",116-ed->len);
        SetDlgItemText(win,3003,buffer);
        CheckDlgButton(win,3004,BST_CHECKED);
        break;
    case WM_COMMAND:
        ed=(PERSPEDIT*)GetWindowLong(win,DWL_USER);
        switch(wparam) {
        case 3001:
            ed->objsel=0;
            goto updsel;
        case 3002:
            ed->objsel=6;
            goto updsel;
        case 3004:
            ed->tool=0;
updsel:
            ed->newptp=-1;
            ed->newlen=0;
            ed->selpt=-1;
            InvalidateRect(GetDlgItem(win,3000),0,1);
            break;
        case 3005:
            ed->tool=1;
            goto updsel;
        case 3006:
            ed->tool=2;
            goto updsel;
        case 3007:
            ed->tool=3;
            goto updsel;
        }
    }
    return FALSE;
}

int tmap_ofs[]={0x64ed0,0x64e5f,0x654c0,0x65148,0x65292};

BOOL CALLBACK tmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT*ed;
    int i,k,l,m;
    unsigned char*rom;
    BLOCKSEL8*bs;
    HWND hc;
    HPALETTE oldpal;
    HDC hdc;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(TMAPEDIT*)lparam;
        ed->hpal=0;
        ed->bmih=zbmih;
        ed->anim=3;
        ed->modf=0;
        ed->sel=-1;
        ed->selbg=0;
        ed->tool=0;
        ed->withfocus=0;
        ed->mapscrollh=ed->mapscrollv=0;
        rom=ed->ew.doc->rom;
        ed->disp=3;
        ed->layering=98;
        if(!ed->ew.param) {
            ed->gfxtmp=rom[0x64207];
            ed->sprgfx=rom[0x6420c];
            ed->gfxnum=rom[0x64211];
            ed->comgfx=rom[0x6433d];
            ed->anigfx=rom[0x64223];
            ed->pal1=rom[0x145d6];
            ed->pal6=ed->pal2=rom[0x145db];
            ed->pal3=rom[0x145e8];
            ed->pal4=rom[0x145ed];
            ed->pal5=rom[0x145e3];
            
            i = 0;
            
            Loadpal(ed, rom, 0x1be6c8 + 70 * ed->pal1, 0x21, 7, 5);
            Loadpal(ed, rom, 0x1be86c + 42 * ed->pal2, 0x29, 7, 3);
            Loadpal(ed, rom, 0x1be86c + 42 * ed->pal6, 0x59, 7, 3);
            Loadpal(ed, rom, 0x1bd446 + 14 * ed->pal3, 0xe9, 7, 1);
            Loadpal(ed, rom, 0x1bd39e + 14 * ed->pal4, 0x81, 7, 1);
            Loadpal(ed, rom, 0x1be604 + 14 * ed->pal5, 0x71, 7, 1);
            Loadpal(ed, rom, 0x1bd218, 0x91, 15, 4);
            Loadpal(ed, rom, 0x1bd6a0, 0, 16, 2);
            
            ed->back1 = ed->back2 = 0x1ec;
        } else {
            i=ed->ew.param+1;
            if(i==6) {
                Getblocks(ed->ew.doc,220);
                Getblocks(ed->ew.doc,221);
                Getblocks(ed->ew.doc,222);
                ed->gfxtmp=32;
                ed->gfxnum=64;
                ed->sprgfx=128;
                ed->anigfx=90;
                ed->comgfx=116;
                Loadpal(ed,rom,0x1be544,0x20,16,6);
                Loadpal(ed,rom,0x1bd6a0,0,16,2);
                Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
                Loadpal(ed,rom,0x1bd642,0xd9,3,1);
                Loadpal(ed,rom,0x1bd658,0xdc,4,1);
                Loadpal(ed,rom,0x1bd344,0xf1,15,1);
                ed->layering=100;
                ed->back1=768;
                ed->back2=127;
            } else {
                ed->gfxtmp=rom[0x64dd5];
                ed->sprgfx=rom[0x6420c];
                ed->gfxnum=rom[0x64dda];
                ed->comgfx=rom[0x64dd0];
                ed->anigfx=rom[0x64223];
                ed->pal1=rom[0x64db4]>>1;
                ed->pal2=rom[0x64dc4];
                Loadpal(ed,rom, 0x1bd734 + 180 * ed->pal1,0x21,15,6);
                Loadpal(ed,rom, 0x1bd734 + 180 * ed->pal1,0x91,7,1);
                Loadpal(ed,rom, 0x1be604, 0x71, 7, 1);
                Loadpal(ed,rom, 0x1bd660 + 32 * ed->pal2,0,16,2);
                ed->back1=127;
                ed->back2=169;
            }
        }
        ed->datnum=i;
        if(i<7) {
            k = romaddr(rom[0x137d + i] + (rom[0x1386 + i]<<8) + (rom[0x138f + i] << 16));
            i=0;
            for(;;) {
                if(rom[k+i]>=128) break;
                if(rom[k+i+2]&64) i+=6; else i+=((rom[k+i+2]&63)<<8)+rom[k+i+3]+5;
            }
            i++;
        } else {
            k=*(short*)(rom+tmap_ofs[i-7]) + 0x68000;
            if(i==7 || i==9) k++;
            switch(i) {
            case 7:
                i=*(unsigned short*)(rom + 0x64ecd);
                break;
            case 8:
                i=*(unsigned short*)(rom + 0x64e5c)+3;
                ed->buf=malloc(i);
                memcpy(ed->buf,rom+k,i-1);
                ed->buf[i-1]=255;
                goto loaded;
            case 9:
                i=*(unsigned short*)(rom + 0x654bd);
                break;
            case 10:
                i=*(unsigned short*)(rom + 0x65142)+1;
                break;
            case 11:
                i=*(unsigned short*)(rom + 0x6528d)+1;
                break;
            }
        }
        ed->buf=malloc(i);
        memcpy(ed->buf,rom+k,i);
loaded:
        ed->len=i;
        
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        k = 0x6073 + (ed->gfxtmp << 3);
        l = 0x5d97 + (ed->gfxnum << 2);
        for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
        for(i=3;i<7;i++) if(m=rom[l++]) ed->blocksets[i]=m;
        ed->blocksets[8]=ed->anigfx;
        ed->blocksets[9]=ed->anigfx+1;
        ed->blocksets[10]=ed->comgfx + 0x73;
        k = 0x5b57 + (ed->sprgfx << 2);
        for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k+i] + 0x73;
        for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
        if(ed->datnum!=6) Getblocks(ed->ew.doc,224);
        bs=&(ed->bs);
        Addgraphwin((DUNGEDIT*)ed,1);
        Setdispwin((DUNGEDIT*)ed);
        hdc=GetDC(win);
        hc=GetDlgItem(win,3001);
        bs->ed=(OVEREDIT*)ed;
        InitBlksel8(hc,bs,ed->hpal,hdc);
        ReleaseDC(win,hdc);
        Updtmap(ed);
        CheckDlgButton(win,3002,BST_CHECKED);
        CheckDlgButton(win,3004,BST_CHECKED);
        CheckDlgButton(win,3012,BST_CHECKED);
        CheckDlgButton(win,3013,BST_CHECKED);
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,(long)ed);
        Updatesize(hc);
        SetDlgItemInt(win,3008,0,0);
        break;
    case WM_DESTROY:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        ed->ew.doc->tmaps[ed->ew.param]=0;
        Delgraphwin((DUNGEDIT*)ed);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        for(i=0;i<15;i++) Releaseblks(ed->ew.doc,ed->blocksets[i]);
        if(ed->datnum==6) {
            Releaseblks(ed->ew.doc,220);
            Releaseblks(ed->ew.doc,221);
            Releaseblks(ed->ew.doc,222);
        } else Releaseblks(ed->ew.doc,224);
        free(ed->buf);
        free(ed);
        break;
    case 4002:
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        break;
    case 4000:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        i=wparam&0x3ff;
        hc=GetDlgItem(win,3001);
        Changeblk8sel(hc,&(ed->bs));
        ed->bs.sel=i;
        Changeblk8sel(hc,&(ed->bs));
        break;
    case WM_COMMAND:
        ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
        bs=&(ed->bs);
        switch(wparam) {
        case (EN_CHANGE<<16)+3008:
            bs->flags&=0xe000;
            bs->flags|=(GetDlgItemInt(win,3008,0,0)&7)<<10;
updflag:
            if((bs->flags&0xdc00)!=bs->oldflags) {
                bs->oldflags=bs->flags&0xdc00;
updblk:
                oldpal=SelectPalette(objdc,ed->hpal,1);
                SelectPalette(bs->bufdc,ed->hpal,1);
                for(i=0;i<256;i++) Updateblk8sel(bs,i);
                SelectPalette(objdc,oldpal,1);
                SelectPalette(bs->bufdc,oldpal,1);
                InvalidateRect(GetDlgItem(win,3001),0,0);
            }
            break;
        case 3002:
            ed->selbg=0;
            ed->bs.dfl=0;
            goto updblk;
            break;
        case 3003:
            ed->selbg=1;
            ed->bs.dfl=0;
            goto updblk;
        case 3004:
            ed->tool=0;
            break;
        case 3005:
            ed->tool=1;
            break;
        case 3006:
            ed->selbg=2;
            if(ed->datnum==6) ed->bs.dfl=16;
            else ed->bs.dfl=8;
            goto updblk;
        case 3009:
            bs->flags^=0x4000;
            goto updflag;
        case 3010:
            bs->flags^=0x8000;
            goto updflag;
        case 3011:
            bs->flags^=0x2000;
            goto updflag;
        case 3012:
            ed->disp^=1;
upddisp:
            InvalidateRect(GetDlgItem(win,3000),0,1);
            break;
        case 3013:
            ed->disp^=2;
            goto upddisp;
        }
    }
    return FALSE;
}
BOOL CALLBACK lmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    LMAPEDIT*ed;
    unsigned char*rom;
    HWND hc;
    int i,j,k,l,m;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(LMAPEDIT*)lparam;
        rom=ed->ew.doc->rom;
        ed->hpal=0;
        ed->init=1;
        ed->modf=0;
        m=ed->ew.param;
        SetDlgItemInt(win,3002,ed->level=(((char)rom[0x56196 + m])>>1)+1,0);
        i=((short*)(rom + 0x575d9))[m];
        ed->floors=(i>>4)&15;
        ed->basements=i&15;
        ed->bg=i>>8;
        if(i&256) CheckDlgButton(win,3008,BST_CHECKED);
        if(i&512) CheckDlgButton(win,3009,BST_CHECKED);
        ed->bossroom=((short*)(rom + 0x56807))[m];
        ed->bossofs=((short*)(rom + 0x56e5d))[m];
        SendDlgItemMessage(win,3006,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[2]);
        SendDlgItemMessage(win,3007,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[3]);
        CheckDlgButton(win,3011,BST_CHECKED);
        ed->tool=0;
        i=25*(ed->floors+ed->basements);
        ed->rbuf=malloc(i);
        memcpy(ed->rbuf,rom + 0x58000 + ((short*)(rom + 0x57605))[m],i);
        k=0;
        for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
        ed->buf=malloc(k);
        memcpy(ed->buf,rom+((short*)(rom+*(short*)(rom + 0x56640) + 0x58000))[m] + 0x58000,k);
        if(ed->bossroom==15) ed->bosspos=-1;
        else for(j=0;j<i;j++) if(ed->rbuf[j]==ed->bossroom) {ed->bosspos=j;break;}
        ed->len=k;
        ed->init=0;
        ed->gfxtmp=0x20;
        ed->disp=0;
        ed->blksel=0;
        k=0x6173;
        j=0x5e97;
        for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
        for(i=3;i<7;i++) if(l=rom[j++]) ed->blocksets[i]=l;
        k = 0x5b57 + ((128 | m) << 2);
        for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k++] + 0x73;
        ed->blocksets[8]=90;
        ed->blocksets[9]=91;
        ed->blocksets[10]=116;
        for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
        ed->bmih=zbmih;
        ed->anim=0;
        Loadpal(ed,rom,0x1be544,0x20,16,6);
        Loadpal(ed,rom,0x1bd6a0,0,16,2);
        Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
        Loadpal(ed,rom,0x1bd642,0xd9,3,1);
        Loadpal(ed,rom,0x1bd658,0xdc,4,1);
        Loadpal(ed,rom,0x1bd344,0xf1,15,1);
        
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        ed->curfloor=0;
        ed->sel=0;
        if(!ed->floors) ed->curfloor=-1;
        hc=GetDlgItem(win,3010);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        ed->blkscroll=0;
        hc=GetDlgItem(win,3004);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        Updatesize(hc);
        Addgraphwin((DUNGEDIT*)ed,2);
paintfloor:
        Paintfloor(ed);
updscrn:
        hc=GetDlgItem(win,3010);
        InvalidateRect(hc,0,0);
        break;
    case WM_COMMAND:
        ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
        if(!ed) break;
        rom=ed->ew.doc->rom;
        if(ed->init) break;
        switch(wparam) {
        case 3000:
            ed->tool=2;
            goto updscrn;
        case 3002|(EN_CHANGE<<16):
            ed->level=GetDlgItemInt(win,3002,0,0);
            ed->modf=1;
            break;
        case 3005: // The grid checkbox in the dungeon map editor.
            ed->disp^=1;
            InvalidateRect(GetDlgItem(win,3004),0,0);
            InvalidateRect(GetDlgItem(win,3010),0,0);
            break;
        case 3006:
            if(ed->curfloor==ed->floors-1) break;
            ed->curfloor++;
            goto paintfloor;
        case 3007:
            if(ed->curfloor==-ed->basements) break;
            ed->curfloor--;
            goto paintfloor;
        case 3008:
            ed->bg^=1;
            ed->modf=1;
            break;
        case 3009:
            ed->bg^=1;
            ed->modf=1;
            break;
        case 3011:
            ed->tool=0;
            goto updscrn;
        case 3012:
            ed->tool=1;
            goto updscrn;
        case 3013:
            if(ed->basements+ed->floors==2) {
                MessageBox(framewnd,"There must be at least two floors.","Bad error happened",MB_OK);
                break;
            }
            i=(ed->curfloor+ed->basements)*25;
            k=0;
            for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
            l=0;
            for(j=0;j<25;j++) if(ed->rbuf[j+i]!=15) l++;
            memcpy(ed->buf+k,ed->buf+k+l,ed->len-k-l);
            ed->len-=l;
            ed->buf=realloc(ed->buf,ed->len);
            memcpy(ed->rbuf+i,ed->rbuf+i+25,(ed->floors-ed->curfloor-1)*25);
            if(ed->bosspos>=i) ed->bosspos-=25;
            if(ed->bosspos>=i) ed->bossroom=15,ed->bosspos=-1;
            ed->rbuf=realloc(ed->rbuf,(ed->floors+ed->basements)*25);
            if(ed->curfloor>=0) ed->floors--;
            else ed->basements--;
            if(ed->curfloor>=ed->floors) ed->curfloor--;
            if(-ed->curfloor>ed->basements) ed->curfloor++;
            ed->modf=1;
            Paintfloor(ed);
            goto updscrn;
        case 3016:
            ed->curfloor++;
        case 3017:
            i=(ed->curfloor+ed->basements)*25;
            k=0;
            for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
            if(ed->curfloor>=0) ed->floors++;
            else ed->basements++,ed->curfloor--;
            l=(ed->floors+ed->basements)*25;
            ed->rbuf=realloc(ed->rbuf,l);
            memmove(ed->rbuf+i+25,ed->rbuf+i,l-i-25);
            if(ed->bosspos>i) ed->bosspos+=25;
            for(j=0;j<25;j++) ed->rbuf[i+j]=15;
            ed->modf=1;
            Paintfloor(ed);
            goto updscrn;
        }
        break;
    case WM_DESTROY:
        ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
        Delgraphwin((DUNGEDIT*)ed);
        ed->ew.doc->dmaps[ed->ew.param]=0;
        for(i=0;i<15;i++) Releaseblks(ed->ew.doc,ed->blocksets[i]);
        free(ed->rbuf);
        free(ed->buf);
        free(ed);
        break;
    }
    return FALSE;
}

// =============================================================================

BOOL CALLBACK
findblks(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    DWORD read_bytes  = 0;
    DWORD write_bytes = 0;
    
    OVEREDIT*ed;
    HWND hc;
    HANDLE h;
    static OPENFILENAME ofn;
    static char blkname[MAX_PATH]="findblks.dat";
    
    int i,j;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(OVEREDIT*)lparam;
        ed->schpush=-1;
        ed->schtyped=0;
        hc=GetDlgItem(win,IDC_CUSTOM1);
        SetWindowLong(hc,GWL_USERDATA,lparam);
        Updatesize(hc);
        break;
    case WM_COMMAND:
        ed=(OVEREDIT*)GetWindowLong(win,DWL_USER);
        switch(wparam) {
        case IDC_BUTTON1:
            for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=255;
upddisp:
            InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
            break;
        case IDC_BUTTON2:
            for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON3:
            for(i=0;i<0x1d5;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON4:
            for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=0;
            goto upddisp;
        case IDC_BUTTON5:
            for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=255;
            goto upddisp;
        case IDC_BUTTON6:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=blkname;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Load block filter";
            ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            
            if(!GetOpenFileName(&ofn))
                break;
            
            h = CreateFile(ofn.lpstrFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
            
            ReadFile(h, ed->selsch, 1408, &read_bytes, 0);
            CloseHandle(h);
            goto upddisp;
        case IDC_BUTTON7:
            ofn.lStructSize=sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=blkname;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Save block filter";
            ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt=0;
            ofn.lpfnHook=0;
            if(!GetSaveFileName(&ofn)) break;
            h=CreateFile(ofn.lpstrFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,0);
            WriteFile(h,ed->selsch,1408, &write_bytes, 0);
            CloseHandle(h);
            break;
        case IDOK:
            
            j = 0;
            
            for(i=0;i<0xea8;i++)
            {
                if(ed->selsch[i>>3]&(1<<(i&7)))
                {
                    if(j==4)
                    {
                        MessageBox(framewnd,
                                   "Please check only up to 4 \"Yes\" boxes.",
                                   "Bad error happened",
                                   MB_OK);
showpos:
                        ed->schscroll = Handlescroll
                        (
                            GetDlgItem(win, IDC_CUSTOM1),
                            SB_THUMBPOSITION + ( ( i - (ed->schpage >> 1) ) << 16),
                            ed->schscroll,
                            ed->schpage,
                            SB_VERT,
                            0xea8,
                            16
                        );
                        
                        goto brk;
                    }
                    
                    if(ed->selsch[(i + 0xea8) >> 3] & (1 << (i & 7) ) )
                    {
                        MessageBox(framewnd,"You can't require and disallow a block at the same time.","Bad error happened",MB_OK);
                        goto showpos;
                    }
                    
                    ed->schyes[j++]=i;
                }
            }
            for(;j<4;j++) ed->schyes[j]=-1;
            EndDialog(win,1);
brk:
            break;
        case IDCANCEL:
            EndDialog(win,0);
        }
        return 1;
    }
    return 0;
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

BOOL CALLBACK editvarious(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    static unsigned char *rom;
    static EDITCOMMON ec;
    static int gfxnum,sprgfx,palnum;
    static int init = 0;
    static BLOCKSEL8 bs;
    
    unsigned char*buf2;
    
    static int *const set_ofs[4]={&ec.gfxtmp,&gfxnum,&sprgfx,&palnum};
    
    const static unsigned set_max[4]={37,82,144,72};
    const static int ctl_ids[4]={3004,3012,3016,3020};
    const static int stuff_ofs[4]={0x54b7,0x51d3,0x4f8f,0x74894};
    unsigned i;
    int j,k,l;
    static HDC hdc;
    static HWND hc;
    switch(msg) {
    case WM_INITDIALOG:
        memset(&ec,0,sizeof(ec));
        rom=(ec.ew.doc=(FDOC*)lparam)->rom;
        ec.bmih=zbmih;
        ec.gfxtmp=0;
        ec.anim=3;
        gfxnum=0;
        init=1;
        SetDlgItemInt(win,3000,0,0);
        SetDlgItemInt(win,3001,0,0);
        SetDlgItemInt(win,3002,0,0);
        SetDlgItemInt(win,3003,0,0);
        init=0;
        Addgraphwin((void*)&ec,1);
        bs.ed=(void*)&ec;
        hdc=GetDC(win);
        InitBlksel8(hc=GetDlgItem(win,IDC_CUSTOM3),&bs,ec.hpal,hdc);
        ReleaseDC(win,hdc);
        Setdispwin((void*)&ec);
loadnewgfx:
        
        for(i = 0; i < 8; i++)
            ec.blocksets[i] = rom[0x6073 + i + (ec.gfxtmp << 3)];
        
        for(i = 0; i < 4; i++)
        {
            j = rom[0x5d97 + i + (gfxnum << 2)];
            
            if(j != 0)
                ec.blocksets[i + 3] = j;
        }
        
        for(i = 0; i < 8; i++)
            Getblocks(ec.ew.doc, ec.blocksets[i]);
updscrn:
        
        if(init < 2)
        {
            for(i = 0; i < 256; i++)
                Updateblk8sel(&bs, i);
            
            InvalidateRect(hc, 0, 0);
        }
        
        break;
    
    case WM_DESTROY:
        
        DeleteDC(bs.bufdc);
        DeleteObject(bs.bufbmp);
        Delgraphwin( (void*) &ec);
        
        break;
    
    case 4000:
        
        i = wparam & 0x3ff;
        
        Changeblk8sel(hc,&bs);
        
        bs.sel = i;
        
        Changeblk8sel(hc,&bs);
        
        break;
    
    case WM_COMMAND:
        
        if(wparam == IDCANCEL)
        {
            EndDialog(win, 0);
            
            break;
        }
        
        if((wparam>>16)!=EN_CHANGE) break;
        wparam&=65535;
        i=GetDlgItemInt(win,wparam,0,0);
        if(wparam==IDC_EDIT1) {
            bs.flags=(i&7)<<10;
            goto updscrn;
        }
        if(wparam<3004) {
            if(i>=set_max[wparam-3000]) {SetDlgItemInt(win,wparam,set_max[wparam-3000]-1,0);break;}
            *set_ofs[wparam-3000]=i;
            j=3;
            if(wparam==3000) j=7,i<<=1;
            k=ctl_ids[wparam-3000]+j;
            l=k+(i<<2);
            init++;
            for(;j>=0;j--) {
                SetDlgItemInt(win,k,rom[stuff_ofs[wparam-3000]+l],0);
                k--;
                l--;
            }
            init--;
            break;
        }
        if(wparam<3020) if(i>219) {SetDlgItemInt(win,wparam,219,0);break;}
        if(wparam<3012) {
            if(init>1) break;
            if(!init) rom[0x54b7 + (ec.gfxtmp<<3)+wparam]=i,ec.ew.doc->modf=1;
freegfx:
            for(i=0;i<8;i++) Releaseblks(ec.ew.doc,ec.blocksets[i]);
            goto loadnewgfx;
        }
        if(wparam<3016) {
            if(init>1) break;
            if(!init) rom[0x51d3 + (gfxnum<<2)+wparam]=i,ec.ew.doc->modf=1;
            goto freegfx;
        }
        if(wparam<3020) {
            if(!init) rom[0x4f8f + (sprgfx<<2)+wparam]=i,ec.ew.doc->modf=1;
            if(init<2) Releaseblks(ec.ew.doc,ec.blocksets[wparam-3005]);
            i+=115;
            ec.blocksets[wparam-3005]=i;
            Getblocks(ec.ew.doc,i);
            if(init>1 && wparam!=3016) break;
            goto updscrn;
        }
        if(wparam<3024) {
            if(init>1 && wparam!=3020) break;
            if(!init) rom[0x74894 + (palnum<<2)+wparam]=i,ec.ew.doc->modf=1;
            buf2=rom+(palnum<<2) + 0x75460;
            if(palnum<41) {
                i=0x1bd734 + buf2[0]*90;
                Loadpal(&ec,rom,i,0x21,15,6);
                Loadpal(&ec,rom,i,0x89,7,1);
                Loadpal(&ec,rom,0x1bd39e + buf2[1]*14,0x81,7,1);
                Loadpal(&ec,rom,0x1bd4e0 + buf2[2]*14,0xd1,7,1);
                Loadpal(&ec,rom,0x1bd4e0 + buf2[3]*14,0xe1,7,1);
            } else {
                if(buf2[0]<128) Loadpal(&ec,rom,0x1be86c + (((unsigned short*)(rom + 0xdec13))[buf2[0]]),0x29,7,3);
                if(buf2[1]<128) Loadpal(&ec,rom,0x1be86c + (((unsigned short*)(rom + 0xdec13))[buf2[1]]),0x59,7,3);
                if(buf2[2]<128) Loadpal(&ec,rom,0x1be604 + (((unsigned char*)(rom + 0xdebc6))[buf2[2]]),0x71,7,1);
            }
            if(!ec.hpal) goto updscrn;
        }
    }
    return 0;
}

BOOL CALLBACK editbosslocs(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    static int num;
    static FDOC*doc;
    static unsigned char*rom;
    int i;
    switch(msg) {
    case WM_INITDIALOG:
        num=lparam;
        doc=activedoc;
        rom=doc->rom;
        SetDlgItemInt(win,IDC_EDIT1,i=((short*)(rom + 0x10954))[num],0);
        SetDlgItemInt(win,IDC_EDIT2,rom[0x792d + num]+(i&256),0);
        SetDlgItemInt(win,IDC_EDIT3,rom[0x7939 + num]+(i&256),0);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ((short*)(rom + 0x10954))[num]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            rom[0x792d + num]=GetDlgItemInt(win,IDC_EDIT2,0,0);
            rom[0x7939 + num]=GetDlgItemInt(win,IDC_EDIT3,0,0);
            doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
        }
    }
    return FALSE;
}

void Blkedit8load(BLKEDIT8*ed)
{
    int i, j, k, l, m, n = 0;
    int *b, *c;
    
    if(ed->blknum == 260)
        l = 0, n = 24;
    else if(ed->blknum == 259)
        l = 0, n = 8;
    else if(ed->blknum >= 256)
    {
        l = (ed->blknum - 256) << 7,
        n = 16;
    }
    else if(ed->blknum < 8)
        l = ed->blknum << 6;
    else if(ed->blknum == 10)
        l = 0x240;
    else if(ed->blknum >= 32)
        l = (ed->blknum - 31) << 16;
    else if(ed->blknum >= 15)
        l = (ed->blknum - 5) << 6;
    else
    {
        l = (ed->blknum + 1) << 6;
        n = 4;
    }
    
    if(ed->oed->gfxtmp==0xff) m=-1; else if(n==24) m=0x0f0f0f0f; else if(n>=8) m=0x3030303; else m=0x7070707;
    for(i=ed->size-16;i>=0;i-=16) {
        for(j=0;j<128;j+=8) {
            Drawblock(ed->oed,0,24,(j>>3)+i+l,n);
            b = (int*) (ed->buf + ( (ed->size - 16 - i) << 6) + j);
            c = (int*) drawbuf;
            for(k=0;k<8;k++) {
                b[0]=c[0]&m;
                b[1]=c[1]&m;
                b+=32;
                c+=8;
            }
        }
    }
}

BOOL CALLBACK blockdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLKEDIT8 *ed;
    OVEREDIT *oed;
    ZBLOCKS *blk;
    FDOC *doc;
    HWND hc, hc2;
    RECT rc;
    
    unsigned char *b, *b2;
    
    HGLOBAL hgl;
    
    int i, j, k, l, m[2], n, o[8];
    
    BITMAPINFOHEADER bmih;
    
    switch(msg)
    {
    
    case WM_QUERYNEWPALETTE:
        
        ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);
        Setpalette(win, ed->oed->hpal);
        
        break;
    
    case WM_PALETTECHANGED:
        
        InvalidateRect(GetDlgItem(win, IDC_CUSTOM1), 0, 0);
        InvalidateRect(GetDlgItem(win, IDC_CUSTOM2), 0, 0);
        
        break;
    
    case WM_INITDIALOG:
        
        ed = malloc(sizeof(BLKEDIT8));
        SetWindowLong(win, DWL_USER, (int) ed);
        
        oed = (OVEREDIT*) *(int*) lparam;
        hc = GetDlgItem(win, IDC_CUSTOM1);
        SetWindowLong(hc,GWL_USERDATA,(long) ed);
        
        i = ((short*) lparam)[2];
        ed->oed = oed;
        
        if(i >= 145 && i < 147)
            i += 111;
        
        if(i == 260)
            ed->size = 896;
        else if(i == 259)
            ed->size = 256;
        else if(i >= 256)
            ed->size = 128;
        else if(oed->gfxtmp == 0xff)
            ed->size = 256, i = 0;
        else
            ed->size = 64;
        
        ed->blknum = i;
        ed->buf = malloc(ed->size << 6);
        Blkedit8load(ed);
        
        ed->scrollh = 0;
        ed->scrollv = 0;
        ed->bmih = zbmih;
        ed->bmih.biWidth = 128;
        ed->bmih.biHeight = ed->size >> 1;
        memcpy(ed->pal, oed->pal, 1024);
        
        ed->blkwnd = hc;
        
        hc = GetDlgItem(win, IDC_CUSTOM2);
        
        rc = HM_GetClientRect(hc);
        
        ed->pwidth = rc.right;
        ed->pheight = rc.bottom;
        ed->psel = i = ((short*) lparam)[3] << 4;
        
        if(ed->blknum != 260 && ed->blknum >= 256)
            i >>= 2;
        
        for(j = (ed->size << 6) - 1; j >= 0; j--)
        {
            if(ed->buf[j])
                ed->buf[j] |= i;
        }
        
        SetWindowLongPtr(hc, GWLP_USERDATA, (LONG_PTR) ed);
        
        break;
    
    case WM_DESTROY:
        
        ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);
        
        // \note This line commented out in the original source
        // for(i=0;i<256;i++) DeleteObject(ed->brush[i]);
        
        free(ed->buf);
        free(ed);
        
        break;
    
    case WM_COMMAND:
        
        ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);
        
        switch(wparam)
        {
        case IDC_BUTTON1:
            
            if(ed->oed->gfxtmp == 0xff)
            {
                HPALETTE const hpal = ed->oed->hpal;
                
                hgl = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,17448);
                b = GlobalLock(hgl);
                
                ed->bmih.biSizeImage = ed->bmih.biWidth * ed->bmih.biHeight;
                
                if(hpal)
                {
                    memcpy(b, &(ed->bmih), 40);
                    
                    b += 40;
                    
                    for(j = 0; j < 256; j += 8)
                    {
                        GetPaletteEntries(hpal,
                                          ((short*)(ed->pal))[j],
                                          8,
                                          (void*)(o));
                        
                        for(i = 0; i < 8; i++)
                        {
                            ((int*)(b))[i] = ((o[i] & 0xff) << 16) | ((o[i] & 0xff0000) >> 16) | (o[i] & 0xff00);
                        }
                        
                        b += 32;
                    }
                }
                else
                {
                    memcpy(b, &(ed->bmih), 1064);
                    b += 1064;
                }
                
                memcpy(b,ed->buf,16384);
            }
            else
            {
                HPALETTE const hpal = ed->oed->hpal;
                
                bmih.biSize=40;
                bmih.biWidth=128;
                bmih.biHeight=ed->bmih.biHeight;
                bmih.biPlanes=1;
                bmih.biBitCount=4;
                bmih.biCompression=BI_RGB;
                bmih.biSizeImage=bmih.biWidth*bmih.biHeight>>1;
                bmih.biClrUsed=ed->blknum==260?16:8;
                bmih.biClrImportant=0;
                hgl=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,40+(bmih.biClrUsed<<2)+(ed->size<<5));
                b=GlobalLock(hgl);
                *(BITMAPINFOHEADER*)b=bmih;
                j=ed->psel;
                if(ed->blknum>=256) j&=0xfc; else j&=0xf8;
                k=ed->size<<5;
                b2=ed->buf;
                b+=40;
                l=bmih.biClrUsed;
                
                if(hpal != 0)
                {
                    GetPaletteEntries(hpal, 0, 1, (void*) o);
                    GetPaletteEntries(hpal, ((short*) (ed->pal))[1 + j], l - 1, (void*) (o + 1));
                    
                    for(i = 0; i < l; i++)
                    {
                        ((int*) (b))[i] = ((o[i] & 0xff) << 16) | ( (o[i] & 0xff0000) >> 16) | (o[i] & 0xff00);
                    }
                }
                else
                {
                    *(int*)(b) = *(int*)ed->pal;
                    
                    for(i = 1; i < l; i++)
                        ((int*) (b))[i] = ((int*) ed->pal)[i + j];
                }
                
                b += bmih.biClrUsed << 2;
                j = 0;
                l--;
                
                for(i = 0; i < k; i++)
                {
                    *(b++) = ((b2[j] & l) << 4) | (b2[j + 1] & l);
                    j += 2;
                }
            }
            
            GlobalUnlock(hgl);
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_DIB, hgl);
            CloseClipboard();
            
            break;
        case IDC_BUTTON2:
            OpenClipboard(0);
            hgl=GetClipboardData(CF_DIB);
            if(!hgl) {
                MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
                CloseClipboard();
                break;
            }
            b=GlobalLock(hgl);
            bmih=*(BITMAPINFOHEADER*)b;
            if(bmih.biSize!=40 || bmih.biWidth!=128 ||
                (bmih.biHeight!=ed->bmih.biHeight&&bmih.biHeight!=-ed->bmih.biHeight) || bmih.biPlanes!=1
                || (bmih.biBitCount!=4 && bmih.biBitCount!=8) ||
                bmih.biCompression!=BI_RGB) {
                GlobalUnlock(hgl);
                CloseClipboard();
                MessageBox(framewnd,"The image does not have the correct dimensions or is not a 16 color image.","Bad error happened",MB_OK);
                break;
            }
            b+=40+(((bmih.biClrUsed?bmih.biClrUsed:(1<<bmih.biBitCount)))<<2);
            if(ed->blknum==260) k=15; else k=7;
            if(bmih.biBitCount==8)
                memcpy(ed->buf,b,ed->size<<6);
            else {
                j=ed->size<<5;
                b2=ed->buf;
                for(i=0;i<j;i++) {
                    *(b2++)=((*b)>>4);
                    *(b2++)=(*(b++));
                }
            }
            if(ed->oed->gfxtmp!=0xff) {
                if(ed->blknum==260) j=ed->psel&0xf0; else j=ed->psel&0xf8;
                for(i=(ed->size<<6)-1;i>=0;i--) {
                    ed->buf[i]&=k;
                    if(ed->buf[i]) ed->buf[i]|=j;
                }
            }
            if(bmih.biHeight<0) {
                j=(ed->size<<5);
                k=(ed->size<<6)-128;
                for(i=0;i<j;i++) {
                    l=ed->buf[i];
                    ed->buf[i]=ed->buf[i^k];
                    ed->buf[i^k]=l;
                }
            }
            GlobalUnlock(hgl);
            CloseClipboard();
            InvalidateRect(ed->blkwnd,0,0);
            break;
        case IDOK:
            oed=ed->oed;
            doc=oed->ew.doc;
            blk=doc->blks;
            j=ed->size<<6;
            b2=malloc(j);
            for(i=0;i<j;i++)
                b2[(i&7)+((i&0x78)<<3)+((i&0x380)>>4)+(i&0xfc00)]=ed->buf[j-128-(i&0xff80)+(i&127)];
            i=2;
            if(ed->blknum==260) {memcpy(blk[k=225].buf,b2,0xe000);goto endsave3;}
            if(ed->blknum==259) {memcpy(blk[k=224].buf,b2,0x4000);goto endsave3;}
            if(ed->blknum>=256) {memcpy(blk[k=bg3blkofs[ed->blknum-256]].buf,b2,0x2000);goto endsave3;}
            if(ed->blknum>=32) {memcpy(blk[k=ed->blknum-32].buf,b2,0x1000);goto endsave3;}
            if(oed->gfxtmp==0xff) {
                memcpy(blk[223].buf,b2,16384);
                blk[223].modf=1;
                goto endsave2;
            } else {
                j=ed->size<<6;
                for(i=0;i<j;i++) {
                    b2[i]&=7;
                    if(b2[i]) b2[i]|=8;
                }
                j=ed->blknum;
                m[0]=0;
                m[1]=0;
                if(oed->gfxtmp>=0x20) { if(ed->blknum==7) {
                    memcpy(blk[oed->blocksets[7]].buf + 0x800,b2 + 0x800,0x800);
                    if(oed->anim==2) {
                        memcpy(blk[oed->blocksets[9]].buf,b2,0x800);
                        m[1]=1;
                    } else if(oed->anim==1) {
                        memcpy(blk[oed->blocksets[8]].buf + 0x800,b2,0x800);
                        m[0]=1;
                    } else {
                        memcpy(blk[oed->blocksets[8]].buf,b2,0x800);
                        m[0]=1;
                    }
                    goto endsave;
                } } else {
                    if(ed->blknum==6) {
                        memcpy(blk[oed->blocksets[6]].buf,b2,0xc00);
                        memcpy(blk[oed->blocksets[8]].buf+(oed->anim<<10),b2 + 0xc00,0x400);
                        m[0]=1;
                        goto endsave;
                    }
                    else if(ed->blknum==7) {
                        memcpy(blk[oed->blocksets[7]].buf + 0x400, b2 + 0x400,0xc00);
                        memcpy(blk[oed->blocksets[9]].buf+(oed->anim<<10),b2,0x400);
                        m[1] = 1;
                        
                        goto endsave;
                    }
                }
            }
            
            memcpy(blk[(j >= 15) ? (0x6a + j) : oed->blocksets[j]].buf, b2, 0x1000);
endsave:
            for(i = 0; i < 3; i++)
            {
                if(i == 2)
                    k = (j >= 15) ? (0x6a + j) : oed->blocksets[j];
                else if(m[i])
                    k = oed->blocksets[i + 8];
                else
                    continue;
endsave3:
                b = blk[k].buf;
                
                if(k == 225)
                    l = 0xe000;
                else if(k == 224)
                    l = 0x4000;
                else if(k >= 220)
                    l = 0x2000;
                else
                    l = 0x1000;
                
                for(n = 0; n < l; n++)
                    b[n + l] = masktab[b[n]];
                
                for(n = 0; n < l; n++)
                    b[n+l+l] = b[n^7];
                
                b += l << 1;
                
                for(n = 0; n < l; n++)
                    b[n + l] = masktab[b[n]];
                
                blk[k].modf=1;
            }
            
            for(i = 0; i < 160; i++)
            {
                hc = doc->overworld[i].win;
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, 2000);
                    hc = GetDlgItem(hc2, 3000);
                    InvalidateRect(hc,0,0);
                    
                    hc = GetDlgItem(hc2,3001);
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            for(i = 0; i < 0xa8; i++)
            {
                hc = doc->ents[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, 2000);
                    hc = GetDlgItem(hc2, 3011);
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            for(i = 0; i < 11; i++)
            {
                hc = doc->tmaps[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, 2000);
                    hc = GetDlgItem(hc2, 3000);
                    InvalidateRect(hc, 0, 0);
                    
                    hc = GetDlgItem(hc2, 3001);
                    InvalidateRect(hc, 0, 0);
                }
            }
            
            goto nowmap;
endsave2:
            
            for(i = 0; i < 2; i++)
            {
                hc = doc->wmaps[i];
                
                if(hc)
                {
                    hc2 = GetDlgItem(hc, 2000);
                    hc = GetDlgItem(hc2, 3000);
                    
                    InvalidateRect(hc, 0, 0);
                    
                    hc = GetDlgItem(hc2, 3001);
                    
                    SendMessage(hc, 4001, 0, 0);
                }
            }
nowmap:
            doc->modf = 1;
            free(b2);
            EndDialog(win,1);
            
            break;
        
        case IDCANCEL:
            EndDialog(win,0);
        }
    }
    return 0;
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

int
Savesecrets(FDOC          * const doc,
            int             const num,
            uint8_t const * const buf,
            int             const size)
{
    int i,j,k;
    int adr[128];
    unsigned char*rom=doc->rom;
    for(i=0;i<128;i++)
        adr[i]=0xe0000 + ((short*)(rom + 0xdc2f9))[i];
    k=doc->sctend;
    if(rom[0x125ec + (num&63)]!=(num&63)) {
        j=(num&64)?doc->sctend:adr[64];
        for(i=num&63;i<64;i++) {
            if(rom[0x125ec + i]!=i) continue;
            j=adr[i+(num&64)];
            if(*(short*)(rom+j)!=-1) j-=2;
            break;
        }
        ((short*)(rom + 0xdc2f9))[num]=adr[num]=j;
    } else j=adr[num];
    for(i=num+1;i<128;i++) {
        if(rom[0x125ec + (i&63)]!=(i&63)) continue;
        k=adr[i];
        break;
    }
    
    if( is16b_neg1(rom + j) )
    {
        if(!size) return 0;
        if(k>j) k-=2;
        j+=size+2;
        adr[num]+=2;
    } else {
        if(!size) {
            if(j>0xdc3f9) {
                j-=2;
                adr[num]-=2;
            }
        } else j+=size;
        if(*(short*)(rom+k)!=-1) k-=2;
    }
    if(doc->sctend-k+j>0xdc894) {
        MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
        return 1;
    }
    memmove(rom+j,rom+k,doc->sctend-k);
    if(size) memcpy(rom+adr[num],buf,size);
    if(j==k) return 0;
    ((short*)(rom + 0xdc2f9))[num]=adr[num];
    doc->sctend+=j-k;
    for(i=num+1;i<128;i++) {
        ((short*)(rom + 0xdc2f9))[i]=adr[i]+j-k;
    }
    return 0;
}

int fronttest(POINT*pt,POINT*pt2,POINT*pt3)
{
    if(pt2->x==pt->x)
        if(pt2->y<pt->y) return pt3->x<pt->x; else return pt3->x>=pt->x;
    if(pt2->x>pt->x)
        return pt3->y<pt->y+(pt3->x-pt->x)*(pt2->y-pt->y)/(pt2->x-pt->x);
    return pt3->y>=pt->y+(pt3->x-pt->x)*(pt2->y-pt->y)/(pt2->x-pt->x);
}

void rotvec2d(POINT *vec, POINT *pt)
{
    int a=vec->x,b=vec->y,c,d,e;
    
    if(a == 0 && b == 0)
        return;
    
    c = (int) sqrt(1048576 / (a * a + b * b));
    d = ((pt->x*a)-(pt->y*b))*c>>10;
    e = ((pt->y*a)+(pt->x*b))*c>>10;
    pt->x=d;
    pt->y=e;
}

void pnormal(int x1,int y1,int z1,int x2,int y2,int z2,int x3,int y3,int z3)
{
    static int a,b,c,d;
    
    a = ( (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1) ) >> 1;
    b = ( (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1) ) >> 1;
    c = ( (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1) ) >> 1;
    
    if(a==0 && b==0 && c==0) {rptx=0,rpty=0,rptz=1023;return;}
    
    d = (int) sqrt(a*a+b*b+c*c);
    
    rptx=a*1024/d;
    rpty=b*1024/d;
    rptz=c*1024/d;
}

int inpoly(int x,int y,POINT*pts,int k)
{
    int i,j,l,m,n=0;
    for(i=0;i<k;i++) {
        j=i+1;
        if(j==k) j=0;
        if(pts[j].x==pts[i].x) continue;
        if(pts[j].x<pts[i].x) l=pts[j].x,m=pts[i].x;
        else l=pts[i].x,m=pts[j].x;
        if(x>=l && x<m && (y>pts[i].y+(x-pts[i].x)*(pts[j].y-pts[i].y)/(pts[j].x-pts[i].x))) n^=1;
    }
    return n;
}

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

LRESULT CALLBACK
perspdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PERSPEDIT*ed;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rc;
    int i,j,k,l,m,n,o = 0,q,r;
    short p[3];
    const static char a[8]={0,1,2,0,1,2};
    const static char c[8]={2,0,0,2,0,0};
    const static char d[8]={1,2,1,1,2,1};
    char e[4]={0,0,0,0};
    const static short b[6]={-128,-128,128,128,128,-128};
    const static char xyz_text[]="XYZ";
    POINT pt,pt2,pt3,pts[16],pts2[16];
    int rx[16],ry[16],rz[16];
    HBRUSH oldbrush,oldbrush2,oldbrush3;
    switch(msg) {
    case WM_SIZE:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        ed->width=lparam&65535;
        ed->height=lparam>>16;
updscale:
        ed->scalex=ed->width*ed->enlarge>>8;
        ed->scaley=ed->height*ed->enlarge>>8;
        goto upddisp;
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS+DLGC_WANTARROWS;
    case WM_KEYDOWN:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!(lparam&0x1000000)) switch(wparam) {
        case VK_RIGHT: goto moveright;
        case VK_LEFT: goto moveleft;
        case VK_DOWN: goto movedown;
        case VK_UP: goto moveup;
        }
        switch(wparam) {
        case VK_ADD:
            ed->enlarge+=4;
            if(ed->enlarge>640) ed->enlarge=640;
            goto updscale;
        case VK_SUBTRACT:
            ed->enlarge-=4;
            if(ed->enlarge<12) ed->enlarge=12;
            goto updscale;
        case VK_NUMPAD2:
movedown:
            l=1;
            m=-1;
updpos:
            if(!ed->tool) {
                if(ed->selpt==-1) break;
                if(GetKeyState(VK_CONTROL)&128) m<<=2;
                j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
                ed->buf[j+l]+=m;
                ed->modf=1;
                goto upddisp;
            }
            break;
        case VK_NUMPAD4:
moveleft:
            l=0;
            m=-1;
            goto updpos;
        case VK_NUMPAD6:
moveright:
            l=0;
            m=1;
            goto updpos;
        case VK_NUMPAD8:
moveup:
            l=1;
            m=1;
            goto updpos;
        case 'A':
            l=2;
            m=-1;
            goto updpos;
        case 'Z':
            l=2;
            m=1;
            goto updpos;
        case 'Q':
            ed->zrot-=4;
            goto upd;
        case 'W':
            ed->zrot+=4;
            goto upd;
        case VK_UP:
            ed->xrot+=4;
            goto upd;
        case VK_DOWN:
            ed->xrot-=4;
            goto upd;
        case VK_RIGHT:
            ed->yrot+=4;
upd:
            ed->xrot&=255;
            ed->yrot&=255;
            ed->zrot&=255;
upddisp:
            InvalidateRect(win,0,1);
            break;
        case VK_LEFT:
            ed->yrot-=4;
            goto upd;
        case VK_BACK:
            if(ed->tool==1) {
                if(ed->newptp==-1) break;
                ed->newptp=-1;
                goto upddisp;
            }
            if(!ed->tool) {
                if(ed->selpt==-1) break;
                j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
                memcpy(ed->buf+j,ed->buf+j+3,ed->len-j-3);
                *(unsigned short*)(ed->buf+4+ed->objsel)-=3;
                ed->buf[ed->objsel]--;
                if(!ed->objsel) {
                    *(unsigned short*)(ed->buf+8)-=3;
                    *(unsigned short*)(ed->buf+10)-=3;
                }
                ed->len-=3;
                k=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
                for(i=ed->buf[ed->objsel+1];i;i--) {
                    l=ed->buf[k];
                    for(m=0;m<l;m++) if(ed->buf[m+k+1]==ed->selpt) {
                        memcpy(ed->buf+k,ed->buf+k+l+2,ed->len-k-l-2);
                        ed->buf[ed->objsel+1]--;
                        if(!ed->objsel) {
                            *(unsigned short*)(ed->buf+8)-=l+2;
                            *(unsigned short*)(ed->buf+10)-=l+2;
                        }
                        ed->len-=l+2;
                        goto delnext;
                    } else if(ed->buf[m+k+1]>ed->selpt) ed->buf[m+k+1]--;
                    k+=l+2;
delnext:;
                }
                if(ed->selpt==ed->buf[ed->objsel]) ed->selpt--;
                ed->modf=1;
                goto updsize;
            } else if(ed->newlen) {
                ed->newlen--;
                if(ed->newlen) ed->selpt=ed->newface[ed->newlen-1];
                else ed->selpt=-1;
                goto upddisp;
            }
            break;
        }
        break;
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        k=lparam&65535;
        l=lparam>>16;
        switch(ed->tool) {
        case 2:
            if(ed->len+ed->newlen+2>=116) break;
            if(ed->newlen==16) break;
            goto addf;
        case 0:
addf:
            q=ed->selpt;
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8f + ed->buf[ed->objsel]*3;
            for(i=ed->buf[ed->objsel]-1;i>=0;i--) {
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],&pt);
                if(k>=pt.x-3 && k<pt.x+3 && l>=pt.y-3 && l<pt.y+3) {
                    if(ed->tool==2) {
                        for(l=0;l<ed->newlen;l++) if(ed->newface[l]==i) {
                            if(ed->newlen<3) return 0;
                            if(l) return 0;
                            if(!ed->objsel) {
                                o=*(unsigned short*)(ed->buf+8) - 0xff8c;
                                memcpy(ed->buf+o+ed->newlen+2,ed->buf+o,ed->len-o);
                                *(short*)(ed->buf+8)+=ed->newlen+2;
                                *(short*)(ed->buf+10)+=ed->newlen+2;
                            } else o=ed->len;
                            ed->buf[o]=ed->newlen;
                            ed->buf[o+ed->newlen+1]=0;
                            memcpy(ed->buf+o+1,ed->newface,ed->newlen);
                            ed->len+=ed->newlen+2;
                            ed->newlen=0;
                            ed->buf[ed->objsel+1]++;
                            ed->modf=1;
                            goto updsize;
                        }
                        ed->newface[ed->newlen++]=i;
                    }
                    ed->selpt=i;
                    goto upddisp;
                }
                j-=3;
            }
            if(!ed->tool) ed->selpt=-1;
            if(ed->selpt!=q) goto upddisp;
            break;
        case 1:
            if(ed->len>113) break;
            k-=ed->width>>1;
            l-=ed->height>>1;
            for(j=0;j<6;j++) {
                i=a[j];
                p[i]=b[j];
                n=c[j];
                o=d[j];
                p[n]=-128;
                p[o]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                rx[0]=rptx;
                ry[0]=rpty;
                rz[0]=rptz;
                p[n]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt2);
                rx[1]=rptx;
                ry[1]=rpty;
                rz[1]=rptz;
                p[n]=-128;
                p[o]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt3);
                rx[2]=rptx;
                ry[2]=rpty;
                rz[2]=rptz;
                if((j<3) ^fronttest(&pt,&pt2,&pt3)) {
                    pnormal(rx[0],ry[0],rz[0],rx[1],ry[1],rz[1],rx[2],ry[2],rz[2]);
                    m=k*rptx/ed->scalex+l*rpty/ed->scaley+rptz;
                    if(!m) continue;
                    
                    m=(rx[0]*rptx+ry[0]*rpty+rz[0]*rptz)/m;
                    
                    q =
                    (
                        ( (rx[1] - rx[0]) * (m * k / ed->scalex - rx[0]) )
                      + ( (ry[1] - ry[0]) * (m * l / ed->scaley - ry[0]) )
                      + ( (rz[1] - rz[0]) * (m - rz[0]) )
                      - 32768
                    ) >> 8;
                    
                    r =
                    (
                        ( (rx[2] - rx[0]) * (m * k / ed->scalex - rx[0]) )
                      + ( (ry[2] - ry[0]) * (m * l / ed->scaley - ry[0]) )
                      + ( (rz[2] - rz[0]) * (m - rz[0]) )
                      - 32768
                    ) >> 8;
                    
                    if(q<-128 || q>127 || r<-128 || r>127) continue;
                    if(ed->newptp==-1 || ed->newptp==a[j]) {
                        ed->newptp=a[j];
                        ed->newptx=q;
                        ed->newpty=r;
                    } else {
                        o=*(unsigned short*)(ed->buf+ed->objsel+4) - 0xff8c;
                        memcpy(ed->buf+o+3,ed->buf+o,ed->len-o);
                        ed->buf[o+ed->newptp]=(c[ed->newptp]==a[j])?r:q;
                        ed->buf[o+c[ed->newptp]]=ed->newptx;
                        ed->buf[o+d[ed->newptp]]=ed->newpty;
                        ed->buf[o+1]=-ed->buf[o+1];
                        ed->selpt=ed->buf[ed->objsel]++;
                        *(short*)(ed->buf+ed->objsel+4)+=3;
                        if(!ed->objsel) {
                            *(short*)(ed->buf+8)+=3;
                            *(short*)(ed->buf+10)+=3;
                        }
                        ed->len+=3;
                        ed->newptp=-1;
                        ed->modf=1;
                        goto updsize;
                    }
                    goto upddisp;
                }
            }
            break;
        case 3:
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c;
            for(i=0;i<ed->buf[ed->objsel];i++) {
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],pts+i);
                j+=3;
            }
            j=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
            n=-1;
            for(i=ed->buf[ed->objsel+1];i;i--) {
                m=ed->buf[j++];
                if(fronttest(pts+ed->buf[j],pts+ed->buf[j+1],pts+ed->buf[j+2])) {
                    for(q=0;q<m;q++) pts2[q]=pts[ed->buf[j+q]];
                    if(inpoly(k,l,pts2,m)) {
                        n=j;
                        o=m;
                    }
                }
                j+=m+1;
            }
            
            if(n != -1)
            {
                memcpy(ed->buf + n - 1, ed->buf + n + o + 1, ed->len - n - o - 1);
                ed->len-=o+2;
                ed->buf[ed->objsel+1]--;
                
                if(!ed->objsel)
                {
                    *(short*) (ed->buf + 8) -= o + 2;
                    *(short*) (ed->buf + 10) -= o + 2;
                }
                
                ed->modf = 1;
                
updsize:
                
                wsprintf(buffer, "Free: %d", 116 - ed->len);
                SetDlgItemText(ed->dlg,3003,buffer);
                
                goto upddisp;
            }
        }
        
        break;
    
    case WM_PAINT:
        
        ed = (PERSPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        hdc = BeginPaint(win,&ps);
        
        oldbrush=GetCurrentObject(hdc,OBJ_PEN);
        oldbrush2=GetCurrentObject(hdc,OBJ_BRUSH);
        oldbrush3=SelectObject(hdc,trk_font);
        
        SetBkMode(hdc,TRANSPARENT);
        
        for(j = 0; j < 6; j++)
        {
            i=a[j];
            p[i]=b[j];
            k=c[j];
            l=d[j];
            p[k]=-128;
            p[l]=-128;
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            p[l]=128;
            Get3dpt(ed,p[0],p[1],p[2],&pt2);
            p[k]=128;
            Get3dpt(ed,p[0],p[1],p[2],&pt3);
            SelectObject(hdc,white_pen);
            
            if( (j < 3) ^ fronttest(&pt,&pt2,&pt3) )
            {
                m=0;
                for(;;) {
                    if(e[l]) goto nextaxis;
                    if(p[i]<0) p[i]=-144; else p[i]=144;
                    p[k]=-128;
                    p[l]=-128;
                    Get3dpt(ed,p[0],p[1],p[2],&pt);
                    MoveToEx(hdc,pt.x,pt.y,0);
                    p[l]=128;
                    Get3dpt(ed,p[0],p[1],p[2],&pt2);
                    LineTo(hdc,pt2.x,pt2.y);
                    pt.x-=pt2.x;
                    pt.y-=pt2.y;
                    pt3.x=20;
                    pt3.y=8;
                    rotvec2d(&pt,&pt3);
                    LineTo(hdc,pt3.x+pt2.x,pt3.y+pt2.y);
                    MoveToEx(hdc,pt2.x,pt2.y,0);
                    pt3.x=20;
                    pt3.y=-8;
                    rotvec2d(&pt,&pt3);
                    LineTo(hdc,pt3.x+pt2.x,pt3.y+pt2.y);
                    p[l]=0;
                    Get3dpt(ed,p[0],p[1],p[2],&pt2);
                    buffer[0]=xyz_text[l];
                    buffer[1]=0;
                    Paintspr(hdc,pt2.x,pt2.y,0,0,ed->width);
                    e[l]=1;
nextaxis:
                    if(m==1) break;
                    m=k;
                    k=l;
                    l=m;
                    m=1;
                }
                continue;
            }
            for(m=-128;m<=128;m+=16) {
                p[k]=m;
                p[l]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[l]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
            for(m=-128;m<=128;m+=16) {
                p[l]=m;
                p[k]=-128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[k]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
            if(ed->newptp!=-1 && ed->newptp!=i) {
                SelectObject(hdc,green_pen);
                p[ed->newptp]=-128;
                if(c[ed->newptp]==i) m=ed->newpty; else m=ed->newptx;
                if(ed->newptp==k) p[l]=m;
                else p[k]=m;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                p[ed->newptp]=128;
                Get3dpt(ed,p[0],p[1],p[2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
        }
        SelectObject(hdc,white_pen);
        j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c;
        for(i=0;i<ed->buf[ed->objsel];i++) {
            Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],pts+i);
            rx[i]=rptx;
            ry[i]=rpty;
            rz[i]=rptz;
            j+=3;
        }
        j=*(unsigned short*)(ed->buf+4+ed->objsel) - 0xff8c;
        for(i=ed->buf[ed->objsel+1];i;i--) {
            k=ed->buf[j++];
            l=ed->buf[j];
            m=ed->buf[j+1];
            n=ed->buf[j+2];
            if(fronttest(pts+l,pts+m,pts+n)) {
                pnormal(rx[l],ry[l],rz[l],rx[m],ry[m],rz[m],rx[n],ry[n],rz[n]);
                rptz=-rptz>>7;
                if(rptz<0) rptz=0;
                if(rptz>7) rptz=7;
                SelectObject(hdc,shade_brush[rptz]);
                for(l=0;l<k;l++) pts2[l]=pts[ed->buf[j+l]];
                Polygon(hdc,pts2,k);
            }
            j+=k+1;
        }
        for(i=0;i<ed->buf[ed->objsel];i++) {
            rc.left=pts[i].x-2;
            rc.right=pts[i].x+2;
            rc.top=pts[i].y-2;
            rc.bottom=pts[i].y+2;
            FillRect(hdc,&rc,(ed->selpt==i)?green_brush:red_brush);
        }
        if(ed->newlen>1) {
            SelectObject(hdc,blue_pen);
            MoveToEx(hdc,pts[ed->newface[0]].x,pts[ed->newface[0]].y,0);
            for(i=1;i<ed->newlen;i++) {
                k=ed->newface[i];
                LineTo(hdc,pts[k].x,pts[k].y);
            }
        }
        if(ed->tool==1 && ed->newptp!=-1) {
            SelectObject(hdc,blue_pen);
            p[ed->newptp]=b[ed->newptp];
            p[c[ed->newptp]]=ed->newptx;
            p[d[ed->newptp]]=ed->newpty;
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            MoveToEx(hdc,pt.x,pt.y,0);
            p[ed->newptp]=-b[ed->newptp];
            Get3dpt(ed,p[0],p[1],p[2],&pt);
            LineTo(hdc,pt.x,pt.y);
        }
        if(ed->selpt!=-1) {
            j=*(unsigned short*)(ed->buf+2+ed->objsel) - 0xff8c + 3*ed->selpt;
            if(!ed->tool) {
                SelectObject(hdc,green_pen);
                wsprintf(buffer,"X: %02X Y: %02X Z: %02X",ed->buf[j]+128,ed->buf[j+1]+128,ed->buf[j+2]+128);
                Paintspr(hdc,0,0,0,0,ed->width);
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],-128,&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,ed->buf[j],-ed->buf[j+1],128,&pt);
                LineTo(hdc,pt.x,pt.y);
                Get3dpt(ed,ed->buf[j],-128,ed->buf[j+2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,ed->buf[j],128,ed->buf[j+2],&pt);
                LineTo(hdc,pt.x,pt.y);
                Get3dpt(ed,-128,-ed->buf[j+1],ed->buf[j+2],&pt);
                MoveToEx(hdc,pt.x,pt.y,0);
                Get3dpt(ed,128,-ed->buf[j+1],ed->buf[j+2],&pt);
                LineTo(hdc,pt.x,pt.y);
            }
        }
        SelectObject(hdc,oldbrush);
        SelectObject(hdc,oldbrush2);
        SelectObject(hdc,oldbrush3);
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
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
void Savetmap(TMAPEDIT*ed)
{
    int i,j,k = 0,l,m;
    unsigned char*rom;
    j=ed->datnum;
    l=ed->len;
    if(j<7) {
        i=Changesize(ed->ew.doc,ed->datnum+669,ed->len);
        if(!i) return;
    } else {
        rom=ed->ew.doc->rom;
        i=*(short*)(rom+tmap_ofs[j-7]) + 0x68000;
        if(j==7 || j==9) i++;
        switch(j) {
        case 7:
            k=*(unsigned short*)(rom + 0x64ecd);
            break;
        case 8:
            k=*(unsigned short*)(rom + 0x64e5c)+2;
            l--;
            break;
        case 9:
            k=*(unsigned short*)(rom + 0x654bd);
            break;
        case 10:
            k=*(unsigned short*)(rom + 0x65142)+1;
            break;
        case 11:
            k=*(unsigned short*)(rom + 0x6528d)+1;
            break;
        }
        if(l>k) {
            if(!Changesize(ed->ew.doc,4096,ed->ew.doc->tmend3+l-k)) return;
            rom=ed->ew.doc->rom;
            memmove(rom+i+l,rom+i+k,ed->ew.doc->tmend3-i-l);
        } else {
            rom=ed->ew.doc->rom;
            memmove(rom+i+l,rom+i+k,ed->ew.doc->tmend3-i-k);
            Changesize(ed->ew.doc,4096,ed->ew.doc->tmend3+l-k);
        }
        for(m=0;m<5;m++)
            if(*(short*)(rom+tmap_ofs[m]) + 0x68000>i) *(short*)(rom+tmap_ofs[m])+=l-k;
        switch(j) {
        case 7:
            *(unsigned short*)(rom + 0x64ecd)=l;
            break;
        case 8:
            *(unsigned short*)(rom + 0x64e5c)=l-2;
            break;
        case 9:
            *(unsigned short*)(rom + 0x654bd)=l;
            break;
        case 10:
            *(unsigned short*)(rom + 0x65142)=l-1;
            break;
        case 11:
            *(unsigned short*)(rom + 0x6528d)=l-1;
            break;
        }
    }
    memcpy(ed->ew.doc->rom+i,ed->buf,l);
    ed->ew.doc->modf=1;
}

LRESULT CALLBACK
tmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT *ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->modf) {
            wsprintf(buffer,"Confirm modification of %s?",screen_text[ed->ew.param]);
            switch(MessageBox(framewnd,buffer,"Other screen editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savetmap(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        if((HWND)lparam!=win) break;
        ed=((TMAPEDIT*)GetWindowLong(win,GWL_USERDATA));
        activedoc=ed->ew.doc;
        Setdispwin((DUNGEDIT*)ed);
        break;
    case WM_GETMINMAXINFO:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        ed=(TMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&tmapdlg,win,0,0,256,256,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

void Trackchgsel(HWND win,RECT*rc,TRACKEDIT*ed)
{
    rc->top=(mark_start-ed->scroll)*textmetric.tmHeight;
    rc->bottom=(mark_end+1-ed->scroll)*textmetric.tmHeight;
    
    InvalidateRect(win,rc,1);
}

LRESULT CALLBACK
trackerproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT *ed, *ed2;
    HDC hdc;
    PAINTSTRUCT ps;
    HANDLE oldobj,oldobj2,oldobj3;
    SCROLLINFO si;
    SCMD*sc,*sc2,*sc3;
    FDOC*doc;
    RECT rc;
    
    const static int csel_l[]={48,72,96,168,192,216};
    const static int csel_r[]={64,88,160,184,208,232};
    int i, j = 0, k, l, m;
    
    switch(msg)
    {
    case WM_ACTIVATE:
        
        if(wparam == WA_ACTIVE)
            SetFocus(win);
        
        break;
    
    case WM_SIZE:
        
        ed = (TRACKEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        si.cbSize = sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->len+1;
        
        ed->page=si.nPage=(lparam>>16)/textmetric.tmHeight;
        
        SetScrollInfo(win,SB_VERT,&si,1);
        
        ed->scroll=Handlescroll(win,-1,ed->scroll,ed->page,SB_VERT,ed->len,textmetric.tmHeight);
        
        break;
    
    case WM_VSCROLL:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->scroll=Handlescroll(win,wparam,ed->scroll,ed->page,SB_VERT,ed->len,textmetric.tmHeight);
        break;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        SetFocus(win);
        SetCapture(win);
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withcapt=1;
selchg:
        j=(lparam>>16)/textmetric.tmHeight+ed->scroll;
        if(j>ed->len) j=ed->len;
        if(j<0) j=0;
        rc = HM_GetClientRect(win);
        if(msg!=WM_MOUSEMOVE) if(j<ed->len) {
            sc=ed->ew.doc->scmd+ed->tbl[j];     
            i=lparam&65535;
            if(wparam&MK_SHIFT) if(ed->csel==2) m=12; else m=16; else m=1;
            if(msg==WM_LBUTTONDOWN) m=-m;
            if(sc->cmd>=0xe0) l=op_len[sc->cmd - 0xe0]; else l=0;
            if(i>=48 && i<64) k=0;
            else if(i>=72 && i<88) k=1;
            else if(i>=96 && i<160) k=2;
            else if(l && i>=168 && i<184) k=3;
            else if(l>1 && i>=192 && i<208) k=4;
            else if(l>2 && i>=216 && i<232) k=5;
            else k=-1;
            goto dontgetrc2;
updfld:
            rc = HM_GetClientRect(win);
dontgetrc2:
            if(k==ed->csel && j==ed->sel) {
                switch(k) {
                case 0:
                    if(!(sc->flag&1)) sc->flag|=1; else sc->b1+=m;
                    if(!(sc->b1&127)) sc->b1+=m;
                    sc->b1&=0x7f;
                    goto modfrow;
                case 1:
                    if((sc->flag&3)!=3) sc->flag|=2; else sc->b2+=m;
                    if(!(sc->b2&127)) sc->b2+=m;
                    sc->b2&=0x7f;
                    goto modfrow;
                case 2:
                    if(sc->cmd<0xe0) {
                        sc->cmd+=m;
                        if(sc->cmd<0x80) sc->cmd+=0x4a;
                        else if(sc->cmd>=0xca) sc->cmd-=0x4a;
                    } else {
                        sc->cmd+=m;
                        if(sc->cmd<0x80) sc->cmd-=0x20;
                        else if(sc->cmd<0xe0) sc->cmd+=0x20;
                    }
                    goto modfrow;
                case 3:
                    sc->p1+=m;
                    goto modfrow;
                case 4:
                    sc->p2+=m;
                    goto modfrow;
                case 5:
                    sc->p3+=m;
                    goto modfrow;
                }
            } else {
                ed->csel=k;
            }
        } else ed->csel=-1;
setrow:
        if(ed->sel!=-1) {
            rc.top=(ed->sel-ed->scroll)*textmetric.tmHeight;
            rc.bottom=rc.top+textmetric.tmHeight;
            InvalidateRect(win,&rc,1);
        }
        ed->sel=j;
        goto dontgetrc;
updrow:
        rc = HM_GetClientRect(win);
dontgetrc:
        if(j>=ed->scroll+ed->page) {
            k=j-ed->page;
            goto scroll;
        }
        if(j<ed->scroll) {
            k=j;
scroll:
            ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(k<<16),ed->scroll,ed->page,SB_VERT,ed->len+1,textmetric.tmHeight);
        }
        rc.top=(j-ed->scroll)*textmetric.tmHeight;
        rc.bottom=rc.top+textmetric.tmHeight;
        InvalidateRect(win,&rc,1);
        break;
modfrow:
        ed->ew.doc->m_modf=1;
        goto updrow;
    case WM_MOUSEMOVE:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withcapt) goto selchg;
        break;
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withcapt) ReleaseCapture(),ed->withcapt=0;
        break;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_SYSKEYDOWN:
        if(!(lparam&0x20000000)) break;
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_F12:
            ed->debugflag=!ed->debugflag;
            InvalidateRect(win,0,1);
            break;
        case VK_F11:
            sc=ed->ew.doc->scmd;
            for(i=0;i<ed->len;i++) sc[ed->tbl[i]].flag&=-5;
            Getblocktime(ed->ew.doc,ed->tbl[0],0);
            InvalidateRect(win,0,1);
            break;
        case 'N':
            NewSR(ed->ew.doc,ed->ew.doc->sr[ed->ew.param].bank);
            break;
        case 'U':
            if(!mark_doc) break;
            goto unmark;
        case 'L':
            
            if(mark_doc!=ed->ew.doc || mark_sr!=ed->ew.param)
            {
                
            unmark:
                
                if(mark_doc)
                {
                    HWND w = mark_doc->sr[mark_sr].editor;
                    
                    if(w)
                    {
                        HWND sw = GetDlgItem(GetDlgItem(w, 2000), 3000);
                        rc = HM_GetClientRect(sw);
                        Trackchgsel(sw, &rc,
                                    (TRACKEDIT*) GetWindowLongPtr(sw, GWLP_USERDATA));
                    }
                }
                
                if(wparam=='U')
                {
                    mark_doc = 0;
                    break;
                }
                
                mark_doc=ed->ew.doc;
                mark_sr=ed->ew.param;
                mark_first=mark_last=ed->tbl[mark_start=mark_end=ed->sel];
                rc = HM_GetClientRect(win);
            }
            else
            {
                rc = HM_GetClientRect(win);
                Trackchgsel(win,&rc,ed);
                j=ed->sel;
                if(j>=ed->len) j=ed->len-1;
                if(ed->sel>=mark_start) mark_last=ed->tbl[mark_end=j];
                else mark_first=ed->tbl[mark_start=j];
            }
            
            Trackchgsel(win,&rc,ed);
            
            break;
        
        case 'M':
            if(!mark_doc) break;
            if(mark_doc!=ed->ew.doc) {
                MessageBox(framewnd,"Selection is in another file","Bad error happened",MB_OK);
                break;
            }
            mark_doc->m_modf=1;
            oldobj=mark_doc->sr[mark_sr].editor;
            if(oldobj) {
                oldobj=GetDlgItem(GetDlgItem(oldobj,2000),3000);
                InvalidateRect(oldobj,0,1);
                Updatesize(oldobj);
                ed2=(TRACKEDIT*)GetWindowLong(oldobj,GWL_USERDATA);
                
                CopyMemory(ed2->tbl + mark_start,
                           ed2->tbl + mark_end + 1,
                           (ed2->len - mark_end - 1) << 1);
                
                ed2->tbl=realloc(ed2->tbl,(ed2->len+=mark_start-1-mark_end)<<1);
            }
            sc=mark_doc->scmd;
            if(ed->sel)
                sc[i=ed->tbl[ed->sel-1]].next=mark_first; else i=-1,mark_doc->sr[ed->ew.param].first=mark_first;
            if(ed->sel<ed->len)
                sc[j=ed->tbl[ed->sel]].prev=mark_last; else j=-1;
            if(sc[mark_first].prev!=-1) sc[sc[mark_first].prev].next=sc[mark_last].next;
            else mark_doc->sr[mark_sr].first=sc[mark_last].next;
            if(sc[mark_first].next!=-1) sc[sc[mark_last].next].prev=sc[mark_first].prev;
            sc[mark_first].prev=i;
            sc[mark_last].next=j;
            ed->tbl=realloc(ed->tbl,(ed->len+=mark_end+1-mark_start)<<1);
            for(i=ed->sel,j=mark_first;j!=-1;i++,j=sc[j].next) ed->tbl[i]=j;
            mark_sr=ed->ew.param;
            mark_end=ed->sel+mark_end-mark_start;
            mark_start=ed->sel;
            InvalidateRect(win,0,1);
            Updatesize(win);
            break;
        case 'C':
            if(!mark_doc) break;
            sc=mark_doc->scmd;
            sc2=ed->ew.doc->scmd;
            l=ed->sel;
            if(l) k=ed->tbl[l-1]; else k=-1;
            sc3=sc2+k;
            m=mark_end+1-mark_start;
            ed->tbl=realloc(ed->tbl,(ed->len+=mark_end+1-mark_start)<<1);
            
            CopyMemory(ed->tbl + l + m,
                       ed->tbl + l,
                       (ed->len - l - m) << 1);
            
            m=mark_first;
            for(i=mark_start;i<=mark_end;i++) {
                j=AllocScmd(ed->ew.doc);
                if(k!=-1) sc3->next=j; else ed->ew.doc->sr[ed->ew.param].first=j;
                sc3=sc2+j;
                sc3->prev=k;
                *(int*)(&sc3->flag)=*(int*)(&sc[m].flag);
                *(int*)(&sc3->p3)=*(int*)(&sc[m].p3);
                k=ed->tbl[l++]=j;
                m=sc[m].next;
            }
            
            if(l == ed->len)
                sc3->next = -1;
            else
                sc3->next = ed->tbl[l], sc2[sc3->next].prev = j;
            
            ed->ew.doc->m_modf = 1;
            
            InvalidateRect(win,0,1);
            Updatesize(win);
            
            break;
        
        case 'Q':
            
            sc = ed->ew.doc->scmd;
            
            for(i = 0; i < ed->len; i++)
            {
                sc2 = sc + ed->tbl[i];
                
                if(i == 0)
                    j = -1;
                else
                    j = ed->tbl[i - 1];
                
                if(i == ed->len - 1)
                    k = -1;
                else
                    k = ed->tbl[i+1];
                
                if(sc2->prev != j || sc2->next != k)
                {
                    wsprintf(buffer,"Data is bad. Ofs %04X Prev=%04X instead of %04X, Next=%04X instead of %04X",i,sc2->prev,j,sc2->next,k);
                    MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
                    
                    return 0;
                }
            }
            MessageBox(framewnd,"Data is okay.","Check",MB_OK);
        }
        break;
    case WM_KEYDOWN:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_DOWN:
            if(ed->sel==ed->len) break;
            rc = HM_GetClientRect(win);
            j=ed->sel+1;
            goto setrow;
        case VK_UP:
            if(ed->sel<=0) break;
            GetClientRect(win,&rc);
            j=ed->sel-1;
            goto setrow;
        case VK_DELETE:
            if(ed->sel==ed->len) return 0;
            doc=ed->ew.doc;
            sc3=doc->scmd;
            j=ed->tbl[ed->sel];
            if(doc==mark_doc && ed->ew.param==mark_sr && j>=mark_start && j<=mark_end) {
                mark_end--;
                if(mark_end<mark_start) mark_doc=0;
            }
            sc=sc3+j;
            if(sc->prev!=-1) sc3[sc->prev].next=sc->next;
            else doc->sr[ed->ew.param].first=sc->next;
            if(sc->next!=-1) sc3[sc->next].prev=sc->prev;
            sc->next=doc->m_free;
            sc->prev=-1;
            doc->m_free=j;
            ed->len--;
            if(ed->len<ed->sel) ed->sel=ed->len;
            
            CopyMemory(ed->tbl + ed->sel,
                       ed->tbl + ed->sel + 1,
                       (ed->len - ed->sel) << 1);
            
            ed->tbl=realloc(ed->tbl,ed->len<<2);
            ed->ew.doc->m_modf=1;
            InvalidateRect(win,0,1);
            Updatesize(win);
            return 0;
        case VK_INSERT:
            doc=ed->ew.doc;
            k=ed->sel;
            i=AllocScmd(doc);
            sc3=doc->scmd;
            sc2=sc3+i;
            if(doc==mark_doc && ed->ew.param==mark_sr && k>mark_start && k<=mark_end) mark_end++;
            if(k) {
                sc2->prev=ed->tbl[k-1];
                sc3[sc2->prev].next=i;
            } else sc2->prev=-1,ed->ew.doc->sr[ed->ew.param].first=i;
            if(k<ed->len) {
                sc3[j=ed->tbl[k]].prev=i;
                sc2->next=j;
            } else sc2->next=-1;
            sc2->cmd=128;
            sc2->flag=0;
            ed->len++;
            ed->tbl=realloc(ed->tbl,ed->len<<2);
            MoveMemory(ed->tbl + k + 1, ed->tbl + k, (ed->len - k) << 1);
            ed->tbl[k]=i;
            ed->ew.doc->m_modf=1;
            InvalidateRect(win,0,1);
            Updatesize(win);
            return 0;
        case 13:
            if(ed->sel==ed->len) return 0;
            doc=ed->ew.doc;
            sc=doc->scmd+ed->tbl[ed->sel];
            if(sc->cmd==0xef) Edittrack(doc,*(short*)&(sc->p1));
            return 0;
        }
        if(ed->csel==2) switch(wparam) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(sc->cmd>0xc7) break;
            sc->cmd=((sc->cmd-128)%12)+(wparam-49)*12+128;
            goto modfrow;
        case 'Z':
            i=0;
            goto updnote;
        case 'S':
            i=1;
            goto updnote;
        case 'X':
            i=2;
            goto updnote;
        case 'D':
            i=3;
            goto updnote;
        case 'C':
            i=4;
            goto updnote;
        case 'F':
            i=5;
            goto updnote;
        case 'V':
            i=6;
            goto updnote;
        case 'G':
            i=7;
            goto updnote;
        case 'B':
            i=8;
            goto updnote;
        case 'H':
            i=9;
            goto updnote;
        case 'N':
            i=10;
            goto updnote;
        case 'J':
            i=11;
updnote:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(sc->cmd>0xc7) sc->cmd=128+i;
            else sc->cmd=sc->cmd-((sc->cmd-128)%12)+i;
            goto modfrow;
        case ' ':
        case 219:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xc8;
            goto modfrow;
        case VK_BACK:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xc9;
            goto modfrow;
        case 'M':
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            sc->cmd=0xe0;
            goto modfrow;
        } else switch(wparam) {
        case VK_BACK:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            if(ed->csel==0) sc->flag&=-4;
            if(ed->csel==1) sc->flag&=-3;
            goto modfrow;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            wparam-=7;
            goto digkey;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
digkey:
            j=ed->sel;
            if(j==ed->len) return 0;
            sc=ed->ew.doc->scmd+ed->tbl[j];
            k=ed->csel;
            if(j==-1 || k==-1) break;
            rc = HM_GetClientRect(win);
            m=wparam-'0';
            switch(k) {
            case 0:
                sc->flag|=1;
                sc->b1<<=4;
                if(!(sc->b1|m)) sc->b1=112;
                goto updfld;
            case 1:
                sc->flag|=3;
                sc->b2<<=4;
                if(!(sc->b2|m)) sc->b2=112;
                goto updfld;
            case 3:
                sc->p1<<=4;
                goto updfld;
            case 4:
                sc->p2<<=4;
                goto updfld;
            case 5:
                sc->p3<<=4;
                goto updfld;
            }
            break;
        }
        break;
    case WM_PAINT:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        hdc=BeginPaint(win,&ps);
        j=(ps.rcPaint.bottom+textmetric.tmHeight-1)/textmetric.tmHeight+ed->scroll;
        i=ps.rcPaint.top/textmetric.tmHeight+ed->scroll;
        if(j>=ed->len) j=ed->len;
        k=(i-ed->scroll)*textmetric.tmHeight;
        oldobj=SelectObject(hdc,trk_font);
        doc=ed->ew.doc;
        SetROP2(hdc,R2_COPYPEN);
        for(;i<j;i++) {
            l=ed->tbl[i];
            sc=doc->scmd+l;
            wsprintf(buffer,"%04X: ",l);
            if(sc->flag&2) wsprintf(buffer+6,"%02X %02X ",sc->b1,sc->b2);
            else if(sc->flag&1) wsprintf(buffer+6,"%02X    ",sc->b1);
            else wsprintf(buffer+6,"      ");
            if(sc->cmd<0xe0) {
                l=sc->cmd - 0x80;
                if(l==0x48) wsprintf(buffer+12,"--       ");
                else if(l==0x49) wsprintf(buffer+12,"Off      ");
                else wsprintf(buffer+12,"%s %d     ",note_str[l%12],l/12+1);
            } else {
                l=sc->cmd - 0xe0;
                wsprintf(buffer+12,"%s ",cmd_str[l]);
                if(op_len[l]) wsprintf(buffer+21,"%02X ",sc->p1);
                if(op_len[l]>1) wsprintf(buffer+24,"%02X ",sc->p2);
                if(op_len[l]>2) wsprintf(buffer+27,"%02X ",sc->p3);
            }
            TextOut(hdc,0,k,buffer,lstrlen(buffer));
            if(ed->debugflag) {
                wsprintf(buffer,"%04X: Flag %d Time %04X Time2 %04X",sc->addr,sc->flag,sc->tim,sc->tim2);
                TextOut(hdc,256,k,buffer,lstrlen(buffer));
            }
            
            k += textmetric.tmHeight;
        }
        
        k = (ed->sel - ed->scroll) * textmetric.tmHeight;
        
        SetROP2(hdc, R2_NOTXORPEN);
        
        oldobj3=SelectObject(hdc,null_pen);
        if(mark_doc==ed->ew.doc && mark_sr==ed->ew.param) {
            rc.top=(mark_start-ed->scroll)*textmetric.tmHeight;
            rc.bottom=(mark_end+1-ed->scroll)*textmetric.tmHeight;
            oldobj2=SelectObject(hdc,green_brush);
            Rectangle(hdc,ps.rcPaint.left-1,rc.top,ps.rcPaint.right+1,rc.bottom+1);
            SelectObject(hdc,black_brush);
        } else oldobj2=SelectObject(hdc,black_brush);
        rc.top=k;
        rc.bottom=k+textmetric.tmHeight;
        Rectangle(hdc,ps.rcPaint.left-1,rc.top,ps.rcPaint.right+1,rc.bottom+1);
        SelectObject(hdc,oldobj3);
        if(ed->csel!=-1)
        {
            rc.left=csel_l[ed->csel];
            rc.right=csel_r[ed->csel];
            DrawFocusRect(hdc,&rc);
        }
        
        SelectObject(hdc,oldobj);
        SelectObject(hdc,oldobj2);
        EndPaint(win,&ps);
        break;
        
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

char const *
Getsprstring(DUNGEDIT const * const ed,
             int              const i)
{
    int j = ed->ebuf[i+2];
    
    if(ed->ebuf[i + 1] >= 224)
        j += 256;
    
    if(j >= 0x11c)
        return "Crash";
    
    return sprname[j];
}
void Tmapobjchg(TMAPEDIT*ed,HWND win)
{
    RECT rc;
    int i,j;
    if(ed->sel!=-1) {
        Gettmapsize(ed->buf+ed->sel,&rc,0,0,0);
        ed->selrect=rc;
        rc.top&=511;
        rc.bottom&=511;
        i=ed->mapscrollh<<5;
        j=ed->mapscrollv<<5;
        rc.left-=i;
        rc.right-=i;
        rc.top-=j;
        rc.bottom-=j;
        InvalidateRect(win,&rc,0);
    }
}

// =============================================================================

void
TileMapDisplay_OnPaint(TMAPEDIT const * const p_ed,
                       HWND             const p_win)
{
    int k = 0,
        l = 0,
        n = 0,
        o = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    RealizePalette(hdc);
    
    k = ((ps.rcPaint.right + 31) & 0xffffffe0);
    l = ((ps.rcPaint.bottom + 31) & 0xffffffe0);
    n = p_ed->mapscrollh << 5;
    o = p_ed->mapscrollv << 5;
    
    if(l + o > 0x200)
        l = 0x200 - o;
    
    if(k + n > 0x200)
        k = 0x200 - n;
    
    Paintdungeon((DUNGEDIT*) p_ed,
                 hdc,
                 &(ps.rcPaint),
                 ps.rcPaint.left & 0xffffffe0,
                 ps.rcPaint.top & 0xffffffe0,
                 k,
                 l,
                 n,
                 o,
                 p_ed->nbuf);
    
    if(p_ed->sel != -1)
    {
        RECT rc;
        
        Gettmapsize(p_ed->buf + p_ed->sel, &rc, 0, 0, 0);
        
        rc.top    &= 511;
        rc.bottom &= 511;
        
        rc.left -= n;
        rc.top  -= o;
        
        rc.right  -= n;
        rc.bottom -= o;
        
        FrameRect(hdc,
                  &rc,
                  (p_ed->withfocus & 1) ? green_brush : gray_brush);
    }
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

// Tilemap... display portion window procedure?
LRESULT CALLBACK
tmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TMAPEDIT*ed;
    SCROLLINFO si;
    HWND hc;
    int i,j,k,l,n,o,p;
    unsigned char*b;
    
    RECT rc;
    short bg_ids[3]={3002,3003,3006};
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS|DLGC_WANTARROWS;
    case WM_SIZE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=16;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,16,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,16,32);
        
        break;
    
    case WM_VSCROLL:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollv=Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,16,32);
        
        break;
    
    case WM_HSCROLL:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollh=Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,16,32);
        break;
    case WM_SETFOCUS:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withfocus|=1;
        Tmapobjchg(ed,win);
        break;
    case WM_KILLFOCUS:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->withfocus&=-2;
        Tmapobjchg(ed,win);
        break;  
    
    case WM_PAINT:
        
        ed = (TMAPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            TileMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetFocus(win);
        o=(lparam&65535)+(ed->mapscrollh<<5);
        p=(lparam>>16)+(ed->mapscrollv<<5)+(ed->selbg<<9);
        if(msg==WM_RBUTTONDOWN) {
            if(ed->tool) goto draw;
            Tmapobjchg(ed,win);
            ed->len+=6;
            ed->buf=realloc(ed->buf,ed->len);
            
            i = ((o & 0xf8) >> 3) + ( ( (o & 0x100) + (p & 0xf8) ) << 2) + ((p & 0x700) << 3);
            
            if(i >= 0x2000)
                i += 0x4000;
            
            *(short*)(ed->buf+ed->len-7)=(i>>8)|(i<<8);
            *(short*)(ed->buf+ed->len-5)=256;
            *(short*)(ed->buf+ed->len-3)=0;
            ed->buf[ed->len-1] = u8_neg1;
            ed->sel=ed->len-7;
            ed->selofs=i;
            ed->sellen=1;
            Tmapobjchg(ed,win);
            break;
        }
        b=ed->buf;
        if(ed->sel!=-1 && o>=ed->selrect.left && o<ed->selrect.right && p>=ed->selrect.top && p<ed->selrect.bottom) goto movesel;
        Tmapobjchg(ed,win);
        ed->sel=-1;
        for(i=0;;) {
            if(b[i]>=128) break;
            Gettmapsize(b+i,&rc,&k,&l,&n);
            if(o>=rc.left && o<rc.right && p>=rc.top && p<rc.bottom) {
                ed->sel=i;
                ed->selofs=l;
                ed->sellen=n>>1;
            }
            i+=k;
        }
        if(ed->sel!=-1) {
movesel:
            if(ed->tool) o&=-9,p&=-9;
            ed->dragx=o;
            ed->dragy=p;
            ed->withfocus|=2;
            Tmapobjchg(ed,win);
            SetCapture(win);
            if(ed->tool) goto draw;
        }
        break;
    case WM_LBUTTONUP:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->withfocus&2) ed->withfocus&=-3,ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        o=(lparam&65535)+(ed->mapscrollh<<5);
        p=(lparam>>16)+(ed->mapscrollv<<5)+(ed->selbg<<9);
        if(ed->withfocus&2) {
            if(o>ed->dragx+7 || o<ed->dragx || p>ed->dragy+7 || p<ed->dragy) {
draw:
                if(ed->tool) {
                    if(ed->buf[ed->sel+2]&64)
                    {
                        if(msg==WM_RBUTTONDOWN)
                        {
                            k=*(short*)(ed->buf+ed->sel+4);
                            goto getblk;
                        } else *(short*)(ed->buf+ed->sel+4)=ed->bs.sel+ed->bs.flags; goto updblk;
                    }
                    else if(ed->buf[ed->sel+2]&128)
                    {
                        if(p>=ed->selrect.top && p<ed->selrect.bottom)
                        {
                            if(msg==WM_RBUTTONDOWN)
                            {
                                k = ldle16b
                                (
                                    ed->buf + ed->sel + 4
                                  + ( ( ( p - ed->selrect.top ) >> 2) & -2 )
                                );
                                
                                goto getblk;
                            }
                            
                            stle16b
                            (
                                ed->buf + ed->sel + 4
                              + ( ( (p - ed->selrect.top) >> 2) & -2 ),
                                ed->bs.sel + ed->bs.flags
                            );
                            
                            goto updblk;
                        }
                    }
                    else
                    {
                        i = ( (o & 0xf8) >> 3 )
                          + ( ( (o & 0x100) + (p & 0xf8) ) << 2)
                          + ( (p & 0x700) << 3 );
                        
                        if(i >= 0x2000)
                            i += 0x4000;
                        
                        if(i>=ed->selofs && i<ed->selofs+ed->sellen)
                        {
                            if(msg==WM_RBUTTONDOWN)
                            {
                                k = *(short*)( ed->buf + ed->sel + 4 + ( (i - ed->selofs) << 1) );
getblk:
                                hc=GetDlgItem(ed->dlg,3001);
                                Changeblk8sel(hc,&(ed->bs));
                                ed->bs.sel=k;
                                ed->bs.flags=ed->bs.sel&0xfc00;
                                ed->bs.sel&=0x3ff;
                                
                                if((ed->bs.flags&0xdc00)!=ed->bs.oldflags)
                                {
                                    HPALETTE const
                                    oldpal = SelectPalette(objdc,
                                                           ed->bs.ed->hpal,
                                                           1);
                                    
                                    HPALETTE const
                                    oldpal2 = SelectPalette(ed->bs.bufdc,
                                                            ed->bs.ed->hpal,
                                                            1);
                                    
                                    ed->bs.oldflags=ed->bs.flags;
                                    
                                    for(i=0;i<256;i++)
                                        Updateblk8sel(&(ed->bs),i);
                                    
                                    SelectPalette(objdc,oldpal,1);
                                    SelectPalette(ed->bs.bufdc,oldpal2,1);
                                    
                                    InvalidateRect(hc,0,0);
                                }
                                else
                                    Changeblk8sel(hc,&(ed->bs));
                                
                                CheckDlgButton(ed->dlg,3011,(ed->bs.flags&0x2000)>>13);
                                CheckDlgButton(ed->dlg,3009,(ed->bs.flags&0x4000)>>14);
                                CheckDlgButton(ed->dlg,3010,(ed->bs.flags&0x8000)>>15);
                                
                                break;
                            }
                            
                            *(short*)(ed->buf + ed->sel + 4 + ( (i - ed->selofs) << 1))
                                = ed->bs.sel+ed->bs.flags;
updblk:
                            Updtmap(ed);
                            Tmapobjchg(ed,win);
                            ed->modf=1;
                        }
                    }
                    
                    ed->dragx = o & -9;
                    ed->dragy = p & -9;
                    
                    break;
                }
                
                i = (o - ed->dragx) >> 3;
                j = (p - ed->dragy) >> 3;
                
                if(!(i||j)) break;
                
                Tmapobjchg(ed,win);
                k=((ed->buf[ed->sel])<<8)+ed->buf[ed->sel+1];
                l=(k&31)+((k&1024)>>5)+i;
                n=((k&992)>>5)+((k&2048)>>6)+j;
                if(l<0 || l>63 || n<0 || n>63) ed->withfocus|=4,SetCursor(forbid_cursor); else {
                    n+=ed->selbg<<6;
                    k=(l&31)+((l&32)<<5)+((n&31)<<5)+((n&224)<<6);
                    if(k>=0x2000) k+=0x4000;
                    ed->buf[ed->sel]=k>>8;
                    ed->buf[ed->sel+1]=k;
                    ed->dragx+=i<<3;
                    ed->dragy+=j<<3;
                    ed->modf=1;
                    Updtmap(ed);
                    Tmapobjchg(ed,win);
                    if(ed->withfocus&4) {SetCursor(normal_cursor);ed->withfocus&=-5;}
                }
            }
        }
        break;
    case WM_KEYDOWN:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_RIGHT:
            Tmapobjchg(ed,win);
            i=ed->buf[ed->sel+2];
            j=((i&63)<<8)+ed->buf[ed->sel+3]+1;
            if(i&64) ed->sel+=6; else ed->sel+=j+4;
            if(ed->buf[ed->sel]>=128) ed->sel=-1;
            Tmapobjchg(ed,win);
            break;
        case VK_LEFT:
            if(ed->sel==-1) break;
            Tmapobjchg(ed,win);
            if(!ed->sel) i=-1; else for(i=0;;) {
                k=ed->buf[ed->sel+2];
                j=((k&63)<<8)+ed->buf[ed->sel+3]+1;
                if(i&64) l=i+6;
                else l=i+4+j;
                if(l==ed->sel) break;
            }
            ed->sel=i;
            Tmapobjchg(ed,win);
            break;
        }
        break;
    case WM_CHAR:
        ed=(TMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(wparam>=96) wparam-=32;
        switch(wparam) {
        case '1': case '2': case '3':
            if(ed->sel==-1) break;
            i=ed->buf[ed->sel];
            ed->buf[ed->sel]=(i&15)|"\0\20\140"[wparam-49];
            CheckDlgButton(ed->dlg,3002,wparam==49);
            CheckDlgButton(ed->dlg,3003,wparam==50);
            CheckDlgButton(ed->dlg,3006,wparam==51);
            SendMessage(ed->dlg,WM_COMMAND,bg_ids[wparam-49],0);
            Updtmap(ed);
            Tmapobjchg(ed,win);
            break;
        case 8:
            Tmapobjchg(ed,win);
            Gettmapsize(ed->buf+ed->sel,&rc,&k,&l,&n);
            ed->len-=k;
            memcpy(ed->buf+ed->sel,ed->buf+ed->sel+k,ed->len-ed->sel);
            ed->buf=realloc(ed->buf,ed->len);
            ed->sel=-1;
            Updtmap(ed);
            break;
        case 'M':
            if(ed->sel!=-1) {
                Tmapobjchg(ed,win);
                ed->buf[ed->sel+2]^=128;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            break;
        case ',':
            if(ed->sel!=-1) {
                Tmapobjchg(ed,win);
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]-1;
                if(!j) break;
                if(!(i&64)) {
                    memcpy(ed->buf+ed->sel+4+j,ed->buf+ed->sel+6+j,ed->len-ed->sel-6-j);
                    ed->buf=realloc(ed->buf,ed->len-=2);
                }
                j--;
                ed->sellen--;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j;
                Updtmap(ed);
                ed->modf=1;
            }
            break;
        case '.':
            if(ed->sel!=-1) {
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]+3;
                if(!(i&64)) {
                    ed->buf=realloc(ed->buf,ed->len+=2);
                    memmove(ed->buf+ed->sel+4+j,ed->buf+ed->sel+2+j,ed->len-4-j-ed->sel);
                    *(short*)(ed->buf+ed->sel+2+j)=0;
                }
                j--;
                ed->sellen++;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            break;
        case '-':
            if(ed->sel!=-1) {
                i=ed->buf[ed->sel+2];
                j=((i&63)<<8)+ed->buf[ed->sel+3]-1;
                if(i&64) {
                    j++;
                    ed->buf=realloc(ed->buf,ed->len+=j),memmove(ed->buf+ed->sel+6+j,ed->buf+ed->sel+6,ed->len-ed->sel-6-j);
                    for(k=j;k;k-=2) *(short*)(ed->buf+ed->sel+4+k)=*(short*)(ed->buf+ed->sel+4);
                } else memcpy(ed->buf+ed->sel+6,ed->buf+ed->sel+6+j,ed->len-ed->sel-6-j),ed->buf=realloc(ed->buf,ed->len-=j),j--;
                i^=64;
                ed->buf[ed->sel+2]=(i&192)+(j>>8);
                ed->buf[ed->sel+3]=j+1;
                Updtmap(ed);
                Tmapobjchg(ed,win);
                ed->modf=1;
            }
            
            break;
        }
        
        break;
    
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
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

// =============================================================================

int Fixscrollpos(unsigned char*rom,int x,int y,int sx,int sy,int cx,int cy,int dp,int m,int l,int door1,int door2)
{
    int j,k,n,o,p,q,r,s,t,u;
    r=((short*)(rom+x))[m];
    s=((short*)(rom+y))[m];
    if(door1) if(((unsigned short*)(rom+door2))[m]+1>=2) s+=39; else s+=19;
    n=rom[0x125ec + (r>>9)+(s>>9<<3)]+(l&64);
    j=((short*)(rom+sx))[m];
    k=((short*)(rom+sy))[m];
    o=(n&56)<<6;
    p=(n&7)<<9;
    q=rom[0x12844 + n]?1024:512;
    t=((short*)(rom+cy))[m];
    u=((short*)(rom+cx))[m];
    j+=r-u;
    k+=s-t;
    t=s;
    u=r;
    if(k<o) {
        t+=o-k;
        k=o;
    }
    if(k>o+q-256) {
        t+=o-k+q-256;
        k=o+q-256;
    }
    if(j<p) {
        u+=p-j;
        j=p;
    }
    if(j>p+q-240) {
        u+=p-j+q-240;
        j=p+q-240;
    }
    ((short*)(rom+cy))[m]=t;
    ((short*)(rom+cx))[m]=u;
    ((short*)(rom+sx))[m]=j;
    ((short*)(rom+sy))[m]=k;
    ((short*)(rom+dp))[m]=(((k-((n&56)<<6))&0xfff0)<<3)|(((j-((n&7)<<9))&0xfff0)>>3);
    if(door1) {
        if(((unsigned short*)(rom+door1))[m]+1>=2) ((short*)(rom+door1))[m]+=(((l-n)&7)<<6)+(((l&56)-(n&56))<<3);
        if(((unsigned short*)(rom+door2))[m]+1>=2) ((short*)(rom+door2))[m]+=(((l-n)&7)<<6)+(((l&56)-(n&56))<<3);
    }
    return n;
}

// =============================================================================

void lmapblkchg(HWND win,LMAPEDIT*ed)
{
    RECT rc;
    rc.left = ed->blksel % ed->blkrow << 4;
    rc.top = (ed->blksel / ed->blkrow - ed->blkscroll) << 4;
    rc.right=rc.left+16;
    rc.bottom=rc.top+16;
    InvalidateRect(win,&rc,0);
}

// =============================================================================

LRESULT CALLBACK
lmapblksproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k,l,n,o,p,q;
    
    HDC hdc;
    HPALETTE oldpal;
    HGDIOBJ oldobj;
    SCROLLINFO si;
    LMAPEDIT*ed;
    PAINTSTRUCT ps;
    RECT rc;
    
    unsigned char*rom;
    
    switch(msg)
    {
    
    case WM_LBUTTONDOWN:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        i=(lparam&65535)>>4;
        j=lparam>>20;
        if(i<0 || i>=ed->blkrow || j<0) break;
        i+=(j+ed->blkscroll)*ed->blkrow;
        if(i>=0xba) break;
        lmapblkchg(win,ed);
        ed->blksel=i;
        lmapblkchg(win,ed);
        InvalidateRect(win,&rc,0);
        break;
    case WM_PAINT:
        
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        hdc=BeginPaint(win,&ps);
        oldpal=SelectPalette(hdc,ed->hpal,1);
        RealizePalette(hdc);
        k=((ps.rcPaint.right+31)&0xffffffe0);
        l=((ps.rcPaint.bottom+31)&0xffffffe0);
        j=ps.rcPaint.top&0xffffffe0;
        o=((j>>4)+ed->blkscroll)*ed->blkrow;
        rom=ed->ew.doc->rom;
        oldobj=SelectObject(hdc,white_pen);
        for(;j<l;j+=32) {
            i=ps.rcPaint.left&0xffffffe0;
            n=i>>4;
            for(;i<k;i+=32) {
                if(n>=ed->blkrow) break;
                for(q=0;q<32;q+=16) {
                    for(p=0;p<32;p+=16) {
                        if(n+o<0xba) {
                            Drawblock((OVEREDIT*)ed,p,q,((short*)(rom + 0x57009))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p+8,q,((short*)(rom + 0x5700b))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p,q+8,((short*)(rom + 0x5700d))[(n+o) << 2],0);
                            Drawblock((OVEREDIT*)ed,p+8,q+8,((short*)(rom + 0x5700f))[(n+o) << 2],0);
                        } else {
                            Drawblock((OVEREDIT*)ed,p,q,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p+8,q,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p,q+8,0xf00,0);
                            Drawblock((OVEREDIT*)ed,p+8,q+8,0xf00,0);
                        }
                        n++;
                    }
                    n+=ed->blkrow-2;
                }
                n+=2;
                n-=ed->blkrow<<1;
                Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)ed);
                if(ed->disp&1) {
                    MoveToEx(hdc,i+32,j,0);
                    LineTo(hdc,i,j);
                    LineTo(hdc,i,j+32);
                    MoveToEx(hdc,i+16,j,0);
                    LineTo(hdc,i+16,j+32);
                    MoveToEx(hdc,i,j+16,0);
                    LineTo(hdc,i+32,j+16);
                }
            }
            o+=ed->blkrow<<1;
        }
        
        rc.left=(ed->blksel%ed->blkrow)<<4;
        rc.top=(ed->blksel/ed->blkrow-ed->blkscroll)<<4;
        rc.right=rc.left+16;
        rc.bottom=rc.top+16;
        
        FrameRect(hdc, &rc, green_brush);
        
        SelectObject(hdc, oldobj);
        SelectPalette(hdc, oldpal, 1);
        
        EndPaint(win,&ps);
        
        break;
    
    case WM_SIZE:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        ed->blkrow=(lparam&65535)>>4;
        si.nMax=(0xba + ed->blkrow-1)/ed->blkrow;
        ed->maxrow=si.nMax;
        si.nPage=(lparam>>20);
        ed->blkpage=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->blkscroll=Handlescroll(win,-1,ed->blkscroll,ed->blkpage,SB_VERT,si.nMax,16);
        break;
    case WM_VSCROLL:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->blkscroll=Handlescroll(win,wparam,ed->blkscroll,ed->blkpage,SB_VERT,ed->maxrow,16);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

void
LevelMapDisplay_OnPaint(LMAPEDIT const * const p_ed,
                        HWND             const p_win)
{
    unsigned short const * b2 = 0;
    
    unsigned short const * b4 = 0;
    
    int k = 0,
        l = 0,
        j = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    k = ( (ps.rcPaint.right + 31) & 0xffffffe0 );
    l = ( (ps.rcPaint.bottom + 31) & 0xffffffe0 );
    j = ps.rcPaint.top & 0xffffffe0;
    b4 = p_ed->nbuf + (j << 2) + 0x6f;
    
    for( ; j < l; j += 32)
    {
        int i = ps.rcPaint.left&0xffffffe0;
        int s = i >> 3;
        
        if(j >= 0xe0)
            break;
        
        b2 = b4 + s;
        
        for( ; i < k; i += 32)
        {
            int q = 0;
            
            if(i >= 0x1f0)
                break;
            
            for(q=0;q<32;q+=8)
            {
                int p = 0;
                
                for(p=0;p<32;p+=8)
                {
                    Drawblock((OVEREDIT*) p_ed,p,q,b2[(p>>3)+(q<<2)],0);
                }
            }
            
            b2 += 4;
            s += 4;
            
            Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)p_ed);
        }
        
        b4 += 128;
    }
    
    if(p_ed->tool)
    {
        RECT rc;
        
        HGDIOBJ oldfont;
        
        k = (p_ed->basements + p_ed->curfloor) * 25;
        
        if(p_ed->tool==2 && p_ed->bosspos>=k && p_ed->bosspos<k+25)
        {
            HGDIOBJ const oldobj = SelectObject(hdc, red_brush);
            
            l=p_ed->bosspos-k;
            
            rc.left=((l%5)<<4)+20+(p_ed->bossofs>>8);
            rc.top=((l/5)<<4)+4+(p_ed->bossofs&255);
            rc.right=rc.left+16;
            rc.bottom=rc.top+16;
            
            Drawdot(hdc,&rc,0x1f0,0,0);
            SelectObject(hdc, oldobj);
        }
        
        oldfont = SelectObject(hdc, trk_font);
        SetBkMode(hdc,TRANSPARENT);
        
        for(j = 0; j < 5; j++)
        {
            int i = 0;
            
            for(i=0;i<5;i++)
            {
                l=p_ed->rbuf[k];
                rc.left=(i<<4)+24;
                rc.top=(j<<4)+8;
                
                if(p_ed->tool==1 && k==p_ed->sel)
                {
                    HGDIOBJ const oldobj2 = SelectObject(hdc, green_pen);
                    HGDIOBJ const oldobj3 = SelectObject(hdc, black_brush);
                    
                    rc.right=rc.left+16;
                    rc.bottom=rc.top+16;
                    
                    Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom);
                    
                    SelectObject(hdc,oldobj2);
                    SelectObject(hdc,oldobj3);
                }
                
                if(l != 15)
                {
                    wsprintf(buffer,"%02X",l);
                    Paintspr(hdc,rc.left,rc.top,0,0,0x1f0);
                }
                
                k++;
            }
        }
        
        SelectObject(hdc, oldfont);
    }
    
    if( (p_ed->disp & 1) && !p_ed->tool )
    {
        int i = 0;
        
        HGDIOBJ const oldobj = SelectObject(hdc, white_pen);
        
        // -----------------------------
        
        k = (p_ed->basements + p_ed->curfloor) * 25;
        
        for(i = 0; i < 5; i++)
        {
            if((p_ed->rbuf[k] != 15))
            {
                MoveToEx(hdc,(i<<4)+24,8,0);
                LineTo(hdc,(i<<4)+41,8);
            }
            k++;
        }
        
        for(j = 1; j < 5; j++)
        {
            for(i=0;i<5;i++) {
                if((p_ed->rbuf[k]==15)==(p_ed->rbuf[k-5]!=15)) {
                    MoveToEx(hdc,(i<<4)+24,(j<<4)+8,0);
                    LineTo(hdc,(i<<4)+41,(j<<4)+8);
                }
                k++;
            }
        }
        
        k-=5;
        
        for(i=0;i<5;i++) {
            if((p_ed->rbuf[k]!=15)) {
                MoveToEx(hdc,(i<<4)+24,88,0);
                LineTo(hdc,(i<<4)+41,88);
            }
            k++;
        }
        
        k-=25;
        
        for(j=0;j<5;j++)
        {
            int i = 0;
            
            if((p_ed->rbuf[k]!=15))
            {
                MoveToEx(hdc,24,(j<<4)+8,0);
                LineTo(hdc,24,(j<<4)+24);
            }
            
            k++;
            
            for(i=1;i<5;i++)
            {
                if((p_ed->rbuf[k]==15)==(p_ed->rbuf[k-1]!=15)) {
                    MoveToEx(hdc,(i<<4)+24,(j<<4)+8,0);
                    LineTo(hdc,(i<<4)+24,(j<<4)+24);
                }
                k++;
            }
            
            if((p_ed->rbuf[k-1]!=15))
            {
                MoveToEx(hdc,104,(j<<4)+8,0);
                LineTo(hdc,104,(j<<4)+24);
            }
        }
        
        SelectObject(hdc, oldobj);
    }
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
lmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k = 0,l = 0,p,q,s;
    
    LMAPEDIT *ed;
    
    RECT rc;
    
    switch(msg)
    {
    
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS | DLGC_WANTARROWS;
    
    case WM_CHAR:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(wparam>=0x40) wparam&=0xdf;
        switch(wparam) {
        case 8:
            if(ed->tool==2) goto setboss;
            if(!ed->tool) break;
            if(ed->rbuf[ed->sel]==15) break;
            k=0;
            for(j=0;j<ed->sel;j++) if(ed->rbuf[j]!=15) k++;
            ed->len--;
            memcpy(ed->buf+k,ed->buf+k+1,ed->len-k);
            ed->buf=realloc(ed->buf,ed->len);
            ed->rbuf[ed->sel]=15;
            goto updblk;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            wparam-=55;
            goto digit;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            wparam-=48;
digit:
            if(ed->tool!=1) break;
            i=ed->rbuf[ed->sel];
            if(i==15) {
                k=0;
                for(j=0;j<ed->sel;j++) if(ed->rbuf[j]!=15) k++;
                ed->len++;
                ed->buf=realloc(ed->buf,ed->len);
                memmove(ed->buf+k+1,ed->buf+k,ed->len-k-1);
                ed->buf[k]=0;
            }
            i=((i<<4)+wparam)&255;
            if(i==15) i=255;
            ed->rbuf[ed->sel]=i;
updblk:
            ed->modf=1;
            Paintfloor(ed);
            s=(ed->curfloor+ed->basements)*25;
            if(ed->sel>=s && ed->sel<s+25) {
                rc.left=(((ed->sel-s)%5)<<4)+24;
                rc.top=(((ed->sel-s)/5)<<4)+8;
                rc.right=rc.left+16;
                rc.bottom=rc.top+16;
                InvalidateRect(win,&rc,0);
            }
        }
        return 0;
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        k=lparam&65535;
        l=lparam>>16;
        if(k>=24 && k<104 && l>=8 && l<88) {
            k-=24;
            l-=8;
setboss:
            s=(ed->curfloor+ed->basements)*25;
            switch(ed->tool) {
            case 1:
                j=ed->sel;
                p=q=0;
                goto chgloc;
            case 2:
                j=ed->bosspos;
                p=(ed->bossofs>>8)-4;
                q=(ed->bossofs&255)-4;
chgloc:
                if(j>=s && j<s+25) {
                    rc.left=(((j-s)%5)<<4)+24+p;
                    rc.top=(((j-s)/5)<<4)+8+q;
                    rc.right=rc.left+16;
                    rc.bottom=rc.top+16;
                    InvalidateRect(win,&rc,0);
                }
            }
            s+=(k>>4)+(l>>4)*5;
            switch(ed->tool) {
            case 0:
                j=ed->rbuf[s];
                if(j==15) break;
                q=0;
                for(i=0;;i++) {
                    p=ed->rbuf[i];
                    if(p!=15) if(p==j) break; else q++;
                }
                if(msg==WM_RBUTTONDOWN) {
                    lmapblkchg(win,ed);
                    ed->blksel=ed->buf[q];
                    lmapblkchg(win,ed);
                    break;
                }
                ed->buf[q] = (unsigned char) ed->blksel;
                Paintfloor(ed);
                ed->modf=1;
                k&=0xfff0;
                l&=0xfff0;
                break;
            case 1:
                ed->sel=s;
                k&=0xfff0;
                l&=0xfff0;
                break;
            case 2:
                if(msg==WM_CHAR) {
                    ed->bosspos=-1;
                    ed->bossroom=15;
                } else {
                    ed->bosspos=((ed->bossroom=ed->rbuf[s])!=15)?s:-1;
                    ed->bossofs=((k&15)<<8)+(l&15);
                    ed->modf=1;
                }
                if(ed->bossroom==15) return 0;
                k-=4;
                l-=4;
                break;
            }
            rc.left=k+24;
            rc.top=l+8;
            rc.right=rc.left+16;
            rc.bottom=rc.top+16;
            InvalidateRect(win,&rc,0);
        } else if(k<16 && l>=80 && l<88) {
            if(msg==WM_RBUTTONDOWN) {
                if(ed->basements) ed->basements--,ed->floors++,ed->curfloor++;
                else break;
            } else {
                if(ed->floors) ed->basements++,ed->floors--,ed->curfloor--;
                else break;
            }
            Paintfloor(ed);
            rc.left=0;
            rc.top=80;
            rc.right=16;
            rc.bottom=88;
            InvalidateRect(win,&rc,0);
            ed->modf=1;
        }
        
        break;
    
    case WM_PAINT:
        
        ed = (LMAPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            LevelMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

void Updateblk32disp(BLOCKEDIT32 *ed, int num)
{
    int k,p=num<<2;
    unsigned short *o;
    RECT rc;
    rc.left=blkx[p];
    rc.right=rc.left+16;
    rc.top=blky[p];
    rc.bottom=rc.top+16;
    o=(unsigned short*)(ed->bs.ed->ew.doc->rom + 0x78000 + (ed->blks[num]<<3));
    for(k=0;k<4;k++) {
        Drawblock(ed->bs.ed,blkx[p],blky[p],o[k],0);
        p++;
    }
    Paintblocks(&rc,objdc,0,0,(DUNGEDIT*)(ed->bs.ed));
    StretchBlt(ed->bufdc,(num&1)*ed->w>>1,(num&2)*ed->h>>2,ed->w>>1,ed->h>>1,objdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SRCCOPY);
}

void Updateblk16disp(BLOCKEDIT16 *ed, int num)
{
    RECT rc;
    rc.left=blkx[num];
    rc.right=rc.left+8;
    rc.top=blky[num];
    rc.bottom=rc.top+8;
    Drawblock(ed->bs.ed,blkx[num],blky[num],ed->blks[num],0);
    Paintblocks(&rc,objdc,0,0,(DUNGEDIT*)(ed->bs.ed));
    StretchBlt(ed->bufdc,(num&1)*ed->w>>1,(num&2)*ed->h>>2,ed->w>>1,ed->h>>1,objdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SRCCOPY);
}

LRESULT CALLBACK
blkedit16proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    BLOCKEDIT16*ed;
    PAINTSTRUCT ps;
    HPALETTE oldpal;
    HDC hdc;
    HBRUSH hbr;
    int i;
    switch(msg) {
    case WM_LBUTTONDOWN:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        ed->blks[i]=ed->bs.sel|ed->bs.flags;
        oldpal=SelectPalette(objdc,ed->bs.ed->hpal,1);
        SelectPalette(ed->bufdc,ed->bs.ed->hpal,1);
        Updateblk16disp(ed,i);
        SelectPalette(objdc,oldpal,1);
        SelectPalette(ed->bufdc,oldpal,1);
        InvalidateRect(win,0,0);
        break;
    case WM_RBUTTONDOWN:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        SendMessage(GetParent(win),4000,ed->blks[i],0);
        break;
    case WM_PAINT:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        hdc=BeginPaint(win,&ps);
        oldpal=SelectPalette(hdc,ed->bs.ed->hpal,1);
        RealizePalette(hdc);
        BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top,ed->bufdc,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
        hbr=SelectObject(hdc,white_pen);
        MoveToEx(hdc,ed->w>>1,0,0);
        LineTo(hdc,ed->w>>1,ed->h-1);
        MoveToEx(hdc,0,ed->h>>1,0);
        LineTo(hdc,ed->w-1,ed->h>>1);
        SelectObject(hdc,hbr);
        SelectPalette(hdc,oldpal,1);
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
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
BOOL CALLBACK editblock16(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT*oe;
    BLOCKEDIT16*ed;
    BLOCKSEL8*bs;
    BLOCKEDIT32*be;
    HWND hc,hc2;
    HDC hdc;
    RECT rc;
    FDOC*doc;
    int i,j;
    short*l;
    unsigned char*rom;
    switch(msg) {
    case WM_QUERYNEWPALETTE:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        Setpalette(win,ed->bs.ed->hpal);
        return 1;
    case WM_PALETTECHANGED:
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        break;
    case WM_INITDIALOG:
        ed=malloc(sizeof(BLOCKEDIT16));
        bs=&(ed->bs);
        be=ed->be=(BLOCKEDIT32*)lparam;
        oe=bs->ed=be->bs.ed;
        doc=oe->ew.doc;
        rom=doc->rom;
        l=(short*)(rom + 0x78000 + (be->bs.sel<<3));
        *(int*)(ed->blks)=*(int*)l;
        *(int*)(ed->blks+2)=((int*)l)[1];
        SetWindowLong(win,GWL_USERDATA,(int)ed);
        hc=GetDlgItem(win,IDC_CUSTOM1);
        rc = HM_GetClientRect(hc);
        ed->w=rc.right;
        ed->h=rc.bottom;
        hdc=GetDC(win);
        ed->bufdc=CreateCompatibleDC(hdc);
        ed->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        SelectObject(ed->bufdc,ed->bufbmp);
        SelectPalette(ed->bufdc,oe->hpal,1);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        hc=GetDlgItem(win,IDC_CUSTOM2);
        InitBlksel8(hc,bs,oe->hpal,hdc);
        ReleaseDC(win,hdc);
        for(i=0;i<4;i++) Updateblk16disp(ed,i);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        Updatesize(hc);
        wparam=0;
    case 4000:
        bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        bs->flags=wparam&0xfc00;
        CheckDlgButton(win,IDC_CHECK1,(wparam&16384)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK2,(wparam&32768)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(win,IDC_CHECK3,(wparam&8192)?BST_CHECKED:BST_UNCHECKED);
        SetDlgItemInt(win,IDC_EDIT2,wparam&1023,0);
        SetDlgItemInt(win,IDC_EDIT1,(wparam>>10)&7,0);
        break;
    case 4001:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        for(i=0;i<4;i++) Updateblk16disp(ed,i);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        hc=GetParent(win);
        be=ed->be;
        for(i=0;i<4;i++) Updateblk32disp(be,i);
        InvalidateRect(GetDlgItem(hc,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(hc,IDC_CUSTOM2),0,0);
        break;
    case WM_DESTROY:
        ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
        DeleteDC(ed->bufdc);
        DeleteObject(ed->bufbmp);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        free(ed);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT2|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            i=GetDlgItemInt(win,IDC_EDIT2,0,0);
            if(i<0) i=0;
            if(i>0x3ff) i=0x3ff;
            j=bs->scroll<<4;
            if(i<j) j=i;
            if(i>=j+256) j=i-240;
            hc=GetDlgItem(win,IDC_CUSTOM2);
            SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
            Changeblk8sel(hc,bs);
            bs->sel=i;
            if(i<0x200) {
                SetDlgItemInt(win,IDC_EDIT3,bs->ed->ew.doc->rom[0x71459 + i],0);
                ShowWindow(GetDlgItem(win,IDC_EDIT3),SW_SHOW);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_SHOW);
            } else {
                ShowWindow(GetDlgItem(win,IDC_EDIT3),SW_HIDE);
                ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            }
            Changeblk8sel(hc,bs);
            break;
        case IDC_EDIT1|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            bs->flags&=0xe000;
            bs->flags|=(GetDlgItemInt(win,IDC_EDIT1,0,0)&7)<<10;
            goto updflag;
            break;
        case IDC_EDIT3|(EN_CHANGE<<16):
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            if(bs->sel<0x200)
            bs->ed->ew.doc->rom[0x71459 + bs->sel]=GetDlgItemInt(win,IDC_EDIT3,0,0);
            break;
        case IDC_CHECK1:
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            bs->flags&=0xbc00;
            if(IsDlgButtonChecked(win,IDC_CHECK1)) bs->flags|=0x4000;
            goto updflag;
        case IDC_CHECK2:
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            bs->flags&=0x7c00;
            if(IsDlgButtonChecked(win,IDC_CHECK2)) bs->flags|=0x8000;
updflag:
            if((bs->flags&0xdc00)!=bs->oldflags)
            {
                HPALETTE const
                oldpal = SelectPalette(objdc, bs->ed->hpal, 1);
                
                HPALETTE const
                oldpal2 = SelectPalette(bs->bufdc, bs->ed->hpal, 1);
                
                bs->oldflags = bs->flags & 0xdc00;
                
                for(i = 0; i < 256; i++)
                    Updateblk8sel(bs, i);
                
                SelectPalette(objdc, oldpal, 1);
                SelectPalette(bs->bufdc, oldpal2, 1);
                
                InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
            }
            
            break;
        case IDC_CHECK3:
            bs=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
            bs->flags&=0xdc00;
            if(IsDlgButtonChecked(win,IDC_CHECK3)) bs->flags|=0x2000;
            break;
        case IDOK:
            ed=(BLOCKEDIT16*)GetWindowLong(win,GWL_USERDATA);
            oe=ed->bs.ed;
            doc=oe->ew.doc;
            rom=doc->rom;
            i=oe->selblk;
            l=(short*)(rom + 0x78000 + (ed->be->bs.sel<<3));
            *(int*)l=*(int*)(ed->blks);
            ((int*)l)[1]=*(int*)(ed->blks+2);
            
            for(i=0;i<160;i++)
            {
                hc2 = doc->overworld[i].win;
                
                if(hc2 != 0)
                {
                    hc2=GetDlgItem(hc2,2000);
                    hc=GetDlgItem(hc2,3000);
                    InvalidateRect(hc,0,0);
                    hc=GetDlgItem(hc2,3001);
                    InvalidateRect(hc,0,0);
                }
            }
            
            doc->modf=1;
            
            EndDialog(win,1);
            
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

LRESULT CALLBACK
blkedit32proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLOCKEDIT32*ed;
    PAINTSTRUCT ps;
    
    int i;
    
    switch(msg)
    {
    
    case WM_LBUTTONDOWN:
        
        ed=(BLOCKEDIT32*)GetWindowLong(win,GWL_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        ed->blks[i]=ed->bs.sel;
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->bs.ed->hpal, 1);
            
            Updateblk32disp(ed, i);
            
            SelectPalette(objdc, oldpal, 1);
        }
        
        InvalidateRect(win, 0, 0);
        
        break;
    
    case WM_RBUTTONDOWN:
        ed=(BLOCKEDIT32*)GetWindowLong(win,GWL_USERDATA);
        i=((lparam&65535)<<1)/ed->w+((((lparam>>16)<<2)/ed->h)&2);
        SendMessage(GetParent(win),4000,ed->blks[i],0);
        break;
    
    case WM_PAINT:
        
        ed = (BLOCKEDIT32*) GetWindowLong(win, GWL_USERDATA);
        
        if(always)
        {
            HDC const hdc = BeginPaint(win,&ps);
            
            HPALETTE const oldpal = SelectPalette(hdc, ed->bs.ed->hpal, 1);
            
            HGDIOBJ oldpen;
            
            RealizePalette(hdc);
            
            BitBlt(hdc,
                   ps.rcPaint.left,
                   ps.rcPaint.top,
                   ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - ps.rcPaint.top,
                   ed->bufdc,
                   ps.rcPaint.left,
                   ps.rcPaint.top,
                   SRCCOPY);
            
            oldpen = SelectObject(hdc, white_pen);
            
            MoveToEx(hdc, ed->w >> 1, 0, 0);
            LineTo(hdc, ed->w >> 1, ed->h - 1);
            
            MoveToEx(hdc, 0, ed->h >> 1, 0);
            LineTo(hdc, ed->w - 1, ed->h >> 1);
            
            SelectObject(hdc, oldpen);
            SelectPalette(hdc, oldpal, 1);
            
            EndPaint(win, &ps);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

LRESULT CALLBACK
blkedit8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLKEDIT8*ed;
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch(msg)
    {
    
    case WM_PAINT:
        
        ed = (BLKEDIT8*)GetWindowLong(win,GWL_USERDATA);
        
        if(!ed) break;
        
        hdc=BeginPaint(win,&ps);
        
        if(ed->hpal!=ed->oed->hpal) memcpy(ed->pal,ed->oed->pal,1024);
        
        ed->hpal=ed->oed->hpal;
        
        if(ed->oed->hpal)
        {
            HPALETTE const oldpal = SelectPalette(hdc,ed->oed->hpal,1);
            
            RealizePalette(hdc);
            
            SetDIBitsToDevice(hdc,
                              -ed->scrollh,
                              -ed->scrollv,
                              128,
                              ed->size >> 1,
                              0,
                              0,
                              0,
                              ed->size >> 1,
                              ed->buf,
                              (BITMAPINFO*) &(ed->bmih),
                              DIB_PAL_COLORS);
            
            SelectPalette(hdc, oldpal, 1);
        }
        else
            SetDIBitsToDevice(hdc,-ed->scrollh,-ed->scrollv,128,ed->size>>1,0,0,0,ed->size>>1,ed->buf,(BITMAPINFO*)&(ed->bmih),DIB_RGB_COLORS);
        
        EndPaint(win,&ps);
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

LRESULT CALLBACK
blksel8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLOCKSEL8*ed;
    SCROLLINFO si;
    RECT rc;
    PAINTSTRUCT ps;
    int i,j,k,n;
    switch(msg) {
//  case WM_DESTROY:
//      ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
//      DeleteDC(ed->bufdc);
//      DeleteObject(ed->bufbmp);
//      break;
    case WM_SIZE:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=16;
        si.nMin=0;
        si.nMax=ed->ed->gfxtmp==0xff?0:63;
        SetScrollInfo(win,SB_VERT,&si,1);
        break;
    case WM_VSCROLL:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        j=i=ed->scroll;
        switch(wparam&65535) {
        case SB_BOTTOM:
            i=56;
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
            i+=8;
            break;
        case SB_PAGEUP:
            i-=8;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        }
        if(i<0) i=0;
        if(i>48) i=48;
        if(i==ed->scroll) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        si.nPos=i;
        SetScrollInfo(win,SB_VERT,&si,1);
        ScrollWindowEx(win,0,(j*ed->h>>4)-(i*ed->h>>4),0,0,0,0,SW_INVALIDATE|SW_ERASE);
        ed->scroll=i;
        i<<=4;
        j<<=4;
        
        if(j<i) j+=256,k=i+256; else k=j,j=i;
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->ed->hpal, 1);
            HPALETTE const oldpal2 = SelectPalette(ed->bufdc, ed->ed->hpal, 1);
            
            for( ; j < k; j++)
            {
                n = (j - i);
                
                Updateblk8sel(ed, n);
            }
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(ed->bufdc, oldpal2, 1);
        }
        
        break;
    
    case WM_RBUTTONDOWN:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        j=17;
        goto edit;
    case WM_LBUTTONDBLCLK:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        i=(ed->scroll<<4)+((lparam&65535)<<4)/ed->w+((((lparam>>16)<<4)/ed->h)<<4);
        if(ed->dfl==16) if(i<0x180) j=256+(i>>7); else break;
        else if(ed->dfl==8) if(i<0x100) j=259; else break;
        else if(ed->ed->gfxnum==0xff) j=0;
        else if(i<0x200) j=i>>6;
        else if(i<0x240) break;
        else if(i<0x280) j=10;
        else if(i<0x300) j=5+(i>>6);
        else j=(i>>6)-1;
        j|=(ed->flags&0x1c00)<<6;
        
        if(ed->sel>=0x200)
            j |= 0x80000;
        
    edit:
        
        if( Editblocks(ed->ed, j, win) )
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->ed->hpal, 1);
            
            HPALETTE const oldpal2 = SelectPalette(ed->bufdc, ed->ed->hpal, 1);
            
            for(i = 0; i < 256; i++)
                Updateblk8sel(ed,i);
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(ed->bufdc, oldpal2, 1);
            
            InvalidateRect(win, &rc, 0);
            
            SendMessage(GetParent(win), 4001, 0, 0);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(BLOCKSEL8*)GetWindowLong(win,GWL_USERDATA);
        SendMessage(GetParent(win),4000,(ed->scroll<<4)+((lparam&65535)<<4)/ed->w+((((lparam>>16)<<4)/ed->h)<<4)+ed->flags,0);
        break;
    case WM_PAINT:
        
        ed = (BLOCKSEL8*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        if(always)
        {
            HDC const hdc = BeginPaint(win, &ps);
            
            HPALETTE const oldpal = SelectPalette(hdc, ed->ed->hpal, 1);
            
            RealizePalette(hdc);
            
            k=(ed->scroll&15)*ed->h>>4;
            i=ed->h-k;
            if(i>ps.rcPaint.bottom) j=ps.rcPaint.bottom; else j=i;
            if(j>ps.rcPaint.top)
                BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,j-ps.rcPaint.top,ed->bufdc,ps.rcPaint.left,ps.rcPaint.top+k,SRCCOPY);
            if(i<ps.rcPaint.top) j=ps.rcPaint.top; else j=i;
            if(j<ps.rcPaint.bottom)
                BitBlt(hdc,ps.rcPaint.left,j,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-j,ed->bufdc,ps.rcPaint.left,j+k-ed->h,SRCCOPY);
            i=ed->sel-(ed->scroll<<4);
            if(i>=0 && i<256) {
                rc.left=(i&15)*ed->w>>4;
                rc.top=(i&240)*ed->h>>8;
                rc.right=rc.left+(ed->w>>4);
                rc.bottom=rc.top+(ed->h>>4);
                FrameRect(hdc,&rc,green_brush);
            }
            
            SelectPalette(hdc, oldpal, 1);
            EndPaint(win, &ps);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

void
Blksel16_OnPaint(BLOCKSEL16 const * const p_ed,
                 HWND               const p_win)
{
    unsigned char const * const rom = p_ed->ed->ew.doc->rom;
    
    int i = 0,
        j = 0;
    
    RECT rc;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->ed->hpal, 1);
    
    HGDIOBJ const oldbrush = SelectObject(hdc, white_pen);
    
    RealizePalette(hdc);
    
    rc = HM_GetClientRect(p_win);
    
    j = ( (ps.rcPaint.bottom + 31) >> 5) << 2;
    
    for(i = (ps.rcPaint.top >> 5) << 2; i < j; i++)
    {
        int const m = i + (p_ed->scroll << 2);
        
        int l = 0,
            n = 0,
            p = 0;
        
        RECT rc2;
        
        if(m > 0xea7)
            break;
        
        n = ((m & 3) << 1) + ((m & 0xffc) << 2);
        
        for(l = 0; l < 4; l++)
        {
            int k = 0;
            
            unsigned char const * const o =
            (
                rom + 0x78000
              + ( ( n + (l & 1) + ( (l & 2) << 2 ) ) << 3 )
            );
            
            for(k = 0; k < 4; k++)
            {
                Drawblock(p_ed->ed,
                          blkx[p],
                          blky[p],
                          ldle16b_i(o, k),
                          0);
                
                p++;
            }
        }
        
        if(m >= 0xea0)
            for(l = 512; l < 1024; l++)
                drawbuf[l] = 0;
        
        rc2.top = (i >> 2) << 5;
        rc2.left=(rc.right >> 1) - 64 + ((i & 3) << 5);
        
        Paintblocks(&(ps.rcPaint),
                    hdc,
                    rc2.left,
                    rc2.top,
                    (DUNGEDIT*) (p_ed->ed));
        // MoveToEx(hdc,rc2.left+31,rc2.top,0);
        // LineTo(hdc,rc2.left+31,rc2.top+31);
        // LineTo(hdc,rc2.left-1,rc2.top+31);
        // MoveToEx(hdc,rc2.left+15,rc2.top,0);
        // LineTo(hdc,rc2.left+15,rc2.top+31);
        // MoveToEx(hdc,rc2.left,rc2.top+15,0);
        // LineTo(hdc,rc2.left+31,rc2.top+15);
        
        if(m == ( (p_ed->sel & 7) >> 1) + ( (p_ed->sel & 0xfff0) >> 2 ) )
        {
            rc2.left += (p_ed->sel & 1) << 4;
            rc2.top += (p_ed->sel & 8) << 1;
            rc2.bottom = rc2.top + 16;
            rc2.right = rc2.left + 16;
            
            FrameRect(hdc,
                      &rc2,
                      green_brush);
        }
    }
    
    SelectObject(hdc, oldbrush);
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
blksel16proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLOCKSEL16* ed;
    SCROLLINFO si;
    RECT rc;
    int i,j;
    switch(msg) {
    case WM_SIZE:
        ed=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=lparam>>21;
        si.nMin=0;
        si.nMax=0xea;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->page=si.nPage;
        break;
    
    case WM_MOUSEWHEEL:
        
        ed=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
        i=ed->scroll;
        
        {
#if 0
            signed distance = HM_GetSignedHiword(wparam);
            
            unsigned flags = LOWORD(wparam);
            
            unsigned x_coord = LOWORD(lparam);
            unsigned y_coord = LOWORD(lparam);
            
            unsigned const is_horiz = (flags & MK_CONTROL);
            
            unsigned which_sb = (is_horiz) ? SB_HORZ : SB_VERT;
            
            int const which_scroll = (is_horiz) ? ed->mapscrollh : ed->mapscrollv;
            int const which_page   = (is_horiz) ? ed->mappageh : ed->mappagev;
            
            int * const which_return = (is_horiz) ? &ed->mapscrollh : &ed->mapscrollv;
            
            (*which_return) = Handlescroll(win,
                                           (distance > 0) ? 0 : 1,
                                           which_scroll,
                                           which_page,
                                           which_sb,
                                           ed->mapsize ? 32 : 16, 32);
#endif
        }
        
        break;
    
    case WM_VSCROLL:
        ed=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
        i=ed->scroll;
        switch(wparam&65535) {
        case SB_BOTTOM:
            i=0x200 - ed->page;
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
            i+=ed->page;
            break;
        case SB_PAGEUP:
            i-=ed->page;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        }
        if(i<0) i=0;
        if(i>0xea - ed->page) i = 0xea - ed->page;
        if(i==ed->scroll) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS;
        si.nPos=i;
        SetScrollInfo(win,SB_VERT,&si,1);
        ScrollWindowEx(win,0,(ed->scroll-i)<<5,0,0,0,0,SW_INVALIDATE|SW_ERASE);
        ed->scroll=i;
        break;
    case WM_LBUTTONDOWN:
        ed=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
        rc = HM_GetClientRect(win);
        
        i = (rc.right >> 1) - 64;
        
        j = lparam & 65535;
        
        if(j < i || j >= rc.right - i)
            break;
        
        SendMessage(GetParent(win),
                    4000,
                    ( (ed->scroll << 4) + ( (lparam >> 20) << 3) )
                  + ( (j - i) >> 4),
                    0);
        
        break;
    
    case WM_LBUTTONDBLCLK:
        ed=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
        if(ShowDialog(hinstance,(LPSTR)IDD_DIALOG8,win,editblock16,GetWindowLong(win,GWL_USERDATA)))
            SendMessage(GetParent(win),4001,0,0);
        break;
    
    case WM_PAINT:
        
        ed = (BLOCKSEL16*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            Blksel16_OnPaint(ed, win);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

BOOL CALLBACK editblock32(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT*oe;
    BLOCKEDIT32*ed;
    BLOCKSEL16*bs;
    HWND hc,hc2;
    HDC hdc;
    
    RECT rc;
    FDOC*doc;
    int i,j;
    short*l;
    unsigned char*rom;
    switch(msg) {
    case WM_QUERYNEWPALETTE:
        ed=(BLOCKEDIT32*)GetWindowLong(win,GWL_USERDATA);
        Setpalette(win,ed->bs.ed->hpal);
        return 1;
    case WM_PALETTECHANGED:
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        break;
    case WM_INITDIALOG:
        ed = (BLOCKEDIT32*) malloc(sizeof(BLOCKEDIT32));
        bs=&(ed->bs);
        oe=bs->ed=(OVEREDIT*)lparam;
        Getblock32(oe->ew.doc->rom,oe->selblk,ed->blks);
        bs->scroll=0;
        bs->sel=0;
        hc=GetDlgItem(win,IDC_CUSTOM2);
        SetWindowLong(win,GWL_USERDATA,(int)ed);
        SetWindowLong(hc,GWL_USERDATA,(int)ed);
        Updatesize(hc);
        hc=GetDlgItem(win,IDC_CUSTOM1);
        rc = HM_GetClientRect(hc);
        ed->w=rc.right;
        ed->h=rc.bottom;
        hdc=GetDC(win);
        ed->bufdc=CreateCompatibleDC(hdc);
        ed->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        ReleaseDC(win,hdc);
        SelectObject(ed->bufdc,ed->bufbmp);
        SelectPalette(ed->bufdc,ed->bs.ed->hpal,1);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->bs.ed->hpal, 1);
            
            for(i = 0; i < 4; i++)
                Updateblk32disp(ed,i);
            
            SelectPalette(objdc, oldpal, 1);
        }
        
        SetWindowLong(hc, GWL_USERDATA, (int) ed);
        
        wparam = 0;
    
    // \task Is the fallthrough here intentional?
    
    case 4000:
        SetDlgItemInt(win,IDC_EDIT1,wparam,0);
        break;
    case WM_DESTROY:
        ed=(BLOCKEDIT32*)GetWindowLong(win,GWL_USERDATA);
        DeleteDC(ed->bufdc);
        DeleteObject(ed->bufbmp);
        free(ed);
        break;
    
    case 4001:
        
        ed = (BLOCKEDIT32*) GetWindowLong(win, GWL_USERDATA);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->bs.ed->hpal, 1);
            
            for(i = 0; i < 4; i++)
                Updateblk32disp(ed, i);
            
            SelectPalette(objdc, oldpal, 1);
        }
        
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
        InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
        
        break;
    
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT1|(EN_CHANGE<<16):
            bs=(BLOCKSEL16*)GetWindowLong(win,GWL_USERDATA);
            SetBS16(bs,GetDlgItemInt(win,IDC_EDIT1,0,0),GetDlgItem(win,IDC_CUSTOM2));
            break;
        case IDOK:
            ed=(BLOCKEDIT32*)GetWindowLong(win,GWL_USERDATA);
            oe=ed->bs.ed;
            doc=oe->ew.doc;
            rom=doc->rom;
            i=oe->selblk;
            j=(i>>2)*6;
            l=ed->blks;
            switch(i&3) {
            case 0:
                rom[0x18000 + j] = (l[0] & 0xff);
                rom[0x18004 + j] = (rom[0x18004 + j] & 15) + ((l[0] >> 4) & 240);
                rom[0x1b400 + j] = (l[1] & 0xff);
                rom[0x1b404 + j] = (rom[0x1b404 + j]&15)+((l[1]>>4)&240);
                rom[0x20000 + j] = (l[2] & 0xff);
                rom[0x20004 + j] = (rom[0x20004 + j]&15)+((l[2]>>4)&240);
                rom[0x23400 + j] = (l[3] & 0xff);
                rom[0x23404 + j] = (rom[0x23404 + j]&15)+((l[3]>>4)&240);
                break;
            case 1:
                rom[0x18001 + j] = (l[0] & 0xff);
                rom[0x18004 + j] = (rom[0x18004 + j]&240)+(l[0]>>8);
                rom[0x1b401 + j] = (l[1] & 0xff);
                rom[0x1b404 + j] = (rom[0x1b404 + j]&240)+(l[1]>>8);
                rom[0x20001 + j] = (l[2] & 0xff);
                rom[0x20004 + j] = (rom[0x20004 + j]&240)+(l[2]>>8);
                rom[0x23401 + j] = (l[3] & 0xff);
                rom[0x23404 + j] = (rom[0x23404 + j]&240)+(l[3]>>8);
                break;
            case 2:
                rom[0x18002 + j] = (l[0] & 0xff);
                rom[0x18005 + j] = (rom[0x18005 + j]&15)+((l[0]>>4)&240);
                rom[0x1b402 + j] = (l[1] & 0xff);
                rom[0x1b405 + j] = (rom[0x1b405 + j]&15)+((l[1]>>4)&240);
                rom[0x20002 + j] = (l[2] & 0xff);
                rom[0x20005 + j] = (rom[0x20005 + j]&15)+((l[2]>>4)&240);
                rom[0x23402 + j] = (l[3] & 0xff);
                rom[0x23405 + j] = (rom[0x23405 +j]&15)+((l[3]>>4)&240);
                break; 
            case 3:
                rom[0x18003 + j] = (l[0] & 0xff);
                rom[0x18005 + j] = (rom[0x18005 + j]&240)+(l[0]>>8);
                rom[0x1b403 + j] = (l[1] & 0xff);
                rom[0x1b405 + j] = (rom[0x1b405 + j] & 240) + (l[1] >> 8);
                rom[0x20003 + j] = (l[2] & 0xff);
                rom[0x20005 + j] = (rom[0x20005 + j] & 240) + (l[2] >> 8);
                rom[0x23403 + j] = (l[3] & 0xff);
                rom[0x23405 + j] = (rom[0x23405 + j] & 240) + (l[3] >> 8);
                break;
            }
            
            for(i=0;i<160;i++)
            {
                hc2 = doc->overworld[i].win;
                
                if(hc2 != 0)
                {
                    hc2=GetDlgItem(hc2,2000);
                    hc=GetDlgItem(hc2,3000);
                    InvalidateRect(hc,0,0);
                    hc=GetDlgItem(hc2,3001);
                    InvalidateRect(hc,0,0);
                }
            }
            
            doc->modf=1;
        
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

LRESULT CALLBACK
blksel32proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    int i,j,m;
    OVEREDIT*ed;
    RECT rc,rc2;
    HBRUSH oldbrush;
    HPALETTE oldpal;
    SCROLLINFO si;
    switch(msg)
    {
    
    case WM_SIZE:
        
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nPage=lparam>>21;
        si.nMin=0;
        
        si.nMax = ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa;
        
        SetScrollInfo(win,SB_VERT,&si,1);
        
        ed->sel_page=si.nPage;
        
        ed->sel_scroll = Handlescroll(win,
                                      -1,
                                      ed->sel_scroll,
                                      ed->sel_page,
                                      SB_VERT,
                                      ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        
        break;
    
    case WM_MOUSEWHEEL:
        
        {
            HM_MouseWheelData const d = HM_GetMouseWheelData(wparam, lparam);
            
            unsigned scroll_type = SB_LINEUP;
            
            WPARAM fake_wp;
            
            ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
            
            if(d.m_distance > 0)
            {
                // wheel moving up or left
                if(d.m_control_key)
                {
                    scroll_type = SB_PAGEUP;
                }
                else
                {
                    scroll_type = SB_LINEUP;
                }
            }
            else
            {
                if(d.m_control_key)
                {
                    scroll_type = SB_PAGEDOWN;
                }
                else
                {
                    scroll_type = SB_LINEDOWN;
                }
            }
            
            fake_wp =  MAKEWPARAM(scroll_type, 0);
            
            ed->sel_scroll = Handlescroll(win,
                                          fake_wp,
                                          ed->sel_scroll,
                                          ed->sel_page,
                                          SB_VERT,
                                          ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        }
        
        break;
    
    case WM_VSCROLL:
        
        ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
        
        ed->sel_scroll = Handlescroll(win,
                                      wparam,
                                      ed->sel_scroll,
                                      ed->sel_page,
                                      SB_VERT,
                                      ed->schflag ? ( (ed->schnum + 3) >> 2) : 0x8aa, 32);
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        rc = HM_GetClientRect(win);
        i=(rc.right>>1)-64;
        j=lparam&65535;
        
        if(j < i || j >= rc.right - i)
            break;
        
        m = ( (ed->sel_scroll + (lparam >> 21) ) << 2) + ( (j - i) >> 5);
        
        if(m<0 || m>=(ed->schflag?ed->schnum:0x22a8)) break;
        if(ed->schflag) m=ed->schbuf[m];
        SetDlgItemInt(ed->dlg,3005,m,0);
        break;
    case WM_LBUTTONDBLCLK:
        ShowDialog(hinstance,(LPSTR)IDD_DIALOG7,framewnd,editblock32,GetWindowLong(win,GWL_USERDATA));
        break;
    case WM_PAINT:
        
        ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed)
            break;
        
        hdc = BeginPaint(win, &ps);
        
        oldpal = SelectPalette(hdc, ed->hpal, 1);
        
        RealizePalette(hdc);
        rc = HM_GetClientRect(win);
        
        j = ((ps.rcPaint.bottom + 31) >> 5) << 2;
        
        oldbrush=SelectObject(hdc,white_pen);
        
        for(i = (ps.rcPaint.top >> 5) << 2; i < j; i++)
        {
            m = i + (ed->sel_scroll << 2);
            
            if(m >= (ed->schflag ? ed->schnum : 0x22a8))
                break;
            
            rc2.top = (i >> 2) << 5;
            rc2.left = (rc.right >> 1) - 64 + ((i & 3) << 5);
            
            if(rc2.top < ps.rcPaint.top - 31 || rc2.left < ps.rcPaint.left - 31 || rc2.left >= ps.rcPaint.right || rc2.top >= ps.rcPaint.bottom)
                continue;
            
            Drawblock32(ed,ed->schflag?ed->schbuf[m]:m,0);
            Paintblocks(&(ps.rcPaint),hdc,rc2.left,rc2.top,(DUNGEDIT*)ed);
            
            if(m == ed->sel_select)
            {
                rc2.bottom = rc2.top + 32;
                rc2.right = rc2.left+32;
                FrameRect(hdc, &rc2, green_brush);
            }
            else if(ed->disp & 2)
            {
                MoveToEx(hdc, rc2.left + 31, rc2.top, 0);
                LineTo(hdc, rc2.left + 31, rc2.top + 31);
                LineTo(hdc, rc2.left - 1, rc2.top + 31);
            }
        }
        
        SelectObject(hdc,oldbrush);
        SelectPalette(hdc,oldpal,1);
        
        EndPaint(win,&ps);
        
        break;
    
    default:
        
        return DefWindowProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

// =============================================================================

BOOL CALLBACK
aboutfunc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    if(msg == WM_COMMAND && wparam == IDCANCEL)
        EndDialog(win,0);
    
    return FALSE;
}

// =============================================================================

void
Updatesprites(void)
{
    FDOC *doc;
    HWND win;
    OVEREDIT*oe;
    DUNGEDIT*ed;
    RECT rc;
    int i,j,k,m;
    unsigned char*l;
    for(doc=firstdoc;doc;doc=doc->next) {
        for(i=0;i<144;i++) {
            win=doc->overworld[i].win;
            if(win)
            {
                text_buf_ty text_buf;
                
                win=GetDlgItem(GetDlgItem(win,2000),3001);
                oe=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
                
                if( ! (oe->disp & SD_OverShowMarkers) )
                    continue;
                
                k=oe->esize[oe->sprset];
                l=oe->ebuf[oe->sprset];
                
                for(j=0;j<k;j++)
                {
                    rc.left=(l[j+1]<<4)-(oe->mapscrollh<<5);
                    rc.top=(l[j]<<4)-(oe->mapscrollv<<5);
                    
                    GetOverString(oe, 5, j, text_buf);
                    
                    Getstringobjsize(text_buf, &rc);
                    
                    InvalidateRect(win,&rc,0);
                }
                
                if(i < 128)
                {
                    k=oe->ssize;
                    l=oe->sbuf;
                    
                    for(j=0;j<k;j++)
                    {
                        m=*(short*)(l+j);
                        
                        rc.left=((m&0x7e)<<3)-(oe->mapscrollh<<5);
                        rc.top=((m&0x1f80)>>3)-(oe->mapscrollv<<5);
                        
                        GetOverString(oe, 10, j, text_buf);
                        
                        Getstringobjsize(text_buf, &rc);
                        
                        InvalidateRect(win,&rc,0);
                    }
                }
            }
        }
        for(i=0;i<0xa8;i++) {
            win=doc->ents[i];
            if(win) {
                win=GetDlgItem(GetDlgItem(win,2000), ID_DungEditWindow);
                ed=(DUNGEDIT*)GetWindowLong(win,GWL_USERDATA);
                
                if( ! (ed->disp & SD_OverShowMarkers) )
                    break;
                
                k=ed->esize;
                l=ed->ebuf;
                
                for(j=1;j<k;j++) {
                    rc.left=((l[j+1]&31)<<4)-(ed->mapscrollh<<5);
                    rc.top=((l[j]&31)<<4)-(ed->mapscrollv<<5);
                    Getstringobjsize(Getsprstring(ed,j),&rc);
                    InvalidateRect(win,&rc,0);
                }
                k=ed->ssize;
                l=ed->sbuf;
                for(j=0;j<k;j++) {
                    m=*(short*)(l+j);
                    rc.left=((m&0x7e)<<2)-(ed->mapscrollh<<5);
                    rc.top=((m&0x1f80)>>4)-(ed->mapscrollv<<5);
                    Getstringobjsize(Getsecretstring(doc->rom,l[j+2]),&rc);
                    InvalidateRect(win,&rc,0);
                }
            }
        }
    }
}

// =============================================================================

BOOL CALLBACK
trackdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT*ed;
    FDOC*doc;
    SCMD*sc;
    short*l;
    int i,j,k,m,n,o;
    HWND hc;
    
    (void) wparam;
    
    // -----------------------------
    
    switch(msg)
    {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(TRACKEDIT*)lparam;
        ed->dlg=win;
        doc=ed->ew.doc;
        sc=doc->scmd;
        i=0;
        k=0;
        l=0;
        m=ed->ew.param&65535;
        o=ed->ew.param>>16;
        ed->ew.param=m;
        n=-1;
        for(j=doc->sr[m].first;j!=-1;j=sc[j].next) {
            if(i==k) k+=64,l=realloc(l,k*sizeof(SCMD));
            if(j==o) n=i;
            if(doc==mark_doc && m==mark_sr) {
                if(j==mark_first) mark_start=i;
                if(j==mark_last) mark_end=i;
            }
            l[i++]=j;
        }
        ed->debugflag=0;
        ed->tbl=l;
        ed->len=i;
        ed->sel=n;
        ed->csel=-1;
        ed->withcapt=0;
        SetDlgItemInt(win,3001,doc->sr[m].bank,0);
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,lparam);
        Updatesize(hc);
        break;
    case 3001|(EN_CHANGE<<16):
        ed=(TRACKEDIT*)GetWindowLong(win,DWL_USER);
        ed->ew.doc->sr[ed->ew.param].bank=GetDlgItemInt(win,3001,0,0);
        break;
    }
    return FALSE;
}

SD_ENTRY over_sd[] = 
{
    {"BLKSEL32","",152,70,0,0,3000,WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,11},
    {"OVERWORLD","",0,92,160,0,3001,WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Draw",0,0,60,20,3002,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_GROUP|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Select",64,0,60,20,3003,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Rectangle",128,0,60,20,3004,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Frame 1",192,0,60,20,3008,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Entrance",64,72,60,20,3013,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Copy",408,48,40,20,3021,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Paste",448,48,40,20,3022,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Warp Swch",335,0,60,20,3028,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Undo",0,48,60,20,3014,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Addr.Calc",64,48,60,20,3015,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Sprite",320,72,60,20,3016,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Background",128,48,80,20,3011,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Grid",208,48,50,20, SD_OverGrid32CheckBox, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Markers",258,48,70,20,3019,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Overlay",328,48,70,20, SD_OverOverlayChkBox, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Exit",0,72,60,20,3023,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Properties",256,0,60,20,3024,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Hole",128,72,60,20,3025,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Transport",192,72,60,20,3026,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Item",256,72,60,20,3027,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","",400,0,20,20,3029,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",425,0,20,20,3030,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",450,0,20,20, SD_OverUpArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",475,0,20,20, SD_OverDownArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",329,24,70,80,3020,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"EDIT","0",56,0,0,20,3005,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,3},
    {"STATIC","GFX#:",0,24,40,20,3006,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",50,24,30,20,3007,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Palette:",90,24,40,20,3009,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",130,24,30,20,3010,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr GFX#:",165,24,55,20,3017,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",220,24,30,20,3018,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr pal:",255,24,45,20, SD_OverSprTileSetStatic, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",300,24,25,20, SD_OverSprTileSetEditCtl, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Search",56,20,0,20, SD_OverMapSearchBtn, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BUTTON","Search2",56,42,0,20, SD_OverAdjustSearchBtn, WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BLKSEL16","",152,70,0,0, SD_OverMap16_Selector, WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,11},
    
    // For testing window focus
    {"STATIC", "Window Focus: ",
     250, 20,
     0, 20,
     SD_OverWindowFocus,
     WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
     0,
     3},
};

SD_ENTRY dung_sd[]={
    {"STATIC","",0,0,60,20, ID_DungRoomNumber, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"STATIC","Room:",4,72,40,52, ID_DungStatic1, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",54,72,30,52, ID_DungEntrRoomNumber, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Y:",4,48,15,28, ID_DungStatic2, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",29,48,40,28, ID_DungEntrYCoord, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","X:",4,24,15,4, ID_DungStatic3, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",29,24,40,4, ID_DungEntrXCoord, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Y scroll:",79,48,40,28, ID_DungStatic4, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",129,48,40,28, ID_DungEntrYScroll, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","X scroll:",79,24,40,4, ID_DungStatic5, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",129,24,40,4, ID_DungEntrXScroll, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","CX:",173,24,20,4, ID_DungStatic17, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",193,24,40,4, ID_DungEntrCameraX, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","CY:",239,24,20,4, ID_DungStatic18, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",259,24,40,4, ID_DungEntrCameraY, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"BUTTON","More",301,24,36,4, ID_DungEntrProps, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"STATIC","Blockset:",104,72,45,52, ID_DungStatic8, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"NumEdit","",154,72,40,52, ID_DungEntrTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Music:",204,72,40,52, ID_DungStatic9, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"COMBOBOX","",254,72,80,-44, ID_DungEntrSong, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,12},
    {"STATIC","Dungeon:",204,48,48,28, ID_DungStatic10, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"COMBOBOX","",254,48,80,-88, ID_DungEntrPalaceID, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,12},
    {"DUNGEON","",0,50,0,90, ID_DungEditWindow, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,10},
    {"STATIC","",344,86,74,4, ID_DungDetailText, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","",60,0,20,20, ID_DungLeftArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",85,0,20,20, ID_DungRightArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",110,0,20,20, ID_DungUpArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",135,0,20,20, ID_DungDownArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Jump",160,0,40,20, ID_DungChangeRoom, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"STATIC","Floor 1:",204,0,55,20, ID_DungStatic6, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",264,0,30,20, ID_DungFloor1, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Floor 2:",204,24,55,20, ID_DungStatic7, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",264,24,30,20, ID_DungFloor2, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Blockset:",298,0,55,20, ID_DungStatic13, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",360,0,30,20, ID_DungTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","EnemyBlk:",395,0,55,20, ID_DungStatic16, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",450,0,30,20, ID_DungSprTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Palette:",298,24,55,20, ID_DungStatic14, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",360,24,30,20, ID_DungPalette, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Collision:",395,24,50,20, ID_DungStatic15, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",450,24,90,120, ID_DungCollSettings, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"BUTTON","Exit",550,24,40,24, ID_DungExit, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"STATIC","Layout:",0,24,40,20, ID_DungStatic11, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","More",490,0,30,24, ID_DungEditHeader, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"NumEdit","",40,24,30,20, ID_DungLayout, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","BG2:",75,24,30,20, ID_DungStatic12, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",105,24,95,100, ID_DungBG2Settings, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"BUTTON","Starting location",0,86,340,0, ID_DungStartLocGroupBox, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|BS_GROUPBOX,0,12},
    {"BUTTON","BG1",430,48,40,36, ID_DungShowBG1, BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","BG2",430,34,40,22, ID_DungShowBG2, BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Spr",430,20,40,8, ID_DungShowSprites, BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Frm1",430,72,40,52, ID_DungAnimateButton, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Display",425,86,50,0, ID_DungDispGroupBox, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|BS_GROUPBOX,0,12},
    {"BUTTON","1",485,64,40,52, ID_DungObjLayer1, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_GROUP,0,12},
    {"BUTTON","2",485,48,40,36, ID_DungObjLayer2, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","3",485,32,40,20, ID_DungObjLayer3, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Sprite",485,16,50,4, ID_DungSprLayer, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Item",525,64,40,52, ID_DungItemLayer, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Block",525,48,50,36, ID_DungBlockLayer, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Torch",525,32,50,20, ID_DungTorchLayer, BS_AUTORADIOBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"BUTTON","Edit",480,86,100,0, ID_DungEditGroupBox, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|BS_GROUPBOX,0,12},
    {"BUTTON","Sort spr",524,0,60,20, ID_DungSortSprites, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0}
};
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

SD_ENTRY samp_sd[]={
    {"STATIC","Edit:",0,48,60,20, ID_Samp_SampleIndexStatic, WS_VISIBLE|WS_CHILD,0,0},
    {"NumEdit","",64,48,40,20, ID_Samp_SampleIndexEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"SAMPEDIT","",0,96,0,0, ID_Samp_Display, WS_VISIBLE|WS_CHILD|WS_HSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Copy of:",0,72,60,20, ID_Samp_SampleIsCopyCheckBox, WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"NumEdit","",64,72,40,20, ID_Samp_SampleCopyOfIndexEdit, WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Copy",110,48,50,20, ID_Samp_CopyToClipboardButton, WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Paste",110,72,50,20, ID_Samp_PasteFromClipboardButton, WS_VISIBLE|WS_CHILD,0,0},
    {"STATIC","Length:",170,48,60,20,3007,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",234,48,60,20,3008,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Loop:",170,72,60,20,3009,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"EDIT","",234,72,40,20,3010,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Ed.Inst:",0,0,40,20,3011,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",44,0,40,20,3012,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Freq:",90,0,40,20,3013,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",134,0,40,20,3014,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","ADSR:",180,0,40,20,3015,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",224,0,40,20,3016,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Gain:",0,24,40,20,3017,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",44,24,40,20,3018,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"STATIC","Samp:",90,24,40,20,3019,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",134,24,40,20,3020,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Play",180,24,36,20,3021,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Stop",220,24,36,20,3022,WS_VISIBLE|WS_CHILD,0,0}
};

SD_ENTRY patch_sd[]={
    {"LISTBOX","",0,20,100,70,3000,WS_VISIBLE|WS_CHILD|LBS_NOTIFY|WS_VSCROLL,WS_EX_CLIENTEDGE,10},
    {"STATIC","Loaded modules:",0,0,0,0,3001,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Add",80,20,0,20,3002,WS_VISIBLE|WS_CHILD,0,3},
    {"BUTTON","Remove",80,44,0,20,3003,WS_VISIBLE|WS_CHILD,0,3},
    {"BUTTON","Build",80,68,0,20,3004,WS_VISIBLE|WS_CHILD,0,3}
};

// =============================================================================

SUPERDLG sampdlg={
    "",sampdlgproc,WS_CHILD|WS_VISIBLE,300,100,23,samp_sd
};

SUPERDLG dungdlg = {
    "",dungdlgproc,WS_CHILD|WS_VISIBLE, 600, 200, ID_DungNumControls, dung_sd
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

SUPERDLG patchdlg={
    "",patchdlgproc,WS_CHILD|WS_VISIBLE,200,200,5,patch_sd
};

SD_ENTRY z3_sd[]={
    {WC_TREEVIEW,"",0,0,0,0,3000,WS_VISIBLE|WS_TABSTOP|WS_BORDER|WS_CHILD|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_HASLINES|TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP,WS_EX_CLIENTEDGE,10},
};

SUPERDLG z3_dlg={
    "",z3dlgproc,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,60,60,1,z3_sd
};

// =============================================================================

LRESULT CALLBACK
lmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    LMAPEDIT *ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->modf) {
            wsprintf(buffer,"Confirm modification of %s palace?",level_str[ed->ew.param+1]);
            switch(MessageBox(framewnd,buffer,"Level map editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savedungmap(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        if((HWND)lparam!=win) break;
        ed=((LMAPEDIT*)GetWindowLong(win,GWL_USERDATA));
        activedoc=ed->ew.doc;
        Setdispwin((DUNGEDIT*)ed);
        break;
    case WM_GETMINMAXINFO:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(LMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        ed=(LMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&lmapdlg,win,0,0,256,256,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

LRESULT CALLBACK
perspproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PERSPEDIT*ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->modf) {
            switch(MessageBox(framewnd,"Confirm modification of 3D objects?","3D object editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savepersp(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        activedoc=((PERSPEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(PERSPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->perspwnd=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(PERSPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&perspdlg,win,0,0,100,100,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

// =============================================================================

LRESULT CALLBACK
trackeditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT*ed;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        activedoc=((TRACKEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(TRACKEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->sr[ed->ew.param].editor=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(TRACKEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->ew.doc->sr[ed->ew.param&65535].editor=win;
        CreateSuperDialog(&trackdlg,win,0,0,0,0,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

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

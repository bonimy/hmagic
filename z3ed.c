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
*/

#include "structs.h"
#include "Callbacks.h"

#include "DungeonEnum.h"

// =============================================================================

short dm_x;
short dm_k;

unsigned short copy_w,copy_h;

unsigned short * copybuf = 0;

uint8_t const u8_neg1 = (uint8_t) (~0);

static HGDIOBJ gray_pen = 0;

HPEN tint_pen = 0;
HBRUSH tint_br = 0;

uint16_t const u16_neg1 = (uint16_t) (~0);

uint32_t const u32_neg1 = (uint32_t) (~0);

int const always = 1;

int mouse_x, mouse_y;

#if 1


#include "stdio.h"

    void
    quick_u16_signedness_test(void)
    {
        uint16_t i = 0;
        
        // unsigned
        uint16_t a_u = 0xffff;
        uint16_t a_n = ~a_u;
        
        // indeterminate? standard probably indicates signed though
        short a_std = 0xffff;
        
        // signed
        int16_t a_s = 0xffff;

        unsigned int b_u = a_std;
        
        int b_s = a_std;

        (void) a_s;
        if( ~a_u == 0 )
        {
            printf("unexpected");
        }

        if( ~a_u == -1 )
        {
            printf("unexpected");
        }
        
        if( a_n == 0 )
        {
            printf("unexpected");
        }


        if(a_u == ~0)
        {
            printf("unexpected");
        }

        if(a_u == -1)
        {
            printf("unexpected");
        }
        
        if(a_u == ~0)
        {
            printf("unexpected");
        }
        
        if( (a_u + 1) != 0 )
        {
            uint16_t c_u = (a_u + 1);
            
            if(c_u != 0)
            {
                printf("unexpected");
            }
            
        }

        if( (a_u + 1) != 0 )
        {
            printf("unexpected");
        }

        if(b_s != -1)
        {
            printf("probably happening");
        }
        
        if(b_s != ~0)
        {
            printf("probably happening");
        }

        if(b_u == -1)
        {
            printf("probably happening");
        }
        
        if(b_u == ~0)
        {
            printf("probably happening");
        }
        
        for(i = 0;  ; i += 1)
        {
            uint16_t j = ~i;
            
            if( (i + j) == i )
            {
                printf("hrm... %d", i);
            }
            
            if(i == 0xffff)
            {
                break;
            }
        }
    }

#endif


enum
{
    CFG_NOFLAGS         = 0,
    CFG_SPRNAMES_LOADED = 1 << 0,
    CFG_MRU_LOADED      = 1 << 1,
    CFG_SNDVOL_LOADED   = 1 << 2,
    CFG_MIDI_LOADED     = 1 << 3,
    CFG_ASM_LOADED      = 1 << 4
};

enum
{
    ID_MRU1 = 5000,
    ID_MRU2,
    ID_MRU3,
    ID_MRU4
};

int cfg_modf = 0;

int cfg_flag = CFG_NOFLAGS;

// =============================================================================

    // Looking to call out IDs by name to hide when editing overlays
    // rather than by their offset from 3000 <__<
    unsigned const overlay_hide[] =
    {
        ID_DungRoomNumber,
        ID_DungStatic1,
        ID_DungEntrRoomNumber,
        ID_DungStatic2,
        ID_DungEntrYCoord,
        ID_DungStatic3,
        ID_DungEntrXCoord,
        ID_DungStatic4,
        ID_DungEntrYScroll,
        ID_DungStatic5,
        
        ID_DungEntrXScroll,
        ID_DungStartLocGroupBox,
        ID_DungLeftArrow,
        ID_DungRightArrow,
        ID_DungUpArrow,
        ID_DungDownArrow,
        ID_DungStatic7,
        ID_DungFloor2,
        ID_DungStatic8,
        ID_DungEntrTileSet,
        
        ID_DungStatic9,
        ID_DungEntrSong,
        ID_DungStatic10,
        ID_DungEntrPalaceID,
        ID_DungStatic11,
        ID_DungLayout,
        ID_DungObjLayer1,
        ID_DungObjLayer2,
        ID_DungObjLayer3,
        ID_DungEditGroupBox,
        
        ID_DungStatic12,
        ID_DungBG2Settings,
        ID_DungStatic15,
        ID_DungSprTileSet,
        ID_DungCollSettings,
        ID_DungSprLayer,
        ID_DungChangeRoom,
        ID_DungShowSprites,
        ID_DungStatic16,
        ID_DungStatic17,
        
        ID_DungEntrCameraX,
        ID_DungStatic18,
        ID_DungEntrCameraY,
        ID_DungEntrProps,
        ID_DungEditHeader,
        ID_DungItemLayer,
        ID_DungBlockLayer,
        ID_DungTorchLayer,
        ID_DungSortSprites,
        ID_DungExit
    };

    enum
    {
        ID_DungOverlayHideCount = sizeof(overlay_hide) / sizeof(unsigned),
    };

// =============================================================================

    enum
    {
        ID_TextFirst = 3000,
        
        ID_TextEntriesListControl = ID_TextFirst,
        ID_TextEditWindow,
        ID_TextSetTextButton,
        ID_TextEditTextRadio,
        ID_TextEditDictionaryRadio,
        
        ID_TextAfterLast,
        ID_TextNumControls = (ID_TextAfterLast - ID_TextFirst)
    
    };

// =============================================================================

    enum
    {
        SD_GenericFirst = 3000,
        SD_OverFirst = 3000,
        
        SD_OverGrid32CheckBox = 3012,
        
        // \task Fill in others.
        
        SD_OverWindowFocus = 3039,
        SD_OverAfterLast,
        
        SD_OverNumControls = (SD_OverAfterLast - SD_OverFirst),
    };

// =============================================================================

// Sound volume level.
uint16_t soundvol = 180;

// windows version number
int wver;

int palmode=0;
int mruload=0;
int mark_sr;
int mark_start,mark_end,mark_first,mark_last;


int door_ofs, issplit;

char namebuf[MAX_PATH] = "";

char vererror_str[] = "This file was edited with a newer version of Gigasoft Hyrule Magic. You need version %d.%00d or higher to open it.";
char *mrulist[4] = { 0, 0, 0, 0 };
char currdir[MAX_PATH];

char buffer[0x400];

unsigned char drawbuf[0x400];

unsigned char map16ofs[] = {0, 1, 64, 65, 0, 1, 32, 33};

uint16_t *dm_rd;
uint16_t *dm_wr;
uint16_t *dm_buf;
uint16_t *dm_tmp;

unsigned char dm_l, dm_dl;

// "current secret"?
// Doesn't actually appear to be used, except to calculate the size of the
// boxes for droppable items (the red dots).
char *cur_sec;

static unsigned char masktab[16] = {255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

const static int sprset_loc[3] = {0x4c881,0x4c901,0x4ca21};
int palhalf[8] = {1,0,0,1,1,1,0,0};

const static int map_ofs[5] = {0,16,512,528};
const static int map_ind[4] = {0,1,8,9};

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

unsigned short ovlblk[4];

HPEN green_pen,
     null_pen,
     white_pen,
     blue_pen;

HBRUSH black_brush,
       white_brush,
       green_brush,
       yellow_brush,
       purple_brush,
       red_brush,
       blue_brush,
       gray_brush;

HGDIOBJ trk_font;

HCURSOR normal_cursor,
        forbid_cursor,
        wait_cursor;

HBITMAP arrows_imgs[4];

HANDLE shade_brush[8];
HANDLE sizecsor[5];

// The handle to the program
HINSTANCE hinstance;

HWND framewnd, clientwnd;

RGBQUAD greencolor={72,152,72,0};
RGBQUAD darkcolor={80,136,144,0};
RGBQUAD deathcolor={96,96,48,0};
RGBQUAD blackcolor={0,0,0,0};

HMENU filemenu;

BITMAPINFOHEADER zbmih={sizeof(BITMAPINFOHEADER),32,32,1,8,BI_RGB,0,0,0,256,0};

SDCREATE *firstdlg = 0, *lastdlg = 0;

FDOC *mark_doc = 0;
FDOC *firstdoc = 0, *lastdoc = 0, *activedoc = 0;

DUNGEDIT * dispwnd = 0;

DPCEDIT * dpce = 0;

OVEREDIT * oved = 0;

SSBLOCK **ssblt = 0;

// =============================================================================

// this calculates rom addresses from (lo-rom) cpu addresses
int romaddr(int const addr)
{
    int ret = 0;
    
    if( !(addr & 0x8000) )
    {
        // e.g. 0x44444 -> 0x00000 -> return 0;
        ret = 0;
    }
    else
    {
        // e.g. 0x289AB -> 0x109AB
        ret = ( (addr & 0x3f0000) >> 1) | (addr & 0x7fff );
    }
    
    return ret;
}

// =============================================================================

/// Converts a cpu address to a rom address by combining a bank byte and
/// a 16-bit address within that bank.
uint32_t
rom_addr_split(uint8_t  const p_bank,
               uint16_t const p_addr)
{
    BOOL const even_bank = ( (p_bank % 2) == 0 );
    
    uint32_t const upper = (p_bank >> 1) << 16;
    
    uint32_t const lower = (even_bank) ? p_addr - 0x8000
                                       : p_addr;
    
    // -----------------------------
    
    if(p_addr < 0x8000)
    {
        return 0;
    }
    
    return (upper | lower); 
}

// =============================================================================

// this calculates (lo-rom) cpu addresses from rom addreses.
int
cpuaddr(int addr)
{
    return ((addr&0x1f8000)<<1) | (addr&0x7fff) | 0x8000;
}

// =============================================================================

void*
recalloc(void   const * const p_old_buf,
         size_t         const p_new_count,
         size_t         const p_old_count,
         size_t         const p_element_size)
{
    void * const new_buf = calloc(p_new_count, p_element_size);
    
    if(new_buf)
    {
        // Copy using the smaller of the two count values.
        size_t const limit_count = p_old_count > p_new_count ? p_new_count
                                                             : p_old_count;
        
        memcpy(new_buf, p_old_buf, (limit_count * p_element_size) );
    }
    
    return new_buf;
}

// =============================================================================

// "load little endian value at the given byte offset and shift to get its
// value relative to the base offset (powers of 256, essentially)"
unsigned
ldle(uint8_t const * const p_arr,
     unsigned        const p_index)
{
    uint32_t v = p_arr[p_index];
    
    v <<= (8 * p_index);
    
    return v;
}

// Helper function to get the first byte in a little endian number
uint32_t
ldle0(uint8_t const * const p_arr)
{
    return ldle(p_arr, 0);
}

// Helper function to get the second byte in a little endian number
uint32_t
ldle1(uint8_t const * const p_arr)
{
    return ldle(p_arr, 1);
}

// Helper function to get the third byte in a little endian number
uint32_t
ldle2(uint8_t const * const p_arr)
{
    return ldle(p_arr, 2);
}

// Helper function to get the third byte in a little endian number
uint32_t
ldle3(uint8_t const * const p_arr)
{
    return ldle(p_arr, 3);
}

// =============================================================================

unsigned long
get_32_le(unsigned char const * const p_arr)
{
    unsigned v = 0;
    
    v |= (p_arr[0] <<  0);
    v |= (p_arr[1] <<  8);
    v |= (p_arr[2] << 16);
    v |= (p_arr[3] << 24);
    
    return v;
}

/// Convenience function so we can work with char* in addition to
/// unsigned char*
unsigned long
get_32_le_str(char const * const p_arr)
{
    return get_32_le( (unsigned char const * ) p_arr );
}

// =============================================================================

uint16_t
get_16_le(unsigned char const * const p_arr)
{
    uint16_t v = 0;
    
    v |= (p_arr[0] << 0);
    v |= (p_arr[1] << 8);
    
    return v;
}

/// Convenience function so we can work with char* in addition to
/// unsigned char*
uint16_t
get_16_le_str(char const * const p_arr)
{
    return get_16_le( (unsigned char const * ) p_arr );
}

/// Get little endian 16-bit value from an offset plus an index
/// This saves you from having to multiply an index by two.
uint16_t
get_16_le_i(unsigned char const * const p_arr,
            size_t                const p_index)
{
    return get_16_le(p_arr + (p_index * 2));
}

// =============================================================================

void
put_16_le(unsigned char * const p_arr,
          uint16_t        const p_val)
{
    p_arr[0] = ( (p_val >> 0) & 0xff );
    p_arr[1] = ( (p_val >> 8) & 0xff );
}

// =============================================================================

void
add_16_le(uint8_t  * const p_arr,
          uint16_t   const p_val)
{
    uint16_t v = get_16_le(p_arr);
    
    v += p_val;
    
    put_16_le(p_arr, v);
}

// =============================================================================

void
add_16_le_i(uint8_t  * const p_arr,
            uint16_t   const p_val,
            size_t     const p_index)
{
    size_t const offset = (p_index * 2);
    
    put_16_le(p_arr + offset, p_val);
}

// =============================================================================

void
put_16_le_str(char     * const p_arr,
              uint16_t   const p_val)
{
    put_16_le( (unsigned char *) p_arr, p_val);
}

// =============================================================================

void
put_16_le_i(unsigned char * const p_arr,
            size_t          const p_index,
            short           const p_val)
{
    put_16_le(p_arr + (p_index * 2), p_val);
}

// =============================================================================

void
put_32_le(uint8_t  * const p_arr,
          uint32_t   const p_val)
{
    p_arr[0] = ( (p_val >>  0) & 0xff );
    p_arr[1] = ( (p_val >>  8) & 0xff );
    p_arr[2] = ( (p_val >> 16) & 0xff );
    p_arr[3] = ( (p_val >> 24) & 0xff );
}

void
put_32_le_str(char     * const p_arr,
              uint32_t   const p_val)
{
    put_32_le( (unsigned char *) p_arr, p_val);
}

// =============================================================================

// Load little endian halfword (16-bit) dereferenced from 
uint16_t
ldle16b(uint8_t const * const p_arr)
{
    uint16_t v = 0;
    
    v |= ( ldle0(p_arr) | ldle1(p_arr) );
    
    return v;
}

// =============================================================================

// Load little endian halfword (16-bit) dereferenced from an arrays of bytes.
// This version provides an index that will be multiplied by 2 and added to the
// base address.
uint16_t
ldle16b_i(uint8_t const * const p_arr,
          size_t          const p_index)
{
    return ldle16b(p_arr + (2 * p_index) );
}

// =============================================================================

uint16_t
ldle16h(uint16_t const * const p_arr)
{
    return ldle16b( (uint8_t *) p_arr);
}

// =============================================================================

// "load little endian 24-bit value using a byte pointer"
uint32_t
ldle24b(uint8_t const * const p_arr)
{
    uint32_t v = ldle0(p_arr) | ldle1(p_arr) | ldle2(p_arr);
    
    return v;
}

// =============================================================================

// "indexed load little endian 24-bit value using a byte pointer"
uint32_t
ldle24b_i(uint8_t const * const p_arr,
          unsigned        const p_index)
{
    uint32_t v = ldle24b( p_arr + (p_index * 3) );
    
    return v;
}

// =============================================================================

// "store little endian 24-bit value using a byte pointer"
void
stle24b(uint8_t  *const p_arr,
        uint32_t  const p_val)
{
    p_arr[0] = ( (p_val >>  0) & 0xff );
    p_arr[1] = ( (p_val >>  8) & 0xff );
    p_arr[2] = ( (p_val >> 16) & 0xff );
}

// =============================================================================

// Read a half word at the given octec-pointer and if its bit pattern is
// 0xffff, return one. Else, zero. Note that endianness is irrelevant for
// this check.
int
is16b_neg1(uint8_t const * const p_arr)
{
    uint16_t v = ((uint16_t*) p_arr)[0];
    
    return (v == 0xffff);
}

int
is16b_neg1_i(uint8_t const * const p_arr,
             size_t          const p_index)
{
    return is16b_neg1(p_arr + (2 * p_index) );
}

// Load little endian halfword (16-bit) dereferenced from a pointer to a
// halfword.
uint16_t
ldle16h_i(uint16_t const * const p_arr,
          size_t           const p_index)
{
    return get_16_le_i((uint8_t *) p_arr,
                       p_index);
}

// =============================================================================

// Read a half word at the given halfword-pointer and if its bit patter is
// 0xffff, return one. Else, zero.
int
is16h_neg1(uint16_t const * const p_arr)
{
    return (p_arr[0] == 0xffff);
}

int
is16h_neg1_i(uint16_t const * const p_arr,
             size_t           const p_index)
{
    return is16h_neg1(p_arr + p_index);
}

// =============================================================================

RECT
HM_GetWindowRect(HWND const p_win)
{
    RECT r;
    
    GetWindowRect(p_win, &r);
    
    return r;
}

RECT
HM_GetClientRect(HWND const p_win)
{
    RECT r;
    
    GetClientRect(p_win, &r);
    
    return r;
}

POINT
HM_ClientToScreen(HWND const p_win)
{
    POINT pt;
    
    ClientToScreen(p_win, &pt);
    
    return pt;
}

// =============================================================================

POINT
HM_GetWindowPos(HWND const p_win)
{
    RECT const r = HM_GetWindowRect(p_win);
    
    POINT const pt = { r.left, r.top };
     
    return pt;
}


POINT
HM_GetClientPos(HWND const p_win)
{
    RECT const r = HM_GetClientRect(p_win);
    
    POINT const pt = { r.left, r.top };
    
    return pt;
}

POINT
HM_PointClientToScreen(HWND  const p_win,
                       POINT const p_rel_pos)
{
    POINT screen_pos = HM_GetWindowPos(p_win);
    
    screen_pos.x += p_rel_pos.x;
    screen_pos.y += p_rel_pos.y;
    
    return screen_pos;
}

// =============================================================================

signed int
HM_GetSignedLoword(LPARAM p_ptr)
{
    return ( (int)(short) LOWORD(p_ptr) );
}

// =============================================================================

signed int
HM_GetSignedHiword(LPARAM p_ptr)
{
    return ( (int)(short) HIWORD(p_ptr) );
}

// =============================================================================

HM_MouseMoveData
HM_GetMouseMoveData(HWND   const p_win,
                    WPARAM const wparam,
                    LPARAM const lparam)
{
    unsigned const flags = wparam;
    
    HM_MouseMoveData d = { FALSE, 0, 0, {0, 0} };
    
    POINT const rel_pos =
    {
        HM_GetSignedLoword(lparam),
        HM_GetSignedHiword(lparam)
    };
    
    // The absolute screen coordinates of the Window itself.
    POINT const win_screen_pos = HM_GetWindowPos(p_win);
    
    // The absolute screen coordinates of the location indicated by the event
    // (obviously this will be inside of the window, so further to the right,
    // further down.)
    POINT const screen_pos =
    {
        rel_pos.x + win_screen_pos.x,
        rel_pos.y + win_screen_pos.y
    };
    
    d.m_rel_pos = rel_pos;
    
    d.m_screen_pos = screen_pos;
    
    d.m_control_down = (flags & MK_CONTROL);
    
    return d;
}

// =============================================================================

int
truth(int value)
{
    return (value != 0);
}

// =============================================================================

HM_MouseWheelData
HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp)
{
    HM_MouseWheelData d;
    
    d.m_distance = HM_GetSignedHiword(p_wp);
    
    d.m_flags = LOWORD(p_wp);
    
    d.m_shift_key   = truth(d.m_flags & MK_SHIFT);
    d.m_control_key = truth(d.m_flags & MK_CONTROL);

    d.m_alt_key     = (GetKeyState(VK_MENU) < 0 );
    
    d.m_screen_pos.x = HM_GetSignedLoword(p_lp);
    d.m_screen_pos.y = HM_GetSignedHiword(p_lp);
    
    return d;
}

// =============================================================================

HM_MdiActivateData
HM_GetMdiActivateData(WPARAM const p_wp, LPARAM const p_lp)
{
    HM_MdiActivateData d;
    
    // -----------------------------
    
    d.m_deactivating = (HWND) p_wp;
    d.m_activating   = (HWND) p_lp;
    
    return d;
}

// =============================================================================

BOOL
HM_DrawRectangle(HDC const p_device_context,
                 RECT const p_rect)
{
    return Rectangle(p_device_context,
                     p_rect.left,
                     p_rect.top,
                     p_rect.right,
                     p_rect.bottom);
}

// =============================================================================

void*
hm_memdup(void const * const p_arr,
          size_t             p_len)
{
    void * const new_arr = calloc(1, p_len);
    
    if(new_arr)
    {
        memcpy(new_arr, p_arr, p_len);
    }
    
    return new_arr;
}

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

HWND debug_window = 0;

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
        unsigned const screen_width  = GetSystemMetrics(SM_CXSCREEN);
        unsigned const screen_height = GetSystemMetrics(SM_CYSCREEN);

        RECT const r = HM_GetWindowRect(win);
        
        SetWindowPos(win, HWND_NOTOPMOST,
                     screen_width - (r.right - r.left),
                     0,
                     0, 0, SWP_NOSIZE);
    
        ShowWindow(win, SW_SHOW);
    }
    
    return win;    
}

// =============================================================================

SCROLLINFO
HM_GetVertScrollInfo(HWND const p_win)
{
    SCROLLINFO si;
    
    si.cbSize = sizeof(SCROLLINFO);
    
    si.fMask = SIF_ALL;
     
    GetScrollInfo(p_win, SB_VERT, &si);
    
    return si;
}

// =============================================================================

SCROLLINFO
HM_GetHorizScrollInfo(HWND const p_win)
{
    SCROLLINFO si;
    
    si.cbSize = sizeof(SCROLLINFO);
    
    si.fMask = SIF_ALL;
     
    GetScrollInfo(p_win, SB_HORZ, &si);
    
    return si;
}

// =============================================================================

BOOL
HM_IsEmptyRect(RECT const p_rect)
{
    if
    (
        (p_rect.right == p_rect.left)
     && (p_rect.top   == p_rect.bottom)
    )
    {
        return TRUE;
    }

    return FALSE;
}

// =============================================================================


// \task Dummied out on the HMagic2 side for now.

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
    buf1 = src_rom + 0x2736a;
    buf2 = port_rom + 0x2736a;

    if(buf1 == buf2)
    {
        MessageBox(framewnd,"Source and target locations are the same","Bad error happened",MB_OK); 
        goto sametreasure;
    }

    // find the data for the first rom.
    l = *(short*)(src_rom + 0x88c11);
    
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

    l = *(short*)(port_rom + 0x88c11);
    
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
        
        put_16_le(rom + romHeaderDest + j, pointerEntry);
        
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

void Updatesize(HWND win)
{
    RECT rc;
    
    GetClientRect(win,&rc);
    
    // the '0' is SIZE_RESTORED.
    SendMessage(win,WM_SIZE,0,(rc.bottom<<16)+rc.right);
}

long CALLBACK superdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k,l,m,w,h;
    
    SDCREATE *sdc;
    SD_ENTRY *sde;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_NCCREATE:
        goto deflt;
    
    case WM_GETMINMAXINFO:
        sdc=(SDCREATE*)GetWindowLong(win,GWL_USERDATA);
        
        if(sdc)
        {
            ((LPMINMAXINFO)lparam)->ptMinTrackSize.x=sdc->dlgtemp->minw;
            ((LPMINMAXINFO)lparam)->ptMinTrackSize.y=sdc->dlgtemp->minh;
        }
        
        return 0;
    
    case WM_CREATE:
        
        SetWindowLong(win,
                      GWL_USERDATA,
                      (long)(sdc=((CREATESTRUCT*)lparam)->lpCreateParams));
        
        sde = sdc->dlgtemp->sde;
        
        for(i = 0; i < sdc->dlgtemp->numsde; i++)
        {
            if(sde[i].flags & 1)
                j = sdc->w-sde[i].x;
            else
                j = sde[i].x;
            
            if(sde[i].flags & 2)
                l = sdc->w - sde[i].w - j;
            else
                l = sde[i].w;
            
            if(sde[i].flags & 4)
                k = sdc->h-sde[i].y;
            else
                k = sde[i].y;
            
            if(sde[i].flags & 8)
                m = sdc->h-sde[i].h-k;
            else
                m = sde[i].h;
            
            hc = CreateWindowEx(sde[i].exstyle,sde[i].cname,sde[i].caption,sde[i].style,j,k,l,m,win,(HMENU)(sde[i].id),hinstance,0);
            
            SendMessage(hc,
                        WM_SETFONT,
                        (WPARAM) GetStockObject(ANSI_VAR_FONT),
                        0);
        }
        
        sdc->win = win;
        sdc->prev = lastdlg;
        sdc->next = 0;
        
        if(lastdlg)
            lastdlg->next = sdc;
        else
            firstdlg = sdc;
        
        lastdlg = sdc;
        
        sdc->dlgtemp->proc(win,WM_INITDIALOG,0,sdc->lparam);
        
        return 0;
    
    case WM_SIZE:
        
        sdc = (SDCREATE*) GetWindowLong(win,GWL_USERDATA);
        sde = sdc->dlgtemp->sde;
        
        w = LOWORD(lparam);
        h = HIWORD(lparam);
        
        for(i = 0; i < sdc->dlgtemp->numsde; i++)
        {
            if(sde[i].flags & 1)
                j = w - sde[i].x;
            else
                j = sde[i].x;
            
            if(sde[i].flags & 2)
                l = w - sde[i].w - j;
            else
                l = sde[i].w;
            
            if(sde[i].flags & 4)
                k = h - sde[i].y;
            else
                k = sde[i].y;
            
            if(sde[i].flags & 8)
                m = h - sde[i].h - k;
            else
                m = sde[i].h;
            
            SetWindowPos(GetDlgItem(win,sde[i].id),
                         0,
                         j,
                         k,
                         l,
                         m,
                         SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        }
        
        break;
    
    case WM_DESTROY:
        sdc = (SDCREATE*) GetWindowLong(win, GWL_USERDATA);
        sdc->dlgtemp->proc(win, msg, wparam, lparam);
        
        if(sdc->next)
            sdc->next->prev = sdc->prev;
        else
            lastdlg = sdc->prev;
        
        if(sdc->prev)
            sdc->prev->next = sdc->next;
        else
            firstdlg = sdc->next;
        
        free((SDCREATE*)sdc);
        SetWindowLong(win,GWL_USERDATA,0);
        
        break;
        
    default:
        
        sdc = (SDCREATE*) GetWindowLong(win, GWL_USERDATA);
        
        if(!sdc)
            goto deflt;
        
        SetWindowLong(win, DWL_MSGRESULT, 0);
        
        if( !sdc->dlgtemp->proc(win,msg,wparam,lparam) )
deflt:
            return DefWindowProc(win,msg,wparam,lparam);
        else
            return GetWindowLong(win,DWL_MSGRESULT);
    }
    
    return 0;
}

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

HWND CreateSuperDialog(SUPERDLG *dlgtemp,HWND owner,int x,int y,int w,int h, LPARAM lparam)
{
    SDCREATE * const sdc = (SDCREATE*) calloc(1, sizeof(SDCREATE));
    
    HWND hc;
    sdc->dlgtemp = dlgtemp;
    sdc->owner = owner;
    sdc->lparam = lparam;
    sdc->w = w;
    sdc->h = h;
    hc=CreateWindowEx(0,"SUPERDLG",dlgtemp->title,dlgtemp->style,x,y,w,h,owner,(HMENU)2000,hinstance,(void*)sdc);
    return hc;
}

HWND Editwin(FDOC*doc,char*wclass,char*title,int param,int size)
{
    char buf[1024];
    HWND hc;
    MDICREATESTRUCT mdic;
    
    EDITWIN * const a = (EDITWIN*) calloc(1, size);
    
    wsprintf(buf,"%s - %s",doc->filename,title);
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

//Compress****************************************

unsigned char* Compress(unsigned char *src, int oldsize, int *size, int flag)
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

//Compress************************************

//Uncompress**********************************

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
            
            d = get_16_le(src);
            
            src += 2;
            
            while(c > 1)
            {
                // copy that 16-bit number c/2 times into the b2 buffer.
                put_16_le(b2 + bd, d);
                
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
                d = get_16_le(src);
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

int Changesize(FDOC * const doc,
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

    if(x & 65536)
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
    put_16_le(rom + 0x9c2a, (j & 0xffff) );
    
    rom[0xcbad] = (uint8_t) (j >> 16);
    put_16_le(rom + 0xcba2, (j & 0xffff) );
    
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

void Releaseblks(FDOC*doc,int b)
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
                
                pal[k].rgbRed   = (BYTE) ((l & 0x1f) << 3);
                pal[k].rgbGreen = (BYTE) ((l & 0x3e0) >> 2);
                pal[k].rgbBlue  = (BYTE) ((l & 0x7c00) >> 7);
                pal[k].rgbReserved = 0;
                
                ++k;
            }
            ofs += 16;
        }
    }
}

//LoadPal***********************************

void Changeselect(HWND hc,int sel)
{
    int sc;
    int i;
    RECT rc;
    OVEREDIT*ed;
    ed=(OVEREDIT*)GetWindowLong(hc,GWL_USERDATA);
    GetClientRect(hc,&rc);
    rc.top=((ed->sel_select>>2)-ed->sel_scroll)<<5;
    rc.bottom=rc.top+32;
    ed->selblk=sel;
    if(ed->schflag) {
        for(i=0;i<ed->schnum;i++) if(ed->schbuf[i]==sel) {
            sel=i;
            goto foundblk;
        }
        sel=-1;
    }
foundblk:
    ed->sel_select=sel;
    InvalidateRect(hc,&rc,0);
    if(sel==-1) return;
    sel>>=2;
    sc=ed->sel_scroll;
    if(sel>=sc+ed->sel_page) sc=sel+1-ed->sel_page;
    else if(sel<sc) sc=sel;
    SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|(sc<<16),0);
    rc.top=(sel-ed->sel_scroll)<<5;
    rc.bottom=rc.top+32;
    InvalidateRect(hc,&rc,0);
}
const static short nxtmap[4]={-1,1,-16,16};

// Specific configurations that are checked involving BG2 settings.
// Dunno what they signify yet.
const static unsigned char bg2_ofs[]={
    0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0, 0x01
};

// =============================================================================

void
Initroom(DUNGEDIT * const ed,
         HWND       const win)
{
    unsigned char *buf2;
    
    int i;
    int j;
    int l;
    int m;
    
    unsigned char *rom = ed->ew.doc->rom;
    
    buf2 = rom + (ed->hbuf[1] << 2) + 0x75460;
    
    // Loadpal(ed,rom,0x1bd734 + *((unsigned short*)(rom + 0xdec4b + buf2[0])),0x21,15,6);
    // I didn't comment the above out, to the best of my knowledge, -MON
    
    j = 0x6073 + (ed->gfxtmp << 3);
    
    ed->palnum = ed->hbuf[1];
    ed->gfxnum = ed->hbuf[2];
    ed->sprgfx = ed->hbuf[3];
    
    // gives an offset (bunched in 4's)
    l = 0x5d97 + (ed->hbuf[2] << 2);
    
    // determine blocsets 0-7
    for(i = 0; i < 8; i++)
        ed->blocksets[i] = rom[j++];
    
    // these are uniquely determined
    ed->blocksets[8] = (rom + 0x1011e)[ed->gfxnum];
    ed->blocksets[9] = 0x5c;
    ed->blocksets[10] = 0x7d;
    
    // rewrite blocksets 3-6, if necessary
    for(i = 3; i < 7; i++)
    {
        m = rom[l++];
        
        if(m)
            ed->blocksets[i] = m;
    }
    
    l = 0x5c57 + (ed->sprgfx << 2);
    
    // determine blocksets 11-14, which covers them all.
    for(i = 0; i < 4; i++)
        ed->blocksets[i + 11] = rom[l + i] + 0x73;
    
    //get the block graphics for our tilesets?
    for(i = 0; i < 15; i++)
        Getblocks(ed->ew.doc, ed->blocksets[i]);
    
    ed->layering = (ed->hbuf[0] & 0xe1);
    
    // take bits 2-4, shift right 2 to get a 3 bit number.
    ed->coll = ((ed->hbuf[0] & 0x1c) >> 2);
    
    // the header is unmodified, nor is the room, so far.
    // if something changes, the flag will be set.
    ed->modf  = 0;
    ed->hmodf = 0;
    
    SetDlgItemInt(win, ID_DungFloor1, ed->buf[0] & 15, 0); // floor 1
    SetDlgItemInt(win, ID_DungFloor2, ed->buf[0] >> 4, 0); // floor 2
    SetDlgItemInt(win, ID_DungTileSet, ed->gfxnum, 0); //blockset
    SetDlgItemInt(win, ID_DungPalette, ed->palnum, 0); //palette
}

//Initroom******************************

//LoadHeader

void
LoadHeader(DUNGEDIT * const ed,
           int        const map)
{
    // we are passed in a dungeon editing window, and a map number (room number)
    
    uint8_t const * const rom = ed->ew.doc->rom;
    
    /// address of the header of the room indicated by parameter 'map'.
    uint16_t i = 0;
    
    // upper limit for the header offset.
    uint16_t m = 0;
    
    /// counter variable for looping through all dungeon rooms.
    int j = 0;
    
    // size of the header
    int l = 0;
    
    // -----------------------------
    
    i = get_16_le_i(rom + 0x27502, map);
    
    l = 14;
    
    // sort through all the other header offsets
    for(j = 0; j < 0x140; j++)
    {
        // m gives the upper limit for the header.
        // if is less than 14 bytes from i.
        m = get_16_le_i(rom + 0x27502, j);
        
        // \task When merging with other branches, note that
        // m and i are compared. If one is 16-bit, for example,
        // and the other is 32-bit, that is a big potential
        // problem.
        if( (m > i) && (m < (i + 14) ) )
        {
            l = (m - i);
            
            break;
        }
    }
    
    // determine the size of the header
    ed->hsize = l;
    
    // copy 14 bytes from the i offset.
    memcpy(ed->hbuf, rom + rom_addr_split(0x04, i), 14);
}

// =============================================================================

BOOL
HM_BinaryCheckDlgButton(HWND     const p_win,
                        unsigned const p_dlg_control,
                        BOOL     const p_is_checked)
{
    return CheckDlgButton(p_win,
                          p_dlg_control,
                          p_is_checked ? BST_CHECKED : BST_UNCHECKED);
}

// =============================================================================

void
LoadDungeonObjects(DUNGEDIT * const p_ed,
                   unsigned   const p_map)
{
    uint8_t const * const rom = p_ed->ew.doc->rom;
    
    // Get the base address for this room in the rom.
    uint8_t const * const buf = rom
                              + romaddr( ldle24b_i(rom + 0xf8000, p_map) );

    // Offset into the raw object data array.
    // The first two bytes are floor tile fill pattern (floor1 / floor2),
    // layout and other information that are irrelevant to loading objects,
    // so skip over them.
    uint16_t i = 2;
    
    // Indicates which layer we're loading from. Even values represent the
    // 3 byte object portion of a layer, odd values represent the 2 byte or
    // door portion. E.g. if the value of this is 5, that represents the
    // door objects of layer 3.
    int j = 0;
    
    // -----------------------------
    
    p_ed->selobj = 0;
    p_ed->selchk = 0;
    
    for(j = 0; j < 6; j++)
    {
        p_ed->chkofs[j] = i;
        
        for( ; ; )
        {
            uint16_t k = ldle16b(buf + i);
            
            // code indicating that this layer has terminated
            if(k == 0xffff)
                break;
            
            // code indicating that we should begin loading door objects.
            if(k == 0xfff0)
            {
                j++;
                
                p_ed->chkofs[j] = i + 2;
                
                for( ; ; )
                {
                    i += 2;
                    
                    k = ldle16b(buf + i);
                    
                    if(k == 0xffff)
                        goto end;
                    
                    if( ! p_ed->selobj )
                    {
                        p_ed->selobj = i;
                        p_ed->selchk = j;
                    }
                }
            }
            else
            {
                i += 3;
            }
            
            // if there is no selected object, pick one.
            if( ! p_ed->selobj )
            {
                p_ed->selobj = i - 3;
                p_ed->selchk = j;
            }
        }
        
        j++;
        
        p_ed->chkofs[j] = i + 2;
end:
        i += 2;
    }

    p_ed->len = i; // size of the buffer.
    
    // generate the buffer of size i.
    // copy the data from buf to ed->buf.
    p_ed->buf = (uint8_t*) hm_memdup(buf, i);
}

// =============================================================================

void
Openroom(DUNGEDIT * const ed,
         int        const map)
{
    uint8_t const * const rom = ed->ew.doc->rom;
    
    // Get the base address for this room in the rom.
    uint8_t const * buf = 0;

    int i = 0;
    
    int num_torches = 0;
    
    int layer = 0;
    
    HWND win = ed->dlg;
    
    // -----------------------------
    
    ed->mapnum = map;
    
    ed->ew.doc->dungs[map] = win;
    
    LoadDungeonObjects(ed, map);
    
    layer = ed->selchk & ~1;
    
    // do some initial settings, determine which radio button to check first 1, 2, 3,...
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer1, (layer == 0) );
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer2, (layer == 2) );
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer3, (layer == 4) );
    HM_BinaryCheckDlgButton(win, ID_DungSprLayer, (layer == 6) );
    HM_BinaryCheckDlgButton(win, ID_DungItemLayer, (layer == 7) );
    HM_BinaryCheckDlgButton(win, ID_DungBlockLayer, (layer == 8) );
    HM_BinaryCheckDlgButton(win, ID_DungTorchLayer, (layer == 9) );
    
    // this is the "layout", ranging from 0-7
    SetDlgItemInt(win, ID_DungLayout, ed->buf[1] >> 2, 0);
    
    // load the header information.
    LoadHeader(ed, map);
    
    Initroom(ed,win);
    
    // prints the room string in the upper left hander corner.
    wsprintf(buffer,"Room %d",map);
    
    // ditto.
    SetDlgItemText(win, ID_DungRoomNumber, buffer);
    
    if(map > 0xff)
    {
        // If we're in the upper range, certain buttons might be grayed out so we can't
        // get back to the lower range
        for(i = 0; i < 4; i++)
        {
            EnableWindow(GetDlgItem(win, ID_DungLeftArrow + i),
                         ((map + nxtmap[i]) & 0xff) < 0x28);
        }
    }
    else
    {
        EnableWindow( GetDlgItem(win, ID_DungLeftArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungRightArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungUpArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungDownArrow), 1);
    }
    
    CheckDlgButton(win, ID_DungObjLayer1, BST_CHECKED);
    CheckDlgButton(win, ID_DungObjLayer2, BST_UNCHECKED);
    CheckDlgButton(win, ID_DungObjLayer3, BST_UNCHECKED);
    
    for(i = 0; i < 9; i++)
    {
        if(ed->layering == bg2_ofs[i])
        {
            // \task Is this buggy? What if the value is not in this list?
            // Probably not the case in a vanilla rom, but still...
            SendDlgItemMessage(win, ID_DungBG2Settings, CB_SETCURSEL, i, 0);
            
            break;
        }
    }
    
    SendDlgItemMessage(win, ID_DungCollSettings, CB_SETCURSEL, ed->coll, 0);
    SetDlgItemInt(win, ID_DungSprTileSet, ed->sprgfx, 0);
    
    buf = rom + 0x50000 + ((short*)(rom + ed->ew.doc->dungspr))[map];
    
    for(i = 1; ; i += 3)
    {
        if(buf[i] == 0xff)
            break;
    }
    
    ed->esize = i;
    ed->ebuf = (uint8_t*) malloc(i);
    
    memcpy(ed->ebuf,buf,i);
    
    CheckDlgButton(win, ID_DungSortSprites, *ed->ebuf ? BST_CHECKED : BST_UNCHECKED);
    
    // load the secrets for the dungeon room
    buf = rom + 0x10000 + ((short*) (rom + 0xdb69))[map];
    
    for(i = 0; ; i += 3)
        if( is16b_neg1(buf + i) )
            break;
    
    ed->ssize = i;
    ed->sbuf = malloc(i);
    
    memcpy(ed->sbuf,buf,i);
    
    buf = rom + 0x2736a;
    
    num_torches = ldle16b(rom + 0x88c1);
    
    for(i = 0; ; i += 2)
    {
        int j = i;
        
        if(i >= num_torches)
        {
            ed->tsize = 0;
            ed->tbuf = 0;
            
            break;
        }
        
        for( ; ; )
        {
            i += 2;
            
            if( is16b_neg1(buf + i) )
                break;
        }
        
        if(*(short*)(buf + j) == map)
        {
            ed->tbuf = (uint8_t*) malloc(ed->tsize = i - j);
            
            memcpy(ed->tbuf, buf + j, i - j);
            
            break;
        }
    }
}

//OpenRoom*****************************

//SaveDungSecrets**********************

void Savedungsecret(FDOC*doc,int num,unsigned char*buf,int size)
{
    int i,j,k;
    int adr[0x140];
    unsigned char*rom=doc->rom;
    for(i=0;i<0x140;i++)
        adr[i]=0x10000 + ((short*)(rom + 0xdb69))[i];
    j=adr[num];
    k=adr[num+1];
    
    if( is16b_neg1(rom + j) )
    {
        if(!size) return;
        j+=size+2;
        adr[num]+=2;
    } else {
        if(!size) {
            if(j>0xdde9) {
                j-=2;
                adr[num]-=2;
            }
        } else j+=size;
    }
    if(*(short*)(rom+k)!=-1) k-=2;
    if(adr[0x13f]-k+j>0xe6b0) {
        MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
        return;
    }
    memmove(rom+j,rom+k,adr[0x13f]+2-k);
    if(size) memcpy(rom+adr[num],buf,size);
    if(j==k) return;
    ((short*)(rom + 0xdb69))[num]=adr[num];
    for(i=num+1;i<0x140;i++) {
        ((short*)(rom + 0xdb69))[i]=adr[i]+j-k;
    }
}
int Savesprites(FDOC*doc,int num,unsigned char*buf,int size)
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

//Saveroom******************************************

void Saveroom(DUNGEDIT *ed)
{
    unsigned char *rom = ed->ew.doc->rom; // get our romfile from the Dungedit struct.
    
    int l;
    int m;
    int n;
    
    if( !ed->modf ) // if the dungeon map hasn't been modified, do nothing.
        return;
    
    // if it has... save it.
    if( ed->ew.param >= 0x8c) // if the map is an overlay... 
    {
        int i = Changesize(ed->ew.doc, ed->ew.param + 0x301f5, ed->len - 2);
        
        if(!i)
            return;
        
        memcpy(rom + i, ed->buf + 2, ed->len - 2);
    }
    else
    {
        short i = 0;
        short j = 0;
        short k = 0;
        
        int ret_size = 0;
        
        Savesprites(ed->ew.doc, 0x160 + ed->mapnum, ed->ebuf, ed->esize);
        
        for(i = 5; i >= 1; i -= 2)
        {
            if(ed->chkofs[i] != ed->chkofs[i + 1])
                break;
        }
        
        door_ofs = ed->chkofs[i];
        
        ret_size = Changesize(ed->ew.doc, ed->mapnum + 0x30140, ed->len);
        
        if( !ret_size )
            return;
        
        memcpy(rom + ret_size, ed->buf, ed->len);
        
        Savedungsecret(ed->ew.doc, ed->mapnum, ed->sbuf, ed->ssize);
        
        k = get_16_le(rom + 0x88c1);
        
        if( is16b_neg1(rom + 0x2736a) )
            k = 0;
        
        for(i = 0; i < k; i += 2)
        {
            j = i;
            
            // test block
            quick_u16_signedness_test();
            
            for( ; ; )
            {
                j += 2;
                
                if( is16b_neg1(rom + 0x2736a + j) )
                    break;
            }
            
            if( get_16_le(rom + 0x2736a + i) == ed->mapnum )
            {
                if(!ed->tsize)
                    j += 2;
                
                if(k + i + ed->tsize - j > 0x120)
                    goto noroom;
                
                memmove(rom + 0x2736a + i + ed->tsize, rom + 0x2736a + j, k - j);
                memcpy(rom + 0x2736a + i, ed->tbuf, ed->tsize);
                
                k += i + ed->tsize - j;
                
                break;
            }
            else
                i = j;
        }
        
        if(ed->tsize && i == k)
        {
            j = ed->tsize + 2;
            
            if(k + j > 0x120)
            noroom:
            {
                MessageBox(framewnd,
                           "Not enough room for torches",
                           "Bad error happened",
                           MB_OK);
            }
            else
            {
                memcpy(rom + 0x2736a + k, ed->tbuf, ed->tsize);
                
                put_16_le(rom + 0x2736a + k + j - 2, u16_neg1);
                
                k += j;
            }
        }
        
        // \task Is this a bug? The second expression just tests for
        // equality but doesn't change any part of memory.
        // \note Update: This appears to maybe be checking whether
        // the user has cleared all the torches. This has the effect
        // of changing the immediate operand of a CPX command (number of bytes
        // worth of torch data) and checking whether there is valid torch
        // data in the first two entries. Effectively if there are two
        // 0xffff words in the torch data and the operand of the instruction
        // is 4, it means no room can load torch data.
        if(!k)
        {
            k = 4;
            
            get_32_le(rom + 0x2736a) == u32_neg1;
        }
        
        put_16_le(rom + 0x88c1, k);
        
        m = ed->hmodf;
        
        // if the headers have been modified, save them.
        if(m != 0)
        {
            // some sort of upper bound.
            int k = 0x28000 + get_16_le(rom + 0x27780);
            
            // some sort of lower bound.
            int i = 0x28000 + get_16_le_i(rom + 0x27502, ed->mapnum);
            
            for(j = 0; j < 0x140; j++)
            {
                // if we hit the map number we're currently on, keep moving.
                if(j == ed->mapnum)
                    continue;
                
                if( 0x28000 + get_16_le_i(rom + 0x27502, j) == i )
                {
                    if(m > 1)
                        goto headerok;
                    
                    wsprintf(buffer,"The room header of room %d is reused. Modify this one only?",ed->mapnum);
                    
                    if(MessageBox(framewnd,buffer,"Bad error happened",MB_YESNO)==IDYES)
                    {
headerok:
                        k = i;
                        
                        goto changeroom;
                    }
                    
                    break;
                }
            }
            
            for(j = 0; j < 0x140; j++)
            {
                l = 0x28000 + ((short*)(rom + 0x27502 ))[j];
                
                if(l > i && l < k)
                    k = l;
            }
changeroom:
            
            if(m > 1)
            {
                ( (short*) ( rom + 0x27502 ) )[ed->mapnum] = ( (short*) ( rom + 0x27502 ) )[m - 2];
                
                n = 0;
            }
            else
            {
                n = ed->hsize;
            }
            
            // \task Should this actually be a signed load?
            if( get_16_le(rom + 0x27780) + (i + n - k) > 0 )
            {
                MessageBox(framewnd,
                           "Not enough room for room header",
                           "Bad error happened",
                           MB_OK);
            }
            else
            {
                short const delta = (short) (i + n - k);
                
                memmove(rom + i + n, rom + k, 0x28000 + *((short*)(rom + 0x27780)) - k);
                
                add_16_le(rom + 0x27780, delta);
                
                memcpy(rom + i, ed->hbuf, n);
                
                for(j = 0; j < 0x140; j++)
                {
                    if
                    (
                        j != ed->mapnum
                     && get_16_le_i(rom + 0x27502, j) + 0x28000 >= k
                    )
                    {
                        add_16_le_i(rom + 0x27502, j, delta);
                    }
                }
            }
            
            if(n)
            {
                rom[i]   = ed->layering | (ed->coll << 2);
                rom[i+1] = (uint8_t) ed->palnum;
                rom[i+2] = (uint8_t) ed->gfxnum;
                rom[i+3] = (uint8_t) ed->sprgfx;
            }
        }
    }
    
    ed->modf=0;
    ed->ew.doc->modf=1;
}

//SaveRoom***********************************

//Closeroom*********************************

int Closeroom(DUNGEDIT *ed)
{
    int i;
    
    if(ed->ew.doc->modf != 2 && ed->modf)
    {
        if(ed->ew.param < 0x8c)
            wsprintf(buffer,"Confirm modification of room %d?",ed->mapnum);
        
        else
            wsprintf(buffer,"Confirm modification of overlay map?");
        
        switch(MessageBox(framewnd,buffer,"Dungeon editor",MB_YESNOCANCEL))
        {
        
        case IDYES:
            Saveroom(ed);
            
            break;
        
        case IDCANCEL:
            return 1;
        }
    }
    
    for(i = 0; i < 15; i++)
        Releaseblks(ed->ew.doc,ed->blocksets[i]);
    
    if(ed->ew.param<0x8c)
        ed->ew.doc->dungs[ed->mapnum] = 0;
    
    return 0;
}

//Closeroom**********************************

// =============================================================================

void
fill4x2(uint8_t  const * const rom,
        uint16_t       * const nbuf,
        uint16_t const * const buf)
{
    int i;
    int j;
    int k;
    int l;
    int m;
    
    for(l = 0; l < 4; l++)
    {
        i = ((short*) (rom + 0x1b02))[l] >> 1;
        
        for(m = 0; m < 8; m++)
        {
            for(k = 0; k < 8; k++)
            {
                for(j = 0; j < 2; j++)
                {
                    nbuf[i] = buf[0];
                    nbuf[i+1] = buf[1];
                    nbuf[i+2] = buf[2];
                    nbuf[i+3] = buf[3];
                    nbuf[i+64] = buf[4];
                    nbuf[i+65] = buf[5];
                    nbuf[i+66] = buf[6];
                    nbuf[i+67] = buf[7];
                    i += 128;
                }
                
                i -= 252;
            }
            
            i += 224;
        }
    }
}

void len1(void)
{
    if(!dm_l) dm_l=0x20;
}

void len2(void)
{
    if(!dm_l) dm_l=0x1a;
}

void draw2x2(void)
{
    dm_wr[0]  = dm_rd[0];
    dm_wr[64] = dm_rd[1];
    dm_wr[1]  = dm_rd[2];
    dm_wr[65] = dm_rd[3];
    
    dm_wr += 2;
}
void drawXx4(int x)
{
    while(x--)
    {
        dm_wr[0]   = dm_rd[0];
        dm_wr[64]  = dm_rd[1];
        dm_wr[128] = dm_rd[2];
        dm_wr[192] = dm_rd[3];
        
        dm_rd += 4;
        
        dm_wr++;
    }
}
void drawXx4bp(int x)
{
    while(x--)
    {
        dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
        dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
        dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
        dm_buf[0x10c0 + dm_x] = dm_buf[0xc0 + dm_x] = dm_rd[3];
        
        dm_x++;
        
        dm_rd += 4;
    }
}
void drawXx3bp(int x)
{
    while(x--)
    {
        dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
        dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
        dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
        
        dm_x++;
        
        dm_rd += 3;
    }
}
void drawXx3(int x)
{
    while(x--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[64]=dm_rd[1];
        dm_wr[128]=dm_rd[2];
        dm_rd+=3;
        dm_wr++;
    }
}

void draw3x2(void)
{
    int x=2;
    while(x--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[1];
        dm_wr[2]=dm_rd[2];
        dm_wr[64]=dm_rd[3];
        dm_wr[65]=dm_rd[4];
        dm_wr[66]=dm_rd[5];
    }
}

void draw1x5(void)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[64]=dm_rd[1];
    dm_wr[128]=dm_rd[2];
    dm_wr[192]=dm_rd[3];
    dm_wr[256]=dm_rd[4];
}

void draw8fec(uint16_t n)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[1]=dm_rd[1];
    
    dm_wr[65]=dm_wr[64] = n;
}

void draw9030(uint16_t n)
{
    dm_wr[1]=dm_wr[0]=n;
    dm_wr[64]=dm_rd[0];
    dm_wr[65]=dm_rd[1];
}

void draw9078(uint16_t n)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[64]=dm_rd[1];
    dm_wr[65]=dm_wr[1]=n;
}

void draw90c2(uint16_t n)
{
    dm_wr[64]=dm_wr[0]=n;
    dm_wr[1]=dm_rd[0];
    dm_wr[65]=dm_rd[1];
}

void draw4x2(int x)
{
    while(dm_l--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[1];
        dm_wr[2]=dm_rd[2];
        dm_wr[3]=dm_rd[3];
        dm_wr[64]=dm_rd[4];
        dm_wr[65]=dm_rd[5];
        dm_wr[66]=dm_rd[6];
        dm_wr[67]=dm_rd[7];
        dm_wr+=x;
    }
}
void draw2x6(uint16_t * const nbuf)
{
    int m = 6;
    
    dm_wr = nbuf + dm_x;
    
    while(m--)
    {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[6];
        dm_wr+=64;
        dm_rd++;
    }
}
void drawhole(int l, uint16_t * nbuf)
{
    draw2x6(nbuf);
    dm_x+=2;
    dm_rd+=6;
    while(l--) {
        dm_buf[dm_x]=dm_buf[dm_x+64]=dm_buf[dm_x+128]=
        dm_buf[dm_x+192]=dm_buf[dm_x+256]=dm_buf[dm_x+320]=
        dm_rd[0];
        dm_x++;
    }
    draw2x6(nbuf);
}
void draw4x4X(int n)
{
    int x;
    while(n--) {
        x=2;
        while(x--) {
            dm_wr[0]=dm_rd[0];
            dm_wr[1]=dm_rd[1];
            dm_wr[2]=dm_rd[2];
            dm_wr[3]=dm_rd[3];
            dm_wr[64]=dm_rd[4];
            dm_wr[65]=dm_rd[5];
            dm_wr[66]=dm_rd[6];
            dm_wr[67]=dm_rd[7];
            dm_wr+=128;
        }
        dm_wr-=252;
    }
}

void draw12x12(void)
{
    int m,
        l = 12;
    
    while(l--)
    {
        m = 12;
        
        while(m--)
        {
            dm_buf[dm_x + 0x1000] = dm_rd[0];
            dm_x++;
            dm_rd++;
        }
        
        dm_x+=52;
    }
}

void draw975c(void)
{
    unsigned char m;
    m=dm_l;
    dm_wr[0]=dm_rd[0];
    while(m--) dm_wr[1]=dm_rd[3],dm_wr++;
    dm_wr[1]=dm_rd[6];
    dm_wr[2]=dm_wr[3]=dm_wr[4]=dm_wr[5]=dm_rd[9];
    m=dm_l;
    dm_wr[6]=dm_rd[12];
    while(m--) dm_wr[7]=dm_rd[15],dm_wr++;
    dm_wr[7]=dm_rd[18];
    dm_tmp+=64;
    dm_wr=dm_tmp;
}

void draw93ff(void)
{
    int m;
    uint16_t *tmp;
    m = dm_l;
    
    tmp=dm_wr;
    dm_wr[0] = dm_rd[0];
    
    while(m--)
        dm_wr[1] = dm_rd[1], dm_wr[2] = dm_rd[2], dm_wr += 2;
    
    dm_wr[1] = dm_rd[3];
    tmp += 64;
    dm_wr = tmp;
}

void drawtr(void)
{
    unsigned char n=dm_l,l;
    for(l=0;l<n;l++)
        dm_wr[l] = dm_rd[0];
}

void tofront4x7(int n)
{
    int m=7;
    while(m--) {
        dm_buf[n]|=0x2000;
        dm_buf[n+1]|=0x2000;
        dm_buf[n+2]|=0x2000;
        dm_buf[n+3]|=0x2000;
        n+=64;
    }
}
void tofront5x4(int n)
{
    int m=5;
    while(m--) {
        dm_buf[n]|=0x2000;
        dm_buf[n+64]|=0x2000;
        dm_buf[n+128]|=0x2000;
        dm_buf[n+192]|=0x2000;
        n++;
    }
}
void tofrontv(int n)
{
    int m=n;
    n&=0x783f;
    while(n!=m) {
        dm_buf[n]|=0x2000;
        dm_buf[n+1]|=0x2000;
        dm_buf[n+2]|=0x2000;
        dm_buf[n+3]|=0x2000;
        n+=64;
    }
}
void tofronth(int n)
{
    int m=n;
    n&=0x7fe0;
    while(n!=m) {
        dm_buf[n]|=0x2000;
        dm_buf[n+64]|=0x2000;
        dm_buf[n+128]|=0x2000;
        dm_buf[n+192]|=0x2000;
        n++;
    }
}

void drawadd4(uint8_t const * const rom,
              int             const p_x)
{
    int m = 4;
    int x = p_x;
    
    dm_rd = (uint16_t*) ( rom + 0x1b52 + ldle16b_i(rom + 0x4e06, dm_l) );
    
    while(m--)
    {
        dm_buf[x + 0x1040] = dm_rd[0];
        dm_buf[x + 0x1080] = dm_rd[1];
        dm_buf[x +   0xc0] = dm_rd[2];
        
        dm_rd += 3;
        
        x++;
    }
    
    x -= 4;
    
    tofrontv(x + 0x100);
}

void drawaef0(uint8_t const * const rom,
              int             const p_x)
{
    int m = 2;
    int x = p_x;
    
    dm_rd = (uint16_t*) ( rom + 0x1b52 + ldle16b_i(rom + 0x4ec6, dm_l) );
    
    while(m--)
    {
        dm_buf[x + 0x1001] = dm_rd[0];
        dm_buf[x + 0x1041] = dm_rd[1];
        dm_buf[x + 0x1081] = dm_rd[2];
        dm_buf[x + 0x10c1] = dm_rd[3];
        
        dm_rd += 4;
        
        x++;
    }
    
    dm_buf[x +   1] = dm_rd[0];
    dm_buf[x +  65] = dm_rd[1];
    dm_buf[x + 129] = dm_rd[2];
    dm_buf[x + 193] = dm_rd[3];
    
    x += 2;
    
    while(x & 0x3f)
    {
        dm_buf[x      ] |= 0x2000;
        dm_buf[x +  64] |= 0x2000;
        dm_buf[x + 128] |= 0x2000;
        dm_buf[x + 192] |= 0x2000;
        
        x++;
    }
}

const static char obj_w[64]={
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,4,4,4,4,
    2,2,2,2,4,2,2,2,2,2,4,4,4,4,2,2,4,4,4,2,6,4,4,4,
    4,4,4,4,2,4,4,10,4,4,4,4,24,3,6,1
};
const static char obj_h[64]={
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,
    2,2,2,2,4,3,2,2,2,3,5,3,4,4,3,2,5,4,2,3,3,4,4,4,
    4,4,4,4,2,2,2,4,3,3,3,3,6,3,3,7
};
const static char obj2_w[128]={
    4,4,4,1,1,1,1,1,1,1,1,1,1,16,1,1,
    2,2,5,2,4,10,2,16,2,2,2,4,4,4,4,4,
    4,4,2,2,2,2,4,4,4,4,44,2,4,14,28,2,
    2,4,4,4,3,3,6,6,3,3,4,4,6,6,2,2,
    2,2,2,2,2,2,2,4,4,2,2,8,6,6,4,2,
    2,2,2,2,14,3,2,2,2,2,4,3,6,6,2,2,
    3,3,22,2,2,2,4,4,4,3,3,4,4,4,3,3,
    4,8,10,64,8,2,8,8,8,4,4,20,2,2,2,1
};
const static char obj2_h[128]={
    3,5,7,1,1,1,1,1,1,1,1,1,1,4,1,1,
    2,2,8,2,3,8,2,4,2,2,2,4,4,4,4,4,
    4,4,2,2,2,2,4,4,4,4,44,2,4,14,25,2,
    2,3,3,4,2,2,3,3,2,2,6,6,4,4,2,2,
    2,2,2,2,2,2,2,4,4,2,2,3,8,3,3,2,
    2,2,2,2,14,5,2,2,2,2,2,5,4,3,2,2,
    6,6,13,2,2,2,4,3,3,4,4,4,3,3,4,4,
    10,8,8,64,8,2,3,3,8,3,4,8,2,2,2,1
};

const char obj3_w[248]={
    0,0,0,0,0,-4,-4,0,0,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,2,3,2,2,2,2,2,2,2,2,2,2,2,2,13,
    13,1,1,0,3,1,-2,-2,-2,-4,-4,-4,-2,-4,-12,2,
    2,2,2,2,2,2,2,2,2,0,0,-12,2,2,2,2,
    1,2,2,0,1,-8,-8,1,1,1,1,2,2,5,-2,22,
    2,4,4,4,4,4,4,2,2,1,1,1,2,2,1,1,
    4,1,1,4,4,2,3,3,2,1,1,2,1,2,1,2,
    2,3,3,3,3,3,3,2,2,2,1,1,1,1,1,2,
    4,4,2,2,4,2,2,1,1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    7,7,0,2,2,0,0,0,0,0,0,-2,0,0,1,1,
    0,12,0,0,0,0,0,0,0,0,0,1,1,3,3,1,
    1,0,0,1,1,1,1,0,4,0,4,0,8,2,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
};

const char obj3_h[248]={
    2,4,4,4,4,4,4,2,2,9,9,9,9,9,9,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,3,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,4,1,1,4,4,3,4,3,3,8,4,2,1,
    1,1,1,1,1,1,1,5,3,2,2,2,3,4,4,4,
    1,3,3,2,1,2,2,1,1,1,1,3,3,3,2,1,
    0,0,0,0,0,-4,-4,0,0,3,0,0,13,13,1,1,
    0,3,1,-2,-2,-2,-4,-4,-12,0,0,-12,1,0,1,-8,
    -8,-4,-4,-4,-4,2,2,-2,5,-2,22,7,7,0,0,3,
    0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,1,4,1,1,4,4,4,2,2,4,2,2,2,1,1,
    0,6,0,0,0,0,0,0,0,0,0,1,1,0,0,1,
    1,0,0,1,1,1,1,0,4,0,4,0,5,2,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
};
const char obj3_m[248]={
    2,2,2,2,2,6,6,2,2,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,4,1,0,6,6,4,6,8,8,4,6,14,1,
    1,1,1,1,1,1,1,2,2,4,4,14,2,2,2,2,
    1,2,2,2,0,12,12,0,0,0,0,2,2,1,4,1,
    2,2,2,2,2,6,6,2,2,1,1,1,1,1,0,0,
    4,1,0,6,6,6,8,8,14,1,1,14,1,2,0,12,
    12,8,8,8,8,2,2,6,1,4,1,1,1,1,1,2,
    2,2,2,2,4,2,2,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
    1,1,4,1,1,2,2,2,2,2,4,4,2,2,0,0,
    4,2,4,3,4,4,4,4,4,4,4,0,0,1,1,0,
    0,4,4,0,0,0,0,3,4,4,4,4,2,2,2,4,
    4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

const unsigned char obj3_t[248]={
    0,2,2,1,1,1,1,1,1,97,65,65,97,97,65,65,
    97,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,
    97,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    128,130,130,129,129,129,129,129,129,129,129,129,129,129,129,129,
    129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
    129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
    130,130,128,128,129,129,129,129,129,129,129,129,129,129,129,129,
    65,65,65,113,65,65,65,65,113,65,65,65,113,129,129,129,
    1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,1,1,20,4,1,
    1,3,3,1,1,1,1,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

void getobj(unsigned char const * map)
{
    unsigned short i;
    unsigned char j;
    
    i = *(unsigned short*) map;
    
    j = i & 0xfc;
    
    if(j == 0xfc) // subtype 3 object
    {
        dm_x = ((i & 3) << 4) | (i >> 12);
        map++;
        i = *(unsigned short*) map;
        dm_x += ((i & 15) << 8) | ((i & 0xc000u) >> 8);
        dm_k = ((i & 0x3f00) >> 8) + 0x100;
    }
    else
    {
        dm_x = ((j >> 2) | ((i & 0xfc00u) >> 4));
        dm_k = map[2];
        if(dm_k < 0xf8) // subtype 1 object
            dm_l = ((i & 3) << 2)|((i & 0x300u) >> 8);
        else
            dm_l = (i & 3) | ((i & 0x300u) >> 6);
    }
}

char sprname[0x11c][16];

HDC objdc;

HBITMAP objbmp;

void Getstringobjsize(char*str,RECT*rc)
{
    GetTextExtentPoint(objdc,str,strlen(str),(LPSIZE)&(rc->right));
    rc->bottom++;
    rc->right++;
    if(rc->bottom<16) rc->bottom=16;
    if(rc->right<16) rc->right=16;
    rc->right+=rc->left;
    rc->bottom+=rc->top;
}
const static char obj4_h[4]={5,7,11,15};
const static char obj4_w[4]={8,16,24,32};
void Getdungobjsize(int chk,RECT*rc,int n,int o,int p)
{
    // Loads object sizes
    int a = 0,
        b = 0,
        c = 0,
        d = 0,
        e = 0;
    
    char*f;
    switch(chk) {
    case 0: case 2: case 4:
        if(dm_k >= 0x100)
            rc->right=obj_w[dm_k - 0x100]<<3,rc->bottom=obj_h[dm_k - 0x100]<<3;
        else if(dm_k >= 0xf8)
        {
            rc->right = obj2_w[ ( (dm_k - 0xf8) << 4) + dm_l] << 3;
            rc->bottom = obj2_h[ ( (dm_k - 0xf8) << 4) + dm_l] << 3;
        }
        else {
            c=0;
            d=dm_l;
            e=obj3_t[dm_k];
            switch(e&15) {
            case 0:
                len1();
                break;
            case 1:
                dm_l++;
                break;
            case 2:
                len2();
                break;
            case 3:
                c=((dm_l&3)+1)*obj3_m[dm_k];
                dm_l>>=2;
                dm_l++;
                break;
            
            case 4:
                
                c = ( obj4_h[dm_l >> 2] + 3 ) << 1;
                
                dm_l = obj4_w[dm_l & 3];
                
                if(e & 16)
                    rc->left -= dm_l << 3;
                
                break;
            }
            
            dm_l*=obj3_m[dm_k];
            switch(e&192) {
            case 0:
                a=dm_l;
                b=c;
                break;
            case 64:
                a=dm_l;
                b=dm_l;
                break;
            case 128:
                a=0;
                b=dm_l;
                break;
            }
            
            if(e & 32)
                rc->top -= ( b + ( (e & 16) ? 2 : 4) ) << 3;
            
            a += obj3_w[dm_k];
            b += obj3_h[dm_k];
            
            rc->right = a << 3;
            rc->bottom = b << 3;
            
            dm_l=d;
        }
        if(dm_k==0xff) {if(dm_l==8) rc->left-=16; else if(dm_l==3) rc->left=-n,rc->top=-o;}
        else if(dm_k==0xfa) {
            if(dm_l==14) rc->left+=16,rc->top+=32;
            else if(dm_l==10) rc->left=80-n,rc->top=64-o;
        }
        break;
    case 1: case 3: case 5:
        switch(dm_k) {
        case 0:
            if(dm_l==24) {
                rc->right=176;
                rc->bottom=96;
            } else if(dm_l==25) rc->right=rc->bottom=32; else if(dm_dl>8) {
                rc->top-=120;
                rc->bottom=144;
                rc->right=32;
            } else if(dm_dl>5) {
                rc->top-=72;
                rc->bottom=96;
                rc->right=32;
            } else rc->right=32,rc->bottom=24;
            break;
        case 1:
            if(dm_l==5 || dm_l==6) {
                rc->right=96;
                rc->bottom=64;
                rc->left-=32;
                rc->top-=32;
            } else if(dm_l==2 || dm_l==7 || dm_l==8) {
                rc->right=rc->bottom=32;
            } else rc->right=32,rc->bottom=24,rc->top+=8;
            break;
        case 2:
            rc->bottom=32;
            if(dm_dl>8) {
                rc->left-=104;
                rc->right=128;
            } else if(dm_dl>5) {
                rc->left-=56;
                rc->right=80;
            } else rc->right=24;
            break;
        case 3:
            rc->right=24,rc->bottom=32,rc->left+=8;
        }
        break;
    case 8: case 9:
        rc->right=16;
        rc->bottom=16;
        break;
    case 7:
        Getstringobjsize(cur_sec, rc);
        goto blah2;
    case 6:
        if(dm_k>=0x11c) f="Crash"; else f=sprname[dm_k];
        Getstringobjsize(f,rc);
blah2:
        if(rc->right>512-n) rc->right=512-n;
        if(rc->bottom>512-o) rc->bottom=512-o;
        goto blah;
    }
    rc->right+=rc->left;
    rc->bottom+=rc->top;
blah:
    if(!p)
    {
        if(rc->left < -n)
        {
            rc->left = -n;
            rc->top -= (-rc->left - n) >> 9 << 3;
            rc->right = 512 - n;
        }
        
        if(rc->top<-o)
        {
            rc->top = -o;
            rc->bottom = 512 - o;
        }
        
        if(rc->right > 512 - n)
        {
            rc->left = -n;
            rc->bottom += (rc->right + n) >> 9 << 3;
            rc->right = 512 - n;
        }
        
        if(rc->bottom > 512 - o)
        {
            rc->top    = -o;
            rc->bottom = 512 - o;
        }
    }
}
void setobj(DUNGEDIT*ed,unsigned char*map)
{
    unsigned char c=0;
    short k,l,m,n,o;
    unsigned char*rom;
    dm_x&=0xfff;
    dm_l&=0xf;
    o=map[2];
    if(dm_k>0xff) {
        if((dm_x&0xf3f)==0xf3f) goto invalobj;
        map[0]=0xfc + ((dm_x>>4)&3);
        map[1]=(dm_x<<4)+(dm_x>>8);
        map[2]=(dm_x&0xc0)|dm_k;
    } else {
        if((dm_x&0x3f)==0x3f) goto invalobj;
        if(dm_k<0xf8) {
            if((dm_l==3||!dm_l) && dm_x==0xffc) {
invalobj:
                if(ed->withfocus&10) ed->withfocus|=4;
                else MessageBox(framewnd,"You cannot place that object there.","No",MB_OK);
                getobj(map);
                return;
            }
            map[0]=(dm_x<<2)|(dm_l>>2);
            map[1]=((dm_x>>4)&0xfc)|(dm_l&3);
        } else {
            if((dm_l==12||!dm_l) && dm_x==0xffc) goto invalobj;
            if((dm_k==0xf9 && dm_l==9) || (dm_k==0xfb && dm_l==1)) c=1;
            map[0]=(dm_x<<2)|(dm_l&3);
            map[1]=((dm_x>>4)&0xfc)|(dm_l>>2);
        }
        map[2]=(unsigned char)dm_k;
    }
    if(c && !ed->ischest)
    {
        rom = ed->ew.doc->rom;
        
        m = 0;
        
        for(l=0;l<0x1f8;l+=3)
            if( is16b_neg1(rom + 0xe96e + l) )
                break;
        
        if(l!=0x1f8) {
            for(k=0;k<0x1f8;k+=3) {
                n=*(short*)(rom + 0xe96e + l);
                if(n==ed->mapnum) {
                    if(ed->chestloc[m++]>map-ed->buf) {
                        if(l<k)
                            MoveMemory(rom + 0xe96e + l, rom + 0xe96e + l + 3,
                                       k - l - 3);
                        else
                            MoveMemory(rom + 0xe96e + k + 3, rom + 0xe96e + k, l - k - 3);
setchest:
                        *(short*)(rom + 0xe96e + k) = ed->mapnum | ((dm_k == 0xfb) ? 0x8000 : 0);
                        rom[0xe970 + k] = 0;
                        break;
                    }
                }
            }
            if(k==0x1f8) {k=l; goto setchest; }
        }
    } else if(ed->ischest && (map[2]!=o || !c)) {
        for(k=0;k<ed->chestnum;k++) if(map-ed->buf==ed->chestloc[k]) break;
        for(l=0;l<0x1f8;l+=3) {
            if((*(short*)(ed->ew.doc->rom + 0xe96e + l)&0x7fff)==ed->mapnum) {
                k--;
                if(k<0) {
                    *(short*)(ed->ew.doc->rom + 0xe96e + l)=c?(ed->mapnum+((o==0xf9)?32768:0)):-1;
                    break;
                }
            }
        }
    }
    
    if(ed->withfocus&4) {SetCursor(normal_cursor);ed->withfocus&=-5;}
    
    ed->modf=1;
}

void getdoor(unsigned char const *       map,
             unsigned char const * const rom)
{
    unsigned short i = *(unsigned short*) map;
    
    dm_dl = (i & 0xf0) >> 4;
    dm_l = i >> 9;
    dm_k = i & 3;
    
    if(dm_l == 24 && !dm_k)
        dm_x = ( ldle16b_i(rom + 0x19de, dm_dl) ) >> 1;
    else
        dm_x = ( ldle16b_i(rom + 0x197e, dm_dl + dm_k * 12) ) >> 1;
}

void setdoor(unsigned char*map)
{
    dm_k&=3;
    map[0]=dm_k+(dm_dl<<4);
    map[1]=dm_l<<1;
}

unsigned char const *
Drawmap(unsigned char  const * const rom,
        unsigned short       * const nbuf,
        unsigned char  const *       map,
        DUNGEDIT             * const ed)
{
    unsigned short i;
    unsigned char l,m,o;
    short n;
    unsigned char ch = ed->chestnum;
    
    uint16_t *dm_src, *tmp;
    
    for( ; ; )
    {
        i=*(unsigned short*)map;
        if(i==0xffff) break;
        if(i==0xfff0) {
            for(;;) {
                
                map+=2;
                
                i=*(unsigned short*)(map);
                
                if(i == 0xffff)
                    goto end;
                
                getdoor(map, rom);
                dm_wr=nbuf+dm_x;
                switch(dm_k) {
                case 0:
                    switch(dm_l) {
                    case 24:
                        dm_l=42;
                        dm_rd = (uint16_t*) (rom + 0x1b52 + ((unsigned short*)(rom + 0x4e06))[dm_l]);
                        drawhole(18,nbuf);
                        dm_rd = (uint16_t*) (rom + 0x1b52 + ((unsigned short*)(rom + 0x4d9e))[dm_l]);
                        dm_x+=364;
                        drawhole(18,nbuf);
                        break;
                    case 11: case 10: case 9:
                        break;
                    case 25:
                        dm_rd = (uint16_t*) (rom + 0x22dc);
                        drawXx4(4);
                        break;
                    case 3:
                        tofrontv(dm_x+64);
                        break;
                    case 1:
                        tofront4x7(dm_x&0x783f);
                    default:
                        if(dm_l<0x20) {
                            if(dm_dl>5) {
                                dm_wr=nbuf+((((unsigned short*)(rom + 0x198a))[dm_dl])>>1);
                                
                                if(dm_l==1)
                                    tofront4x7(dm_x+256);
                                
                                dm_rd = (uint16_t*)
                                ( rom + 0x1b52 + ldle16b_i(rom + 0x4e06, dm_l) );
                                
                                m=4;
                                while(m--) {
                                    dm_wr[64]=dm_rd[0];
                                    dm_wr[128]=dm_rd[1];
                                    dm_wr[192]=dm_rd[2];
                                    dm_rd+=3;
                                    dm_wr++;
                                }
                            }
                            
                            n = 0;
                            
                            dm_rd = (uint16_t*)
                            (
                                rom + 0x1b52
                              + get_16_le_i(rom + 0x4d9e, dm_l)
                            );
                            
                            dm_wr=nbuf+dm_x;
                            m=4;
                            while(m--) {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_rd+=3;
                                dm_wr++;
                            }
                        } else {
                            n=dm_x;
                            if(dm_dl>5 && dm_l!=0x23) {
                                dm_x=(((unsigned short*)(rom + 0x198a))[dm_dl])>>1;
                                drawadd4(rom,dm_x);
                            }
                            
                            dm_x = n;
                            
                            dm_rd = (uint16_t*)
                            (
                                rom + 0x1b52
                              + get_16_le_i(rom + 0x4d9e, dm_l)
                            );
                            
                            m=4;
                            while(m--) {
                                dm_buf[dm_x]=dm_rd[0];
                                dm_buf[dm_x + 0x1040]=dm_rd[1];
                                dm_buf[dm_x + 0x1080]=dm_rd[2];
                                dm_rd+=3;
                                dm_x++;
                            }
                            dm_x-=4;
                            if(dm_l!=0x23) tofrontv(dm_x);
                        }
                    }
                    break;
                case 1:
                    switch(dm_l) {
                    case 3:
                        while(dm_x&0x7c0) {
                            dm_buf[dm_x]|=0x2000;
                            dm_buf[dm_x+1]|=0x2000;
                            dm_buf[dm_x+2]|=0x2000;
                            dm_buf[dm_x+3]|=0x2000;
                            dm_x+=64;
                        }
                        break;
                    case 11: case 10: case 9:
                        break;
                    case 5: case 6:
                        dm_rd = (uint16_t*)(rom + 0x41a8);
                        o=10;
                        l=8;
                        dm_wr-=0x103;
                        tmp=dm_wr;
                        while(l--) {
                            m=o;
                            while(m--) *(dm_wr++)=*(dm_rd++);
                            tmp+=64;
                            dm_wr=tmp;
                        }
                        break;
                    case 2: case 7: case 8:
                        tofront4x7(dm_x + 0x100);
                        dm_rd = (uint16_t*)(rom + 0x4248);
                        drawXx4(4);
                        dm_x+=188;
                        m=4;
                        while(m--) dm_buf[dm_x++]|=0x2000;
                        break;
                    case 1:
                        tofront4x7(dm_x);
                    
                    // \task falls through. Intentional?
                    default:
                        
                        dm_rd = (uint16_t*)
                        ( rom + 0x1b52 + get_16_le_i(rom + 0x4e06, dm_l) );
                        
                        if(dm_l<0x20) {
                            m=4;
                            while(m--) {
                                dm_wr[64]=dm_rd[0];
                                dm_wr[128]=dm_rd[1];
                                dm_wr[192]=dm_rd[2];
                                dm_rd+=3;
                                dm_wr++;
                            }
                        } else drawadd4(rom,dm_x);
                    }
                    break;
                case 2:
                    switch(dm_l) {
                    case 10: case 11:
                        break;
                    case 3:
                        tofronth(dm_x+1);
                        break;
                    case 1:
                        tofront5x4(dm_x&0x7fe0);
                    default:
                        if(dm_l<0x20) {
                            if(dm_dl > 5)
                            {
                                dm_wr = nbuf
                                      + ( ( ldle16b_i(rom + 0x19ba, dm_dl) ) >> 1);
                                
                                dm_rd = (uint16_t*)
                                (
                                    rom + 0x1b52
                                  + ldle16b_i(rom + 0x4ec6, dm_l)
                                );
                                
                                m = 3;
                                
                                dm_wr++;
                                
                                while(m--)
                                {
                                    dm_wr[0]   = dm_rd[0];
                                    dm_wr[64]  = dm_rd[1];
                                    dm_wr[128] = dm_rd[2];
                                    dm_wr[192] = dm_rd[3];
                                    dm_rd+=4;
                                    dm_wr++;
                                }
                            }
                            n=0;
                            
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + get_16_le_i(rom + 0x4e66, dm_l) );
                            
                            dm_wr=nbuf+dm_x;
                            m=3;
                            while(m--) {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_wr[192]=dm_rd[3];
                                dm_rd+=4;
                                dm_wr++;
                            }
                        }
                        else
                        {
                            if(dm_dl > 5)
                                drawaef0(rom,(((uint16_t*)(rom + 0x19ba))[dm_dl])>>1);
                            
                            m = 2;
                            
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + ldle16b_i(rom + 0x4e66, dm_l) );
                            
                            dm_buf[dm_x      ] = dm_rd[0];
                            dm_buf[dm_x +  64] = dm_rd[1];
                            dm_buf[dm_x + 128] = dm_rd[2];
                            dm_buf[dm_x + 192] = dm_rd[3];
                            
                            while(m--) {
                                dm_buf[dm_x + 0x1001]=dm_rd[4];
                                dm_buf[dm_x + 0x1041]=dm_rd[5];
                                dm_buf[dm_x + 0x1081]=dm_rd[6];
                                dm_buf[dm_x + 0x10c1]=dm_rd[7];
                                dm_rd+=4;
                                dm_x++;
                            }
                        }
                    }
                    break;
                case 3:
                    switch(dm_l) {
                    case 10: case 11:
                        break;
                    case 3:
                        dm_x+=2;
                        while(dm_x&0x3f) {
                            dm_buf[dm_x]|=0x2000;
                            dm_buf[dm_x+64]|=0x2000;
                            dm_buf[dm_x+128]|=0x2000;
                            dm_buf[dm_x+192]|=0x2000;
                            dm_x++;
                        }
                        break;
                    case 1:
                        tofront5x4(dm_x+4);
                    default:
                        
                        if(dm_l < 0x20)
                        {
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + ldle16b_i(rom + 0x4ec6, dm_l) );
                            
                            dm_wr++;
                            
                            m = 3;
                            
                            while(m--)
                            {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_wr[192]=dm_rd[3];
                                dm_rd+=4;
                                dm_wr++;
                            }
                        }
                        else
                            drawaef0(rom, dm_x);
                    }
                }
            }
        }
        
        getobj(map);
        
        map += 3;
        
        dm_wr=nbuf+dm_x;
        
        if(dm_k >= 0x100)
        {
            dm_src = dm_rd = (uint16_t*)
            ( rom + 0x1b52 + ldle16b_i(rom + 0x81f0, dm_k) );
            
            switch(dm_k - 0x100) {
            case 59:
//              dm_rd=(short*)(rom + 0x2ce2);
                goto d43;
            case 58:
//              dm_rd=(short*)(rom + 0x2cca);
                goto d43;
            case 57:
//              dm_rd=(short*)(rom + 0x2cb2);
                goto d43;
            case 56:
//              dm_rd=(short*)(rom + 0x2c9a);
d43:
                drawXx3(4);
                break;
            case 45: case 46: case 47:
            case 50: case 51:
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            case 36: case 37: case 41: case 28:
                drawXx4(4);
                break;
            case 48: case 49:
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
                drawXx4bp(4);
                break;
            case 16: case 17: case 18: case 19:
                drawXx4bp(3);
                break;
            case 20: case 21: case 22: case 23:
                drawXx3bp(4);
                break;
            case 52:
            case 24: case 25: case 26: case 27: case 30: case 31: case 32:
            case 39:
                draw2x2();
                break;
            case 29: case 33: case 38: case 43:
                drawXx3(2);
                break;
            case 34: case 40:
                dm_l=5;
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[1]=dm_rd[1];
                    dm_wr[2]=dm_rd[2];
                    dm_wr[3]=dm_rd[3];
                    dm_rd+=4;
                    dm_wr+=64;
                }
                break;
            case 35:
                drawXx3(4);
                break;
            case 42: case 53:
                dm_l=1;
                draw4x2(1);
                break;
            case 62:
            case 44:
                drawXx3(6);
                break;
            case 54:
                
                dm_rd = (uint16_t*) (rom + 0x2c5a);
                
                dm_l = 1;
                
                goto case99;
            
            case 55:
                drawXx4(10);
                break;
            case 60:
                n=*dm_rd;
                dm_l=6;
                while(dm_l--) {
                    dm_buf[dm_x+1]=dm_buf[dm_x+5]=dm_buf[dm_x+9]=
                    dm_buf[dm_x+15]=dm_buf[dm_x+19]=dm_buf[dm_x+23]=
                    (dm_buf[dm_x]=dm_buf[dm_x+4]=dm_buf[dm_x+8]=
                    dm_buf[dm_x+14]=dm_buf[dm_x+18]=dm_buf[dm_x+22]=dm_rd[0])|0x4000;
                    dm_buf[dm_x+3]=dm_buf[dm_x+7]=dm_buf[dm_x+17]=
                    dm_buf[dm_x+21]=
                    (dm_buf[dm_x+2]=dm_buf[dm_x+6]=dm_buf[dm_x+16]=
                    dm_buf[dm_x+20]=dm_rd[6])|0x4000;
                    dm_rd++;
                    dm_x+=64;
                }
                dm_rd+=6;
                dm_wr+=10;
                drawXx3(4);
                break;
            case 61:
                drawXx3(3);
                break;
            case 63:
                dm_l=8;
                while(dm_l--) {
                    dm_buf[dm_x]=dm_rd[0];
                    dm_buf[dm_x + 0x40]=dm_rd[1];
                    dm_buf[dm_x + 0x80]=dm_rd[2];
                    dm_buf[dm_x + 0xc0]=dm_rd[3];
                    dm_buf[dm_x + 0x100]=dm_rd[4];
                    dm_buf[dm_x + 0x140]=dm_rd[5];
                    dm_buf[dm_x + 0x180]=dm_rd[6];
                    dm_rd+=7;
                }
                break;
            }
        }
        else
        {
            if(dm_k >= 0xf8)
            {
                dm_k &= 7;
                dm_k <<= 4;
                dm_k |= dm_l;
                
                dm_src = dm_rd = (uint16_t*)
                (rom + 0x1b52 + ldle16b_i(rom + 0x84f0, dm_k) );
                
                switch(dm_k)
                {
                case 0:
                    m=3;
rows4:
                    while(m--) {
                        dm_wr[0]=dm_rd[0];
                        dm_wr[1]=dm_rd[1];
                        dm_wr[2]=dm_rd[2];
                        dm_wr[3]=dm_rd[3];
                        dm_rd+=4;
                        dm_wr+=64;
                    }
                    break;
                case 1:
                    m=5;
                    goto rows4;
                case 2:
                    m=7;
                    goto rows4;
                case 3: case 14:
                case 4: case 5: case 6: case 7: case 8: case 9:
                case 10: case 11: case 12: case 15:
                    
                    dm_wr[0]=dm_rd[0];
                    
                    break;
                case 13:
                    dm_rd = (uint16_t*) (rom + 0x2fda);
                case 23:
                    m=5;
                    tmp=dm_wr;
                    while(m--) {
                        // \task Endianness issues with derived pointers like
                        // this. These will be tough to track down.
                        dm_wr[2]=dm_wr[9]=dm_rd[1];
                        dm_wr[73]=(dm_wr[66]=dm_rd[2])|0x4000;
                        dm_wr[137]=(dm_wr[130]=dm_rd[4])|0x4000;
                        dm_wr[201]=(dm_wr[194]=dm_rd[5])|0x4000;
                        dm_wr++;
                    }
                    dm_wr=tmp;
                    dm_wr[15]=(dm_wr[0]=dm_rd[0])|0x4000;
                    dm_wr[14]=dm_wr[8]=dm_wr[7]=dm_wr[1]=dm_rd[1];
                    dm_wr[142]=(dm_wr[129]=dm_rd[3])|0x4000;
                    break;
                case 25:
                    if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
                case 16: case 17: case 19: case 34: case 35: case 36:
                case 37:
                case 22: case 24: case 26: case 47: case 48: case 62:
                case 63: case 64: case 65: case 66: case 67: case 68: case 69:
                case 70: case 73: case 74: case 79: case 80: case 81: case 82:
                case 83: case 86: case 87: case 88: case 89: case 94:
                case 95: case 99: case 100: case 101: case 117: case 124:
                case 125: case 126:
                    draw2x2();
                    break;
                case 18:
                    m=3;
                    while(m--) {
                        dm_wr[384]=dm_wr[192]=dm_wr[0]=dm_rd[0];
                        dm_wr[448]=dm_wr[256]=dm_wr[64]=dm_rd[1];
                        dm_wr+=2;
                    }
                    break;
                case 20:
                    drawXx3(4);
                    break;
                case 21: case 114:
                    o=10;
                    l=8;
                    tmp=dm_wr;
                    while(l--) {
                        m=o;
                        while(m--) *(dm_wr++)=*(dm_rd++);
                        tmp+=64;
                        dm_wr=tmp;
                    }
                    break;
                case 27: case 28: case 30: case 31: case 32:
                case 33:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_buf[dm_x]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_buf[dm_x + 0x40]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_buf[dm_x + 0x80]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_buf[dm_x + 0xc0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 112:
                    drawXx4(4);
                    dm_wr+=124;
                    dm_rd-=16;
                    drawXx4(4);
                    dm_wr+=252;
                    drawXx4(4);
                    break;
                case 113:
                    drawXx4(4);
                    drawXx4(4);
                    dm_wr+=248;
                    drawXx4(4);
                    drawXx4(4);
                    break;
                case 120:
                    drawXx4(4);
                    dm_wr+=250;
                    drawXx4(4);
                    dm_rd-=16;
                case 29: case 102: case 107:
                case 122:
                    drawXx4(4);
                    break;
                case 38: case 39:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_buf[dm_x]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 40: case 41:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_buf[dm_x + 0xc0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 42:
                    dm_rd+=0xb6e;
                    dm_x=0x20a;
                    draw12x12();
                    dm_rd-=3;
                    dm_x=0x22a;
                    draw12x12();
                    dm_rd--;
                    dm_x=0xa0a;
                    draw12x12();
                    dm_rd-=5;
                    dm_x=0xa2a;
                    draw12x12();
                    break;
                case 44:
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    dm_wr+=124;
                    dm_rd+=4;
                    draw2x2();
                    dm_rd+=4;
                case 43:
                    draw2x2();
                    break;
                case 45:
                    dm_rd+=0xda5;
                    m=14;
                    while(m--) {
                        dm_wr[13]=(dm_wr[0]=dm_rd[0])|0x4000;
                        dm_wr[12]=dm_wr[11]=
                        (dm_wr[2]=dm_wr[1]=dm_rd[14])^0x4000;
                        dm_wr[10]=(dm_wr[3]=dm_rd[28])^0x4000;
                        dm_wr[9]=(dm_wr[4]=dm_rd[42])^0x4000;
                        dm_wr[8]=(dm_wr[5]=dm_rd[56])^0x4000;
                        dm_wr[7]=(dm_wr[6]=dm_rd[70])^0x4000;
                        dm_wr+=64;
                        dm_rd++;
                    }
                    break;
                case 46:
                    n=dm_x;
                    m=6;
                    dm_rd+=0xdf9;
                    while(m--) {
                        dm_buf[dm_x + 0x107]=dm_buf[dm_x + 0x10d]=dm_buf[dm_x + 0x113]=dm_rd[0];
                        dm_buf[dm_x + 0x147]=dm_buf[dm_x + 0x14d]=dm_buf[dm_x + 0x153]=dm_rd[1];
                        dm_buf[dm_x + 0x187]=dm_buf[dm_x + 0x18d]=dm_buf[dm_x + 0x193]=dm_rd[2];
                        dm_buf[dm_x + 0x1c7]=dm_buf[dm_x + 0x1cd]=dm_buf[dm_x + 0x1d3]=dm_rd[3];
                        dm_rd+=4;
                        dm_x++;
                    }
                    m=5;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x  + 0x117]=dm_buf[dm_x + 0x158]=dm_buf[dm_x + 0x199]=
                        dm_buf[dm_x  + 0x1da]=dm_buf[dm_x + 0x21b]=dm_buf[dm_x + 0x25c]=dm_buf[dm_x + 0x29d]=
                        (dm_buf[dm_x + 0x282]=dm_buf[dm_x + 0x243]=dm_buf[dm_x + 0x204]=
                        dm_buf[dm_x  + 0x1c5]=dm_buf[dm_x + 0x186]=dm_buf[dm_x + 0x147]=dm_buf[dm_x + 0x108]=dm_rd[0])|0x4000;
                        dm_rd++;
                        dm_x+=64;
                    }
                    m=6;
                    dm_x=n;
                    while(m--) {
                         dm_buf[dm_x + 0x2dd] = dm_buf[dm_x + 0x45d]=dm_buf[dm_x + 0x5dd]=
                        (dm_buf[dm_x + 0x2c2] = dm_buf[dm_x + 0x442]=dm_buf[dm_x + 0x5c2]=dm_rd[0])|0x4000;
                         dm_buf[dm_x + 0x2dc] = dm_buf[dm_x + 0x45c]=dm_buf[dm_x + 0x5dc]=
                        (dm_buf[dm_x + 0x2c3] = dm_buf[dm_x + 0x443]=dm_buf[dm_x + 0x5c3]=dm_rd[1])|0x4000;
                         dm_buf[dm_x + 0x2db] = dm_buf[dm_x + 0x45b]=dm_buf[dm_x + 0x5db]=
                        (dm_buf[dm_x + 0x2c4] = dm_buf[dm_x + 0x444]=dm_buf[dm_x + 0x5c4]=dm_rd[2])|0x4000;
                         dm_buf[dm_x + 0x2da] = dm_buf[dm_x + 0x45a]=dm_buf[dm_x + 0x5da]=
                        (dm_buf[dm_x + 0x2c5] = dm_buf[dm_x + 0x445]=dm_buf[dm_x + 0x5c5]=dm_rd[3])|0x4000;
                        dm_rd+=4;
                        dm_x+=64;
                    }
                    m=6;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x24c]=dm_buf[dm_x + 0x252]=dm_rd[0];
                        dm_buf[dm_x + 0x28c]=dm_buf[dm_x + 0x292]=dm_rd[6];
                        dm_rd++;
                        dm_x++;
                    }
                    dm_rd+=6;
                    m=6;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x387]=dm_buf[dm_x + 0x507]=dm_rd[0];
                        dm_buf[dm_x + 0x388]=dm_buf[dm_x + 0x508]=dm_rd[1];
                        dm_rd+=2;
                        dm_x+=64;
                    }
                    m=5;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x247]=dm_rd[0];
                        dm_buf[dm_x + 0x287]=dm_rd[1];
                        dm_buf[dm_x + 0x2c7]=dm_rd[2];
                        dm_buf[dm_x + 0x307]=dm_rd[3];
                        dm_buf[dm_x + 0x347]=dm_rd[4];
                        dm_rd+=5;
                        dm_x++;
                    }
                    m=4;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x70e]|=0x2000;
                        dm_buf[dm_x + 0x74e]|=0x2000;
                        dm_x++;
                    }
                    break;
                case 49:
                    if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
                case 50: case 103: case 104: case 121:
                    drawXx3(4);
                    break;
                case 51: case 72:
                    drawXx4(4);
                    break;
                case 52: case 53: case 56: case 57:
                    draw3x2();
                    break;
                case 54: case 55: case 77: case 93:
                    drawXx3(6);
                    break;
                case 58: case 59:
                    drawXx3(4);
                    dm_wr+=188;
                    drawXx3(4);
                    break;
                case 78:
                    drawXx3(4);
                    break;
                case 60: case 61: case 92:
                    drawXx4(6);
                    break;
                case 71:
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    dm_rd+=4;
                    dm_wr+=124;
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    break;
                case 75: case 118: case 119:
                    drawXx3(8);
                    break;
                case 76:
                    l=8;
                    while(l--) {
                        m=6;
                        while(m--) {
                            dm_wr[0]=dm_rd[0];
                            dm_wr++;
                            dm_rd++;
                        }
                        dm_wr+=58;
                    }
                    break;
                case 84:
                    m=6;
                    tmp=dm_wr;
                    while(m--) {
                        dm_wr[1]=dm_wr[2]=dm_wr[65]=dm_wr[66]=dm_rd[0];
                        dm_wr[130]=(dm_wr[129]=dm_rd[1])|0x4000;
                        dm_wr+=2;
                    }
                    dm_wr=tmp;
                    m=3;
                    while(m--) {
                        dm_wr[0xc1]=dm_wr[0xc3]=dm_wr[0xcb]=dm_wr[0xcd]=
                        (dm_wr[0xc0]=dm_wr[0xc2]=dm_wr[0xca]=dm_wr[0xcc]=dm_rd[2])|0x4000;
                        dm_wr[0xc5]=dm_wr[0xc7]=dm_wr[0xc9]=
                        (dm_wr[0xc4]=dm_wr[0xc6]=dm_wr[0xc8]=dm_rd[5])|0x4000;
                        dm_rd++;
                        dm_wr+=64;
                    }
                    dm_wr=tmp;
                    dm_wr[77]=dm_wr[13]=(dm_wr[64]=dm_wr[0]=dm_rd[5])|0x4000;
                    dm_wr[141]=(dm_wr[128]=dm_rd[6])|0x4000;
                    m=4;
                    dm_wr=tmp;
                    dm_rd=dm_src;
                    while(m--) {
                        dm_wr[0x28a]=(dm_wr[0x283]=dm_rd[10])|0x4000;
                        dm_wr[0x289]=(dm_wr[0x284]=dm_rd[14])|0x4000;
                        dm_wr[0x288]=(dm_wr[0x285]=dm_rd[18])|0x4000;
                        dm_wr[0x287]=(dm_wr[0x286]=dm_rd[22])|0x4000;
                        dm_rd++;
                        dm_wr+=64;
                    }
                    break;
                case 85: case 91:
                    m=3;
                    dm_wr[0]=dm_rd[0];
                    dm_wr[1]=dm_rd[1];
                    dm_wr[2]=dm_rd[2];
                    while(m--) {
                        dm_wr[64]=dm_rd[3];
                        dm_wr[65]=dm_rd[4];
                        dm_wr[66]=dm_rd[5];
                        dm_wr+=64;
                    }
                    dm_wr[64]=dm_rd[6];
                    dm_wr[65]=dm_rd[7];
                    dm_wr[66]=dm_rd[8];
                    break;
                case 90:
                    m=2;
                    while(m--) {
                        dm_wr[0]=dm_rd[0];
                        dm_wr[1]=dm_rd[1];
                        dm_wr[2]=dm_rd[2];
                        dm_wr[3]=dm_rd[3];
                        dm_rd+=4;
                        dm_wr+=64;
                    }
                    break;
                case 96: case 97:
                    drawXx3(3);
                    dm_wr+=189;
                    drawXx3(3);
                    break;
                case 98:
                    m=22;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_rd[3];
                        dm_buf[dm_x + 0x1100]=dm_rd[4];
                        dm_buf[dm_x + 0x1140]=dm_rd[5];
                        dm_buf[dm_x + 0x1180]=dm_rd[6];
                        dm_buf[dm_x + 0x11c0]=dm_rd[7];
                        dm_buf[dm_x + 0x1200]=dm_rd[8];
                        dm_buf[dm_x + 0x1240]=dm_rd[9];
                        dm_buf[dm_x + 0x1280]=dm_rd[10];
                        dm_rd+=11;
                        dm_x++;
                    }
                    dm_x-=22;
                    m=3;
                    while(m--) {
                        dm_buf[0x12c9 + dm_x] = dm_rd[0];
                        dm_buf[0x1309 + dm_x] = dm_rd[3];
                        dm_rd++;
                        dm_x++;
                    }
                    break;
                case 105: case 106: case 110: case 111:
                    drawXx4(3);
                    break;
                case 109: case 108:
                    drawXx3(4);
                    break;
                case 115:
                    fill4x2(rom,nbuf,dm_rd + 0x70);
                    break;
                case 123:
                    tmp=dm_wr;
                    draw4x4X(5);
                    tmp+=256;
                    dm_wr=tmp;
                    draw4x4X(5);
                    break;
                case 116:
                    drawXx4(4);
                    drawXx4(4);
                    dm_wr+=248;
                    drawXx4(4);
                    drawXx4(4);
                    break;
                }
                continue;
            }
            
            dm_src = dm_rd = (uint16_t*)
            ( rom + 0x1b52 + ldle16b_i(rom + 0x8000, dm_k) );
            
            switch(dm_k)
            {
            
            case 0:
                len1();
                while(dm_l--) draw2x2();
                break;
            
            case 1: case 2:
                len2();
                while(dm_l--) drawXx4(2),dm_rd=dm_src;
                break;
            
            case 3: case 4:
                dm_l++;
                while(dm_l--) {
                    dm_buf[0x1000 + dm_x] = dm_buf[     + dm_x] = dm_rd[0];
                    dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
                    dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
                    dm_buf[0x10c0 + dm_x] = dm_buf[0xc0 + dm_x] = dm_rd[3];
                    dm_buf[0x1001 + dm_x] = dm_buf[0x1  + dm_x] = dm_rd[4];
                    dm_buf[0x1041 + dm_x] = dm_buf[0x41 + dm_x] = dm_rd[5];
                    dm_buf[0x1081 + dm_x] = dm_buf[0x81 + dm_x] = dm_rd[6];
                    dm_buf[0x10c1 + dm_x] = dm_buf[0xc1 + dm_x] = dm_rd[7];
                    
                    dm_x += 2;
                }
                break;
            case 5: case 6:
                dm_l++;
                while(dm_l--) drawXx4(2),dm_rd=dm_src,dm_wr+=4;
                break;
            case 7: case 8:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 9: case 12: case 13: case 16: case 17: case 20:
                dm_l+=6;
                while(dm_l--) draw1x5(),dm_wr-=63;
                break;
            case 21: case 24: case 25: case 28: case 29: case 32:
                n=-63;
case25:
                dm_l+=6;
                while(dm_l--) {
                    dm_buf[dm_x + 0x1000]=dm_buf[dm_x]=dm_rd[0];
                    dm_buf[dm_x + 0x1040]=dm_buf[dm_x + 0x40]=dm_rd[1];
                    dm_buf[dm_x + 0x1080]=dm_buf[dm_x + 0x80]=dm_rd[2];
                    dm_buf[dm_x + 0x10c0]=dm_buf[dm_x + 0xc0]=dm_rd[3];
                    dm_buf[dm_x + 0x1100]=dm_buf[dm_x + 0x100]=dm_rd[4];
                    dm_x+=n;
                }
                break;
            case 23: case 26: case 27: case 30: case 31:
                n=65;
                goto case25;
            case 10: case 11: case 14: case 15: case 18: case 19: case 22:
                dm_l+=6;
                while(dm_l--) draw1x5(),dm_wr+=65;
                break;
            case 33:
                dm_l=dm_l*2+1;
                drawXx3(2);
                while(dm_l--) dm_rd-=3,drawXx3(1);
                drawXx3(1);
                break;
            case 34:
                dm_l+=2;
case34b:
                if(*dm_wr!=0xe2) *dm_wr=*dm_rd;
case34:
                dm_wr++;
                dm_rd++;
                while(dm_l--) *(dm_wr++)=*dm_rd;
                dm_rd++;
                *dm_wr=*dm_rd;
                break;
            case 35: case 36: case 37: case 38: case 39: case 40: case 41:
            case 42: case 43: case 44: case 45: case 46: case 179: case 180:
                dm_l++;
                n=(*dm_wr)&0x3ff;
                if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc))
                    *dm_wr=*dm_rd;
                goto case34;
            case 47:
                dm_l+=10;
                n=*(dm_rd++);
                if(((*dm_wr)&0x3ff)!=0xe2) draw8fec(n);
                dm_wr+=2;
                dm_rd+=2;
                while(dm_l--) {
                    dm_wr[0]=*dm_rd;
                    dm_wr[64]=n;
                    dm_wr++;
                }
                dm_rd++;
                draw8fec(n);
                break;
            case 48:
                dm_l+=10;
                n=*(dm_rd++);
                if(((dm_wr[64])&0x3ff)!=0xe2) draw9030(n);
                dm_wr+=2;
                dm_rd+=2;
                while(dm_l--) {
                    dm_wr[0]=n;
                    dm_wr[64]=*dm_rd;
                    dm_wr++;
                }
                dm_rd++;
                draw9030(n);
                break;
            case 51:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4);
                break;
            case 52:
                dm_l+=4;
                n=*dm_rd;
                while(dm_l--) *(dm_wr++)=n;
                break;
            case 53:
                break;
            case 54: case 55:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=2;
                break;
            case 56:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx3(2),dm_wr+=2;
                break;
            case 61:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
                break;
            case 57:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
                break;
            case 58: case 59:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx3(4),dm_wr+=4;
                break;
            case 60:
                dm_l++;
                while(dm_l--) {
                    dm_rd=dm_src,draw2x2();
                    dm_wr+=0x17e;
                    dm_rd+=4;
                    draw2x2();
                    dm_wr-=0x17e;
                }
                break;
            case 62: case 75:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=12;
                break;
            case 63: case 64: case 65: case 66: case 67: case 68: case 69:
            case 70:
                dm_l++;
                n=(*dm_wr)&0x3ff;
                if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc)) *dm_wr=*dm_rd;
                dm_rd++;
                dm_wr++;
                n=*(dm_rd++);
                while(dm_l--) *(dm_wr++)=n;
                *dm_wr=*dm_rd;
                break;
            case 71:
                dm_l++;
                dm_l<<=1;
                draw1x5();
                dm_rd+=5;
                dm_wr++;
                while(dm_l--) draw1x5(),dm_wr++;
                dm_rd+=5;
                draw1x5();
                break;
            case 72:
                dm_l++;
                dm_l<<=1;
                drawXx3(1);
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[64]=dm_rd[1];
                    dm_wr[128]=dm_rd[2];
                    dm_wr++;
                }
                dm_rd+=3;
                drawXx3(1);
                break;
            case 73: case 74:
                dm_l++;
                draw4x2(4);
                break;
            case 76:
                dm_l++;
                dm_l<<=1;
                drawXx3(1);
                while(dm_l--) drawXx3(1),dm_rd-=3;
                dm_rd+=3;
                drawXx3(1);
                break;
            case 77: case 78: case 79:
                dm_l++;
                drawXx4(1);
                while(dm_l--) drawXx4(2),dm_rd-=8;
                dm_rd+=8;
                drawXx4(1);
                break;
            case 80:
                dm_l+=2;
                n=*dm_rd;
                while(dm_l--) *(dm_wr++)=n;
                break;
            case 81: case 82: case 91: case 92:
                drawXx3(2);
                while(dm_l--) drawXx3(2),dm_rd-=6;
                dm_rd+=6;
                drawXx3(2);
                break;
            case 83:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 85: case 86:
                dm_l++;
                draw4x2(12);
                break;
            case 93:
                dm_l+=2;
                drawXx3(2);
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[64]=dm_rd[1];
                    dm_wr[128]=dm_rd[2];
                    dm_wr++;
                }
                dm_rd+=3;
                drawXx3(2);
                break;
            case 94:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=2;
                break;
            case 95:
                dm_l+=21;
                goto case34b;
            case 96: case 146: case 147:
                len1();
                while(dm_l--) draw2x2(),dm_wr+=126;
                break;
            case 97: case 98:
                len2();
                draw4x2(0x80);
                break;
            case 99: case 100:
                dm_l++;
case99:
                while(dm_l--) {
                    dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
                    dm_buf[0x1001 + dm_x] = dm_buf[1    + dm_x] = dm_rd[1];
                    dm_buf[0x1002 + dm_x] = dm_buf[2    + dm_x] = dm_rd[2];
                    dm_buf[0x1003 + dm_x] = dm_buf[3    + dm_x] = dm_rd[3];
                    dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[4];
                    dm_buf[0x1041 + dm_x] = dm_buf[0x41 + dm_x] = dm_rd[5];
                    dm_buf[0x1042 + dm_x] = dm_buf[0x42 + dm_x] = dm_rd[6];
                    dm_buf[0x1043 + dm_x] = dm_buf[0x43 + dm_x] = dm_rd[7];
                    
                    dm_x+=128;
                }
                break;
            case 101: case 102:
                dm_l++;
                draw4x2(0x180);
                break;
            case 103: case 104:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=126;
                break;
            case 105:
                dm_l+=2;
                if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
                dm_wr+=64;
                while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
                *dm_wr=dm_rd[2];
                break;
            case 106: case 107: case 121: case 122:
                dm_l++;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 108:
                dm_l+=10;
                n=*(dm_rd++);
                if((*dm_wr&0x3ff)!=0xe3) draw9078(n);
                dm_rd+=2;
                dm_wr+=128;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr[1]=n,dm_wr+=64;
                dm_rd++;
                draw9078(n);
                break;
            case 109:
                dm_l+=10;
                n=*(dm_rd++);
                if((dm_wr[1]&0x3ff)!=0xe3) draw90c2(n);
                dm_rd+=2;
                dm_wr+=128;
                while(dm_l--) *dm_wr=n,dm_wr[1]=*dm_rd,dm_wr+=64;
                dm_rd++;
                draw90c2(n);
                break;
            case 112:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=252;
                break;
            case 113:
                dm_l+=4;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 115: case 116:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0x17c;
                break;
            case 117:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
                break;
            case 118: case 119:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
                break;
            case 120: case 123:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x37e;
                break;
            case 124:
                dm_l+=2;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 125:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x7e;
                break;
            case 127: case 128:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x2fe;
                break;
            case 129: case 130: case 131: case 132:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
                break;
            case 133: case 134:
                draw3x2(),dm_wr+=128,dm_rd+=6;
                while(dm_l--) draw3x2(),dm_wr+=128;
                dm_rd+=6;
                draw3x2();
                break;
            case 135:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
                break;
            case 136:
                dm_l++;
                draw2x2(),dm_wr+=0x7e;
                dm_rd+=4;
                while(dm_l--) dm_wr[0]=dm_rd[0],dm_wr[1]=dm_rd[1],dm_wr+=64;
                dm_rd+=2;
                drawXx3(2);
                break;
            case 137:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0xfe;
                break;
            case 138:
                dm_l+=21;
                if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
                dm_wr+=64;
                while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
                *dm_wr=dm_rd[2];
                break;
            case 140: case 139:
                dm_l+=8;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 141: case 142:
                dm_l++;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 143:
                dm_l+=2;
                dm_l<<=1;
                dm_wr[0]=dm_rd[0];
                dm_wr[1]=dm_rd[1];
                while(dm_l--) dm_wr[64]=dm_rd[2],dm_wr[65]=dm_rd[3],dm_wr+=64;
                break;
            case 144: case 145:
                len2();
                draw4x2(0x80);
                break;
            case 148:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0xfc;
                break;
            case 149: case 150:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=0x7e;
                break;
            case 160: case 169:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    n=dm_l;
                    drawtr();
                    dm_wr+=64;
                }
                break;
            case 161: case 166: case 170:
                dm_l+=4;
                m=0;
                while(dm_l--) {
                    m++;
                    n=m;
                    for(l=0;l<n;l++) dm_wr[l]=*dm_rd;
                    dm_wr+=64;
                }
                break;
            case 162: case 167: case 171:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    drawtr();
                    dm_wr+=65;
                }
                break;
            case 163: case 168: case 172:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    drawtr();
                    dm_wr-=63;
                }
                break;
            case 164:
                dm_l+=4;
                l=dm_l;
                tmp=dm_wr;
                
                while(l--)
                {
                    drawtr();
                    dm_src = dm_wr;
                    dm_wr += 64;
                }
                
                // \task We can track down endianness issues by searching
                // for like, "dm_rd[" and "dm_wr" perhaps.
                dm_rd = (uint16_t*) (rom + 0x218e);
                
                m = 2;
                
                dm_wr = tmp;
                
                while(m--) {
                    l=dm_l-2;
                    dm_wr[0]=dm_rd[0];
                    while(l--) dm_wr[1]=dm_rd[1],dm_wr++;
                    dm_wr[1]=dm_rd[2];
                    dm_rd+=3;
                    dm_wr=dm_src;
                }
                dm_wr=tmp+64;
                m=dm_l-1;
                l=m-1;
                dm_wr+=m;
                dm_src=dm_wr;
                dm_wr=tmp+64;
                m=2;
                
                dm_rd = (uint16_t*) (rom + 0x219a);
                
                while(m--) {
                    n=l;
                    while(n--)
                        // \task This look an error in the context of using
                        // comma delimited statements. What gets assigned to
                        // dm_wr at the end?
                        *dm_wr = *dm_rd, dm_wr += 64;
                    dm_wr=dm_src;
                    dm_rd++;
                }
                break;
            case 165:
                dm_l+=4;
                for(;dm_l;dm_l--) drawtr(),dm_wr+=64;
                break;
            case 176: case 177:
                dm_l+=8;
                drawtr();
                break;
            
            case 178:
                
                dm_l++;
                
                while(dm_l--)
                {
                    dm_rd = dm_src;
                    
                    drawXx4(4);
                }
                
                break;
            
            case 181:
                dm_l++; goto c182;
            case 182: case 183:
                len2();
c182:
                while(dm_l--) drawXx4(2),dm_rd-=8;
                break;
            case 184: // yeah!
                len2();
                while(dm_l--) draw2x2();
                break;
            case 185:
                len1();
                while(dm_l--) draw2x2();
                break;
            case 186:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4);
                break;
            case 187:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=2;
                break;
            case 188: case 189:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 192: case 194:
                
                l = (dm_l & 3) + 1;
                
                dm_l >>= 2;
                dm_l++;
                
                while(l--)
                {
                    m=dm_l;
                    dm_src=dm_wr;
                    
                    while(m--)
                    {
                        dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[3]=
                        dm_wr[64]=dm_wr[65]=dm_wr[66]=dm_wr[67]=
                        dm_wr[128]=dm_wr[129]=dm_wr[130]=dm_wr[131]=
                        dm_wr[192]=dm_wr[193]=dm_wr[194]=dm_wr[195] = dm_rd[0];
                        
                        dm_wr += 4;
                    }
                    
                    dm_wr = dm_src + 256;
                }
                break;
            case 193:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l+=4;
                tmp=dm_wr;
                m=dm_l;
                drawXx3(3);
                while(m--) {
                    drawXx3(2);
                    dm_rd-=6;
                }
                dm_rd+=6;
                drawXx3(3);
                dm_src=dm_rd;
                tmp+=0xc0;
                o=l;
                while(o--) {
                    dm_wr=tmp;
                    dm_rd=dm_src;
                    m=dm_l;
                    draw3x2();
                    dm_rd+=6;
                    dm_wr+=3;
                    while(m--) draw2x2();
                    dm_rd+=4;
                    draw3x2();
                    tmp+=0x80;
                }
                dm_rd+=6;
                dm_wr=tmp;
                m=dm_l;
                drawXx3(3);
                while(m--) {
                    drawXx3(2);
                    dm_rd-=6;
                }
                dm_rd+=6;
                drawXx3(3);
                tmp-=(1+l)<<6;
                dm_l+=2;
                dm_wr=tmp+dm_l;
                
                dm_rd = (uint16_t*) (rom + 0x20e2);
                
                draw2x2();
                
                break;
            case 195: case 215:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l++;
                while(l--) {
                    tmp=dm_wr;
                    m=dm_l;
                    while(m--) {
                        dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[64]=dm_wr[65]=dm_wr[66]=
                        dm_wr[128]=dm_wr[129]=dm_wr[130]=*dm_rd;
                        dm_wr+=3;
                    }
                    dm_wr=tmp + 0xc0;
                }
                break;
            case 196:
                dm_rd+=(ed->buf[0]&15)<<3;
            case 200: case 197: case 198: case 199: case 201: case 202:
                l=(dm_l&3);
                dm_l>>=2;
                dm_l++;
                l++;
                goto grid4x4;
            case 205:
                l=dm_l&3;
                dm_l>>=2;
                m=(unsigned char)(((short*)(rom + 0x1b0a))[dm_l]);
                n=((short*)(rom + 0x1b12))[l];
                dm_src=dm_wr;
                dm_wr-=n;
                
                tmp = dm_wr;
                
                dm_rd = (uint16_t*) (rom + 0x1f2a);
                
                while(n--) {
                    dm_wr[0]=dm_rd[0];
                    o=(m<<1)+4;
                    while(o--) {
                        dm_wr[64]=dm_rd[1];
                        dm_wr+=64;
                    }
                    dm_wr[64]=dm_rd[2];
                    tmp++;
                    dm_wr=tmp;
                }
                dm_wr=dm_src;
                dm_x=dm_src-nbuf-1;
                o=dm_x&31;
                if(dm_x&32) o|=0x200;
                o|=0x800;
                dm_wr = dm_src;
                dm_rd = (uint16_t*) (rom + 0x227c);
                
                drawXx3(3);
                dm_wr += 0xbd;
                
                while(m--)
                    draw3x2(), dm_wr += 128;
                
                dm_rd+=6;
                drawXx3(3);
                break;
            case 206:
                
                dm_rd = (uint16_t*) (rom + 0x22ac);
                
                tmp=dm_wr;
                drawXx3(3);
                l=dm_l&3;
                dm_l>>=2;
                o=m=(unsigned char)(((short*)(rom + 0x1b0a))[dm_l]);
                n=((short*)(rom + 0x1b12))[l];
                dm_wr=tmp + 0xc0;
                while(o--) draw3x2(),dm_wr+=128;
                dm_rd+=6;
                drawXx3(3);
                tmp+=3;
                
                dm_rd = (uint16_t*) (rom + 0x1f2a);
                
                m <<= 1;
                m+=4;
                while(n--)
                {
                    dm_wr = tmp;
                    dm_wr[0] = dm_rd[0];
                    
                    o = m;
                    
                    while(o--)
                    {
                        dm_wr[64] = dm_rd[1];
                        dm_wr += 64;
                    }
                    
                    dm_wr[64] = dm_rd[2];
                    tmp++;
                }
                break;
            
            case 209: case 210: case 217: case 223: case 224: case 225:
            case 226: case 227: case 228: case 229: case 230: case 231:
            case 232:
                l=(dm_l&3);
                dm_l>>=2;
                dm_l++;
                l++;
grid4x4:
                while(l--) {
                    tmp=dm_wr;
                    draw4x4X(dm_l);
                    dm_wr=tmp + 0x100;
                }
                break;
            case 216: case 218:
                l=dm_l&3;
                dm_l>>=2;
                dm_l=(unsigned char)(((unsigned short*)(rom + 0x1b3a))[dm_l]);
                l=(unsigned char)(((unsigned short*)(rom + 0x1b3a))[l]);
                dm_rd=(unsigned short*)(rom + 0x1c62);
                goto grid4x4;
                break;
            case 219:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l++;
                dm_rd+=(ed->buf[0]&240)>>1;
                goto grid4x4;
                break;
            case 220:
                l=((dm_l&3)<<1)+5;
                dm_l=(dm_l>>2)+1;
                dm_tmp=dm_wr;
                while(l--) {
                    draw975c();
                }
                dm_rd++;
                draw975c();
                dm_rd++;
                draw975c();
                break;
            case 221:
                
                l = ((dm_l&3)<<1)+1;
                dm_l = (dm_l>>2)+1;
                tmp = dm_wr;
                
                draw93ff();
                
                dm_rd += 4;
                
                while(l--)
                {
                    draw93ff();
                }
                
                dm_rd+=4;
                
                draw93ff();
                
                dm_rd+=4;
                
                draw93ff();
                
                break;
            case 222: // Yup. It's 222!
                
                l=(dm_l&3)+1;
                dm_l=(dm_l>>2)+1;
                while(l--) {
                    m=dm_l;
                    tmp=dm_wr;
                    while(m--) draw2x2();
                    tmp+=128;
                    dm_wr=tmp;
                }
                
                break;
            }
        }
    }
end:
    ed->chestnum = ch;
    return map+2;
}

// =============================================================================

const static char *mus_str[]={
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
void Modifywaves(FDOC*doc,int es)
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
                    put_16_le_i(trtbl->buf,
                                n,
                                Savescmd(doc, sp->tbl[n], p, q) );
                    
                    if( get_16_le_i(trtbl->buf, n) )
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

void Loadsongs(FDOC*doc)
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
HWAVEOUT hwo;
int sndinit=0;

// Sound device?
int sounddev=0x10001;

WAVEHDR *wbufs;

int wcurbuf;
int wnumbufs;
int wbuflen;
FDOC*sounddoc=0;
SONG*playedsong;
int playedpatt;
int envdat[]={
    0,0x3ff00,0x54900,0x64d31,0x83216,0xaec33,0xc9a63,0x10624d,0x154725,
    0x199999,0x202020,0x2B1DA4,0x333333,0x3F03F0,0x563B48,
    0x667000,0x7E07E0,0xAAAAAA,0xCCCCCC,0x1000000,0x1555555,0x1999999,
    0x2000000,0x2AAAAAA,0x3333333,0x4000000,0x5555555,0x6670000,
    0x8000000,0xAAAAAAA,0x10000000,0x20000000
};

char midinst[16],midivol[16],midipan[16],midibank[16],midipbr[16];
short midipb[16];
ZMIXWAVE zwaves[8];
ZCHANNEL zchans[8];
short *wdata;
int*mixbuf;
int exitsound=0;
int songtim,songspd=100;
int songspdt=0,songspdd=0;
unsigned short globvol,globvolt,globvold;
short globtrans;

//CRITICAL_SECTION cs_song;

void Playpatt(void)
{
    int i;
    ZCHANNEL*zch=zchans;
    SONGPART*sp;
    sp=playedsong->tbl[playedpatt];
    for(i=0;i<8;i++) zch->playing=1,zch->tim=1,zch->loop=0,(zch++)->pos=sp->tbl[i];
}
const int freqvals[13]={0x157,0x16c,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x221,
   0x241,0x263,0x288,0x2ae};
const unsigned char nlength[8]={
    0x32,0x65,0x7f,0x98,0xb2,0xcb,0xe5,0xfc
};
const unsigned char nvol[16]={
    0x19,0x32,0x4c,0x65,0x72,0x7f,0x8c,0x98,0xa5,0xb2,0xbf,0xcb,0xd8,0xe5,0xf2,0xfc
};

unsigned char pbwayflag,volflag,pbstatflag,pbflag,fqflag,noteflag;

unsigned char activeflag=255,sndinterp=0;

void midinoteoff(ZCHANNEL*zch)
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

void Stopsong(void)
{
    int i;
    ZMIXWAVE*zw=zwaves;
    sounddoc=0;
    if(sounddev<0x20000)
    for(i=0;i<8;i++) (zw++)->pflag=0;
    else for(i=0;i<8;i++) midinoteoff(zchans+i);
}
int mix_freq,mix_flags;

enum
{
    MIDI_ARR_WORDS = 0x19,
    MIDI_ARR_BYTES = MIDI_ARR_WORDS * 2,
};

unsigned short midi_inst[MIDI_ARR_WORDS] =
{
    0x0050,0x0077,0x002f,0x0050,0x0051,0x001b,0x0051,0x0051,
    0x004d,0x0030,0x002c,0x0039,0x00b1,0x004f,0x000b,0x0004,
    0x10a5,0x0038,0x003c,0x00a8,0x00a8,0x0036,0x0048,0x0035,
    0x001a
};

unsigned short midi_trans[MIDI_ARR_WORDS]={
    0x00fb,0x0005,0x000c,0x0c00,0x0000,0x0002,0xfb02,0xf602,
    0x0027,0x0000,0x0000,0xf400,0x0024,0x0000,0x0018,0x000c,
    0x0020,0x0000,0x0000,0x0028,0x0028,0x000c,0x000c,0x0001,
    0x0000
};

int midi_timer;
int ws_freq=22050,ws_bufs=24,ws_len=256,ws_flags=3;
int miditimeres=5;
int ms_tim1=5,ms_tim2=20,ms_tim3=5;

void Updatesong(void)
{
    ZCHANNEL*zch;
    ZMIXWAVE*zw;
    SCMD*sc;
    ZINST*zi;
    
    HMIDIOUT const hmo = (HMIDIOUT) hwo;
    
    int i,j,k,l,m;
    
    unsigned char chf;
    
    if(!(sounddoc&&playedsong)) return;
    
    if(sounddev < 0x20000)
    {
        songtim += ( (songspd * wbuflen) << ( 3 - (mix_flags & 1) ) )
                 / mix_freq;
    }
    else songtim+=songspd*ms_tim2/20;
    for(;songtim>0;) {
        k=0;
        zch=zchans;
        zw=zwaves;
        chf=1;
        for(i=0;i<8;i++,zch++,zw++,chf<<=1) {
            if(!zch->playing) continue;
            zch->tim--;
            k=1;
            if(zch->ntim) {
                zch->ntim--;
                if(!zch->ntim) {
                    l=zch->pos;
                    j=zch->loop;
nexttime:
                    if(l==-1) {
                        j--;
                        if(j<=0) goto endnote;
                        l=zch->lopst;
                        goto nexttime;
                    }
                    sc=sounddoc->scmd+l;
                    if(sc->cmd!=0xc8) {
                        if(sc->cmd==0xef) {
                            l=*(short*)&(sc->p1);
                            goto nexttime;
                        }
                        if(sc->cmd>=0xe0) {
                            l=sc->next;
                            goto nexttime;
                        }
endnote:
                        if(sounddev<0x20000) zw->envs=3;
                        else midinoteoff(zch);
                    }
                }
            }
            if(zch->volt) {
                zch->volt--;
                zch->vol+=zch->vold;
                volflag|=chf;
            }
            if(zch->pant) {
                zch->pant--;
                zch->pan+=zch->pand;
                volflag|=chf;
            }
            
            if(pbflag&chf)
            {
                if(!zch->pbt) {
                    if(!zch->pbtim) zch->pbtim++;
                    if(pbstatflag&chf) pbflag&=~chf; else {
                        zch->pbt=zch->pbtim+1;
                        
                        // What is the significance of this constant?
                        // Midi vs. non-midi?
                        if(sounddev < 0x20000)
                        {
                            zch->note = j = zch->pbdst;
                            
                            l = j % 12;
                            
                            j =
                            (
                                freqvals[l]
                              + ( (freqvals[l + 1] - freqvals[l]) * zch->ft )
                            ) << (j / 12 + 4);
                            
                            zch->fdlt = (j - zch->freq) / zch->pbtim;
                        }
                        else
                        {
                            j = zch->pbdst - zch->note;
                            zch->midpbdlt = ((j << 13) / midipbr[zch->midch] + 0x2000 - zch->midpb) / zch->pbtim;
                        }
                        pbstatflag|=chf;
                    }
                } else if(pbstatflag&chf) {
                    if(sounddev<0x20000)
                        zch->freq+=zch->fdlt;
                    else zch->midpb+=zch->midpbdlt;
                    fqflag|=chf;
                }
                zch->pbt--;
            }
            if(zch->vibt) {
                zch->vibt--;
                zch->vibdep+=zch->vibdlt;
            }
            zch->vibcnt+=zch->vibspd;
            if(zch->vibdlt) fqflag|=chf;
            if(!zch->tim) {
again:
                if(zch->pos==-1) {zch->playing=0;continue;}
                sc=sounddoc->scmd+zch->pos;
                j=sc->next;
                if(j==-1 && zch->loop) {
                    zch->loop--;
                    if(!zch->loop) zch->pos=zch->callpos;
                    else zch->pos=zch->lopst;
                } else {
                    if(j>=sounddoc->m_size) zch->playing=0,zch->pos=-1;
                    else zch->pos=j;
                }
                if(sc->flag&1) zch->t1=sc->b1;
                if(!zch->t1) zch->t1=255;
                if(sc->flag&2) {
                    j=sc->b2;
                    zch->nvol=nvol[j&15];
                    zch->nlength=nlength[(j>>4)&7];
                }
                if(sc->cmd<0xc8) {
                    zch->ftcopy=zch->ft;
                    j=sc->cmd - 0x80 + zch->trans+globtrans;
                    
                    if(pbwayflag & chf)
                        j -= zch->pbdlt;
                    
                    if(sounddev < 0x20000)
                    {
                        zi = sounddoc->insts+zch->wnum;
                        l = j % 12;
                        
                        zch->freq =
                        (
                            freqvals[l]
                          + ( (freqvals[l + 1] - freqvals[l]) * zch->ft >> 8 )
                        ) << (j / 12 + 4);
                        
                        zw->pos=0;
                        zw->wnum=zi->samp;
                        zw->envclk=0;
                        if(zi->ad&128) {
                            zw->atk=((zi->ad<<1)&31)+1;
                            zw->dec=((zi->ad>>3)&14)+16;
                            zw->sus=zi->sr>>5;
                            zw->rel=zi->sr&31;
                            zw->envs=0;
                            zw->envx=0;
                        } else {
                            zw->sus=7;
                            zw->rel=0;
                            zw->atk=31;
                            zw->dec=0;
                            zw->envs=0;
                        }
                        zw->envclklo=0;
                        zw->pflag=1;
                    } else {
                        midinoteoff(zch);
                        if(activeflag&chf) {
                            if(zch->wnum>=25) goto nonote;
                            zch->midch=(midi_inst[zch->wnum]&0x80)?9:i;
                            
                            zch->midnum = (unsigned char) zch->wnum;
                            
                            if(zch->midch<8)
                            {
                                if(midibank[zch->midch]!=midi_inst[zch->wnum]>>8) {
                                    midibank[zch->midch]=midi_inst[zch->wnum]>>8;
                                    midiOutShortMsg(hmo, 0xb0 + zch->midch|(midibank[zch->midch]<<16));
                                }
                                if(midinst[zch->midch]!=midi_inst[zch->wnum]) {
                                    midinst[zch->midch] = (char) midi_inst[zch->wnum];
                                    midiOutShortMsg(hmo, 0xc0 + zch->midch|(midinst[zch->midch]<<8));
                                }
                                l=j+(char)(midi_trans[zch->wnum])+24;
                                if(l>=0 && l<0x80) zch->midnote=l;
                                if(midi_trans[zch->wnum]&0xff00) {
                                    l=j+(char)(midi_trans[zch->wnum]>>8)+24;
                                    if(l>=0 && l<0x80) zch->midnote|=l<<8;
                                }
                            } else {
                                l=midi_inst[zch->wnum]&0x7f;
                                if(midinst[zch->midch]!=midi_inst[zch->wnum]>>8) {
                                    midinst[zch->midch]=midi_inst[zch->wnum]>>8;
                                    midiOutShortMsg(hmo, 0xc0 + zch->midch|(midinst[zch->midch]<<8));
                                }
                                zch->midnote=l;
                                zch->pbdst=(j<<1)-midi_trans[zch->wnum];
                                zch->pbtim=0;
                                pbflag|=chf;
                            }
                            zch->midpb = 0x2000 + (zch->ftcopy<<5)/midipbr[zch->midch];
                            noteflag|=chf;
                        }
                    }
                    fqflag|=chf;
                    zch->pbt=zch->pbdly;
                    zch->note=j;
                    zch->vibt=zch->vibtim;
                    zch->vibcnt=0;
                    zch->vibdep=0;
                    pbstatflag&=~chf;
                    if(zch->pbdlt) pbflag|=chf,zch->pbdst=zch->pbdlt+zch->note;
                    volflag|=chf;
nonote:;
                }
                if(sc->cmd == 0xc9)
                {
                    if(sounddev < 0x20000)
                        zw->envs = 3;
                    else
                        midinoteoff(zch);
                    
                    pbflag&=~chf;
                }
                if(sc->cmd < 0xe0)
                {
                    zch->tim = zch->t1;
                    zch->ntim = (zch->nlength*zch->tim)>>8;
                    j = zch->pos;
                    
                    if(j != -1)
                    {
                        sc = sounddoc->scmd + j;
                        
                        if(sc->cmd == 249)
                        {
                            zch->pbt = sc->p1;
                            zch->pbtim = sc->p2;
                            zch->pbdst = sc->p3-128+zch->trans+globtrans;
                            pbstatflag &= ~chf;
                            pbflag |= chf;
                        }
                        
                        j = sc->next;
                    }
                    
                    if(sounddev >= 0x20000)
                    {
                        if(zch->midpb == 0x2000 && (pbflag & chf))
                        {
                            l = zch->pbdst-zch->note;
                            
                            if(l < 0)
                                l = -l;
                            
                            while(j != -1)
                            {
                                sc = sounddoc->scmd + j;
                                
                                if(sc->cmd < 0xe0 && sc->cmd == 0xc8)
                                    break;
                                
                                if(sc->cmd == 0xe9)
                                {
                                    m = sc->p3 - 128 + zch->trans + globtrans - zch->note;
                                    
                                    if(m < 0)
                                        m = -m;
                                    
                                    if(m > l)
                                        l = m;
                                }
                                
                                j = sc->next;
                            }
                            
                            l += 2;
                            
                            if(midipbr[zch->midch] != l)
                            {
                                midiOutShortMsg(hmo, 0x6b0 + zch->midch + (l << 16));
                                midipbr[zch->midch] = l;
                            }
                        }
                        
                        if((pbflag & chf) && (!zch->pbtim))
                        {
                            pbflag &= ~chf;
                            j = zch->pbdst - zch->note;
                            zch->midpb = ((j << 13) + (zch->ftcopy << 5)) / midipbr[zch->midch] + 0x2000;
                        }
                    }
                }
                else
                {
                    switch(sc->cmd)
                    {
                    case 224:
                        
                        zch->wnum=sc->p1;
                        
                        break;
                    
                    case 225:
                        
                        zch->pan=sc->p1<<8;
                        volflag|=chf;
                        
                        break;
                    
                    case 226:
                        
                        j=sc->p1+1;
                        zch->pant=j;
                        zch->pand=(((sc->p2<<8)-zch->pan)/j);
                        
                        break;
                    case 227:
                        zch->vibtim=sc->p1+1;
                        zch->vibspd=sc->p2;
                        zch->vibdlt=(sc->p3<<8)/zch->vibtim;
                        break;
                    case 229:
                        globvol=sc->p1<<8;
                        break;
                    case 230:
                        j=sc->p1+1;
                        globvolt=j;
                        globvold=((sc->p2<<8)-globvol)/j;
                        volflag|=chf;
                        break;
                    case 231:
                        songspd=sc->p1<<8;
                        break;
                    case 232:
                        j=sc->p1+1;
                        songspdt=j;
                        songspdd=((sc->p2<<8)-songspd)/j;
                        break;
                    case 233:
                        globtrans=(char)sc->p1;
                        break;
                    case 234:
                        zch->trans=sc->p1;
                        break;
                    case 237:
                        zch->vol=sc->p1<<8;
                        volflag|=chf;
                        break;
                    case 238:
                        j=sc->p1+1;
                        zch->volt=j;
                        zch->vold=((sc->p2<<8)-zch->vol)/j;
                        volflag|=chf;
                        break;
                    case 239:
                        if(*(unsigned short*)&(sc->p1)>=sounddoc->m_size) break;
                        zch->callpos=zch->pos;
                        zch->lopst=zch->pos=*(unsigned short*)&(sc->p1);
                        if(zch->pos>=sounddoc->m_size) zch->playing=0,zch->pos=zch->lopst=0;
                        zch->loop=sc->p3;
                        break;
                    case 241:
                        zch->pbdly=sc->p1;
                        zch->pbtim=sc->p2;
                        zch->pbdlt=sc->p3;
                        pbwayflag&=~chf;
                        break;
                    case 242:
                        zch->pbdly=sc->p1;
                        zch->pbtim=sc->p2;
                        zch->pbdlt=sc->p3;
                        pbwayflag|=chf;
                        break;
                    case 244:
                        zch->ft=sc->p1;
                        break;
                    }
                    goto again;
                }
            }
            if(volflag&chf)
            {
                volflag&=~chf;
                if(sounddev<0x20000) {
                    zw->vol1 = 0x19999 * (zch->nvol*zch->vol*((zch->pan>>8))>>22)>>16;
                    zw->vol2=(0x19999 * ((zch->nvol*zch->vol*(20-((zch->pan>>8)))>>22))>>16);
                } else {
                    l=((zch->vol>>9)*globvol>>16)*soundvol>>8;
                    if(l!=midivol[zch->midch]) {
                        midivol[zch->midch]=l;
                        midiOutShortMsg(hmo,(0x7b0 + zch->midch)|(l<<16));
                    }
                    l=((0x1400 - zch->pan) * 0x666)>>16;
                    if(l!=midipan[zch->midch]) {
                        midipan[zch->midch]=l;
                        midiOutShortMsg(hmo,(0xab0 + zch->midch)|(l<<16));
                    }
                }
            }
            if(fqflag&chf) {
                fqflag&=~chf;
                if(sounddev<0x20000) {
                    zi=sounddoc->insts+zch->wnum;
                    l=zch->freq*((zi->multhi<<8)+zi->multlo)>>14;
                    if(zch->vibdep) if(zch->vibcnt<128)
                        l=l*(65536+(zch->vibdep*(zch->vibcnt-64)>>11))>>16;
                    else l=l*(65536+(zch->vibdep*(191-zch->vibcnt)>>11))>>16;
                    zw->freq=l;
                } else {
                    l=zch->midpb;
                    if(zch->vibdep) if(zch->vibcnt<128)
                    l+=(zch->vibdep*(zch->vibcnt-64)>>10)/midipbr[zch->midch];
                    else l+=(zch->vibdep*(191-zch->vibcnt)>>10)/midipbr[zch->midch];
                    if(l!=midipb[zch->midch]) {
                        midipb[zch->midch]=l;
                        midiOutShortMsg(hmo, 0xe0 + zch->midch+((l&0x7f)<<8)+((l&0x3f80)<<9));
                    }
                }
            }
            if(noteflag&chf) {
                noteflag&=~chf;
                if(zch->midnote!=0xff) {
                    midiOutShortMsg(hmo, 0x90 + zch->midch|(zch->midnote<<8)|((zch->nvol<<15)&0x7f0000));
                    if(zch->midnote&0xff00)
                    midiOutShortMsg(hmo, 0x90 + zch->midch|(zch->midnote&0xff00)|((zch->nvol<<15)&0x7f0000));
                }
            }
        }
        
        if(songspdt)
        {
            songspdt--;
            songspd += songspdd;
        }
        
        if(globvolt)
        {
            globvolt--;
            globvol += globvold;
        }
        
        if(!k)
        {
            playedpatt++;
            if(playedpatt>=playedsong->numparts) if(playedsong->flag&2) playedpatt=playedsong->lopst; else {Stopsong();break;}
            if(playedpatt>=playedsong->numparts) {Stopsong();break;}
            Playpatt();
        } else songtim-=6100;
    }
}
void Playsong(FDOC*doc,int num)
{
    ZCHANNEL*zch=zchans;
    int i;
//  EnterCriticalSection(&cs_song);
    Stopsong();
    playedsong=doc->songs[num];
    for(i=0;i<8;i++) {
        zch->pand=zch->pant=zch->volt=zch->vold=zch->vibtim=0;
        zch->vibdep=zch->vibdlt=zch->pbdlt=0;
        zch->vol=65535;
        zch->pan=2048;
        zch->midpb=0x2000;
        zch->midch=i;
        zch->trans=0;
        zch->ft=0;
        zch->t1=255;
        zch++;
    }
    songspd=5500;
    songtim=0;
    songspdt=0;
    globvol=65535;
    globvolt=0;
    globtrans=0;
    playedpatt=0;
    volflag=pbflag=0;
    sounddoc=doc;
    Playpatt();
//  LeaveCriticalSection(&cs_song);
}

int soundthr=0;

void Mixbuffer(void)
{
    static int i,j,k,l,m,n,o;
    ZMIXWAVE*zmw=zwaves;
    ZWAVE*zw;
    short chf=1;
    int*b;
    short*c;
    unsigned char*d;
    static char v1,v2;
    
    // \task Nitpicky, but why are all these static. Just makes things even
    // less re-entrant for no apparent reason.
    static unsigned short f;
    static unsigned envx,
                    envclk,
                    envclklo,
                    envclkadd,
                    envmul;
    
    // if(soundthr) EnterCriticalSection(&cs_song);
    
    Updatesong();
    ZeroMemory(mixbuf,wbuflen<<2);
    envmul = ( wbuflen << ( 16 - (mix_flags & 1) ) ) / mix_freq << 16;
    for(i=0;i<8;i++,zmw++,chf<<=1) {
        if(!(activeflag&chf)) continue;
        if(!zmw->pflag) continue;
        n=zmw->wnum;
        if(n<0 || n>=sounddoc->numwave) continue;
        zw=sounddoc->waves+n;
        b=mixbuf;
        j=zmw->pos;
        c=zw->buf;
        envx=zmw->envx;
        envclk=zmw->envclk;
        envclklo=zmw->envclklo;
        switch(zmw->envs)
        {
        
        case 0:
            envclkadd=envdat[zmw->atk];
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm adc edx,edx
            __asm add envx,edx
            
            if(envx > 0xfe0000)
            {
                envx = 0xfe0000;
                envclklo = 0;
                
                if(zmw->sus == 7)
                    zmw->envs = 2;
                else
                    zmw->envs = 1;
            }
            
            break;
        
        case 1:
            
            envclkadd=envdat[zmw->dec];
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm adc envclk,edx
            
            while(envclk > 0x20000)
            {
                envclk -= 0x20000;
                envx -= envx >> 8;
            }
            
            if(envx < (unsigned int) (zmw->sus << 21))
            {
                envx = zmw->sus << 21;
                envclklo = 0;
                zmw->envs = 2;
            }
            
            break;
        
        case 2:
            envclkadd=envdat[zmw->rel];
            
#if 1
            __asm mov eax, envclkadd
            __asm mov edx, envmul
            __asm imul edx
            __asm add envclklo, eax
            __asm adc envclk, edx
#else
            // \task Not exactly equivalent. does carry really matter here?
            // Do regression test.
            {
                uint64_t w = envclkadd;
                uint64_t x = envmul;
                uint64_t y = envclklo;
                uint64_t z = ( (uint64_t) envclk ) << 32;
                
                uint64_t a = (w * x);
                uint64_t b = (y | z) + a;
                
                envclklo = ( 0xffffffff &        b  );
                envclk   = ( 0xffffffff & (b >> 32) );
            }
#endif
            
            while(envclk > 0x20000)
            {
                envclk -= 0x20000,
                envx -= envx >> 8;
            }
            
            break;
        
        case 3:
            envclkadd=envdat[0x1f]>>1;
            
#if 1
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm sbb envx,edx
            
#else
            // \task not exactly equivalent? (sbb vs. sub) Do regression test.
            {
                signed borrow = 0;
                
                int32_t a = (envclkadd * envmul);
                
                envclklo += a;
                
                envx -= (envmul - borrow);
            }
#endif
            
            if(envx>=0x80000000) {
                zmw->pflag=0;
                envx=0;
            }
            break;
        }
        zmw->envx=envx;
        zmw->envclk=envclk;
        zmw->envclklo=envclklo;
        v1=((zmw->vol1*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
        v2=((zmw->vol2*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
        f=(zmw->freq<<12)/mix_freq;
        k=wbuflen;
        if(zw->lopst<zw->end) l=zw->lopst<<12; else l=0;
        m=zw->end<<12;
        if(!m) continue;
        if(mix_flags&1) {
            k>>=1;
            if(sndinterp) {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    *(b++)+=o*v2;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    *(b++)+=o*v2;
                    j+=f;
                }
            } else {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    *(b++)+=c[j>>12]*v1;
                    *(b++)+=c[j>>12]*v2;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    *(b++)+=c[j>>12]*v1;
                    *(b++)+=c[j>>12]*v2;
                    j+=f;
                }
            }
        } else {
            v1 = (v1 + v2) >> 1;
            if(sndinterp) {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    j+=f;
                }
            } else {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    *(b++)+=c[j>>12]*v1;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    *(b++)+=c[j>>12]*v1;
                    j+=f;
                }
            }
        }
        zmw->pos=j;
    }
    
    // if(soundthr) LeaveCriticalSection(&cs_song);
    
    k = wbuflen;
    b = mixbuf;
    
    if(mix_flags & 2)
    {
        c = wdata + (wcurbuf * wbuflen);
        
        while(k--)
        {
            *(c++) = ( *(b++) ) >> 7;
        }
    }
    else
    {
        d = (wdata) + (wcurbuf * wbuflen);
        
        while(k--)
        {
            *(d++) = ( (*(b++) ) >> 15 ) ^ 0x80;
        }
    }
}

HANDLE wave_end;

void CALLBACK midifunc(UINT timerid,UINT msg,DWORD inst,DWORD p1,DWORD p2)
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
void CALLBACK midifunc2(HWND win,UINT msg,UINT timerid,DWORD systime)
{
    (void) win, msg, timerid, systime;
    
    if(exitsound)
        return;
    
    Updatesong();
}

int CALLBACK soundproc(HWAVEOUT bah,UINT msg,DWORD inst,DWORD p1,DWORD p2)
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

void Exitsound(void)
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

void Initsound(void)
{
    int n,sh;
    
    WAVEFORMATEX wfx;
    WAVEHDR *wh;
    
    char *err;
    const unsigned char blkal[4]={1,2,2,4};
    
    if(sndinit)
        Exitsound();
    
    if((sounddev >> 16) == 2)
    {
        HMIDIOUT hmo;
        
        n = midiOutOpen(&hmo, sounddev - 0x20001, 0, 0, 0);
        
        if(n)
            goto openerror;
        
        hwo = (HWAVEOUT) hmo;
        
        for(n = 0; n < 16; n++)
        {
            midinst[n] = midivol[n]=midipan[n]=midibank[n]=255;
            midipbr[n] = 2;
            midipb[n] = 65535;
            midiOutShortMsg(hmo, 0x64b0 + n);
            midiOutShortMsg(hmo, 0x65b0 + n);
            midiOutShortMsg(hmo, 0x206b0 + n);
        }
        
        noteflag = 0;
        
        for(n = 0; n < 8; n++)
            zchans[n].midnote = 0xff;
        
        if(wver)
        {
//          miditimeres=ms_tim1;
//          timeBeginPeriod(miditimeres);
//          midi_timer=timeSetEvent(ms_tim2,ms_tim3,testfunc,0,TIME_PERIODIC);
            
            midi_timer=SetTimer(framewnd,1,ms_tim2,(TIMERPROC)midifunc2);
        }
        else
        {
            miditimeres = ms_tim1;
            timeBeginPeriod(miditimeres);
            midi_timer=timeSetEvent(ms_tim2,ms_tim3,midifunc,0,TIME_PERIODIC);
            soundthr = 1;
        }
        
        if(!midi_timer)
        {
            wsprintf(buffer,"Can't initialize timer: %08X",GetLastError());
            MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
            
            if(!wver)
                timeEndPeriod(miditimeres);
            
            midiOutClose(hmo);
            
            return;
        }
        
//      InitializeCriticalSection(&cs_song);
        
        sndinit = 1;
        
        goto endinit;
    }

    soundthr=0;
//  ht=CreateThread(0,0,soundfunc,0,0,&sndthread);
//  if(ht) CloseHandle(ht);
    wbuflen=ws_len<<(ws_flags&1);
    sh=(ws_flags>>1);
    wfx.wFormatTag=1;
    wfx.nChannels=(ws_flags&1)+1;
    wfx.nSamplesPerSec=ws_freq;
    wfx.nAvgBytesPerSec = ws_freq << ( sh + ( (ws_flags & 2) >> 1 ) );
    wfx.nBlockAlign=blkal[ws_flags];
    wfx.wBitsPerSample=(ws_flags&2)?16:8;
    wfx.cbSize=0;
    wnumbufs=ws_bufs;
//  if(ht) n=waveOutOpen(&hwo,sounddev - 0x10002,&wfx,sndthread,0,CALLBACK_THREAD),soundthr=1;
/*  else*/
    
    n = waveOutOpen(&hwo,
                    sounddev - 0x10002,
                    &wfx,
                    (int) soundproc,
                    0,
                    CALLBACK_FUNCTION);
    
    if(n)
    {
openerror:
        switch(n) {
        case MIDIERR_NODEVICE:
            err="No MIDI port was found.";
            break;
        case MMSYSERR_ALLOCATED:
            err="Already in use";
            break;
        case MMSYSERR_BADDEVICEID:
            err="The device was removed";
            break;
        case MMSYSERR_NODRIVER:
            err="No driver found";
            break;
        case MMSYSERR_NOMEM:
            err="Not enough memory";
            break;
        case WAVERR_BADFORMAT:
            err="Unsupported playback quality";
            break;
        case WAVERR_SYNC:
            err="It is synchronous";
            break;
        default:
            wsprintf(buffer,"Unknown error %08X",n);
            err=buffer+256;
            break;
        }
        wsprintf(buffer,"Cannot initialize sound (%s)",err);
        MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
        return;
    }
//  InitializeCriticalSection(&cs_song);
    wh=wbufs=malloc(sizeof(WAVEHDR)*ws_bufs);
    wdata=malloc(wbuflen*ws_bufs<<sh);
    for(n=0;n<8;n++) zwaves[n].pflag=0;
    for(n=0;n<wnumbufs;n++) {
        wh->lpData=(LPSTR)(wdata)+(n*wbuflen<<sh);
        wh->dwBufferLength=wbuflen<<sh;
        wh->dwFlags=0;
        wh->dwUser=n;
        waveOutPrepareHeader(hwo,wh,sizeof(WAVEHDR));
        wh++;
    }
    sndinit=1;
    mixbuf=malloc(wbuflen*ws_bufs<<2);
    mix_freq=ws_freq/6;
    mix_flags=ws_flags;
    for(wcurbuf=0;wcurbuf<wnumbufs;wcurbuf++)
        Mixbuffer();
    wcurbuf=0;
    for(n=0;n<wnumbufs;n++)
        waveOutWrite(hwo,wbufs+n,sizeof(WAVEHDR));
endinit:
    wave_end=CreateEvent(0,0,0,0);
}

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
LoadText(FDOC * const doc)
{
    int i = 0;
    int data_pos = 0xe0000;
    
    int k;
    int l;
    
    int m = 64;
    
    unsigned char a;
    unsigned char *rom = doc->rom;
    
    // temporary buffer for each individual text message
    unsigned char *b;
    
    int bd, bs;
    
    // text buffer?
    doc->tbuf = malloc(256);
    
    for( ; ; )
    {
        bs = 128; // bs is the current maximum size for the buffer (can change as needed)
        
        bd = 2; // the size of the useful data in the message buffer
        
        b = (uint8_t*) malloc(128); // temporary buffer...
        
        for( ; ; )
        {
            a = rom[data_pos];
            
            if(a == 0x80)
            {
                // 0x80 tells us to go to the second data set 
                data_pos = 0x75f40;
            }
            else if(a == 0xff)
            {
                // 0xff is the terminator byte for all text data, period
                
                doc->t_number = i; // the number of loaded pieces
                doc->t_loaded = 1; // indicate that text is loaded and return
                
                return;
            }
            
            else if(a < 0x67)
            {
                // if it's a character, increment the position in the main buffer
                // updating that second buffer... not sure what it's for
                data_pos++;
                b[bd++] = a;
            }
            else if(a >= 0x88)
            {
                // use the dictionary to load the characters up
                k = *(short*)(rom + 0x745f3 + (a << 1));
                l = *(short*)(rom + 0x745f5 + (a << 1));
                
                while(k < l)
                {
                    b[bd++] = rom[0x78000 + k];
                    k++;
                }
                
                data_pos++;
            }
            
            // 0x7f is a terminator byte for each message
            else if(a == 0x7f)
            {
                // 0x7f is a terminator byte for each message
                data_pos++;
                
                break;
            }
            else
            {
                // 0x7536B is a length indicator for each byte code
                l = rom[0x7536b + a];
                
                while(l--)
                    b[bd++] = rom[data_pos++];
            }
            
            if(bd >= bs - 64)
            {
                // if the text data won't fit into the current buffer, reallocate
                // and add 128 bytes to the temporary buffer
                bs += 128;
                b = (uint8_t*) realloc(b, bs);
            }
        }
        
        *(short*) b = bd;
        
        b = realloc(b, bd);
        
        if(i == m)
        {
            m += 64;
            doc->tbuf = realloc(doc->tbuf, m << 2);
        }
        
        doc->tbuf[i++] = b;
    }
}

// =============================================================================

void
Savetext(FDOC*doc)
{
    int i,bd,j,k;
    short l,m,n,o,p,q,r,v,t,u,w;
    
    // \task Can b2 overflow or is this just... fantasy?
    unsigned char*b, b2[2048];
    unsigned char*rom=doc->rom;
    
    size_t write_pos = 0xe0000;
    
    if(!doc->t_modf)
        return;
    
    doc->t_modf=0;
    
    w = ( get_16_le(rom + 0x74703) - 0xc705 ) >> 1;
    
    for(i=0;i<doc->t_number;i++) {
        b=doc->tbuf[i];
        m=bd=0;
        k=*(short*)b;
        j=2;
        r=w;
        for(;j<k;)
        {
            q=b2[bd++]=b[j++];
            if(q>=0x67) {
                l=rom[0x7536b + q] - 1;
                while(l--) b2[bd++]=b[j++];
                m=bd;
            }
            if(bd>m+1) {
                o=*(short*)(rom + 0x74703);
                v=255;
                t=w;
                for(l=0;l<w;l++) {
                    n=o;
                    o=*(short*)(rom + 0x74705 + (l<<1));
                    p=o-n-bd+m;
                    if(p>=0) if(!memcmp(rom + 0x78000 + n,b2+m,bd-m)) {
                        if(p<v) t=l,v=p;
                        if(!p) {r=t;u=j;break;}
                    }
                }
                if(t==w || b[j] >= 0x67) {
                    if(r!=w) {
                        b2[m]=r + 0x88;
                        m++;
                        j=u;
                        bd=m;
                        r=w;
                    } else m=bd-1;
                }
            }
        }
        
        write_pos += bd;
        
        // The subtraction of two accounts for the need for a message
        // terminator code (0x7f) and the potential need for a master
        // terminator code for all of the text data (0xff).
        if( (write_pos < 0xe0000) && (write_pos > (0x77400 - 2) ) )
        {
            doc->t_modf = 1;
            
            MessageBox(framewnd,
                       "Not enough space for monologue.",
                       "Bad error happened",
                       MB_OK);
            
            return;
        }
        
        // Check if writing this message would put us past the end of the first
        // bank where text data can reside. The subtraction of two is to
        // account for a message terminator code (0x7f) as well as a possible
        // additional bank switch code (0x80).
        if( write_pos > (0xe8000 - 2) )
        {
            rom[write_pos - bd] = 0x80;
            write_pos = 0x75f40 + bd;
        }
        
        memcpy(rom + write_pos - bd, b2, bd);
        
        rom[write_pos++] = 0x7f;
    }
    
    rom[write_pos] = 0xff;
    
    doc->modf = 1;
}

// =============================================================================

const char z_alphabet[]=
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?-.,\0>()@£${}\"\0\0\0\0'\0\0\0\0\0\0\0 <\0\0\0\0";
const static char*tsym_str[]=
{
    "Up",
    "Down",
    "Left",
    "Right",
    0,
    "1HeartL",
    "1HeartR",
    "2HeartL",
    "3HeartL",
    "3HeartR",
    "4HeartL",
    "4HeartR",
    0,0,
    "A",
    "B",
    "X",
    "Y"
};
const static char*tcmd_str[]={
    "NextPic",
    "Choose",
    "Item",
    "Name",
    "Window",
    "Number",
    "Position",
    "ScrollSpd",
    "Selchg",
    "Crash",
    "Choose3",
    "Choose2",
    "Scroll",
    "1",
    "2",
    "3",
    "Color",
    "Wait",
    "Sound",
    "Speed",
    "Mark",
    "Mark2",
    "Clear",
    "Waitkey",
};

// =============================================================================

void
Makeasciistring(FDOC          const * const doc,
                char                * const buf,
                unsigned char const * const buf2,
                int                   const bufsize)
{
    int i;
    
    short j,k,l,m,n;
    j=*(short*)buf2;
    l=2;
    for(i=0;i<bufsize-1;) {
        if(l>=j) break;
        k=buf2[l++];
        if(k<0x5f)
        {
            if(!z_alphabet[k])
            {
                if(k==0x43) m=wsprintf(buffer,"...");
                else m=wsprintf(buffer,"[%s]",tsym_str[k - 0x4d]);
                goto longstring;
            }
            else
            {
                buf[i++]=z_alphabet[k];
            }
        }
        else if(k >= 0x67 && k < 0x7f)
        {
            m=wsprintf(buffer,"[%s",tcmd_str[k - 0x67]);
            n=doc->rom[0x7536b + k] - 1;
            while(n--) m+=wsprintf(buffer+m," %02X",buf2[l++]);
            buffer[m++]=']';
longstring:
            n = 0;
            
            while(m--)
            {
                buf[i++] = buffer[n++];
                
                if(i == bufsize - 1)
                    break;
            }
        }
        else
        {
            m = wsprintf(buffer, "[%02X]",k);
            
            goto longstring;
        }
    }
    
    buf[i] = 0;
}

// =============================================================================

char * text_error;

// =============================================================================

uint8_t * Makezeldastring(FDOC *doc, char *buf)
{
    uint8_t * b2 = (uint8_t*) malloc(128);
    char *n;
    
    int bd = 2, bs = 128;
    
    short j,l,m,k;
    
    for(;;)
    {
        j = *(buf++);
        
        // look for a [ character
        if(j == '[')
        {
            // m is the distance to the ] character
            m = strcspn(buf," ]");
            
            for(l = 0; l < 18; l++)
                if(tsym_str[l] && (!tsym_str[l][m]) && !_strnicmp(buf,tsym_str[l],m))
                    break;
            
            // the condition l == 18 means it did not find any string in the
            // special symbol strings list to match this one
            if(l == 18)
            {
                for(l = 0; l < 24; l++)
                    if((!tcmd_str[l][m]) && !_strnicmp(buf,tcmd_str[l],m))
                        break;
                
                // if this condition is true it means we didn't find a match in the 
                // command strings either
                if(l == 24)
                {
                    // strtol converts a string to a long data type
                    j = (short) strtol(buf, &n, 16);
                    
                    // k is the distance from the start of the command to the 
                    k = n - buf;
                    
                    // if the string doesn't match the pattern [XX] fogedda boud it                 
                    if(k > 2 || k < 1)
                    {
                        buf[m] = 0;
                        wsprintf(buffer, "Invalid command \"%s\"", buf);
                        
error:
                        
                        MessageBox(framewnd, buffer, "Bad error happened", MB_OK);
                        free(b2);
                        text_error = buf;
                        
                        return 0;
                    };
                    
                    m = k;
                    b2[bd++] = (char) j;
                    l = 0;
                }
                else
                    b2[bd++] = l + 0x67, l = doc->rom[0x753d2 + l] - 1;
            }
            else
                b2[bd++] = l + 0x4d, l = 0;
            
            buf += m;
            
            while(l--)
            {
                if(*buf!=' ')
                {
syntaxerror:
                    wsprintf(buffer,"Syntax error: '%c'",*buf);
                    
                    goto error;
                }
                
                buf++;
                
                j = (short) strtol(buf,&n,16);
                m = n - buf;
                
                if(m > 2 || m < 1)
                {
                    wsprintf(buffer,"Invalid number");
                    goto error;
                };
                
                buf += m;
                b2[bd++] = (char) j;
            }
            
            if(*buf!=']')
                goto syntaxerror;
            
            buf++;
        }
        else
        {
            if(!j)
                break;
            
            for(l = 0; l < 0x5f; l++)
                if(z_alphabet[l] == j)
                    break;
            
            if(l == 0x5f)
            {
                wsprintf(buffer,"Invalid character '%c'",j);
                goto error;
            }
            
            b2[bd++] = (char) l;
        }
        
        if(bd > bs - 64)
        {
            bs += 128;
            b2 = (uint8_t*) realloc(b2, bs);
        }
    }
    
    // \task Shouldn't we just structurize this stuff and store the length
    // as a member? for pete's sake...
    *(unsigned short*) b2 = bd;
    
    return b2;
}

void Removepatches(FDOC*doc)
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
char asmpath[MAX_PATH];
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

int Buildpatches(FDOC*doc)
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

BOOL CALLBACK
textdlgproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    TEXTEDIT * ed;
    
    FDOC *doc;
    
    int i, j, k, l, m;
    
    char *b = 0;
    
    uint8_t * c = 0;
    
    unsigned char *rom;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        ed = (TEXTEDIT*) lparam;
        
        CheckDlgButton(win, 3003, BST_CHECKED);
        
        ed->dlg = win;
        ed->num = 0;
        
        doc = ed->ew.doc;
        
        if( ! doc->t_loaded)
        {
            LoadText(doc);
        }
        
    updstrings:
        
        hc = GetDlgItem(win,3000);
        
        b = (char*) malloc(256);
        
        if(ed->num)
        {
            rom = doc->rom;
            
            j = ( get_16_le(rom + 0x74703) - 0xc705 ) >> 1;
            
            l = get_16_le(rom + 0x74703);
            
            for(i = 0; i < j; i++)
            {
                k = get_16_le_i(rom + 0x74705, i);
                
                memcpy(b+130,rom+l + 0x78000,k-l);
                
                *(short*)(b+128)=k-l+2;
                
                Makeasciistring(doc, b, b + 128, 128);
                
                wsprintf(buffer, "%03d: %s", i, b);
                
                SendMessage(hc, LB_ADDSTRING, 0, (long) buffer);
                
                l = k;
            }
        }
        else
        {
            for(i = 0; i < doc->t_number; i++)
            {
                Makeasciistring(doc, b, doc->tbuf[i], 256);
                
                wsprintf(buffer, "%03d: %s", i, b);
                
                SendMessage(hc, LB_ADDSTRING, 0, (long) buffer);
            }
        }
        
        free(b);
        
        break;
    
    case WM_CLOSE:
        
        return 1;
    
    case WM_COMMAND:
        
        ed = (TEXTEDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed)
            break;
        
        doc = ed->ew.doc;
        
        rom = doc->rom;
        
        switch(wparam)
        {
        
        case 3000 | (LBN_DBLCLK << 16):
            
            i = SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            
            if(i != -1)
            {
                b = malloc(2048);
                
                if(ed->num)
                {
                    k=((short*)(rom + 0x74705))[i];
                    l=((short*)(rom + 0x74703))[i];
                    memcpy(b+1026,rom+l + 0x78000,k-l);
                    *(short*)(b+1024)=k-l+2;
                    Makeasciistring(doc,b,b+1024,1024);
                }
                else
                {
                    Makeasciistring(doc, b, doc->tbuf[i], 2048);
                }
                
                SetDlgItemText(win, 3001, b);
                
                free(b);
            }
            else
            {
                SetDlgItemText(win, 3001, 0);
            }
            
            break;
        
        case 3002:
            
            i = SendDlgItemMessage(win,
                                   3000,
                                   LB_GETCURSEL,
                                   0,
                                   0);
            
            if(i != -1)
            {
                b = malloc(2048);
                
                GetDlgItemText(win, 3001, b, 2048);
                
                c = Makezeldastring(doc, b);
                
                hc = GetDlgItem(win, 3000);
                
                if(c)
                {
                    if(ed->num)
                    {
                        m = (*(short*)c) - 2;
                        j = *(unsigned short*) (rom + 0x77ffe + *(short*)(rom + 0x74703));
                        k = ((unsigned short*) (rom + 0x74705))[i];
                        l = ((unsigned short*) (rom + 0x74703))[i];
                        
                        if(j + m + l - k > 0xc8d9)
                        {
                            MessageBeep(0);
                            free(c);
                            free(b);
                            
                            break;
                        }
                        
                        memcpy(rom + 0x68000 + l + m, rom + 0x68000 + k, j - k);
                        memcpy(rom + 0x68000 + l, c + 2, m);
                        k -= l;
                        
                        l = ( get_16_le(rom + 0x74703) - 0xc703 ) >> 1;
                        
                        for(j = i + 1; j < l; j++)
                            ((short*) (rom + 0x74703))[j] += m - k;
                    }
                    else
                    {
                        free(doc->tbuf[i]);
                        doc->tbuf[i] = c;
                    }
                    
                    Makeasciistring(doc,b,c,256);
                    wsprintf(buffer,"%03d: %s",i,b);
                    
                    if(ed->num)
                        free(c);
                    
                    SendMessage(hc, LB_DELETESTRING, i, 0);
                    SendMessage(hc, LB_INSERTSTRING, i, (long)buffer);
                    SendMessage(hc, LB_SETCURSEL, i, 0);
                }
                else
                {
                    i = text_error - b;
                    
                    hc = GetDlgItem(win, 3001);
                    
                    SendMessage(hc, EM_SETSEL, i, i);
                    
                    SetFocus(hc);
                    
                    free(b);
                    
                    break;
                }
                
                free(b);
                
                doc->t_modf = 1;
            }
            
            break;
        
        case 3003:
            
            ed->num = 0;
            
            SendDlgItemMessage(win, 3000, LB_RESETCONTENT, 0, 0);
            
            goto updstrings;
        
        case 3004:
            
            ed->num = 1;
            
            SendDlgItemMessage(win, 3000, LB_RESETCONTENT, 0, 0);
            
            goto updstrings;
        }
        
        break;
    }
    
    return FALSE;
}

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
void Loadeditinst(HWND win,SAMPEDIT*ed)
{
    ZINST*zi;
    zi=ed->ew.doc->insts+ed->editinst;
    SetDlgItemInt(win,3014,(zi->multhi<<8)+zi->multlo,0);
    wsprintf(buffer,"%04X",(zi->ad<<8)+zi->sr);
    SetDlgItemText(win,3016,buffer);
    wsprintf(buffer,"%02X",zi->gain);
    SetDlgItemText(win,3018,buffer);
    SetDlgItemInt(win,3020,zi->samp,0);
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

const static char* chest_str[]={
    "Swd&&Sh1",
    "Sword 2",
    "Sword 3",
    "Sword 4",
    "Shield 1",
    "Shield 2",
    "Shield 3",
    "Firerod",
    "Icerod",
    "Hammer",
    "Hookshot",
    "Bow",
    "Boomerang",
    "Powder",
    "Bee",
    "Bombos",
    "Ether",
    "Quake",
    "Lamp",
    "Shovel",
    "Flute",
    "Red cane",
    "Bottle",
    "Heart piece",
    "Blue cane",
    "Cape",
    "Mirror",
    "Power glove",
    "Titan glove",
    "Wordbook",
    "Flippers",
    "Pearl",
    "Crystal",
    "Net",
    "Armor 2",
    "Armor 3",
    "Key",
    "Compass",
    "LiarHeart",
    "Bomb",
    "3 bombs",
    "Mushroom",
    "Red boom.",
    "Red pot",
    "Green pot",
    "Blue pot",
    "Red pot",
    "Green pot",
    "Blue pot",
    "10 bombs",
    "Big key",
    "Map",
    "1 rupee",
    "5 rupees",
    "20 rupees",
    "Pendant 1",
    "Pendant 2",
    "Pendant 3",
    "Bow&&Arrows",
    "Bow&&S.Arr",
    "Bee",
    "Fairy",
    "MuteHeart",
    "HeartCont.",
    "100 rupees",
    "50 rupees",
    "Heart",
    "Arrow",
    "10 arrows",
    "Magic",
    "300 rupees",
    "20 rupees",
    "Bee",
    "Sword 1",
    "Flute",
    "Boots",
    "No alternate"
};

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

void Dungselectchg(DUNGEDIT*ed,HWND hc,int f)
{
    RECT rc;
    unsigned char*rom=ed->ew.doc->rom;
    int i,j,k,l;
    static char *dir_str[4]={"Up","Down","Left","Right"};
    
    if(f)
        ed->ischest=0;
    
    if(!ed->selobj)
    {
        if(f)
            buffer[0]=0;
    }
    else if(ed->selchk == 9)
    {
        dm_x = (*(short*) (ed->tbuf + ed->selobj)) >> 1;
        
        if(f)
            wsprintf(buffer,"Torch\nX: %02X\nY: %02X\nBG%d\nP: %d",dm_x&63,(dm_x>>6)&63,((dm_x>>12)&1)+1,(dm_x>>13)&7);
    }
    else if(ed->selchk == 8)
    {
        dm_x = (*(short*) (rom + ed->selobj + 2)) >> 1;
        
        if(f)
            wsprintf(buffer,"Block\nX: %02X\nY: %02X\nBG%d",dm_x&63,(dm_x>>6)&63,(dm_x>>12)+1);
    }
    else if(ed->selchk == 7)
    {
        dm_x = *(short*) (ed->sbuf + ed->selobj - 2) >> 1;
        dm_k = ed->sbuf[ed->selobj];
        cur_sec = Getsecretstring(rom, dm_k);
        
        if(f)
            wsprintf(buffer,"Item %02X\nX: %02X\nY: %02X\nBG%d",dm_k,dm_x&63,(dm_x>>6)&63,(dm_x>>12)+1);
    }
    else if(ed->selchk == 6)
    {
        dm_x = ((ed->ebuf[ed->selobj+1]&31)<<1)+((ed->ebuf[ed->selobj]&31)<<7);
        dm_dl = ed->ebuf[ed->selobj]>>7;
        dm_l = ((ed->ebuf[ed->selobj]&0x60)>>2)|((ed->ebuf[ed->selobj+1]&0xe0)>>5);
        dm_k = ed->ebuf[ed->selobj + 2] + (((dm_l & 7) == 7) ? 256 : 0);
        
        if(f)
            wsprintf(buffer,"Spr %02X\nX: %02X\nY: %02X\nBG%d\nP: %02X",dm_k,dm_x&63,dm_x>>6,dm_dl+1,dm_l);
    }
    else if(ed->selchk & 1)
    {
        getdoor(ed->buf+ed->selobj,rom);
        
        if(f)
            wsprintf(buffer,"Dir: %s\nType: %d\nPos: %d\n",dir_str[dm_k],dm_l,dm_dl);
    }
    else
    {
        getobj(ed->buf + ed->selobj);
        
        if(f)
        {
            if(dm_k >= 0xf8 && dm_k < 0x100) // Subtype 2 object
            {
                wsprintf(buffer,"Obj: %03X:%X\nX: %02X\nY: %02X",dm_k,dm_l,dm_x & 0x3f,dm_x >> 6);
                
                // If it's a chest object...
                if( (dm_k == 0xf9 && dm_l == 9) || (dm_k == 0xfb && dm_l == 1) )
                {
                    for(k = 0; k < ed->chestnum; k++)
                        if(ed->selobj == ed->chestloc[k])
                            break;
                    
                    for(l = 0; l < 0x1f8; l += 3)
                    {
                        if( (*(short*) (rom + 0xe96e + l) & 0x7fff) == ed->mapnum)
                        {
                            k--;
                            
                            if(k < 0)
                            {
                                k = rom[0xe970 + l];
                                i = rom[0x3b528 + k];
                                
                                if(i == 255)
                                    i = 76;
                                
                                wsprintf(buffer + 21,
                                         (rom[0xe96f + l] & 128) ? "\n%d:%s\n(%s)\n(Big)" : "\n%d:%s\n(%s)",
                                         k,
                                         chest_str[k],
                                         chest_str[i]);
                                
                                // This is a chest object >_>.
                                ed->ischest=1;
                                
                                break;
                            }
                        }
                    }
                }
            }
            else
                wsprintf(buffer,"Obj: %03X\nX: %02X\nY: %02X",dm_k,dm_x&0x3f,dm_x>>6);
            
            if(dm_k<0xf8)
                wsprintf(buffer + 20, "\nSize: %02X", dm_l);
        }
    }
    
    if(f)
        SetDlgItemText(ed->dlg, ID_DungDetailText, buffer);
    
    if(!ed->selobj)
        return;
    
    k = ed->mapscrollh;
    l = ed->mapscrollv;
    i = ( (dm_x & 0x3f) << 3) - (k << 5);
    j = ( ( (dm_x >> 6) & 0x3f) << 3) - (l << 5);
    
    if(f && ed->selobj)
    {
        if(i < 0)
            k += i >> 5;
        
        if(j < 0)
            l += j >> 5;
        
        if( ( (i + 32) >> 5) >= ed->mappageh)
            k += ( (i + 32) >> 5) - ed->mappageh;
        
        if( ( (j + 32) >> 5) >= ed->mappagev )
            l += ( (j + 32) >> 5) - ed->mappagev;
        
        if(k!=ed->mapscrollh) SendMessage(hc,WM_HSCROLL,SB_THUMBPOSITION+(k<<16),0);
        if(l!=ed->mapscrollv) SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION+(l<<16),0);
    }
    rc.left=((dm_x&0x3f)<<3);
    rc.top=(((dm_x>>6)&0x3f)<<3);
    Getdungobjsize(ed->selchk,&rc,0,0,0);
    ed->selrect=rc;
    rc.left-=k<<5;
    rc.top-=l<<5;
    rc.right-=k<<5;
    rc.bottom-=l<<5;
    ed->objt=dm_k;
    ed->objl=dm_l;
    InvalidateRect(hc,&rc,0);
}

//Drawblock#********************************

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
        // \task Used with only with Link's graphics dialog, seemingly.
        
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

int gbtnt=0;

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
                          32,32,
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
                 int const i,
                 int const j,
                 int const n,
                 int const o,
                 uint16_t const * const p_buf)
{
    // 
    int const m = ( (i + n) >> 3)
                + ( (j + o) << 3);
    
    int p = 0;
    
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
Paintdungeon(DUNGEDIT const * const ed,
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
     || ( ( ! (ed->disp & SD_DungShowBothBGs) ) && ! (ed->layering >> 5) )
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

void Updateobjdisplay(CHOOSEDUNG*ed,int num)
{
    int i=num+(ed->scroll<<2),j,k,l,m,n,o,p;
    
    uint16_t objbuf[8192];
    
    unsigned char obj[6];
    
    RECT rc;
    
    for(j=0;j<8192;j++) objbuf[j]=0x1e9;
    if(i>=0x260) return;
    else if(i>=0x1b8) {
        obj[0]=0xf0;
        obj[1]=0xff;
        obj[2]=(i - 0x1b8)/42;
        obj[3]=((i - 0x1b8)%42)<<1;
        obj[4]=0xff;
        obj[5]=0xff;
        getdoor(obj+2,ed->ed->ew.doc->rom);
        rc.left=(dm_x&0x3f)<<3;
        rc.top=(dm_x&0xfc0)>>3;
    } else {
        if(i<0x40) {
            obj[0]=0xfc;
            obj[1]=0;
            obj[2]=i;
        } else if(i<0x138) {
            obj[0]=0;
            obj[1]=0;
            switch(obj3_t[i - 0x40]&15) {
            case 0: case 2:
                obj[1]=1;
                break;  
            }
            obj[2]=i - 0x40;
        } else {
            obj[0] = i & 3;
            obj[1] = ( (i - 0x138) >> 2) & 3;
            obj[2] = 0xf8 + ( ( (i - 0x138) >> 4) & 7);
        }
        obj[3]=255;
        obj[4]=255;
        getobj(obj);
        rc.left=0;
        rc.top=0;
    }
    Getdungobjsize(ed->ed->selchk,&rc,0,0,1);
    if(i<0x1b8) {
        if(rc.top<0) j=-rc.top>>3; else j=0;
        if(rc.left<0) k=-rc.left>>3; else k=0;
        if(i<0x40) {
            obj[0]|=k>>4;
            obj[1]=(k<<4)|(j>>2);
            obj[2]|=k<<6;
        } else {
            obj[0]|=k<<2;
            obj[1]|=j<<2;
        }
        rc.right+=k<<3;
        rc.bottom+=j<<3;
        rc.left+=k<<3;
        rc.top+=j<<3;
    }
    dm_buf=objbuf;
    Drawmap(ed->ed->ew.doc->rom,objbuf,obj,ed->ed);
    Paintdungeon(ed->ed,objdc,&rc,rc.left,rc.top,rc.right,rc.bottom,0,0,objbuf);
    rc.right-=rc.left;
    rc.bottom-=rc.top;
    n=rc.right;
    if(rc.bottom>n) n=rc.bottom;
    n++;
    j=ed->w>>2;
    k=ed->h>>2;
    l=(rc.right)*j/n;
    m=(rc.bottom)*k/n;
    o=((((i&3)<<1)+1)*ed->w>>3);
    p=(((((i>>2)&3)<<1)+1)*ed->h>>3);
    StretchBlt(ed->bufdc,o-(l>>1),p-(m>>1),l,m,objdc,rc.left,rc.top,rc.right,rc.bottom,SRCCOPY);
    if(i<0x40) l=i + 0x100;
    else if(i<0x138) l=i - 0x40;
    else if(i<0x1b8) l=i + 0xe48;
    else l=i - 0x1b8;
    wsprintf(buffer,"%03X",l);
    SetTextColor(ed->bufdc,0);
    TextOut(ed->bufdc,o+1,p+1,buffer,3);
    SetTextColor(ed->bufdc,0xffbf3f);
    TextOut(ed->bufdc,o,p,buffer,3);
}

void Getdungselrect(int i,RECT*rc,CHOOSEDUNG*ed)
{
    rc->left=(i&3)*ed->w>>2;
    rc->top=(i>>2)*ed->h>>2;
    rc->right=rc->left+(ed->w>>2);
    rc->bottom=rc->top+(ed->h>>2);
}

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

void Updateblk8sel(BLOCKSEL8*ed,int num)
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
    RECT rc;
    HPALETTE oldpal;
    GetClientRect(hc,&rc);
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
    SetWindowLong(hc,GWL_USERDATA,(int)bs);
    Updatesize(hc);
}


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

char *Getoverstring(OVEREDIT *ed, int t, int n)
{
    int a;
    switch(t) {
    case 3:
        wsprintf(buffer,"%02X",ed->ew.doc->rom[0xdbb73 + n]);
        break;
    case 5:
        a=ed->ebuf[ed->sprset][n+2];
        wsprintf(buffer,"%02X-%s",a,sprname[a]);
        break;
    case 7:
        wsprintf(buffer,"%04X",((unsigned short*)(ed->ew.doc->rom + 0x15d8a))[n]);
        break;
    case 8:
        wsprintf(buffer,"%02X",ed->ew.doc->rom[0xdb84c + n]);
        break;
    case 9:
        if(n>8) wsprintf(buffer,"Whirl-%02X",((unsigned short*)(ed->ew.doc->rom + 0x16cf8))[n-9]);
            else wsprintf(buffer,"Fly-%d",n+1);
        break;
    case 10:
        wsprintf(buffer,"%02X-",ed->sbuf[n+2]);
        strcpy(buffer+3,Getsecretstring(ed->ew.doc->rom,ed->sbuf[n+2]));
        break;
    default:
        wsprintf(buffer,"Invalid object!");
    }
    return buffer;
}

void Overselchg(OVEREDIT *ed, HWND win)
{
    RECT rc;
    if(ed->selobj==-1) return;
    rc.left=ed->objx-(ed->mapscrollh<<5);
    rc.top=ed->objy-(ed->mapscrollv<<5);
    Getstringobjsize(Getoverstring(ed,ed->tool,ed->selobj),&rc);
    InvalidateRect(win,&rc,0);
}

const static short tool_ovt[]={0,0,0,1,0,0,0,2,3,5,4};

void Overtoolchg(OVEREDIT*ed,int i,HWND win)
{
    HWND hc=GetDlgItem(win,3001);
    if(tool_ovt[i]!=tool_ovt[ed->tool]) InvalidateRect(hc,0,0);
    else Overselchg(ed,hc);
    ed->tool=i;
    ed->selobj=-1;
    Overselchg(ed,hc);
}

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

BOOL CALLBACK editdungprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,
        j,
        k,
        l;
    
    unsigned char *rom;
    
    static int radio_ids[3] = {IDC_RADIO10,IDC_RADIO12,IDC_RADIO13};
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        rom = dunged->ew.doc->rom;
        j = dunged->ew.param;
        
        if(j < 0x85)
        {
            i = rom[0x15510 + j];
            
            if(i == 2)
                // what exactly is a horizontal entrance doorway? O_o
                CheckDlgButton(win, IDC_RADIO9, BST_CHECKED);
            else if(i)
                CheckDlgButton(win, IDC_RADIO3, BST_CHECKED);
            else
                CheckDlgButton(win, IDC_RADIO1, BST_CHECKED);
        }
        else
        {
            ShowWindow(GetDlgItem(win,IDC_RADIO9),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_RADIO3),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_RADIO1),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_STATIC3),SW_SHOW);
            ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_SHOW);
            
            SetDlgItemInt(win, IDC_EDIT1, ((short*) (rom + 0x15b36))[j], 0);
        }
        
        i = rom[(j >= 0x85 ? 0x15b98 : 0x15595) + j];
        
        if(i & 0xf)
            CheckDlgButton(win,IDC_RADIO5,BST_CHECKED);
        else
            CheckDlgButton(win,IDC_RADIO4,BST_CHECKED);
        
        SetDlgItemInt(win,IDC_EDIT2,i>>4,0);
        
        i = rom[(j >= 0x85 ? 0x15b9f : 0x1561a) + j];
        
        if(i & 0x20)
            CheckDlgButton(win, IDC_CHECK1, BST_CHECKED);
        
        if(i & 0x02)
            CheckDlgButton(win, IDC_CHECK4, BST_CHECKED);
        
        switch(l = rom[(j >= 0x85 ? 0x15ba6 : 0x1569f) + j])
        {
        
        case 0:
            
            CheckDlgButton(win,IDC_RADIO6,BST_CHECKED);
            
            break;
        
        case 2:
            
            CheckDlgButton(win,IDC_RADIO8,BST_CHECKED);
            
            break;
        
        case 16:
            
            CheckDlgButton(win,IDC_RADIO7,BST_CHECKED);
            
            break;
        
        case 18:
            
            CheckDlgButton(win,IDC_RADIO11,BST_CHECKED);
            
            break;
        }
        
        SetDlgItemInt(win,
                      IDC_EDIT3,
                      (char) rom[(j >= 0x85 ? 0x15b8a : 0x15406) + j], 1);
        
        i = ( (unsigned short*) (rom + (j >= 0x85 ? 0x15b28 : 0x15724) ) )[j];
        
        if(i && i != 0xffff)
        {
            k = (i >> 15) + 1;
            
            SetDlgItemInt(win, IDC_EDIT13, (i & 0x7e) >> 1, 0);
            SetDlgItemInt(win, IDC_EDIT14, (i & 0x3f80) >> 7, 0);
        }
        else
        {
            k = 0;
            
            EnableWindow( GetDlgItem(win, IDC_EDIT13), 0);
            EnableWindow( GetDlgItem(win, IDC_EDIT14), 0);
        }
        
        CheckDlgButton(win, radio_ids[k], BST_CHECKED);
        
        j <<= 3;
        
        if(j >= 0x428)
            j += 0xe37;
        
        SetDlgItemInt(win, IDC_EDIT4, rom[0x1491d + j], 0);
        SetDlgItemInt(win, IDC_EDIT5, rom[0x1491e + j], 0);
        SetDlgItemInt(win, IDC_EDIT22, rom[0x1491f + j], 0);
        SetDlgItemInt(win, IDC_EDIT23, rom[0x14920 + j], 0);
        SetDlgItemInt(win, IDC_EDIT24, rom[0x14921 + j], 0);
        SetDlgItemInt(win, IDC_EDIT25, rom[0x14922 + j], 0);
        SetDlgItemInt(win, IDC_EDIT26, rom[0x14923 + j], 0);
        SetDlgItemInt(win, IDC_EDIT27, rom[0x14924 + j], 0);
        
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_RADIO10:
            EnableWindow(GetDlgItem(win,IDC_EDIT13),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT14),0);
            break;
        case IDC_RADIO12:
        case IDC_RADIO13:
            EnableWindow(GetDlgItem(win,IDC_EDIT13),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT14),1);
            break;
        case IDOK:
            
            rom = dunged->ew.doc->rom;
            
            j = dunged->ew.param;
            
            if(j < 0x85)
            {
                if( IsDlgButtonChecked(win, IDC_RADIO9) )
                    i = 2;
                else if( IsDlgButtonChecked(win, IDC_RADIO3) )
                    i = 1;
                else
                    i = 0;
                
                rom[0x15510 + j] = i;
            }
            else
                ((short*)(rom + 0x15b36))[j]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            
            i = IsDlgButtonChecked(win,IDC_RADIO5);
            
            rom[(j>=0x85?0x15b98:0x15595)+j]=i+(GetDlgItemInt(win,IDC_EDIT2,0,0)<<4);
            i=0;
            if(IsDlgButtonChecked(win,IDC_CHECK4)) i=2;
            if(IsDlgButtonChecked(win,IDC_CHECK1)) i|=32;
            rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
            if(IsDlgButtonChecked(win,IDC_RADIO6)) i=0;
            else if(IsDlgButtonChecked(win,IDC_RADIO8)) i=2;
            else if(IsDlgButtonChecked(win,IDC_RADIO7)) i=16;
            else i=18;
            rom[(j>=0x85?0x15ba6:0x1569f)+j]=i;
            
            rom[(j >= 0x85 ? 0x15b8a : 0x15406) + j] =
            GetDlgItemInt(win, IDC_EDIT3, 0, 1);
            
            if(IsDlgButtonChecked(win,IDC_RADIO10)) i=-1;
            else {
                i=(GetDlgItemInt(win,IDC_EDIT13,0,0)<<1)+(GetDlgItemInt(win,IDC_EDIT14,0,0)<<7);
                if(IsDlgButtonChecked(win,IDC_RADIO13)) i|=0x8000;
            }
            ((short*)(rom+(j>=0x85?0x15b28:0x15724)))[j]=i;
            i=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
            k=(i&496)<<5;
            i=(i&15)<<9;
            j<<=3;
            if(j>=0x428) j+=0xe37;
            rom[0x1491d + j] = GetDlgItemInt(win, IDC_EDIT4, 0, 0);
            rom[0x1491e + j] = GetDlgItemInt(win, IDC_EDIT5, 0, 0);
            rom[0x1491f + j] = GetDlgItemInt(win, IDC_EDIT22, 0, 0);
            rom[0x14920 + j] = GetDlgItemInt(win, IDC_EDIT23, 0, 0);
            rom[0x14921 + j] = GetDlgItemInt(win, IDC_EDIT24, 0, 0);
            rom[0x14922 + j] = GetDlgItemInt(win, IDC_EDIT25, 0, 0);
            rom[0x14923 + j] = GetDlgItemInt(win, IDC_EDIT26, 0, 0);
            rom[0x14924 + j] = GetDlgItemInt(win, IDC_EDIT27, 0, 0);
            dunged->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

BOOL CALLBACK editroomprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,l;
    HWND hc;
    static int hs;
    unsigned char*rom;
    const static char*ef_str[8]={
        "Nothing","01","Moving floor","Moving water","04","Red flashes","Light torch to see floor","Ganon room"
    };
    const static char * tag_str[64] = {
        "Nothing","NW Kill enemy to open","NE Kill enemy to open","SW Kill enemy to open","SE Kill enemy to open","W Kill enemy to open","E Kill enemy to open","N Kill enemy to open","S Kill enemy to open","Clear quadrant to open","Clear room to open",
        "NW Move block to open","NE Move block to open","SW Move block to open","SE Move block to open","W Move block to open","E Move block to open","N Move block to open",
        "S Move block to open","Move block to open","Pull lever to open","Clear level to open door","Switch opens door(Hold)","Switch opens door(Toggle)",
        "Turn off water","Turn on water","Water gate","Water twin","Secret wall (Right)","Secret wall (Left)","Crash","Crash",
        "Use switch to bomb wall","Holes(0)","Open chest for holes(0)","Holes(1)","Holes(2)","Kill enemy to clear level","SE Kill enemy to move block","Trigger activated chest",
        "Use lever to bomb wall","NW Kill enemy for chest","NE Kill enemy for chest","SW Kill enemies for chest","SE Kill enemy for chest","W Kill enemy for chest","E Kill enemy for chest","N Kill enemy for chest",
        "S Kill enemy for chest","Clear quadrant for chest","Clear room for chest","Light torches to open","Holes(3)","Holes(4)","Holes(5)","Holes(6)",
        "Agahnim's room","Holes(7)","Holes(8)","Open chest for holes(8)","Move block to get chest","Kill to open Ganon's door","Light torches to get chest","Kill boss again",
    };
    const static int warp_ids[17]={
        IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,
        IDC_STATIC6,IDC_STATIC7,IDC_STATIC8,IDC_EDIT4,
        IDC_EDIT6,IDC_EDIT7,IDC_EDIT15,IDC_EDIT16,
        IDC_EDIT17,IDC_EDIT18,IDC_EDIT19,IDC_EDIT20,
        IDC_EDIT21
    };
    const static char warp_flag[20]={
        0x07,0x07,0x07,0x0f,0x1f,0x3f,0x7f,0x07,
        0x07,0x0f,0x0f,0x1f,0x1f,0x3f,0x3f,0x7f,
        0x7f,0,0,0
    };
    
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        rom=dunged->ew.doc->rom;
        hc=GetDlgItem(win, IDC_COMBO1);
        for(i=0;i<8;i++) SendMessage(hc,CB_ADDSTRING,0,(long)ef_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[4],0);
        hc=GetDlgItem(win,IDC_COMBO2);
        for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[5],0);
        hc=GetDlgItem(win,IDC_COMBO3);
        for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
        SendMessage(hc,CB_SETCURSEL,dunged->hbuf[6],0);
        SendDlgItemMessage(win,IDC_BUTTON1,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[2]);
        SendDlgItemMessage(win,IDC_BUTTON3,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[3]);
        hs=dunged->hsize;
        SetDlgItemInt(win,IDC_EDIT6,dunged->hbuf[7]&3,0);       // Hole/warp plane
        SetDlgItemInt(win,IDC_EDIT15,(dunged->hbuf[7]>>2)&3,0); // Staircase plane 1
        SetDlgItemInt(win,IDC_EDIT17,(dunged->hbuf[7]>>4)&3,0); // Staircase plane 2
        SetDlgItemInt(win,IDC_EDIT19,(dunged->hbuf[7]>>6)&3,0);
        SetDlgItemInt(win,IDC_EDIT21,dunged->hbuf[8]&3,0);
        i=dunged->mapnum&0xff00;
        SetDlgItemInt(win,IDC_EDIT4,dunged->hbuf[9]+i,0);  // Hole/warp room
        SetDlgItemInt(win,IDC_EDIT7,dunged->hbuf[10]+i,0); // Staircase 1 room
        SetDlgItemInt(win,IDC_EDIT16,dunged->hbuf[11]+i,0); // Staircase 2 room
        SetDlgItemInt(win,IDC_EDIT18,dunged->hbuf[12]+i,0);
        SetDlgItemInt(win,IDC_EDIT20,dunged->hbuf[13]+i,0);
        SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x3f61d))[dunged->mapnum],0);
        
        for(i=0;i<57;i++) if(((short*)(rom + 0x190c))[i]==dunged->mapnum) {
            CheckDlgButton(win,IDC_CHECK5,BST_CHECKED);
            break;
        }
        
    updbtn:
        
        l = 1 << (hs - 7);
        
        for(i=0;i<17;i++)
            ShowWindow(GetDlgItem(win,warp_ids[i]),(warp_flag[i]&l)?SW_HIDE:SW_SHOW);
        
        EnableWindow(GetDlgItem(win,IDC_BUTTON1),hs>7);
        EnableWindow(GetDlgItem(win,IDC_BUTTON3),hs<14);
        
        break;
    
    case WM_COMMAND:
        switch(wparam) {
        case IDC_BUTTON1:
            hs--;
            if(hs==9) hs=7;
            goto updbtn;
        case IDC_BUTTON3:
            hs++;
            if(hs==8) hs=10;
            goto updbtn;
        case IDC_CHECK5:
            rom=dunged->ew.doc->rom;
            if(IsDlgButtonChecked(win,IDC_CHECK5)) {
                for(i=0;i<57;i++)
                    if( is16b_neg1_i(rom + 0x190c, i) )
                    {
                        ((short*)(rom + 0x190c))[i]=dunged->mapnum;
                        break;
                    }
                
                if(i == 57)
                {
                    MessageBox(framewnd,"You can't add anymore.","Bad error happened",MB_OK);
                    CheckDlgButton(win,IDC_CHECK5,BST_UNCHECKED);
                }
            } else for(i=0;i<57;i++) if(((short*)(rom + 0x190c))[i]==dunged->mapnum) {
                ((short*)(rom + 0x190c))[i]=-1;
                break;
            }
            break;
        case IDOK:
            rom=dunged->ew.doc->rom;
            dunged->hsize = hs;
            dunged->hbuf[4] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO1,CB_GETCURSEL,0,0);
            dunged->hbuf[5] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO2,CB_GETCURSEL,0,0);
            dunged->hbuf[6] = (unsigned char) SendDlgItemMessage(win,IDC_COMBO3,CB_GETCURSEL,0,0);
            dunged->hbuf[7]=(GetDlgItemInt(win,IDC_EDIT6,0,0)&3)|
            ((GetDlgItemInt(win,IDC_EDIT15,0,0)&3)<<2)|
            ((GetDlgItemInt(win,IDC_EDIT17,0,0)&3)<<4)|
            ((GetDlgItemInt(win,IDC_EDIT19,0,0)&3)<<6);
            dunged->hbuf[8]=GetDlgItemInt(win,IDC_EDIT21,0,0)&3;
            dunged->hbuf[9]=GetDlgItemInt(win,IDC_EDIT4,0,0);
            dunged->hbuf[10]=GetDlgItemInt(win,IDC_EDIT7,0,0);
            dunged->hbuf[11]=GetDlgItemInt(win,IDC_EDIT16,0,0);
            dunged->hbuf[12]=GetDlgItemInt(win,IDC_EDIT18,0,0);
            dunged->hbuf[13]=GetDlgItemInt(win,IDC_EDIT20,0,0);
            ((short*)(rom + 0x3f61d))[dunged->mapnum]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            
            dunged->modf  = 1;
            dunged->hmodf = 1;
            
            dunged->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
        }
    }
    return FALSE;
}

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

BOOL CALLBACK editexit(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    unsigned char*rom;
    int i,j,k,l,m,n,o;
    HWND hc;
    const static int radio_ids[6]={IDC_RADIO2,IDC_RADIO4,IDC_RADIO5,IDC_RADIO6,IDC_RADIO7,IDC_RADIO8};
    const static int door_ofs[2]={0x16367,0x16405};
    const static int edit_ids[6]={IDC_EDIT8,IDC_EDIT9,IDC_EDIT10,IDC_EDIT11};
    const static int hide_ids[20]={IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,IDC_STATIC6,
        IDC_EDIT15,IDC_EDIT22,IDC_EDIT23,IDC_EDIT24,IDC_EDIT25,
        IDC_STATIC7,IDC_STATIC8,IDC_STATIC9,IDC_STATIC10,IDC_STATIC11,
        IDC_EDIT26,IDC_EDIT27,IDC_EDIT28,IDC_EDIT29,IDC_EDIT30};
    
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        rom=oved->ew.doc->rom;
        i=oved->selobj;
        j=(oved->ew.param&7)<<9;
        k=(oved->ew.param&56)<<6;
        SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x15d8a))[i],0);
        SetDlgItemInt(win,IDC_EDIT2,((short*)(rom + 0x15f15))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT3,((short*)(rom + 0x15fb3))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT4,((short*)(rom + 0x1618d))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT5,((short*)(rom + 0x1622b))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT6,((short*)(rom + 0x16051))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT7,((short*)(rom + 0x160ef))[i]-j,1);
        for(j=0;j<2;j++) {
            l=((unsigned short*)(rom+door_ofs[j]))[i];
            if(l && l!=65535) {
                m=(l>>15)+1;
                SetDlgItemInt(win,edit_ids[j*2],(l&0x7e)>>1,0);
                SetDlgItemInt(win,edit_ids[j*2+1],(l&0x3f80)>>7,0);
            } else {
                m=0;
                EnableWindow(GetDlgItem(win,edit_ids[j*2]),0);
                EnableWindow(GetDlgItem(win,edit_ids[j*2+1]),0);
            }
            m+=3*j;
            CheckDlgButton(win,radio_ids[m],BST_CHECKED);
        }
        SetDlgItemInt(win,IDC_EDIT12,rom[0x15e28 + i], 0);
        SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x162c9 + i], 1);
        SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16318 + i], 1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_EDIT1|(EN_CHANGE<<16):
            i=GetDlgItemInt(win,IDC_EDIT1,0,0);
            rom=oved->ew.doc->rom;
            if(i>=0x180 && i<0x190) {
                j=SW_SHOW;
                SetDlgItemInt(win,IDC_EDIT15,rom[0x16681 + i] >> 1, 0);
                SetDlgItemInt(win,IDC_EDIT22,rom[0x16691 + i], 0);
                SetDlgItemInt(win,IDC_EDIT23,rom[0x166a1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT24,rom[0x166b1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT25,rom[0x166c1 + i], 0);
                SetDlgItemInt(win,IDC_EDIT26,((short*)(rom + 0x163e1))[i], 0);
                SetDlgItemInt(win,IDC_EDIT27,((short*)(rom + 0x16401))[i], 0);
                SetDlgItemInt(win,IDC_EDIT28,((short*)(rom + 0x16421))[i], 0);
                SetDlgItemInt(win,IDC_EDIT29,((short*)(rom + 0x16441))[i], 0);
                SetDlgItemInt(win,IDC_EDIT30,((short*)(rom + 0x164e1))[i], 0);
            } else j=SW_HIDE;
            for(k=0;k<20;k++)
                ShowWindow(GetDlgItem(win,hide_ids[k]),j);
            break;
        case IDC_RADIO2:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),0);
            break;
        case IDC_RADIO4: case IDC_RADIO5:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),1);
            break;
        case IDC_RADIO6:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),0);
            break;
        case IDC_RADIO7: case IDC_RADIO8:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),1);
            break;
        case IDOK:
            hc=GetDlgItem(oved->dlg,3001);
            Overselchg(oved,hc);
            rom=oved->ew.doc->rom;
            i=oved->selobj;
            l = rom[0x15e28 + i] = GetDlgItemInt(win,IDC_EDIT12,0,0);
            if(l!=oved->ew.param) oved->selobj=-1;
            n=oved->mapsize?1023:511;
            j=(l&7)<<9;
            k=(l&56)<<6;
            ((short*)(rom + 0x15d8a))[i]=o=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ((short*)(rom + 0x15f15))[i]=(l=(GetDlgItemInt(win,IDC_EDIT2,0,0)&n))+k;
            ((short*)(rom + 0x15fb3))[i]=(m=(GetDlgItemInt(win,IDC_EDIT3,0,0)&n))+j;
            ((short*)(rom + 0x15e77))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
            ((short*)(rom + 0x1618d))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
            ((short*)(rom + 0x1622b))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
            ((short*)(rom + 0x16051))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
            ((short*)(rom + 0x160ef))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
            m=0;
            for(j=0;j<2;j++) {
                if(IsDlgButtonChecked(win,radio_ids[m])) l=0;
                else {
                    l=(GetDlgItemInt(win,edit_ids[j*2],0,0)<<1)+(GetDlgItemInt(win,edit_ids[j*2+1],0,0)<<7);
                    if(IsDlgButtonChecked(win,radio_ids[m+2])) l|=0x8000;
                }
                ((short*)(rom+door_ofs[j]))[i]=l;
                m+=3;
            }
            rom[0x162c9 + i] = GetDlgItemInt(win,IDC_EDIT13,0,1);
            rom[0x16318 + i] = GetDlgItemInt(win,IDC_EDIT14,0,1);
            if(o>=0x180 && o<0x190)
            {
                rom[0x16681 + o] = GetDlgItemInt(win,IDC_EDIT15,0,0)<<1;
                rom[0x16691 + o] = GetDlgItemInt(win,IDC_EDIT22,0,0);
                rom[0x166a1 + o] = GetDlgItemInt(win,IDC_EDIT23,0,0);
                rom[0x166b1 + o] = GetDlgItemInt(win,IDC_EDIT24,0,0);
                rom[0x166c1 + o] = GetDlgItemInt(win,IDC_EDIT25,0,0);
                
                ((short*)(rom + 0x163e1))[o]=GetDlgItemInt(win,IDC_EDIT26,0,0);
                ((short*)(rom + 0x16401))[o]=GetDlgItemInt(win,IDC_EDIT27,0,0);
                ((short*)(rom + 0x16421))[o]=GetDlgItemInt(win,IDC_EDIT28,0,0);
                ((short*)(rom + 0x16441))[o]=GetDlgItemInt(win,IDC_EDIT29,0,0);
                ((short*)(rom + 0x164e1))[o]=GetDlgItemInt(win,IDC_EDIT30,0,0);
            }
            Overselchg(oved,hc);
            oved->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}
BOOL CALLBACK editwhirl(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    unsigned char*rom;
    int i,j,k,l,m,n;
    HWND hc;
    
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        rom=oved->ew.doc->rom;
        i=oved->selobj;
        j=(oved->ew.param&7)<<9;
        k=(oved->ew.param&56)<<6;
        if(i>8) SetDlgItemInt(win,IDC_EDIT1,((short*)(rom + 0x16ce6))[i],0);
        else {
            ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_HIDE);
        }
        SetDlgItemInt(win,IDC_EDIT2,((short*)(rom + 0x16b29))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT3,((short*)(rom + 0x16b4b))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT4,((short*)(rom + 0x16bb1))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT5,((short*)(rom + 0x16bd3))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT6,((short*)(rom + 0x16b6d))[i]-k,1);
        SetDlgItemInt(win,IDC_EDIT7,((short*)(rom + 0x16b8f))[i]-j,1);
        SetDlgItemInt(win,IDC_EDIT12,((short*)(rom + 0x16ae5))[i],0);
        SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x16bf5 + i],1);
        SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16c17 + i],1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_RADIO2:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),0);
            break;
        case IDC_RADIO4: case IDC_RADIO5:
            EnableWindow(GetDlgItem(win,IDC_EDIT8),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT9),1);
            break;
        case IDC_RADIO6:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),0);
            break;
        case IDC_RADIO7: case IDC_RADIO8:
            EnableWindow(GetDlgItem(win,IDC_EDIT10),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT11),1);
            break;
        case IDOK:
            hc=GetDlgItem(oved->dlg,3001);
            Overselchg(oved,hc);
            rom=oved->ew.doc->rom;
            i=oved->selobj;
            l=((short*)(rom + 0x16ae5))[i]=GetDlgItemInt(win,IDC_EDIT12,0,0);
            n=(rom[0x12884 + l]<<8)|0xff;
            j=(l&7)<<9;
            k=(l&56)<<6;
            if(l!=oved->ew.param) oved->selobj=-1;
            if(i>8) ((short*)(rom + 0x16ce6))[i]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ((short*)(rom + 0x16b29))[i]=(l=GetDlgItemInt(win,IDC_EDIT2,0,0)&n)+k;
            ((short*)(rom + 0x16b4b))[i]=(m=GetDlgItemInt(win,IDC_EDIT3,0,0)&n)+j;
            ((short*)(rom + 0x16b07))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
            ((short*)(rom + 0x16bb1))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
            ((short*)(rom + 0x16bd3))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
            ((short*)(rom + 0x16b6d))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
            ((short*)(rom + 0x16b8f))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
            rom[0x16bf5 + i] = GetDlgItemInt(win,IDC_EDIT13,0,1);
            rom[0x16c17 + i] = GetDlgItemInt(win,IDC_EDIT14,0,1);
            Overselchg(oved,hc);
            oved->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}
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
static char*screen_text[]={
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

static char*level_str[]={
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

//Setpalmode#*******************************

void Setpalmode(DUNGEDIT *ed)
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

//Setpalmode********************************

//Setfullmode#******************************

void Setfullmode(DUNGEDIT *ed)
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

void Updpal(void*ed)
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

// Fix entrance scrolling settings.
// Seems to update those 8 numbers that control scrolls in entrances?
void FixEntScroll(FDOC*doc,int j)
{
    unsigned char *rom = doc->rom;
    HWND win;
    int i,k,l,n,o,p;
    
    const static unsigned char hfl[4]={0xd0,0xb0};
    const static unsigned char vfl[4]={0x8a,0x86};
    
    k=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
    
    win = doc->dungs[k];
    
    if(win)
        n = GetDlgItemInt(win, ID_DungLayout, 0, 0);
    else
        n = rom[romaddr(*(int*)(rom + 0xf8000 + k * 3))+1]>>2;
    
    o = rom[(j >= 0x85 ? 0x15ac7 : 0x14f5a) + (j << 1)] & 1;
    p = rom[(j >= 0x85 ? 0x15ad5 : 0x15064) + (j << 1)] & 1;
    
    i = (((vfl[o] >> n) << 1) & 2) + (((hfl[p] >> n) << 5) & 32);
    
    rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
    rom[(j>=0x85?0x15ba6:0x1569f)+j]=(o<<1)+(p<<4);
    
    i = (k >> 4) << 1;
    l = (k & 15) << 1;
    
    j <<= 3;
    
    if(j >= 0x428)
        j += 0xe37;
    
    rom[0x1491d + j] = i + o;
    rom[0x1491e + j] = i;
    rom[0x1491f + j] = i + o;
    rom[0x14920 + j] = i + 1;
    rom[0x14921 + j] = l + p;
    rom[0x14922 + j] = l;
    rom[0x14923 + j] = l + p;
    rom[0x14924 + j] = l + 1;
}

//dungdlgproc*********************************

BOOL CALLBACK dungdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int j, // the "entrance" number. not a room number. Is used to retrieve the room #
        k, // the room number retrieved using j.
        i, // 
        l,
        m,
        n;
    
    HWND hc;
    
    DUNGEDIT *ed;
    
    uint8_t *buf2;
    
    unsigned char *rom;
    
//  const static int inf_ofs[5]={0x14813,0x14f59,0x15063,0x14e4f,0x14d45};
//  const static int inf_ofs2[5]={0x156be,0x15bb4,0x15bc2,0x15bfa,0x15bec};
    
    // music offsets?
    const static unsigned char mus_ofs[] =
    {
        255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,240,241,242,243
    };
    
    const static char *bg2_str[] =
    {
        "Off",
        "Parallaxing",
        "Dark",
        "On top",
        "Translucent",
        "Parallaxing2",
        "Normal",
        "Addition",
        "Dark room"
    };
    
    const static char *coll_str[] =
    {
        "One",
        "Both",
        "Both w/scroll",
        "Moving floor",
        "Moving water"
    };
    
    // message handling for the window.
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        ed = (DUNGEDIT*) lparam;
        
        // get the 'room' we're looking at.
        j = ed->ew.param;
        
        rom = ed->ew.doc->rom;
        
        // the dungedit has been initialized.
        ed->init = 1;
        
        // pass this win to ed's dialogue.
        ed->dlg = win;
        
        // void the HPALETTE in ed.
        ed->hpal = 0;
        
        if(j >= 0x8c)
        {
            // Only certain controls are necessary for working with a dungeon
            // overlay or layout, so hide the rest.
            for(i = 0; i < ID_DungOverlayHideCount; i++)
                ShowWindow( GetDlgItem(win, overlay_hide[i]), SW_HIDE );
            
            if(j < 0x9f)
                buf2 = rom + romaddr( *(int*) (rom + 0x26b1c + j * 3) );
            else if(j < 0xa7)
                buf2 = rom + romaddr( *(int*) (rom + romaddr( *(int*) (rom + 0x882d) ) + (j - 0x9f) * 3 ) );
            else
                buf2 = rom + (rom[0x9c25] << 15) - 0x8000 + *(unsigned short*) (rom + 0x9c2a);
            
            for(i = 0; ; i += 3)
                if( is16b_neg1(buf2 + i) )
                    break;
            
            ed->chkofs[0] = 2;
            ed->len = ed->chkofs[2] = ed->chkofs[1] = i + 4;
            ed->selobj = 2;
            ed->buf = (uint8_t*) malloc(i + 4);
            
            *(short*)(ed->buf)=15;
            
            memcpy(ed->buf+2,buf2,i+2);
            ed->hbuf[0] = 0;
            ed->hbuf[1] = 1;
            ed->hbuf[2] = 0;
            ed->hbuf[3] = 0;
            ed->hbuf[4] = 0;
            ed->gfxtmp = 0;
            
            CheckDlgButton(win, ID_DungShowBG1, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowBG2, BST_CHECKED);
            
            Initroom(ed,win);
        }
        else
        {
            // not an overlay
            
            k = ( (short*) ( rom + ( j >= 0x85 ? 0x15a64 : 0x14813 ) ) )[j];
            
            // read in the room number, put it to screen.
            SetDlgItemInt(win, ID_DungEntrRoomNumber, k, 0);
            
            i = (k & 0x1f0) << 5;
            l = (k & 0x00f) << 9;
            
            if(j >= 0x85)
            {
                // these are the starting location maps
                SetDlgItemInt(win, ID_DungEntrYCoord, ((short*) (rom + 0x15ac6) )[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXCoord, ((short*) (rom + 0x15ad4) )[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrYScroll, ((short*) (rom + 0x15ab8) )[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXScroll, ((short*) (rom + 0x15aaa) )[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrCameraX, ((short*) (rom + 0x15af0) )[j], 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, ((short*) (rom + 0x15ae2) )[j], 0);
            }
            else
            {
                // these are normal rooms
                SetDlgItemInt(win, ID_DungEntrYCoord, ((short*) (rom + 0x14f59))[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXCoord, ((short*) (rom + 0x15063))[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrYScroll, ((short*) (rom + 0x14e4f))[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXScroll, ((short*) (rom + 0x14d45))[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrCameraX, ((short*) (rom + 0x15277))[j], 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, ((short*) (rom + 0x1516d))[j], 0);
            }
            
            // the show BG1, BG2, Sprite, check boxes.
            // make them initially checked.
            CheckDlgButton(win, ID_DungShowBG1, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowBG2, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowSprites, BST_CHECKED);
            
            for(i = 0; i < 4; i++)
                // map in the arrow bitmaps for those buttons at the top l.eft
                SendDlgItemMessage(win, ID_DungLeftArrow + i, BM_SETIMAGE, IMAGE_BITMAP, (int) arrows_imgs[i]);
            
            for(i = 0; i < 40; i++)
                // map in the names of the music tracks
                SendDlgItemMessage(win, ID_DungEntrSong, CB_ADDSTRING, 0, (int) mus_str[i]);
            
            // if j is a starting location, use 0x15bc9, otherwise 0x1582e as an offset.
            l = rom[ (j >= 0x85 ? 0x15bc9 : 0x1582e) + j ];
            
            for(i = 0; i < 40; i++)
            {
                if(mus_ofs[i] == l)
                {
                    SendDlgItemMessage(win, ID_DungEntrSong, CB_SETCURSEL, i, 0);
                    
                    break;
                }
            }
            
            for(i = 0; i < 9; i++)
                SendDlgItemMessage(win, ID_DungBG2Settings, CB_ADDSTRING, 0, (int) bg2_str[i]);
            
            for(i = 0; i < 5; i++)
                SendDlgItemMessage(win, ID_DungCollSettings, CB_ADDSTRING, 0, (int) coll_str[i]);
            
            for(i = 0; i < 15; i++)
            {
                SendDlgItemMessage(win,
                                   ID_DungEntrPalaceID,
                                   CB_ADDSTRING,
                                   0,
                                   (int) level_str[i]);
            }
            
            SendDlgItemMessage(win,
                               ID_DungEntrPalaceID,
                               CB_SETCURSEL,
                               ( (rom[ (j >= 0x85 ? 0x15b91 : 0x1548b) + j] + 2) >> 1) & 0x7f,
                               0);
            
            ed->gfxtmp = rom[(j >= 0x85 ? 0x15b83 : 0x15381) + j];
            
            SetDlgItemInt(win, ID_DungEntrTileSet, ed->gfxtmp, 0);
            
            Openroom(ed, k);
        }
        
        ed->pal[0] = blackcolor;
        
        // for the first entry of each palette, the first color is "blackcolor"
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        // start at the top left corner of the dungeon map
        ed->mapscrollh = 0;
        ed->mapscrollv = 0;
        
        // it's the bit map information header... whatever that is.
        ed->bmih = zbmih;
        ed->selchk = 0;
        ed->withfocus = 0;
        ed->disp = (SD_DungShowBothBGs | SD_DungShowMarkers);
        ed->anim = 0;
        ed->init = 0;
        
        Getblocks(ed->ew.doc, 0x79);
        Getblocks(ed->ew.doc, 0x7a);
        
        Addgraphwin(ed, 1);
        Updatemap(ed);
        
        goto updpal3;
    
    case 4002: // used as a code to update the palette? so it seems
        
        InvalidateRect( GetDlgItem(win, ID_DungEditWindow), 0, 0);
        
        break;
    
    case WM_COMMAND:
        
        ed = (DUNGEDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed || ed->init)
            break;
        
        switch(wparam)
        {
        
        // the arrow buttons
        case ID_DungLeftArrow:
        case ID_DungRightArrow:
        case ID_DungUpArrow:
        case ID_DungDownArrow:
            
            k = ed->mapnum;
            
            k = ( k + nxtmap[wparam - ID_DungLeftArrow] & 0xff) + (k & 0x100);
            
            if(ed->ew.doc->dungs[k])
            {
                MessageBox(framewnd,"The room is already open in another editor","Bad error happened",MB_OK);
                break;
            }
newroom:
            
            if(Closeroom(ed))
                break;
            
            ed->init = 1;
            
            Openroom(ed, k);
            
            ed->init = 0;
            Updatemap(ed);
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed, hc, 1);
            InvalidateRect(hc,0,0);
            
            goto updpal;
        
        case ID_DungChangeRoom:
            
            k = askinteger(295, "Jump to room", "Room #");
            
            if(k == -1)
                break;
            
            goto newroom;
        
        case ID_DungShowBG1:
            
            // \task What's with these bitpatterns? enumerate them?
            ed->disp &= SD_DungHideBG1;
            
            if(IsDlgButtonChecked(win, ID_DungShowBG1) == BST_CHECKED)
                ed->disp |= SD_DungShowBG1;
            
            goto updscrn;
        
        case ID_DungShowBG2:
            
            ed->disp &= SD_DungHideBG2;
            
            if(IsDlgButtonChecked(win, ID_DungShowBG2) == BST_CHECKED)
                ed->disp |= SD_DungShowBG2;
            
            goto updscrn;
        
        case ID_DungShowSprites:
            
            ed->disp &= SD_DungHideMarkers;
            
            if(IsDlgButtonChecked(win, ID_DungShowSprites) == BST_CHECKED)
                ed->disp |= SD_DungShowMarkers;
            
            goto updscrn;
        
        case ID_DungAnimateButton:
            
            ed->anim++;
            
            if(ed->anim == 3)
                ed->anim = 0;
            
            wsprintf(buffer, "Frm%d", ed->anim + 1);
            
            SetWindowText((HWND)lparam,buffer);
            
            goto updscrn;
        
        case ID_DungObjLayer1:
            
            i = 0;
            
        updchk:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            
            Dungselectchg(ed, hc, 0);
            
            if(ed->chkofs[i + 1] == ed->chkofs[i] + 2)
            {
                i++;
                
                if(ed->chkofs[i + 1] <= ed->chkofs[i] + 2)
                {
                    i--;
                    ed->selobj = 0;
                    
                    goto no_obj;
                }
            }
            
            ed->selobj = ed->chkofs[i];
no_obj:
            ed->selchk = i;
            
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungObjLayer2:
            
            i = 2;
            
            goto updchk;
        
        case ID_DungObjLayer3:
            
            i = 4;
            
            goto updchk;
        
        case ID_DungSprLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            
            Dungselectchg(ed,hc,0);
            
            ed->selchk = 6;
            ed->selobj = ed->esize > 2;
            
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungEntrRoomNumber | (EN_CHANGE << 16):
        case ID_DungEntrYCoord | (EN_CHANGE << 16):
        case ID_DungEntrXCoord | (EN_CHANGE << 16):
        case ID_DungEntrYScroll | (EN_CHANGE << 16):
        case ID_DungEntrXScroll | (EN_CHANGE << 16):
        case ID_DungFloor1 | (EN_CHANGE << 16):
        case ID_DungFloor2 | (EN_CHANGE << 16):
        case ID_DungEntrTileSet | (EN_CHANGE << 16):
        case ID_DungLayout | (EN_CHANGE << 16):
        case ID_DungTileSet | (EN_CHANGE << 16):
        case ID_DungPalette | (EN_CHANGE << 16):
        case ID_DungSprTileSet | (EN_CHANGE << 16):
        case ID_DungEntrCameraX | (EN_CHANGE << 16):
        case ID_DungEntrCameraY | (EN_CHANGE << 16):
            
            wparam &= 65535;
            
            j = GetDlgItemInt(win, wparam, 0, 0);
            
            rom = ed->ew.doc->rom;
            
            // \task consider a different check for this. The enumerations
            // could change order.
            if(wparam > ID_DungEntrXScroll)
            {
                switch(wparam)
                {
                
                case ID_DungFloor1:
                    
                    if(j>15)
                    {
                        SetDlgItemInt(win, ID_DungFloor1, 15, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungFloor1, 0, 0);
                        break;
                    }
                    
                    ed->buf[0] &= 0xf0;
                    ed->buf[0] |= j;
                    
                    if(ed->ew.param < 0x8c)
                        ed->modf = 1;
updmap:
                    
                    Updatemap(ed);
                    
updscrn: // update the screen (obviously)
                    
                    hc = GetDlgItem(win, ID_DungEditWindow);
                    
                    InvalidateRect(hc,0,0);
                    
                    break;
                
                case ID_DungFloor2:
                    
                    if(j > 15)
                    {
                        SetDlgItemInt(win, ID_DungFloor2, 15, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungFloor2, 0, 0);
                        break;
                    }
                    
                    ed->buf[0]&=0xf;
                    ed->buf[0]|=j<<4;
                    ed->modf=1;
                    
                    goto updmap;
                
                case ID_DungLayout:
                    
                    if(j > 7)
                    {
                        SetDlgItemInt(win, ID_DungLayout, 7, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungLayout, 0, 0);
                        break;
                    }
                    
                    ed->buf[1] = j << 2;
                    ed->modf = 1;
                    
                    for(i = 0; i < 0x85; i++)
                    {
                        if(((short*)(rom + 0x14813))[i] == ed->mapnum)
                            FixEntScroll(ed->ew.doc,i);
                    }
                    
                    for( ; i < 0x8c; i++)
                    {
                        if(((short*)(rom + 0x15a64))[i] == ed->mapnum)
                            FixEntScroll(ed->ew.doc, i);
                    }
                    
                    goto updmap;
                
                case ID_DungEntrTileSet:
                    
                    if(j > 30)
                    {
                        SetDlgItemInt(win, ID_DungEntrTileSet, 30, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungEntrTileSet, 0, 0);
                        break;
                    }
                    
                    ed->gfxtmp = j;
                    rom[ed->ew.param + (ed->ew.param >= 0x85 ? 0x15b83 : 0x15381)] = j;
                    ed->ew.doc->modf = 1;
updgfx:
                    
                    for(i = 0; i < 0; i++)
                        Releaseblks(ed->ew.doc,ed->blocksets[i]);
                    
                    j = 0x6073 + (ed->gfxtmp << 3);
                    l = 0x5d97 + (ed->gfxnum << 2);
                    
                    for(i = 0; i < 8; i++)
                        ed->blocksets[i] = rom[j++];
                    
                    for(i = 3; i < 7; i++)
                    {
                        if(m = rom[l++])
                            ed->blocksets[i] = m;
                    }
                    
                    ed->blocksets[8] = rom[0x1011e + ed->gfxnum];
                    
                    for(i=0;i<9;i++)
                        Getblocks(ed->ew.doc,ed->blocksets[i]);
                    
                    goto updscrn;
                
                case ID_DungTileSet:
                    
                    if(j > 23)
                    {
                        SetDlgItemInt(win, ID_DungTileSet, 23, 0);
                        
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungTileSet, 0, 0);
                        
                        break;
                    }
                    
                    ed->gfxnum = j;
                    ed->modf = 1;
                    ed->hmodf = 1;
                    
                    goto updgfx;
                
                case ID_DungSprTileSet:
                    
                    ed->sprgfx = j;
                    ed->modf   = 1;
                    ed->hmodf  = 1;
                    
                    l = 0x5c57 + (j << 2);
                    
                    for(i = 0; i < 4; i++)
                    {
                        Releaseblks(ed->ew.doc, ed->blocksets[i + 11]);
                        
                        ed->blocksets[i+11]=rom[l+i] + 0x73;
                        
                        Getblocks(ed->ew.doc,ed->blocksets[i+11]);
                    }
                    
                    break;
                    
                updpal:
                    
                    rom = ed->ew.doc->rom;
                    
                updpal3:
                    
                    j = ed->palnum;
                    
                    goto updpal2;
                
                case ID_DungPalette:
                    
                    if(j > 71)
                    {
                        SetDlgItemInt(win, ID_DungPalette, 71, 0);
                        
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungPalette, 0, 0);
                        
                        break;
                    }
                    
                    ed->palnum = j;
                    
                    if(ed->ew.param < 0x8c)
                    {
                        ed->modf = 1;
                        ed->hmodf = 1;
                    }
                    
updpal2:
                    
                    buf2 = rom + (j << 2) + 0x75460;
                    i = 0x1bd734 + buf2[0]*90;
                    
                    Loadpal(ed, rom, i, 0x21, 15, 6);
                    Loadpal(ed, rom, i, 0x89, 7, 1);
                    Loadpal(ed, rom, 0x1bd39e + buf2[1] * 14, 0x81, 7, 1);
                    Loadpal(ed, rom, 0x1bd4e0 + buf2[2] * 14, 0xd1, 7, 1);
                    Loadpal(ed, rom, 0x1bd4e0 + buf2[3] * 14, 0xe1, 7, 1);
                    Loadpal(ed, rom, 0x1bd308, 0xf1, 15, 1);
                    Loadpal(ed, rom, 0x1bd648, 0xdc, 4, 1);
                    Loadpal(ed, rom, 0x1bd630, 0xd9, 3, 1);
                    Loadpal(ed, rom, 0x1bd218, 0x91, 15, 4);
                    Loadpal(ed, rom, 0x1bd6a0, 0, 16, 2);
                    
                    Loadpal(ed, rom, 0x1bd4d2, 0xe9, 7, 1);
                    
                    if(ed->hbuf[4] == 6)
                        Loadpal(ed, rom, 0x1be22c, 0x7b, 2, 1);
                    
                    Updpal(ed);
                    
                    break;
                
                case ID_DungEntrCameraX:
                    
                    ((short*)(rom+(ed->ew.param>=0x85?0x15af0:0x15277)))[ed->ew.param]=j;
                    ed->ew.doc->modf=1;
                    
                    break;
                
                case ID_DungEntrCameraY:
                    
                    ((short*)(rom+(ed->ew.param>=0x85?0x15ae2:0x1516d)))[ed->ew.param]=j;
                    ed->ew.doc->modf=1;
                    
                    break;
                }
            }
            else
            {
                j = ed->ew.param;
                k = GetDlgItemInt(win, ID_DungEntrRoomNumber, 0, 0);
                
                if((unsigned)k > 295)
                {
                    ed->init = 1;
                    SetDlgItemInt(win, ID_DungEntrRoomNumber, 295, 0);
                    ed->init = 0;
                    k = 295;
                }
                
                ((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j]=k;
                i=(k&496)<<5;
                l=(k&15)<<9;
                
                m = GetDlgItemInt(win, ID_DungEntrYScroll, 0, 0);
                n = GetDlgItemInt(win, ID_DungEntrXScroll, 0, 0);
                
                if(j>=0x85)
                {
                    ((short*)(rom + 0x15ac6))[j]=GetDlgItemInt(win, ID_DungEntrYCoord, 0, 0)+i;
                    ((short*)(rom + 0x15ad4))[j]=GetDlgItemInt(win, ID_DungEntrXCoord, 0, 0)+l;
                    ((short*)(rom + 0x15ab8))[j] = m + i;
                    ((short*)(rom + 0x15aaa))[j] = n + l;
                }
                else
                {
                    ((short*)(rom + 0x14f59))[j]=GetDlgItemInt(win, ID_DungEntrYCoord, 0, 0)+i;
                    ((short*)(rom + 0x15063))[j]=GetDlgItemInt(win, ID_DungEntrXCoord, 0, 0)+l;
                    ((short*)(rom + 0x14e4f))[j] = m + i;
                    ((short*)(rom + 0x14d45))[j] = n + l;
                }
                
                SetDlgItemInt(win, ID_DungEntrCameraX, n + 127, 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, m + 119, 0);
                
                if(wparam < ID_DungStatic4)
                    FixEntScroll(ed->ew.doc, j);
            }
            
            break;
        
        case ID_DungEntrSong | (CBN_SELCHANGE << 16):
        case ID_DungEntrPalaceID | (CBN_SELCHANGE << 16):
        case ID_DungBG2Settings | (CBN_SELCHANGE << 16):
        case ID_DungCollSettings | (CBN_SELCHANGE << 16):
            
            rom = ed->ew.doc->rom;
            
            j = SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
            
            if(j == -1)
                break;
            
            switch(wparam & 65535)
            {
            
            case ID_DungEntrSong:
                
                rom
                [
                    (ed->ew.param >= 0x85 ? 0x15bc9 : 0x1582e)
                  + ed->ew.param
                ] = mus_ofs[j];
                
                ed->ew.doc->modf = 1;
                
                break;
            
            case ID_DungEntrPalaceID:
                
                rom
                [
                    (ed->ew.param >= 0x85 ? 0x15b91 : 0x1548b)
                  + ed->ew.param
                ] = (j << 1) - 2;
                
                ed->ew.doc->modf = 1;
                
                goto updscrn;
            
            case ID_DungBG2Settings:
                
                ed->layering=bg2_ofs[j];
                ed->hmodf=ed->modf=1;
                
                goto updscrn;
            
            case ID_DungCollSettings:
                
                ed->coll=j;
                ed->hmodf=ed->modf=1;
                
                break;
            }
            
            break;
        
        case ID_DungEntrProps:
            
            dunged = ed;
            
            ShowDialog(hinstance,
                       (LPSTR) IDD_DIALOG14,
                       framewnd,
                       editdungprop,
                       0);
            
            break;
        
        case ID_DungEditHeader:
            
            dunged=ed;
            
            ShowDialog(hinstance,
                       (LPSTR) IDD_HEADER,
                       framewnd,
                       editroomprop,
                       0);
            
            goto updpal;
        
        case ID_DungItemLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk=7;
            ed->selobj=(ed->ssize>2)?2:0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungBlockLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk=8;
            ed->selobj=0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungTorchLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk=9;
            ed->selobj=(ed->tsize>2)?2:0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungSortSprites:
            
            *ed->ebuf^=1;
            ed->modf=1;
            
            break;
        
        case ID_DungExit:
            rom=ed->ew.doc->rom;
            for(i=0;i<79;i++)
            {
                if(((unsigned short*)(rom + 0x15d8a))[i]==ed->mapnum)
                    goto foundexit;
            }
            
            MessageBox(framewnd,"None is set.","Bad error happened",MB_OK);
            
            break;
foundexit:
            
            SendMessage(ed->ew.doc->editwin, 4000, rom[0x15e28 + i] + 0x20000,0);
        }
        
        break;
    }
    
    return FALSE;
}

//dungdlgproc*******************************************

//getgbmap**********************************************

int getbgmap(OVEREDIT*ed,int a,int b) {
    int c,d;
    switch(a) {
    case 0:
        d=0;
        if(b==2) c=158; else c=151;
        break;
    case 64:
        d=0;
        c=151;
        break;
    case 128:
        d=2;
        if(b==2) c=160; else c=151;
        break;
    case 3: case 5: case 7:
        d=1;
        c=149;
        break;
    case 67: case 69: case 71:
        d=1;
        c=156;
        break;
    case 136: case 147:
        d=2;
        c=159;
        break;
    case 91:
        if(b) {
            d=1;
            c=150;
            break;
        }
    default:
        if(b) d=2; else
    case 112:
        d=0;
        c=159;
    }
    if(b || a>=128) ed->layering=d; else ed->layering=0;
    return c;
}

// =============================================================================

void
loadovermap(uint16_t      * const b4,
            int             const m,
            int             const k,
            unsigned char * const rom)
{
    // Index of the current position in the low and upper byte arrays
    // of the decompressed map32 arrays.
    int i = 0;
    
    // Represents the target index in the output buffer of map32.
    int j = 0;
    
    size_t row = 0;
    
    unsigned char *b2, *b3;
    
    if(m > 0x9f)
    {
        // Can't load maps indexed 0xa0 and higher.
        return;
    }
    
    b2 = Uncompress(rom + romaddr(*(int*) (rom + 0x1794d + m * 3)), 0, 1);
    b3 = Uncompress(rom + romaddr(*(int*) (rom + 0x17b2d + m * 3)), 0, 1);
    
    for(row = 0; row < 16; row += 1)
    {
        /// column
        int co = 0;
        
        for(co = 0; co < 16; co++)
        {
            b4[j] = b3[i] + (b2[i] << 8);
            
            i += 1;
            j += 1;
        }
        
        if(k == 0)
        {
            // This to aid in loading 1024x1024 maps, since they consist of four
            // smaller areas pasted together.
            j += 16;
        }
    }
    
    free(b2);
    free(b3);
}

// =============================================================================

const static int wmap_ofs[4]={0,32,2048,2080};

void Wmapselectwrite(WMAPEDIT*ed) {
    int i,j,k,l,m,n,o,p,q;
    m=ed->ew.param?32:64;
    if(ed->stselx!=ed->rectleft || ed->stsely!=ed->recttop) {
        ed->undomodf=ed->modf;
        memcpy(ed->undobuf,ed->buf,0x1000);
        if(ed->rectleft<0) i=-ed->rectleft; else i=0;
        if(ed->recttop<0) j=-ed->recttop; else j=0;
        if(ed->rectright>m) k=m; else k=ed->rectright;
        k-=ed->rectleft;
        if(ed->rectbot>m) l=m; else l=ed->rectbot;
        l-=ed->recttop;
        q=ed->rectright-ed->rectleft;
        p=j*q;
        
        o = ( (ed->recttop + j) << 6) + ed->rectleft;
        
        for( ; j < l; j++)
        {
            for(n = i; n < k; n++)
            {
                ed->buf[n+o]=ed->selbuf[n+p];
            }
            o+=64;
            p+=q;
        }
        ed->modf=1;
    }
    ed->selflag=0;
    free(ed->selbuf);
}

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
        for(i=16;i<256;i+=16) ed->pal[i]=ed->pal[0];
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
        for(i=16;i<256;i+=16) ed->pal[i]=ed->pal[0];
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

BOOL CALLBACK wmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT*ed;
    BLOCKSEL8*bs;
    unsigned char*rom;
    HWND hc;
    HDC hdc;
    RECT rc;
    int i,j,k;
    int *b,*b2;
    static char*mapmark_str[10]={
        "Hyrule castle",
        "Village guy",
        "Sahasrahla",
        "Pendants",
        "Master sword",
        "Agahnim's tower",
        "First crystal",
        "All crystals",
        "Agahnim's other tower",
        "Flute locations"
    };
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(WMAPEDIT*)lparam;
        Getblocks(ed->ew.doc,223);
        ed->hpal=0;
        ed->anim=0;
        ed->gfxtmp=255;
        ed->mapscrollh=0;
        ed->mapscrollv=0;
        ed->bmih=zbmih;
        
        rom = ed->ew.doc->rom;
        
        b = (int*)(rom + 0x54727 + (ed->ew.param << 12));
        
        j=ed->ew.param?1:4;
        
        for(k=0;k<j;k++)
        {
            b2=(int*)(ed->buf+wmap_ofs[k]);
            for(i=0;i<32;i++) {
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                *(b2++)=*(b++);
                b2+=8;
            }
        }
        memcpy(ed->undobuf,ed->buf,0x1000);
        Loadpal(ed,rom,ed->ew.param?0xadc27:0xadb27,0,16,8);
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,(long)ed);
        Updatesize(hc);
        bs=&(ed->bs);
        hdc=GetDC(win);
        hc=GetDlgItem(win,3001);
        GetClientRect(hc,&rc);
        bs->ed=(OVEREDIT*)ed;
        bs->sel=0;
        bs->scroll=0;
        bs->flags=0;
        bs->dfl=0;
        bs->w=rc.right;
        bs->h=rc.bottom;
        bs->bufdc=CreateCompatibleDC(hdc);
        bs->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        ReleaseDC(win,hdc);
        Addgraphwin((DUNGEDIT*)ed,2);
        Setdispwin((DUNGEDIT*)ed);
        SelectObject(bs->bufdc,bs->bufbmp);
        SelectObject(bs->bufdc,white_pen);
        SelectObject(bs->bufdc,black_brush);
        Rectangle(bs->bufdc,0,0,bs->w,bs->h);
        
        if(always)
        {
            HPALETTE const oldpal = SelectPalette(objdc, ed->hpal, 1);
            HPALETTE const oldpal2 = SelectPalette(bs->bufdc, ed->hpal, 1);
            
            for(i = 0; i < 256; i++)
                Updateblk8sel(bs,i);
            
            SelectPalette(objdc, oldpal, 1);
            SelectPalette(bs->bufdc, oldpal2, 1);
        }
        
        SetWindowLong(hc, GWL_USERDATA, (int) bs);
        Updatesize(hc);
        
        ed->tool=1;
        ed->dtool=0;
        ed->selflag=0;
        ed->modf=0;
        ed->undomodf=0;
        ed->selbuf=0;
        ed->marknum=0;
        CheckDlgButton(win,3003,BST_CHECKED);
        hc=GetDlgItem(win,3005);
        for(i=0;i<10;i++) SendMessage(hc,CB_ADDSTRING,0,(long)mapmark_str[i]);
        SendMessage(hc,CB_SETCURSEL,0,0);
        break;
    case 4002:
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        break;
    case WM_DESTROY:
        ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
        ed->ew.doc->wmaps[ed->ew.param]=0;
        Delgraphwin((DUNGEDIT*)ed);
        Releaseblks(ed->ew.doc,223);
        DeleteDC(ed->bs.bufdc);
        DeleteObject(ed->bs.bufbmp);
        free(ed);
        break;
    
    // \task What is this constant?
    case 4000:
        ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
        j=wparam;
        if(j<0) j=0;
        if(j>0xff) j=0xff;
        hc=GetDlgItem(win,3001);
        Changeblk8sel(hc,&(ed->bs));
        ed->bs.sel=j;
        Changeblk8sel(hc,&(ed->bs));
        break;
    case WM_COMMAND:
        ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
        j=ed->tool;
        switch(wparam) {
        case 3002:
            ed->tool=0;
            break;
        case 3003:
            ed->tool=1;
            break;
        case 3004:
            ed->tool=2;
            break;
        case 3005|(CBN_SELCHANGE<<16):
            
            ed->marknum = (short) SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
            
            if(j==4)
                break;
updscrn:
            j=0;
            InvalidateRect(GetDlgItem(win,3000),0,0);
            break;
        case 3006:
            ed->tool=3;
            ed->selmark=0;
            break;
        case 3008:
            ed->tool=4;
            goto updscrn;
        case 3007:
            if(ed->selflag) Wmapselectwrite(ed);
            b2=malloc(0x1000);
            memcpy(b2,ed->buf,0x1000);
            memcpy(ed->buf,ed->undobuf,0x1000);
            memcpy(ed->undobuf,b2,0x1000);
            free(b2);
            i=ed->undomodf;
            ed->undomodf=ed->modf;
            ed->modf=i;
            goto updscrn;
        }
        if(j==4 && ed->tool!=4) goto updscrn;
    }
    return FALSE;
}
const static int blkx[16]={0,8,0,8,16,24,16,24,0,8,0,8,16,24,16,24};
const static int blky[16]={0,0,8,8,0,0,8,8,16,16,24,24,16,16,24,24};
int Keyscroll(HWND win,int wparam,int sc,int page,int scdir,int size,int size2)
{
    int i;
    switch(wparam) {
    case VK_UP:
        i=SB_LINEUP;
        break;
    case VK_DOWN:
        i=SB_LINEDOWN;
        break;
    case VK_PRIOR:
        i=SB_PAGEUP;
        break;
    case VK_NEXT:
        i=SB_PAGEDOWN;
        break;
    case VK_HOME:
        i=SB_TOP;
        break;
    case VK_END:
        i=SB_BOTTOM;
        break;
    default:
        return sc;
    }
    return Handlescroll(win,i,sc,page,scdir,size,size2);
}

// =============================================================================

void
Blk16Search_OnPaint(OVEREDIT const * const p_ed,
                    HWND             const p_win)
{
    unsigned char const * const rom = p_ed->ew.doc->rom;
    
    int i = 0,
        j = 0,
        m = 0;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal,1);
    
    HGDIOBJ const oldbrush = SelectObject(hdc, trk_font);
    HGDIOBJ const oldobj2 = SelectObject(hdc, white_pen);
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    i = (ps.rcPaint.top >> 4);
    j = ( (ps.rcPaint.bottom + 15) >> 4);
    
    FillRect(hdc,&(ps.rcPaint), black_brush);
    
    m = i + p_ed->schscroll;
    
    SetTextColor(hdc, 0xffffff);
    
    SetBkColor(hdc, 0);
    
    for( ; i < j; i += 2)
    {
        int k = 0;
        
        for(k = 0; k < 1024; k++)
            drawbuf[k] = 0;
        
        for(k = 0; k < 2; k++)
        {
            int l = 0;
            
            unsigned short const * o = 0;
            
            RECT rc;
            
            if(m >= 0xea8)
                break;
            
            o = (unsigned short*) (rom + 0x78000 + (m << 3));
            
            for(l = 0; l < 4; l++)
                Drawblock(p_ed,
                          blkx[l] + 8,
                          blky[l] + (k << 4),
                          *(o++),
                          0);
            rc.left=4;
            rc.right=20;
            rc.top = ( (i + k) << 4) + 2;
            rc.bottom=rc.top+12;
            
            DrawFrameControl(hdc,&rc,DFC_BUTTON,
                             ((p_ed->selsch[m>>3]&(1<<(m&7))) ? (DFCS_BUTTONCHECK | DFCS_CHECKED):DFCS_BUTTONCHECK)
                             +((p_ed->schpush == m) ? DFCS_PUSHED : 0));
            
            rc.left=24;
            rc.right=40;
            
            DrawFrameControl(hdc,
                             &rc,
                             DFC_BUTTON,
                             ((p_ed->selsch[0x1d5 + (m >> 3)] & (1 << (m & 7)))
                           ? (DFCS_BUTTONCHECK|DFCS_CHECKED)
                           : DFCS_BUTTONCHECK)
                           +((p_ed->schpush==m + 0xea8)?DFCS_PUSHED:0));
            
            rc.left=44;
            rc.right=60;
            
            DrawFrameControl(hdc,
                             &rc,
                             DFC_BUTTON,
                             ((p_ed->selsch[0x3aa + (m >> 3)] & (1 << (m & 7)))
                           ? (DFCS_BUTTONCHECK|DFCS_CHECKED)
                           : DFCS_BUTTONCHECK)
                           + ((p_ed->schpush== m + 0x1d50) ? DFCS_PUSHED : 0));
            
            wsprintf(buffer,"%04d",m);
            
            TextOut(hdc,64,rc.top,buffer,4);
            
            m++;
        }
        
        k = i << 4;
        
        Paintblocks(&(ps.rcPaint),hdc,100,k, (DUNGEDIT*) p_ed);
        
        MoveToEx(hdc,108,k,0);
        LineTo(hdc,124,k);
        
        MoveToEx(hdc,108,k+16,0);
        LineTo(hdc,124,k+16);
    }
    
    SelectObject(hdc, oldbrush);
    SelectObject(hdc, oldobj2);
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

LRESULT CALLBACK
blk16search(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT *ed;
    RECT rc;
    SCROLLINFO si;
    int i,j,k;
    
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS|DLGC_WANTARROWS;
    case WM_KEYDOWN:
        
        ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(wparam >= 48 && wparam < 58)
        {
            wparam -= 48;
            
            ed->schtyped /* <<=4 */ *=10;
            ed->schtyped+=wparam;
            ed->schtyped%=5000;
            ed->schscroll=Handlescroll(win,SB_THUMBPOSITION|(ed->schtyped<<16),ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        } else ed->schscroll=Keyscroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    case WM_SIZE:
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=0xea8;
        si.nPage=lparam>>20;
        ed->schpage=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        ed->schscroll=Handlescroll(win,-1,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    case WM_VSCROLL:
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->schscroll=Handlescroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
        break;
    
    case WM_PAINT:
        
        ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            Blk16Search_OnPaint(ed, win);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        SetFocus(win);
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        i=(short)lparam;
        j=lparam>>16;
        k=(j>>4)+ed->schscroll;
        if(k<0 || k>=0xea8) break;
        if(ed->schpush!=-1)
            InvalidateRect(win,&(ed->schrc),0);
        if((j&15)>=2 && (j&15)<14) {
            if(i>4 && i<20) j=0,rc.left=4;
            else if(i>24 && i<40) j=0xea8,rc.left=24;
            else if(i>44 && i<60) j=0x1d50,rc.left=44;
            else break;
            ed->schpush=k+j;
            rc.right=rc.left+16;
            rc.top = ( (k - ed->schscroll) << 4) + 2;
            rc.bottom=rc.top+12;
            ed->schrc=rc;
            InvalidateRect(win,&rc,0);
            SetCapture(win);
        }
        break;
    case WM_LBUTTONUP:
        ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->schpush!=-1) {
            i=(short)lparam;
            j=lparam>>16;
            rc=ed->schrc;
            if(i>=rc.left && i<rc.right && j>=rc.top && j<rc.bottom)
            ed->selsch[ed->schpush>>3]^=(1<<(ed->schpush&7));
            ed->schpush=-1;
            InvalidateRect(win,&(ed->schrc),0);
            ReleaseCapture();
        }
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}
BOOL CALLBACK findblks(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

void Getblock32(unsigned char*rom,int m,short*l)
{
    int n;
    if(m==-1) { l[0]=l[1]=l[2]=l[3]=0xdc4; return; }
    n=(m>>2)*6;
    switch(m&3) {
    case 0:
        l[0] = rom[0x18000 + n] | (rom[0x18004 + n] >> 4) << 8;
        l[1] = rom[0x1b400 + n] | (rom[0x1b404 + n] >> 4) << 8;
        l[2] = rom[0x20000 + n] | (rom[0x20004 + n] >> 4) << 8;
        l[3] = rom[0x23400 + n] | (rom[0x23404 + n] >> 4) << 8;
        break;
    case 1:
        l[0] = rom[0x18001 + n] | (rom[0x18004 + n] & 15) << 8;
        l[1] = rom[0x1b401 + n] | (rom[0x1b404 + n] & 15) << 8;
        l[2] = rom[0x20001 + n] | (rom[0x20004 + n] & 15) << 8;
        l[3] = rom[0x23401 + n] | (rom[0x23404 + n] & 15) << 8;
        break;
    case 2:
        l[0] = rom[0x18002 + n] | (rom[0x18005 + n] >> 4) << 8;
        l[1] = rom[0x1b402 + n] | (rom[0x1b405 + n] >> 4) << 8;
        l[2] = rom[0x20002 + n] | (rom[0x20005 + n] >> 4) << 8;
        l[3] = rom[0x23402 + n] | (rom[0x23405 + n] >> 4) << 8;
        break;
    case 3:
        l[0] = rom[0x18003 + n] | (rom[0x18005 + n] & 15) << 8;
        l[1] = rom[0x1b403 + n] | (rom[0x1b405 + n] & 15) << 8;
        l[2] = rom[0x20003 + n] | (rom[0x20005 + n] & 15) << 8;
        l[3] = rom[0x23403 + n] | (rom[0x23405 + n] & 15) << 8;
        break;
    }
}
const char*sprset_str[]={
    "Beginning","First part","Second part"
};

//LoadOverlays*******************************************

void LoadOverlays(FDOC *doc)
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

//LoadOverlays************************************

//SaveOverlays#************************************

void SaveOverlays(FDOC *doc)
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

// SaveOverlays**********************************

// Unloadovl#***************************************

void Unloadovl(FDOC *doc)
{
    // if the overlays are loaded, free the overlay buffer.
    if(doc->o_loaded == 1)
        free(doc->ovlbuf);
    
    // the overlays are regarded as not being loaded. 
    doc->o_loaded = 0;
}

//Unloadovl******************************************

//loadoverovl#***********************************

int loadoverovl(FDOC *doc, uint16_t *buf, int mapnum)
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
void Changeblk16sel(HWND win,BLOCKSEL16*ed)
{
    RECT rc,rc2;
    int i=ed->sel-(ed->scroll<<4);
    GetClientRect(win,&rc);
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
        GetClientRect(hc, &rc);
        
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
        
        SetWindowLong(hc, GWL_USERDATA, (int) ed);
        
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
    int x[2];
    int i;
    
    // \task Pointer problem on 64-bit (and just generally)
    x[0]=(int)ed;
    
    if(num==17)
    {
        num=askinteger(219,"Edit blocks","Which blocks?");
        
        if(num==-1)
            return 0;
        
        Getblocks(ed->ew.doc,num);
        
        num += 32;
    }
    
    x[1]=num;
    
    i=ShowDialog(hinstance,(LPSTR)IDD_DIALOG16,win,blockdlgproc,(long)x);
    
    if(num >= 32 && num < 256)
        Releaseblks(ed->ew.doc, num - 32);
    
    return i;
}

BOOL CALLBACK z3dlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    FDOC*doc;
    HWND hc;
    int i,j,k,l;
    TVINSERTSTRUCT tvi;
    TVHITTESTINFO hti;
    TVITEM*itemstr;
    HTREEITEM hitem;
    RECT rc;
    ZOVER*ov;
    OVEREDIT*oed;
    unsigned char*rom;
    int lp;
    static char *pal_text[]={
        "Sword",
        "Shield",
        "Clothes",
        "World colors 1",
        "World colors 2",
        "Area colors 1",
        "Area colors 2",
        "Enemies 1",
        "Dungeons",
        "Miscellanous colors",
        "World map",
        "Enemies 2",
        "Other sprites",
        "Dungeon map",
        "Triforce",
        "Crystal"
    };
    static char*locs_text[]={
        "Pendant 1",
        "Pendant 2",
        "Pendant 3",
        "Agahnim 1",
        "Crystal 2",
        "Crystal 1",
        "Crystal 3",
        "Crystal 6",
        "Crystal 5",
        "Crystal 7",
        "Crystal 4",
        "Agahnim 2"
    };
    static char pal_num[]={
        4,3,3,6,2,20,16,24,20,2,2,16,18,1,1,1
    };
    char buf[32];
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        doc=(FDOC*)lparam;
        hc=GetDlgItem(win,3000);
        tvi.hParent=0;
        tvi.hInsertAfter=TVI_LAST;
        tvi.item.mask=TVIF_CHILDREN|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
        tvi.item.stateMask=TVIS_BOLD;
        tvi.item.state=0;
        tvi.item.lParam=0;
        tvi.item.pszText="Overworld";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<160;i++)
        {
            tvi.item.lParam=i + 0x20000;
            wsprintf(buf,"Area %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Dungeons";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<133;i++)
        {
            tvi.item.lParam=i + 0x30000;
            wsprintf(buf,"Entrance %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<7;i++)
        {
            tvi.item.lParam=i + 0x30085;
            wsprintf(buf,"Starting location %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<19;i++)
        {
            tvi.item.lParam=i + 0x3008c;
            wsprintf(buf,"Overlay %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<8;i++)
        {
            tvi.item.lParam=i + 0x3009f;
            wsprintf(buf,"Layout %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.lParam=0x300a7;
        tvi.item.pszText="Watergate overlay";
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Music";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<3;i++)
        {
            tvi.item.lParam=i + 0x40000;
            wsprintf(buf,"Bank %d",i+1);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.lParam=0x40003;
        tvi.item.pszText="Waves";
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="World maps";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Normal world";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x60000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Dark world";
        tvi.item.lParam=0x60001;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Monologue";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x50000;
        tvi.hParent=0;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Palettes";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        hitem=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        k=0;
        for(i=0;i<16;i++) {
            l=pal_num[i];
            tvi.item.pszText=pal_text[i];
            tvi.item.cChildren=1;
            tvi.item.lParam=0;
            tvi.hParent=hitem;
            tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
            tvi.item.pszText=buf;
            tvi.item.cChildren=0;
            for(j=0;j<l;j++) {
                tvi.item.lParam=k + 0x70000;
                wsprintf(buf,"%s pal %d",pal_text[i],j);
                SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
                k++;
            }
        }
        tvi.item.pszText="Dungeon maps";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<14;i++)
        {
            tvi.item.lParam=i + 0x80000;
            tvi.item.pszText=level_str[i+1];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Dungeon properties";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<12;i++)
        {
            tvi.item.lParam=i + 0x90000;
            tvi.item.pszText=locs_text[i];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Menu screens";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<11;i++)
        {
            tvi.item.lParam=i + 0xa0000;
            tvi.item.pszText=screen_text[i];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="3D Objects";
        tvi.item.cChildren=0;
        tvi.item.lParam=0xb0000;
        tvi.hParent=0;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Link's graphics";
        tvi.item.lParam=0xc0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="ASM hacks";
        tvi.item.lParam=0xd0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Graphic schemes";
        tvi.item.lParam=0xe0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        break;
    case WM_NOTIFY:
        switch(wparam) {
        case 3000:
            switch(((NMHDR*)lparam)->code) {
            case NM_DBLCLK:
                GetWindowRect(((NMHDR*)lparam)->hwndFrom,&rc);
                
                hti.pt.x = mouse_x-rc.left;
                hti.pt.y = mouse_y-rc.top;
                hitem = (HTREEITEM) SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_HITTEST,0,(long)&hti);
                
                if(!hitem)
                    break;
                
                if(!(hti.flags & TVHT_ONITEM))
                    break;
                
                itemstr = &(tvi.item);
                itemstr->hItem = hitem;
                itemstr->mask = TVIF_PARAM;
                
                SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_GETITEM,0,(long)itemstr);
                
                lp = itemstr->lParam;
open_edt:
                
                doc = (FDOC*)GetWindowLong(win,DWL_USER);
                j = lp & 0xffff;
                
                switch(lp>>16)
                {

                    // double clicked on an overworld area
                case 2:
                    ov = doc->overworld;
                    
                    if(j < 128)
                        j = doc->rom[0x125ec + (j & 0x3f)] | (j & 0x40);
                    
                    for(i=0;i<4;i++)
                    {
                        k = map_ind[i];
                        
                        if(j >= k && ov[j - k].win)
                        {
                            hc = ov[j - k].win;
                            oed = (OVEREDIT*)GetWindowLong(hc,GWL_USERDATA);
                            
                            if(i && !(oed->mapsize))
                                continue;
                            
                            SendMessage(clientwnd,WM_MDIACTIVATE,(int)hc,0);
                            
                            hc = GetDlgItem(oed->dlg,3001);
                            SendMessage(hc,WM_HSCROLL,SB_THUMBPOSITION|((i&1)<<20),0);
                            SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((i&2)<<19),0);
                            
                            return FALSE;
                        }
                    }
                    
                    wsprintf(buf,"Area %02X",j);
                    ov[j].win = Editwin(doc,"ZEOVER",buf,j,sizeof(OVEREDIT));
                    
                    break;
                
                case 3:
                    
                    // double clicked on a dungeon item
                    if(doc->ents[j])
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (int) (doc->ents[j]),
                                    0);
                        
                        break;
                    }
                    
                    if(j < 0x8c)
                    {
                        k = ((short*) (doc->rom + (j >= 0x85 ? 0x15a64 : 0x14813)))[j];
                        
                        if(doc->dungs[k])
                        {
                            MessageBox(framewnd,
                                       "The room is already open in another editor",
                                       "Bad error happened",
                                       MB_OK);
                            
                            break;
                        }
                        
                        if(j >= 0x85)
                            wsprintf(buf,"Start location %02X",j - 0x85);
                        else
                            wsprintf(buf,"Entrance %02X",j);
                    }
                    else if(j < 0x9f)
                        wsprintf(buf,"Overlay %d",j - 0x8c);
                    else if(j < 0xa7)
                        wsprintf(buf,"Layout %d",j - 0x9f);
                    else
                        wsprintf(buf,"Watergate overlay");
                    
                    hc = Editwin(doc,"ZEDUNGEON",buf,j, sizeof(DUNGEDIT));
                     
                    if(hc)
                    {
                        DUNGEDIT * ed = (DUNGEDIT*) GetWindowLong(hc, GWL_USERDATA);
                        HWND map_win = GetDlgItem(ed->dlg, ID_DungEditWindow);
                    
                        Dungselectchg(ed, map_win, 1);
                    }
                    
                    break;
                case 4:
                    if(doc->mbanks[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->mbanks[j]),0);break;}
                    if(j==3) doc->mbanks[3]=Editwin(doc,"MUSBANK","Wave editor",3,sizeof(SAMPEDIT)); else {
                        wsprintf(buf,"Song bank %d",j+1);
                        doc->mbanks[j]=Editwin(doc,"MUSBANK",buf,j,sizeof(MUSEDIT));
                    }
                    break;
                case 6:
                    if(doc->wmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->wmaps[j]),0);break;}
                    wsprintf(buf,"World map %d",j+1);
                    doc->wmaps[j]=Editwin(doc,"WORLDMAP",buf,j,sizeof(WMAPEDIT));
                    break;
                case 5:
                    
                    if(doc->t_wnd)
                    {
                        SendMessage(clientwnd, WM_MDIACTIVATE, (int)(doc->t_wnd), 0);
                        
                        break;
                    }
                    
                    doc->t_wnd = Editwin(doc,"ZTXTEDIT","Text editor",0,sizeof(TEXTEDIT));
                    
                    break;
                
                case 7:
                    if(doc->pals[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->pals[j]),0);break;}
                    
                    k = 0;
                    
                    for(i=0;i<16;i++)
                    {
                        if(k + pal_num[i] > j)
                            break;
                        
                        k += pal_num[i];
                    }
                    
                    wsprintf(buf,"%s palette %d",pal_text[i],j-k);
                    
                    doc->pals[j] = Editwin(doc,
                                           "PALEDIT",
                                           buf,
                                           j | (i << 10) | ( (j - k) << 16),
                                           sizeof(PALEDIT));
                    
                    break;
                
                case 8:
                    if(doc->dmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->dmaps[j]),0);break;}
                    doc->dmaps[j]=Editwin(doc,"LEVELMAP",level_str[j+1],j,sizeof(LMAPEDIT));
                    break;
                case 9:
                    activedoc=doc;
                    ShowDialog(hinstance,MAKEINTRESOURCE(IDD_DIALOG17),framewnd,editbosslocs,j);
                    break;
                case 10:
                    if(doc->tmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->tmaps[j]),0);break;}
                    doc->tmaps[j]=Editwin(doc,"TILEMAP",screen_text[j],j,sizeof(TMAPEDIT));
                    break;
                case 11:
                    if(doc->perspwnd) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->perspwnd),0);break;}
                    doc->perspwnd=Editwin(doc,"PERSPEDIT","3D object editor",0,sizeof(PERSPEDIT));
                    break;
                case 12:
                    
                    oed = (OVEREDIT*) malloc(sizeof(OVEREDIT));
                    
                    oed->bmih=zbmih;
                    oed->hpal=0;
                    oed->ew.doc=doc;
                    oed->gfxnum=0;
                    oed->paltype=3;
                    if(palmode) Setpalmode((DUNGEDIT*)oed);
                    rom=doc->rom;
                    Getblocks(doc,225);
                    Loadpal(oed,rom,0x1bd308,0xf1,15,1);
                    Loadpal(oed,rom,0x1bd648,0xdc,4,1);
                    Loadpal(oed,rom,0x1bd630,0xd9,3,1);
                    Editblocks(oed,0xf0104,framewnd);
                    Releaseblks(doc,225);
                    
                    if(oed->hpal)
                        DeleteObject(oed->hpal);
                    
                    free(oed);
                    
                    break;
                
                case 13:
                    
                    if(doc->hackwnd)
                    {
                        SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->hackwnd),0);
                        
                        break;
                    }
                    
                    doc->hackwnd = Editwin(doc, "PATCHLOAD", "Patch modules", 0, sizeof(PATCHLOAD));
                    
                    break;
                
                case 14:
                    
                    // Graphic Themes
                    ShowDialog(hinstance,
                               MAKEINTRESOURCE(IDD_GRAPHIC_THEMES),
                               framewnd,
                               editvarious,
                               (int) doc);
                    
                    break;
                }
            }
        }
        break;
    case 4000:
        lp=wparam;
        goto open_edt;
    }
    return FALSE;
}

int Savesecrets(FDOC*doc,int num,unsigned char*buf,int size)
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

void Savemap(OVEREDIT*ed)
{
    unsigned char*rom,*b2,*b3,*b5;
    int i,j,k,l,m,n,p;
    short o[4];
    ZOVER*ov;
    FDOC*doc;
    
    unsigned short * b6;
    
    uint16_t * b4 = 0;
    
    static int sprofs[]={0,0x40,0xd0};
    ov=ed->ov;
    if(!ov->modf) return;
    m=ed->ew.param;
    doc=ed->ew.doc;
    rom=doc->rom;
    if(m<0x90) {
        if(ed->ecopy[ed->sprset]!=-1) {
            n=ed->ecopy[ed->sprset];
            ed->esize[n]=ed->esize[ed->sprset];
            ed->e_modf[n]=ed->e_modf[ed->sprset];
            ed->ebuf[n]=ed->ebuf[ed->sprset];
        }
        for(k=0;k<3;k++) o[k]=*(short*)(rom+sprset_loc[k]+m*2);
        for(k=1;k<3;k++)
            if(ed->e_modf[k]) {
                for(l=m>>7;l<k;l++) if(o[l]==o[k]) {
                    if(ed->ecopy[k]==-1) *(unsigned short*)(rom+sprset_loc[k]+m*2)=0xcb41;
                    else *(unsigned short*)(rom+sprset_loc[k]+m*2)=o[ed->ecopy[k]],ed->e_modf[k]=0;
                    break;
                }
            }
        for(k=1;k<3;k++) {
            if(ed->ecopy[k]!=-1) {
                Savesprites(doc,sprofs[k]+m + 0x10000,0,0);
                *(unsigned short*)(rom+sprset_loc[k]+m*2)=*(unsigned short*)(rom+sprset_loc[ed->ecopy[k]]+m*2);
            }
        }
        for(k=0;k<3;k++)
            if(ed->e_modf[k] && ed->ecopy[k]==-1) {
                Savesprites(doc,sprofs[k]+m,ed->ebuf[k],ed->esize[k]);
            }
        if(m<0x80) Savesecrets(doc,m,ed->sbuf,ed->ssize);
    }
    if(m<0x40) {
        rom[0x7a41 + ed->ew.param]=ed->sprgfx[0];
        rom[0x7a81 + ed->ew.param]=ed->sprgfx[1];
        rom[0x7b41 + ed->ew.param]=ed->sprpal[0];
        rom[0x7b81 + ed->ew.param]=ed->sprpal[1];
    }
    rom[0x7ac1 + ed->ew.param]=ed->sprgfx[2];
    rom[0x7bc1 + ed->ew.param]=ed->sprpal[2];
    b2=malloc(0x200);
    b3=b2 + 0x100;
    if(ed->mapsize) n=4; else n=1;
    
    for(k=0;k<n;k++)
    {
        if(m + map_ind[k] > 0x9f)
            continue;
        
        b4 = ov->buf + map_ofs[k];
        
        for(j=0;j<16;j++) {
            for(i=0;i<16;i++) {
                l=*(b4++);
                *(b3++)=l;
                *(b2++)=l>>8;
            }
            b4+=16;
        }
        b2-=0x100;
        b3-=0x100;
        b5=Compress(b2,0x100,&i,1);
        issplit=0;
        memcpy(rom+Changesize(doc,m+map_ind[k] + 0x30000,i),b5,i);
        free(b5);
        b5=Compress(b3,0x100,&i,1);
        memcpy(rom+Changesize(doc,m+map_ind[k]+(issplit?0x100a0:160),i),b5,i);
        free(b5);
    }
    free(b2);
    if(ed->ovlmodf) {
        l=0;
        n=256;
        b6=malloc(512);
        b4=doc->ovlbuf;
        j=0;
        for(;;) {
            k=0xffff;
            for(i=0;i<0x1400;i++) {
                p=ed->ovlmap[i];
                if(p>=j && p<k) k=p;
            }
            j=k+1;
            if(k==0xffff) break;
            if(l) b6[l++]=0xffff;
            b6[l++]=k;
            for(i=0;i<0x1400;i++)
                if(ed->ovlmap[i]==k) b6[l++]=i;
            if(l>=192) n+=256,b6=realloc(b6,n<<1);
        }
        b6[l++]=0xfffe;
        if(!(doc->o_enb[m>>3]&(1<<(m&7)))) {
            k=b4[m];
            doc->o_enb[m>>3]|=(1<<(m&7));
            goto foundnextovl;
        }
        for(i=0;i<128;i++) {
            if(i!=m && b4[i]==b4[m]) {
                wsprintf(buffer,"The overlay in area %02X is used several times. Modify only in this area?",m);
                if(MessageBox(0,buffer,"Gigasoft Hyrule Magic",MB_YESNO)==IDYES) {
                    k=b4[m]; goto foundnextovl; } else goto modallovl;
            }
        }
modallovl:
        k=b4[128];
        for(i=0;i<128;i++) {
            if(i==m) continue;
            j=b4[i];
            if(j<k && j>b4[m]) k=j;
        }
foundnextovl:
        j=b4[m];
        
        if(l+j==k) goto nochgovls;
        
        for(i=0;i<129;i++)
            if(b4[i]>=k) b4[i]+=l+j-k;
        
        b4[m]=j;
        
        if(l+j<k)
            memmove(b4 + l + j,
                    b4 + k,
                    (b4[128] - l - j) << 1);
        
        doc->ovlbuf = (uint16_t*) b4 = (uint16_t*) realloc(b4, b4[128] << 1);
        
        if(l + j > k)
            memmove(b4 + l + j,
                    b4 + k,
                    (b4[128] - l - j) << 1);
        
    nochgovls:
        
        memcpy(b4+j,b6,l<<1);
        free(b6);
        doc->o_modf=1;
    }
    ed->ovlmodf=0;
    ed->ov->modf=0;
    ed->e_modf[0]=ed->e_modf[1]=ed->e_modf[2];
    doc->modf=1;
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

void
PaintSprName(HDC hdc,
             int x,
             int y,
             int n,
             int o,
             int          const p_clip_width,
             char const * const p_name)
{
    size_t len = strlen(p_name);
    
    signed final_len = (signed) len;
    
    // -----------------------------
    
    // \task It would appear that the clip width is being used as height too?
    // Does this always assume that window area are square?
    if( (y + textmetric.tmHeight) > (p_clip_width - o) )
        return;
    
    if( (len * textmetric.tmAveCharWidth) + x > (p_clip_width - n) )
        final_len = (p_clip_width - n - x) / textmetric.tmAveCharWidth;
    
    if(len <= 0)
        return;
    
    SetTextColor(hdc, 0);
    
    TextOut(hdc,x + 1, y + 1, p_name, final_len);
    TextOut(hdc,x - 1, y - 1, p_name, final_len);
    TextOut(hdc,x + 1, y - 1, p_name, final_len);
    TextOut(hdc,x - 1, y + 1, p_name, final_len);
    
#if 0
    SetTextColor(hdc, 0xffbf3f);
#else
    SetTextColor(hdc, 0xfefefe);
#endif
    
    TextOut(hdc,x, y, p_name, final_len);
}

// =============================================================================

void Paintspr(HDC hdc,
              int x,
              int y,
              int n,
              int o,
              int w)
{
    size_t len = strlen(buffer);
    
    signed final_len = (signed) len;
    
    // -----------------------------
    
    if( (y + textmetric.tmHeight) > (w - o) )
        return;
    
    if( (len * textmetric.tmAveCharWidth) + x > (w - n) )
        final_len = (w - n - x) / textmetric.tmAveCharWidth;
    
    if(len <= 0)
        return;
    
    SetTextColor(hdc, 0);
    
    TextOut(hdc,x + 1, y + 1, buffer, final_len);
    TextOut(hdc,x - 1, y - 1, buffer, final_len);
    TextOut(hdc,x + 1, y - 1, buffer, final_len);
    TextOut(hdc,x - 1, y + 1, buffer, final_len);
    
#if 0
    SetTextColor(hdc, 0xffbf3f);
#else
    SetTextColor(hdc, 0xfefefe);
#endif
    
    TextOut(hdc,x, y, buffer, final_len);
}

// =============================================================================

void Getsampsel(SAMPEDIT*ed,RECT*rc)
{
    rc->left=(ed->sell<<16)/ed->zoom-ed->scroll;
    rc->right=(ed->selr<<16)/ed->zoom+1-ed->scroll;
}

long CALLBACK perspdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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
            if((j<3) ^fronttest(&pt,&pt2,&pt3)) {
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

void Getselectionrect(OVEREDIT*ed,RECT*rc)
{
    int i,j;
    i=(ed->rectleft-ed->mapscrollh)<<5;
    j=((ed->rectright-ed->mapscrollh)<<5);
    if(i<j) rc->left=i,rc->right=j; else rc->left=j,rc->right=i;
    i=(ed->recttop-ed->mapscrollv)<<5;
    j=((ed->rectbot-ed->mapscrollv)<<5);
    if(i<j) rc->top=i,rc->bottom=j; else rc->top=j,rc->bottom=i;
}
void Overselectwrite(OVEREDIT*ed) {
    int i,j,k,l,m,n,o,p,q;
    m=ed->mapsize?32:16;
    if(ed->stselx!=ed->rectleft || ed->stsely!=ed->recttop) {
        ed->undomodf=ed->ov->modf;
        memcpy(ed->undobuf,ed->ov->buf,0x800);
        if(ed->rectleft<0) i=-ed->rectleft; else i=0;
        if(ed->recttop<0) j=-ed->recttop; else j=0;
        if(ed->rectright>m) k=m; else k=ed->rectright;
        k-=ed->rectleft;
        if(ed->rectbot>m) l=m; else l=ed->rectbot;
        l-=ed->recttop;
        q=ed->rectright-ed->rectleft;
        
        p=j*q;
        
        o = ( (ed->recttop + j) << 5) + ed->rectleft;
        
        for(;j<l;j++) {
            for(n=i;n<k;n++) {
                ed->ov->buf[n+o]=ed->selbuf[n+p];
            }
            o+=32;
            p+=q;
        }
        ed->ov->modf=1;
    }
    ed->selflag=0;
    free(ed->selbuf);
}

void
Wmapselectionrect(WMAPEDIT const * const ed,
                  RECT           * const rc)
{
    int i = ( ed->rectleft - (ed->mapscrollh << 2) ) << 3;
    
    int j = ( (ed->rectright - (ed->mapscrollh << 2) ) << 3);
    
    if(i < j)
    {
        rc->left  = i;
        rc->right = j;
    }
    else
    {
        rc->left  = j;
        rc->right = i;
    }
    
    i = (ed->recttop-(ed->mapscrollv<<2))<<3;
    j = ((ed->rectbot-(ed->mapscrollv<<2))<<3);
    
    if(i < j)
    {
        rc->top    = i;
        rc->bottom = j;
    }
    else
    {
        rc->top    = j;
        rc->bottom = i;
    }
}

COLORREF custcols[16];
const static int pal_addr[]={
    0x1bd630,0x1bd648,0x1bd308,0x1be6c8,0x1bd218,0x1be86c,0x1be604,0x1bd4e0,
    0x1bd734,0x1bd660,0xadb27,0x1bd39e,0x1bd446,0x1be544,0xcc425,0x1eccd3
};

void Savepal(PALEDIT*ed)
{
    int k;
    short *b;
    if(!ed->modf) return;
    k=ed->palw*ed->palh;
    b=(short*)(ed->ew.doc->rom+romaddr(pal_addr[(ed->ew.param>>10)&63]+(k*(ed->ew.param>>16)<<1)));
    memcpy(b,ed->pal,k<<1);
    ed->modf=0;
    ed->ew.doc->modf=1;
}

long CALLBACK palproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i = 0,j,k,m,n;
    short*b,l;
    HDC hdc;
    HGDIOBJ oldobj;
    RECT rc;
    PAINTSTRUCT ps;
    CHOOSECOLOR cc;
    const static char pal_w[]={
        3,4,15,7,15,7,7,7,15,16,16,7,7,16,8,8
    };
    const static char pal_h[]={
        1,1,1,5,4,3,1,1,6,2,8,1,1,6,1,1
    };
    PALEDIT *ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(PALEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->ew.doc->modf==2) goto deflt;
        if(ed->modf) {
            wsprintf(buffer,"Confirm palette modification?",ed->ew.param);
            switch(MessageBox(framewnd,buffer,"Palette editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savepal(ed);
                goto deflt;
            case IDCANCEL:
                break;
            case IDNO:
                goto deflt;
            }
        }
        goto deflt;
    case WM_MDIACTIVATE:
        activedoc=((WMAPEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        break;
    case WM_GETMINMAXINFO:
        ed=(PALEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        break;
    case WM_PAINT:
        ed=(PALEDIT*)GetWindowLong(win,GWL_USERDATA);
        hdc=BeginPaint(win,&ps);
        GetClientRect(win,&rc);
        k=0;
        
        m = ( rc.right - (ed->palw << 4) ) >> 1;
        n = ( rc.bottom - (ed->palh << 4) ) >> 1;
        
        oldobj=SelectObject(hdc,white_pen);
        
        for(j=0;j<ed->palh;j++)
        {
            for(i = 0; i < ed->palw; i++, k++)
            {
                rc.left=m+(i<<4);
                rc.right=rc.left+16;
                rc.top=n+(j<<4);
                rc.bottom=rc.top+16;
                
                FillRect(hdc,&rc,ed->brush[k]);
                
                if(ed->pal[k] & 0x8000)
                {
                    // I see that this paints a diagonal line on the
                    // palette entry, but does this do anything practical?
                    // In some programs this indicates that a default color
                    // is used instead, but hard to say here.
                    
                    MoveToEx(hdc, rc.left + 3, rc.top + 13, 0);
                    LineTo(hdc, rc.left + 13, rc.top + 3);
                }
            }
        }
        SelectObject(hdc,oldobj);
        
        rc.left = m;
        rc.top = n;
        rc.right = rc.left + (i << 4);
        rc.bottom=rc.top+(j<<4);
        
        FrameRect(hdc,&rc,black_brush);
        
        EndPaint(win,&ps);
        
        break;
    
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        
        ed = (PALEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        GetClientRect(win, &rc);
        
        m = ( rc.right - (ed->palw << 4) ) >> 1;
        n = ( rc.bottom - (ed->palh << 4) ) >> 1;
        i = ( (lparam & 65535) - m ) >> 4;
        j = ( (lparam >> 16) - n ) >> 4;
        
        if(i<0 || i>=ed->palw || j<0 || j>=ed->palh) break;
        
        k = i + j * ed->palw;
        
        if(msg == WM_RBUTTONDOWN)
        {
            ed->pal[k] ^= 0x8000;
            
            goto upd;
        }
        
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = framewnd;
        
        // Not using a custom template.
        cc.hInstance = NULL;
        
        l = ed->pal[k];
        
        cc.rgbResult = ((l & 0x1f) << 3) + ((l & 0x3e0) << 6) + ((l & 0x7c00) << 9);
        cc.lpCustColors=custcols;
        cc.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;
        
        if(ChooseColor(&cc))
        {
            cc.rgbResult&=0xf8f8f8;
            
            ed->pal[k] = (uint16_t)
            (
                ( (cc.rgbResult &     0xf8) >> 3)
              + ( (cc.rgbResult &   0xf800) >> 6)
              + ( (cc.rgbResult & 0xf80000) >> 9)
            );
            
            DeleteObject(ed->brush[k]);
            ed->brush[k]=CreateSolidBrush(cc.rgbResult);
upd:
            ed->modf=1;
            rc.left=m+(i<<4);
            rc.top=n+(j<<4);
            rc.right=rc.left+16;
            rc.bottom=rc.top+16;
            
            InvalidateRect(win,&rc,0);
        }
        
        break;
    
    case WM_DESTROY:
        
        ed = (PALEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        for(i=ed->palw*ed->palh-1;i>=0;i--) DeleteObject(ed->brush[i]);
        free(ed->pal);
        free(ed->brush);
        ed->ew.doc->pals[ed->ew.param&1023]=0;
        
        break;
    
    case WM_CREATE:
        ed=(PALEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        i=(ed->ew.param>>10)&63;
        ed->palw=pal_w[i];
        ed->palh=pal_h[i];
        k=ed->palw*ed->palh;
        ed->pal=malloc(k*2);
        ed->brush=malloc(k*4);
        ed->modf=0;
        b=(short*)(ed->ew.doc->rom+romaddr(pal_addr[i]+(k*(ed->ew.param>>16)<<1)));
        for(j=0;j<k;j++) {
            l=ed->pal[j]=b[j];
            ed->brush[j]=CreateSolidBrush(((l&0x1f)<<3)+((l&0x3e0)<<6)+((l&0x7c00)<<9));
        }
        Updatesize(win);
        
    deflt:
    default:
        
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

void Saveworldmap(WMAPEDIT*ed)
{
    int i,j=ed->ew.param?1:4,k;
    int*b,*b2;
    if(!ed->modf) return;
    b=(int*)(ed->ew.doc->rom + 0x54727 + (ed->ew.param<<12));
    for(i=0;i<j;i++) {
        b2=(int*)(ed->buf+wmap_ofs[i]);
        for(k=0;k<32;k++) {
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            b2+=8;
        }
    }
    ed->modf=0;
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

long CALLBACK tmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

//Drawblock32#******************************

void Drawblock32(OVEREDIT *ed, int m, int t)
{
    int k,
        n,
        p,
        q;
    
    unsigned short *o;
    unsigned char *rom = ed->ew.doc->rom;
    
    short l[4];
    
    Getblock32(rom, m, l);
    
    p = 0;
    
    for(k = 0; k < 4; k++)
    {
        o = (unsigned short*)(rom + 0x78000 + ((((ed->disp & 8) && ovlblk[k] != 65535) ? ovlblk[k] : l[k]) << 3));
        
        for(q = 0; q < 4; q++)
        {
            n = *(o++);
            Drawblock(ed, blkx[p], blky[p], n, t);
            p++;
        }
    }
}

//Drawblock32*******************************

void Trackchgsel(HWND win,RECT*rc,TRACKEDIT*ed)
{
    rc->top=(mark_start-ed->scroll)*textmetric.tmHeight;
    rc->bottom=(mark_end+1-ed->scroll)*textmetric.tmHeight;
    
    InvalidateRect(win,rc,1);
}

long CALLBACK trackerproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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
        GetClientRect(win,&rc);
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
            GetClientRect(win,&rc);
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
        GetClientRect(win,&rc);
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
            if(mark_doc!=ed->ew.doc || mark_sr!=ed->ew.param) {
unmark:
                if(mark_doc) {
                    oldobj=mark_doc->sr[mark_sr].editor;
                    if(oldobj) {
                        oldobj=GetDlgItem(GetDlgItem(oldobj,2000),3000);
                        GetClientRect(oldobj,&rc);
                        Trackchgsel(oldobj,&rc,(TRACKEDIT*)GetWindowLong(oldobj,GWL_USERDATA));
                    }
                }
                if(wparam=='U') {
                    mark_doc=0;
                    break;
                }
                mark_doc=ed->ew.doc;
                mark_sr=ed->ew.param;
                mark_first=mark_last=ed->tbl[mark_start=mark_end=ed->sel];
                GetClientRect(win,&rc);
            } else {
                GetClientRect(win,&rc);
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
            GetClientRect(win,&rc);
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
            GetClientRect(win,&rc);
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
            k+=textmetric.tmHeight;
        }
        k=(ed->sel-ed->scroll)*textmetric.tmHeight;
        SetROP2(hdc,R2_NOTXORPEN);
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
        if(ed->csel!=-1) {
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
                                k = get_16_le
                                (
                                    ed->buf + ed->sel + 4
                                  + ( ( ( p - ed->selrect.top ) >> 2) & -2 )
                                );
                                
                                goto getblk;
                            }
                            
                            put_16_le
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
                    ed->dragx=o&-9;
                    ed->dragy=p&-9;
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

HGDIOBJ
Paintovlocs(HDC hdc,OVEREDIT*ed,int t,int n,int o,int q,unsigned char *rom)
{
    HGDIOBJ oldobj = 0;
    
    RECT rc;
    int i,j,k,m;
    short *b2,*b3;
    unsigned char*b6;
    
    switch(t)
    {
    
    case 0:
        oldobj = SelectObject(hdc, purple_brush);
        if(ed->ew.param<0x90) {
            m=ed->sprset;
            b6=ed->ebuf[m];
            for(i=ed->esize[m]-3;i>=0;i-=3) {
                if(ed->tool==5 && i==ed->selobj) continue;
                rc.left=((b6[i+1])<<4)-n;
                rc.top=((b6[i])<<4)-o;
                Drawdot(hdc,&rc,q,n,o);
                Getoverstring(ed,5,i);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
        break;
    case 1:
        b2=(short*)(ed->ew.doc->rom + 0xdb96f);
        b3=(short*)(ed->ew.doc->rom + 0xdba71);
        oldobj = SelectObject(hdc, yellow_brush);
        for(i=0;i<129;i++) {
            if(ed->tool==3 && i==ed->selobj) continue;
            if(b2[i]==ed->ew.param) {
                j=b3[i];
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                wsprintf(buffer,"%02X",rom[0xdbb73 + i]);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
        break;
    case 2:
        b2=(short*)(rom + 0x160ef);
        b3=(short*)(rom + 0x16051);
        oldobj = SelectObject(hdc, white_brush);
        for(i=0;i<79;i++) {
            if(ed->tool==7 && i==ed->selobj) continue;
            if(rom[0x15e28 + i]==ed->ew.param) {
                j=b2[i];
                k=b3[i];
                j-=((ed->ew.param&7)<<9);
                k-=((ed->ew.param&56)<<6);
                rc.left=j-n;
                rc.top=k-o;
                Drawdot(hdc,&rc,q,n,o);
                wsprintf(buffer,"%04X",((unsigned short*)(rom + 0x15d8a))[i]);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
        break;
    case 3:
        b2=(short*)(ed->ew.doc->rom + 0xdb826);
        b3=(short*)(ed->ew.doc->rom + 0xdb800);
        oldobj = SelectObject(hdc, black_brush);
        for(i=0;i<19;i++) {
            if(ed->tool==8 && i==ed->selobj) continue;
            if(b2[i]==ed->ew.param) {
                j=b3[i];
                j+=0x400;
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                wsprintf(buffer,"%02X",rom[0xdb84c + i]);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
        break;
    case 4:
        oldobj = SelectObject(hdc, red_brush);
        if(ed->ew.param<0x80) {
            for(i=0;i<ed->ssize;i+=3) {
                if(ed->tool==10 && i==ed->selobj) continue;
                j=ed->sbuf[i];
                k=ed->sbuf[i+1];
                j+=k<<8;
                rc.left=((j&0x7e)<<3)-n;
                rc.top=((j&0x1f80)>>3)-o;
                Drawdot(hdc,&rc,q,n,o);
                Getoverstring(ed,10,i);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
        break;
    case 5:
        b2=(short*)(rom + 0x16b8f);
        b3=(short*)(rom + 0x16b6d);
        oldobj = SelectObject(hdc, blue_brush);
        for(i=0;i<17;i++) {
            if(ed->tool==9 && i==ed->selobj) continue;
            if(((short*)(rom + 0x16ae5))[i]==ed->ew.param) {
                j=b2[i];
                k=b3[i];
                j-=((ed->ew.param&7)<<9);
                k-=((ed->ew.param&56)<<6);
                rc.left=j-n;
                rc.top=k-o;
                Drawdot(hdc,&rc,q,n,o);
                Getoverstring(ed,9,i);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
            }
        }
    }
    
    return oldobj;
}

static int wmmark_ofs[7]={
    0x53de4,0x53e2c,0x53e08,0x53e50,0x53e74,0x53e98,0x53ebc
};
static int wmpic_ofs[7]={
    0x53ee0,0x53f04,0x53ef2,0x53f16,0x53f28,0x53f3a,0x53f4c
};

void Getwmapstring(WMAPEDIT const * const ed,
                   int i)
{
    if(ed->marknum==9)
        wsprintf(buffer,"%d",i+1);
    else wsprintf(buffer,"%d-%04X",i+1,((unsigned short*)(ed->ew.doc->rom+wmpic_ofs[i]))[ed->marknum]);
}
void Wmapselchg(WMAPEDIT*ed,HWND win,int c)
{
    RECT rc;
    HWND win2;
    WMAPEDIT*ed2;
    int a,b;
    a=ed->ew.param?0:128;
    b=a-(ed->mapscrollv<<5);
    a-=ed->mapscrollh<<5;
    rc.left=ed->objx+a;
    rc.top=ed->objy+b;
    if(ed->tool==4) {
        rc.right=rc.left+64;
        rc.bottom=rc.top+64;
    } else {
        Getwmapstring(ed,ed->selmark);
        Getstringobjsize(buffer,&rc);
    }
    InvalidateRect(win,&rc,0);
    if(c) {
        win2=ed->ew.doc->wmaps[ed->ew.param^1];
        ed->ew.doc->modf=1;
        if(!win2) return;
        ed2=(WMAPEDIT*)GetWindowLong(win2,GWL_USERDATA);
        if((ed->tool==4 && ed2->tool!=4) || (ed->tool!=4 && 
            (ed2->marknum!=ed->marknum || ed2->tool==4))) return;
        win2=GetDlgItem(ed2->dlg,3000);
        rc.right-=a;
        rc.bottom-=b;
        a=ed2->ew.param?0:128;
        b=a-(ed2->mapscrollv<<5);
        a-=ed2->mapscrollh<<5;
        rc.left=ed->objx+a;
        rc.top=ed->objy+b;
        rc.right+=a;
        rc.bottom+=b;
        InvalidateRect(win2,&rc,0);
    }
}
int Getwmappos(WMAPEDIT      const * const ed,
               unsigned char const * const rom,
               int i,
               RECT *rc,
               int n,
               int o)
{
    int a,b;
    b=ed->marknum;
    if(b==9) {
        rc->left = ( rom[0x53763 + i] + (rom[0x5376b + i] << 8) ) >> 4;
        rc->top  = ( rom[0x53773 + i] + (rom[0x5377b + i] << 8) ) >> 4;
    } else {
        a=((short*)(rom+wmmark_ofs[i]))[b];
        if(a<0) return 1;
        rc->left=a>>4;
        rc->top=((short*)(rom+wmmark_ofs[i]+18))[b]>>4;
    }
    if(!ed->ew.param) rc->left+=128,rc->top+=128;
    rc->left-=n;
    rc->top-=o;
    return 0;
}

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
void lmapblkchg(HWND win,LMAPEDIT*ed)
{
    RECT rc;
    rc.left = ed->blksel % ed->blkrow << 4;
    rc.top = (ed->blksel / ed->blkrow - ed->blkscroll) << 4;
    rc.right=rc.left+16;
    rc.bottom=rc.top+16;
    InvalidateRect(win,&rc,0);
}
long CALLBACK lmapblksproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

long CALLBACK
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

static int sprs[5] = {0, 64, 128, 208, 272};

// =============================================================================

void
WorldMapDisplay_OnPaint(WMAPEDIT const * const p_ed,
                        HWND             const p_win)
{
    unsigned char const * const rom = p_ed->ew.doc->rom;
    
    unsigned char const * b2 = 0;
    unsigned char const * b4 = 0;
    
    int j = 0,
        k = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0,
        t = 0;
    
    RECT rc = empty_rect;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    HGDIOBJ oldobj2;
    
    // -----------------------------
    
    RealizePalette(hdc);
    
    k = ((ps.rcPaint.right+31)&0xffffffe0);
    l = ((ps.rcPaint.bottom+31)&0xffffffe0);
    n = p_ed->mapscrollh<<5;
    o = p_ed->mapscrollv<<5;
    j = ps.rcPaint.top&0xffffffe0;
    
    b4 = p_ed->buf + ( (j + o) << 3);
    
    m = p_ed->ew.param ? 0x100 : 0x200;
    
    t = (j + o) >> 3;
    
    if(p_ed->selflag || p_ed->dtool==2 || p_ed->dtool==3)
        Wmapselectionrect(p_ed,&rc);
    
    for(;j<l;j+=32)
    {
        int i = ps.rcPaint.left & 0xffffffe0;
        int s = (i + n) >> 3;
        
        if(j + o >= m)
            break;
        
        b2 = b4 + s;
        
        for(;i<k;i+=32)
        {
            int q = 0;
            
            if(i + n >= m)
                break;
            
            for(q=0;q<32;q+=8)
            {
                int p = 0;
                
                for(p = 0; p < 32; p += 8)
                {
                    int r = b2[ (p >> 3) + (q << 3) ];
                    
                    if(p_ed->dtool == 3)
                    {
                        if(i + p >= rc.left && i + p < rc.right && j + q >= rc.top && j + q < rc.bottom)
                            r = p_ed->bs.sel;
                    }
                    else if(p_ed->selflag)
                    {
                        if(i + p >= rc.left && i + p < rc.right && j + q >= rc.top && j + q < rc.bottom)
                            r = p_ed->selbuf[s + (p >> 3) - p_ed->rectleft + (t + (q >> 3) - p_ed->recttop) * (p_ed->rectright - p_ed->rectleft)];
                    }
                    
                    Drawblock((OVEREDIT*) p_ed,
                              p,
                              q,
                              r,
                              0);
                }
            }
            
            b2 += 4;
            s += 4;
            
            Paintblocks(&(ps.rcPaint),hdc,i,j, (DUNGEDIT*) p_ed);
        }
        
        b4+=256;
        t+=4;
    }
    
    if(rc.right > m - n)
        rc.right = m - n;
    
    if(rc.bottom > m - o)
        rc.bottom = m - o;
    
    if(p_ed->dtool == 2)
    {
        if(rc.right > rc.left && rc.bottom > rc.top)
            FrameRect(hdc, &rc, white_brush);
    }
    
    if(p_ed->selflag)
        FrameRect(hdc,&rc,green_brush);
    
    oldobj2 = SelectObject(hdc, trk_font);
    
    SetBkMode(hdc, TRANSPARENT);
    
    if(p_ed->tool == 4)
    {
        int i = 0;
        
        HGDIOBJ const oldobj = SelectObject(hdc, white_pen);
        
        for(i = 0; i < 64; i++)
        {
            j=((i&7)<<5)-n;
            k=((i&56)<<2)-o;
            if(m==0x200) j+=128,k+=128;
            l=rom[0x125ec + i];
            wsprintf(buffer,"%02X",l);
            Paintspr(hdc,j+8,k+8,n,o,m);
            
            if(l!=i)
                continue;
            
            l = rom[0x12844 + i] ? 64 : 32;
            
            MoveToEx(hdc,j+l,k,0);
            LineTo(hdc,j,k);
            LineTo(hdc,j,k+l);
        }
        
        j = 0;
        k = 0;
        
        if(m == 0x200)
            j = 128, k = 128;
        
        j -= n;
        k -= o;
        
        MoveToEx(hdc,j+256,k,0);
        LineTo(hdc,j+256,k+256);
        LineTo(hdc,j,k+256);
        
        SelectObject(hdc, oldobj);
    }
    else
    {
        int i = 0;
        
        int const l = p_ed->marknum;
        
        HGDIOBJ const oldobj = SelectObject(hdc, purple_brush);
        
        if(l==9)
            i=7;
        else
            i=6;
        
        for( ; i >= 0; i--)
        {
            if( p_ed->tool == 3 && i == p_ed->selmark )
                continue;
            
            if( Getwmappos(p_ed, rom, i, &rc, n, o) )
                continue;
            
            Drawdot(hdc,&rc,m,n,o);
            Getwmapstring(p_ed,i);
            Paintspr(hdc, rc.left,rc.top,n,o,m);
        }
        
        if(p_ed->tool == 3)
        {
            i = p_ed->selmark;
            
            if(i!=-1)
            {
                if(!Getwmappos(p_ed, rom,i,&rc,n,o))
                {
                    Getwmapstring(p_ed,i);
                    Getstringobjsize(buffer,&rc);
                    if(rc.right>m-n) rc.right=m-n;
                    if(rc.bottom>m-o) rc.bottom=m-o;
                    FillRect(hdc,&rc,black_brush);
                    FrameRect(hdc,&rc,green_brush);
                    Paintspr(hdc,rc.left,rc.top,n,o,m);
                }
            }
        }
        
        SelectObject(hdc,oldobj);
    }
    
    SelectObject(hdc, oldobj2);
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

long CALLBACK wmapdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT*ed;
    int i,j,k,l,m,n,o,p,q;
    FDOC*doc;
    SCROLLINFO si;
    unsigned char*b2,*b4,*rom;
    short*b1,*b3;
    static int sprxpos[4]={0,32,0,32};
    static int sprypos[4]={0,0,32,32};
    static int sctpos[4]={0,64,4096,4160};
    int u[20];
    
    RECT rc = {0,0,0,0};
    
    POINT pt;
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    case WM_CHAR:
        if(wparam==26) wmapdlgproc(GetParent(win),WM_COMMAND,3007,0);
        break;
    case WM_SIZE:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->ew.param?8:16;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,si.nMax,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,si.nMax,32);
        break;
    case WM_VSCROLL:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollv=Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,ed->ew.param?8:16,32);
        break;
    case WM_HSCROLL:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->mapscrollh=Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,ed->ew.param?8:16,32);
        break;
    case WM_LBUTTONDOWN:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->dtool) break;
        if(ed->tool!=1 && ed->selflag) {
            Wmapselectwrite(ed);
            Wmapselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
        }
        ed->dtool=ed->tool+1;
        SetCapture(win);
        SetFocus(win);
        goto mousemove;
    case WM_RBUTTONDOWN:
    case WM_MOUSEMOVE:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
mousemove:
        
        n=ed->mapscrollh<<5;
        o=ed->mapscrollv<<5;
        j=((short)lparam)+n;
        k=(lparam>>16)+o;
        q=ed->ew.param?256:512;
        if(j<0) j=0;
        if(j>=q) j=q-1;
        if(k<0) k=0;
        if(k>=q) k=q-1;
        if(msg==WM_RBUTTONDOWN) {
            if(ed->tool==3) {
                rom=ed->ew.doc->rom;
                if(ed->marknum==9) l=8; else l=7;
                if(q==512) j-=128,k-=128;
                if(j<0) j=0;
                if(k<0) k=0;
                if(j>255) j=255;
                if(k>255) k=255;
                
                for(i = 0; i < l; i++)
                    if( Getwmappos(ed,rom,i,&rc,n,o) )
                        break;
                
                if(i < l || ed->selmark != -1)
                {
                    HMENU menu = CreatePopupMenu();
                    
                    if(i<l)
                    {
                        for(i=0;i<l;i++) if(Getwmappos(ed,rom,i,&rc,n,o)) {
                            wsprintf(buffer,"Insert icon %d",i+1);
                            AppendMenu(menu, MF_STRING, i + 1, buffer);
                        }
                    }
                    
                    if(ed->selmark != -1)
                        AppendMenu(menu, MF_STRING, 10, "Remove icon");
                    
                    GetCursorPos(&pt);
                    
                    i = TrackPopupMenu(menu,
                                       TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY,
                                       pt.x,
                                       pt.y,
                                       0,
                                       win,
                                       0);
                    
                    DestroyMenu(menu);
                    
                    if(i==10) {
                        Wmapselchg(ed,win,1);
                        ((short*)(rom+wmmark_ofs[ed->selmark]))[ed->marknum]=-1;
                        ed->selmark=-1;
                    } else if(i) {
                        Wmapselchg(ed,win,0);
                        i--;
                        b1=((short*)(rom+wmmark_ofs[i]))+ed->marknum;
                        ed->selmark=i;
                        *b1=j<<4;
                        b1[9]=k<<4;
                        ed->objx=j;
                        ed->objy=k;
                        Wmapselchg(ed,win,1);
                    }
                }
                break;
            }
            i=ed->buf[(j>>3)+(k>>3<<6)];
            SendMessage(GetParent(win),4000,i,0);
            break;
        }
        switch(ed->dtool) {
        case 1:
            if(msg==WM_LBUTTONDOWN) {
                memcpy(ed->undobuf,ed->buf,0x1000);
                ed->undomodf=ed->modf;
            }
            j&=0xfff8;
            k&=0xfff8;
            ed->buf[(j>>3)+(k<<3)]=ed->bs.sel;
            ed->modf=1;
            rc.left=j-n;
            rc.right=j-n+8;
            rc.top=k-o;
            rc.bottom=k-o+8;
            InvalidateRect(win,&rc,0);
            break;
        case 2:
        case 3:
            j>>=3;
            k>>=3;
            if(msg==WM_LBUTTONDOWN) {
                if(ed->selflag) if(j>=ed->rectleft && j<ed->rectright && k>=ed->recttop && k<ed->rectbot) {
                    ed->selx=j;
                    ed->sely=k;
                } else {
                    Wmapselectwrite(ed);
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    goto newsel;
                } else newsel: ed->rectleft=ed->rectright=j,ed->recttop=ed->rectbot=k;
            } else {
                if(ed->selflag) {
                    if(j==ed->selx && k==ed->sely) break;
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectleft+=j-ed->selx;
                    ed->rectright+=j-ed->selx;
                    ed->recttop+=k-ed->sely;
                    ed->rectbot+=k-ed->sely;
                    ed->selx=j;
                    ed->sely=k;
                } else {
                    if(j>=ed->rectleft) j++;
                    if(k>=ed->recttop) k++;
                    if(ed->rectright==j && ed->rectbot==k) break;
                    Wmapselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectright=j,ed->rectbot=k;
                }
                Wmapselectionrect(ed,&rc);
                InvalidateRect(win,&rc,0);
            }
            break;
        case 4:
            rom=ed->ew.doc->rom;
            p=ed->marknum;
            if(msg==WM_LBUTTONDOWN) {
                q=ed->ew.param?0:128;
                Wmapselchg(ed,win,0);
                ed->selmark=-1;
                if(p==9) l=8; else l=7;
                for(i=0;i<l;i++) {
                    if(Getwmappos(ed,rom,i,&rc,n,o)) continue;
                    rc.left+=n;
                    rc.top+=o;
                    if(j>=rc.left && k>=rc.top && j<rc.left+16 && k<rc.top+16) {
                        ed->selmark=i;
                        ed->objx = (short) (rc.left - q);
                        ed->objy = (short) (rc.top - q);
                        ed->selx=j;
                        ed->sely=k;
                        Wmapselchg(ed,win,0);
                        return 0;
                    }
                }
                ed->dtool=0;
                ReleaseCapture();
                break;
            } else {
                i=ed->selmark;
                j-=ed->selx;
                k-=ed->sely;
                if(!(j||k)) break;
                Wmapselchg(ed,win,1);
                ed->objx+=j;
                ed->objy+=k;
                if(ed->objx<0) j-=ed->objx,ed->objx=0;
                if(p==9) {
                    l=rom[0x53763 + i]+(rom[0x5376b + i]<<8)+(j<<4);
                    rom[0x53763 + i]=l;
                    rom[0x5376b + i]=l>>8;
                    l=rom[0x53773 + i]+(rom[0x5377b + i]<<8)+(k<<4);
                    rom[0x53773 + i]=l;
                    rom[0x5377b + i]=l>>8;
                } else {
                    ((short*)(rom+wmmark_ofs[i]))[p]+=j<<4;
                    ((short*)(rom+wmmark_ofs[i]+18))[p]+=k<<4;
                }
                ed->selx+=j;
                ed->sely+=k;
                Wmapselchg(ed,win,1);
            }
            break;
        case 5:
            doc=ed->ew.doc;
            rom=doc->rom;
            if(q==512) j-=128,k-=128;
            if(j>=0 && k>=0 && j<256 && k<256) {
                i=(j>>5)+(k>>5<<3);
                if(rom[0x12844 + i]) {
                    i=rom[0x125ec + i];
                    if(doc->overworld[i].win) {
                        MessageBox(framewnd,"You can't change the map size while it is being edited.","Bad error happened",MB_OK);
                        goto maperror;
                    }
                    for(n=0;n<128;n+=64) {
                        m=0xe0000 + ((short*)(rom + 0xdc2f9))[n+i];
                        for(o=0;*(short*)(rom+m+o)!=-1;o+=3);
                        for(l=1;l<4;l++) ((short*)(rom + 0xdc2f9))[n+i+map_ind[l]]=m+o;
                    }
                    b2=malloc(512);
                    b4=malloc(512);
                    for(n=0;n<5;n++) {
                        u[n]=((short*)(rom + 0x4c881))[sprs[n]+i];
                    }
                    for(n=0;n<5;n++) {
                        p=((short*)(rom + 0x4c881))[sprs[n]+i];
                        for(o=0;o<n;o++) {
                            if(u[n]==u[o]) {
                                ((short*)(rom + 0x4c881))[sprs[n]+i]=((short*)(rom + 0x4c881))[sprs[o]+i];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+1]=((short*)(rom + 0x4c881))[sprs[o]+i+1];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+8]=((short*)(rom + 0x4c881))[sprs[o]+i+8];
                                ((short*)(rom + 0x4c881))[sprs[n]+i+9]=((short*)(rom + 0x4c881))[sprs[o]+i+9];
                                goto nextspr;
                            }
                        }
                        memcpy(b2,rom + 0x50000 + p,512);
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            p=0;
                            for(o=0;b2[o]!=0xff;o+=3) {
                                if(((b2[o]&32)==sprypos[l]) &&
                                    ((b2[o+1]&32)==sprxpos[l])) {
                                    b4[p++]=b2[o]&31;
                                    b4[p++]=b2[o+1]&31;
                                    b4[p++]=b2[o+2];
                                }
                            }
                            Savesprites(ed->ew.doc,sprs[n]+m,b4,p);
                        }
nextspr:;
                    }
                    for(n=0;n<128;n+=64) {
                        memcpy(b2,rom + 0xe0000 + ((short*)(rom + 0xdc2f9))[n+i],512);
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            p=0;
                            for(o=0;*(short*)(b2+o)!=-1;o+=3) {
                                q=*(short*)(b2+o);
                                k=(q&0x1f80)>>7;
                                j=(q&0x7e)>>1;
                                if(k>=sprypos[l] && k<sprypos[l]+32 &&
                                    j>=sprxpos[l] && j<sprxpos[l]+32) {
                                    *(short*)(b4+p)=q&0xfbf;
                                    b4[p+2]=b2[o+2];
                                    p+=3;
                                }
                            }
                            if(Savesecrets(ed->ew.doc,n+m,b4,p)) Savesecrets(ed->ew.doc,n+m,0,0);
                        }
                    }
                    free(b4);
                    free(b2);
                    for(l=0;l<4;l++) {
                        m=i+map_ind[l];
                        rom[0x125ec + m]=m;
                        rom[0x12844 + m]=0;
                        rom[0x12884 + m]=1;
                        rom[0x1788d + m]=1;
                        rom[0x178cd + m]=1;
                        rom[0x4c635 + m]=2;
                        rom[0x4c675 + m]=2;
                        rom[0x4c6b5 + m]=2;
                        ((short*)(rom + 0x12634))[m]=96;
                        ((short*)(rom + 0x126b4))[m]=64;
                        ((short*)(rom + 0x12734))[m]=6144;
                        ((short*)(rom + 0x127b4))[m]=4096;
                        ((short*)(rom + 0x13ee2))[m]=(((short*)(rom + 0x128c4))[m]=(m&56)<<6)-224;
                        ((short*)(rom + 0x13f62))[m]=(((short*)(rom + 0x12944))[m]=(m&7)<<9)-256;
                    }
                    goto updlayout;
                } else if(((i&7)<7) && ((i&56)<56) && (!rom[0x12845 + i])
                    && (!rom[0x1284c + i]) && (!rom[0x1284d + i])) {
                    if(doc->overworld[i].win || doc->overworld[i+1].win || doc->overworld[i+8].win || doc->overworld[i+9].win ) {
                        MessageBox(framewnd,"You can't change the map size while it is being edited.","Bad error happened",MB_OK);
                        goto maperror;
                    }
                    for(n=0;n<5;n++) {
                        u[n]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i];
                        u[n+5]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+1];
                        u[n+10]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+8];
                        u[n+15]=((unsigned short*)(rom + 0x4c881))[sprs[n]+i+9];
                    }
                    for(n=0;n<5;n++)
                        for(o=0;o<n;o++)
                            if(((u[n]==u[o]&&u[n]!=0xcb41)
                                || (u[n+5]==u[o+5]&&u[n+5]!=0xcb41)
                                || (u[n+10]==u[o+10]&&u[n+10]!=0xcb41)
                                || (u[n+15]==u[o+15]&&u[n+15]!=0xcb41)) &&
                                !(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15])) {
                                MessageBox(framewnd,"The manner in which sprites sets are reused in the different parts of the game is not the same throughout the 4 areas.","Bad error happened",MB_OK);
                                goto maperror;
                            }
                    ((short*)(rom + 0x12634))[i]=96;
                    ((short*)(rom + 0x12636))[i]=96;
                    ((short*)(rom + 0x12644))[i]=4192;
                    ((short*)(rom + 0x12646))[i]=4192;
                    ((short*)(rom + 0x126b4))[i]=128;
                    ((short*)(rom + 0x126b6))[i]=128;
                    ((short*)(rom + 0x126c4))[i]=4224;
                    ((short*)(rom + 0x126c6))[i]=4224;
                    ((short*)(rom + 0x12734))[i]=6144;
                    ((short*)(rom + 0x12736))[i]=6208;
                    ((short*)(rom + 0x12744))[i]=6144;
                    ((short*)(rom + 0x12746))[i]=6208;
                    ((short*)(rom + 0x127b4))[i]=8192;
                    ((short*)(rom + 0x127b6))[i]=8256;
                    ((short*)(rom + 0x127c4))[i]=8192;
                    ((short*)(rom + 0x127c6))[i]=8256;
                    b4=malloc(512);
                    for(n=0;n<5;n++) {
                        for(o=0;o<n;o++) if(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15]) goto nextspr2;
                        p=0;
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            b2=rom + 0x50000 + ((short*)(rom + 0x4c881))[sprs[n]+m];
                            for(o=0;b2[o]!=0xff;o+=3) {
                                b4[p++]=b2[o]+sprypos[l];
                                b4[p++]=b2[o+1]+sprxpos[l];
                                b4[p++]=b2[o+2];
                            }
                        }
                        Savesprites(ed->ew.doc,sprs[n]+i+1,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i+8,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i+9,0,0);
                        Savesprites(ed->ew.doc,sprs[n]+i,b4,p);
nextspr2:;
                    }
                    for(n=0;n<5;n++)
                        for(o=0;o<n;o++)
                            if(u[n]==u[o] && u[n+5]==u[o+5] && u[n+10]==u[o+10] && u[n+15]==u[o+15])
                                ((short*)(rom + 0x4c881))[sprs[n]+i]=((short*)(rom + 0x4c881))[sprs[o]+i];
                    for(n=0;n<128;n+=64) {
                        p=0;
                        for(l=0;l<4;l++) {
                            m=i+map_ind[l];
                            b2=rom + 0xe0000 + ((short*)(rom + 0xdc2f9))[n+m];
                            for(o=0;*(short*)(b2+o)!=-1;o+=3) {
                                *(short*)(b4+p)=(*(short*)(b2+o))+sctpos[l];
                                b4[p+2]=b2[o+2];
                                p+=3;
                            }
                        }
                        Savesecrets(ed->ew.doc,n+i+1,0,0);
                        Savesecrets(ed->ew.doc,n+i+8,0,0);
                        Savesecrets(ed->ew.doc,n+i+9,0,0);
                        Savesecrets(ed->ew.doc,n+i,b4,p);
                    }
                    free(b4);
                    for(l=0;l<4;l++) {
                        m=i+map_ind[l];
                        rom[0x125ec + m]=i;
                        rom[0x12844 + m]=32;
                        rom[0x12884 + m]=3;
                        rom[0x1788d + m]=0;
                        rom[0x178cd + m]=0;
                        rom[0x4c635 + m]=4;
                        rom[0x4c675 + m]=4;
                        rom[0x4c6b5 + m]=4;
                        ((short*)(rom + 0x13ee2))[m]=(((short*)(rom + 0x128c4))[m]=(i&56)<<6)-224;
                        ((short*)(rom + 0x13f62))[m]=(((short*)(rom + 0x12944))[m]=(i&7)<<9)-256;
                    }
updlayout:
                    b1=(short*)(rom + 0xdb96f);
                    b3=(short*)(rom + 0xdba71);
                    for(m=0;m<129;m++) {
                        l=b1[m];
                        if(l>=128) continue;
                        n=b3[m];
                        j=((l&7)<<9)+((n&0x7e)<<3);
                        k=((l&56)<<6)+((n&0x1f80)>>3);
                        o=rom[0x125ec + (((j>>9)+(k>>9<<3))&63)];
                        b1[m]=o|(l&64);
                        b3[m] =
                        ( ( ( j - ( (short*) (rom + 0x12944))[o] ) >> 3 ) & 0x7e)
                      + ( ( ( k - ( (short*) (rom + 0x128c4))[o] ) << 3 ) & 0x1f80);
                    }
                    for(m=0;m<79;m++) {
                        l=rom[0x15e28 + m];
                        if(l>=128) continue;
                        rom[0x15e28 + m]=Fixscrollpos(rom,0x160ef,0x16051,0x15fb3,0x15f15,0x1622b,0x1618d,0x15e77,m,l,0x16367,0x16405);
                    }
                    for(m=0;m<17;m++) {
                        l=((short*)(rom + 0x16ae5))[m];
                        if(l>=128) continue;
                        ((short*)(rom + 0x16ae5))[m]=Fixscrollpos(rom,0x16b8f,0x16b6d,0x16b4b,0x16b29,0x16bd3,0x16bb1,0x16b07,m,l,0,0);
                    }
                    b1=(short*)(rom + 0xdb826);
                    b3=(short*)(rom + 0xdb800);
                    for(m=0;m<19;m++) {
                        l=b1[m];
                        if(l>=128) continue;
                        n=b3[m] + 0x400;
                        j=((l&7)<<9)+((n&0x7e)<<3);
                        k=((l&56)<<6)+((n&0x1f80)>>3);
                        o=rom[0x125ec + (((j>>9)+(k>>9<<3))&63)];
                        b1[m]=o|(l&64);
                        
                        b3[m] =
                        ( ( ( j - get_16_le_i(rom + 0x12944, o) ) >> 3 ) & 0x7e )
                      + ( ( ( k - get_16_le_i(rom + 0x128c4, o) ) << 3 ) & 0x1f80 )
                      - 0x400;
                    }
                    ed->ew.doc->modf=1;
                    ed->objx=((i&7)<<5);
                    ed->objy=((i&56)<<2);
                    Wmapselchg(ed,win,1);
                }
            }
maperror:
            ed->dtool=0;
            ReleaseCapture();
            break;
        }
        break;
    case WM_LBUTTONUP:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->dtool>1 && ed->dtool<4) {
            Wmapselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
            if(ed->rectleft<ed->rectright) i=ed->rectleft,j=ed->rectright;
            else i=ed->rectright,j=ed->rectleft;
            if(ed->recttop<ed->rectbot) k=ed->recttop,l=ed->rectbot;
            else k=ed->rectbot,l=ed->recttop;
            if(ed->dtool==3) {
                ed->undomodf=ed->modf;
                memcpy(ed->undobuf,ed->buf,0x1000);
                for(m=i;m<j;m++)
                    for(n=k;n<l;n++)
                        ed->buf[m+(n<<6)]=ed->bs.sel;
                ed->modf=1;
            } else if(!ed->selflag) {
                ed->rectleft=i;
                ed->rectright=j;
                ed->recttop=k;
                ed->rectbot=l;
                ed->selflag=1;
                ed->stselx=i;
                ed->stsely=k;
                j-=i;
                l-=k;
                l<<=6;
                i+=k<<6;
                n=0;
                b4=ed->buf;
                ed->selbuf=malloc((ed->rectright-ed->rectleft)*(ed->rectbot-ed->recttop));
                for(o=0;o<l;o+=64) for(m=0;m<j;m++) ed->selbuf[n++]=b4[m+i+o];
            }
        }
        if(ed->dtool) ReleaseCapture();
        ed->dtool=0;
        break;
    
    case WM_PAINT:
        
        ed = (WMAPEDIT*) GetWindowLong(win, GWL_USERDATA);
        
        if(ed)
        {
            WorldMapDisplay_OnPaint(ed, win);
        }
        
        break;
    
    case WM_KEYDOWN:
        ed = (WMAPEDIT*) GetWindowLong(win, GWL_USERDATA);
        i=ed->selmark;
        if(ed->tool!=3 || i==-1 || ed->marknum==9) break;
        if(wparam>=65 && wparam<71) { wparam-=55; goto digit; }
        if(wparam>=48 && wparam<58) {
            wparam-=48;
digit:
            b1=((short*)(ed->ew.doc->rom+wmpic_ofs[i]))+ed->marknum;
            *b1<<=4;
            *b1|=wparam;
            Wmapselchg(ed,win,1);
        }
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

void changeposition(unsigned char*rom,int x,int y,int sx,int sy,int cx,int cy,int dx,int dy,int sp,int map,int i,int q,int door1,int door2)
{
    int a=(map&7)<<9,b=(map&56)<<6,d,e,f,g,h,j,k,l;
    k=((short*)(rom+x))[i];
    l=((short*)(rom+y))[i];
    h=((short*)(rom+x))[i]=k+dx;
    j=((short*)(rom+y))[i]=l+dy;
    if(door1) if(((unsigned short*)(rom+door2))[i]+1>=2) j+=39; else j+=19;
    d=((short*)(rom+sx))[i];
    f=((short*)(rom+cx))[i];
    if((dx<0 && h<f) || (dx>0 && h>f)) dx=h-f; else dx=0;
    d+=dx;
    if(d<a) dx+=a-d,d=a;
    if(d>a+q) dx+=a+q-d,d=a+q;
    ((short*)(rom+sx))[i]=d;
    ((short*)(rom+cx))[i]+=dx;
    e=((short*)(rom+sy))[i];
    g=((short*)(rom+cy))[i];
    if((dy<0 && j<g) || (dy>0 && j>g)) dy=j-g; else dy=0;
    e+=dy;
    if(e<b) dy+=b-e,e=b;
    if(e>b+q+32) dy+=b+q+32-e,e=b+q+32;
    ((short*)(rom+sy))[i]=e;
    ((short*)(rom+cy))[i]+=dy;
    ((short*)(rom+sp))[i]=(((e-b)&0xfff0)<<3)+(((d-a)&0xfff0)>>3);
    
    if(door1)
    {
        if( ((unsigned short*)(rom + door1))[i] + 1 >= 2 )
            ((short*)(rom + door1))[i] +=
            ( ( ( (j - 19) & 0xfff0) - (l & 0xfff0) ) << 3 )
          + ( ( ( h & 0xfff0) - (k & 0xfff0) ) >> 3 );
        
        if( ((unsigned short*)(rom + door2))[i] + 1 >= 2 )
            ((short*)(rom + door2))[i] +=
            ( ( ( (j - 39) & 0xfff0 ) - (l & 0xfff0) ) << 3 )
          + ( ( (h & 0xfff0) - (k & 0xfff0) ) >> 3 );
    }
}

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

long CALLBACK
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
        GetClientRect(hc,&rc);
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

long CALLBACK blkedit32proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

void Palselrect(BLKEDIT8 *ed, HWND win, RECT *rc)
{
    (void) win;
    
    rc->left = (ed->psel & 15) * ed->pwidth >> 4;
    rc->top  = (ed->psel >> 4) * ed->pheight >> 4;
    rc->right  = ((ed->psel & 15) + 1) * ed->pwidth >> 4;
    rc->bottom = ((ed->psel >> 4) + 1) * ed->pheight >> 4;
}

// Palette selection window that gets used when editing 8x8 tiles
// (at the very least in the case of dungeon objects)
long CALLBACK palselproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,k,l,m,n;
    
    BLKEDIT8*ed;
    
    OVEREDIT*oed;
    
    PAINTSTRUCT ps;
    
    RECT rc;
    
    switch(msg)
    {
    
    case WM_PAINT:
        
        ed = (BLKEDIT8*) GetWindowLong(win, GWL_USERDATA);
        
        if(!ed) break;
        
        oed = ed->oed;
        
        if(always)
        {
            HDC const hdc = BeginPaint(win, &ps);
            
            HPALETTE const oldpal = SelectPalette(hdc, ed->oed->hpal, 1);
            
            HGDIOBJ const oldobj = GetCurrentObject(hdc, OBJ_BRUSH);
            
            HBRUSH newobj = 0;
            
            k=(ps.rcPaint.left<<4)/ed->pwidth;
            j=(ps.rcPaint.top<<4)/ed->pheight;
            l=((ps.rcPaint.right<<4)+ed->pwidth-1)/ed->pwidth;
            m=((ps.rcPaint.bottom<<4)+ed->pheight-1)/ed->pheight;
            
            RealizePalette(hdc);
            
            for(;j<m;j++)
            {
                for(i=k;i<l;i++)
                {
                    HBRUSH const oldob2 = newobj;
                    
                    if(oed->hpal)
                        newobj = CreateSolidBrush(0x1000000 + ((short*)(oed->pal))[i+(j<<4)]);
                    else
                    {
                        // \task Useless? No, but confusing (next line consumes
                        // this value)
                        n = i + (j << 4);
                        
                        n = (oed->pal[n].rgbBlue << 16) + (oed->pal[n].rgbGreen << 8) + (oed->pal[n].rgbRed);
                        
                        newobj = CreateSolidBrush(n);
                    }
                    
                    SelectObject(hdc, newobj);
                    
                    if(oldob2)
                        DeleteObject(oldob2);
                    
                    Rectangle(hdc,i*ed->pwidth>>4,j*ed->pheight>>4,(i+1)*ed->pwidth>>4,(j+1)*ed->pheight>>4);
                }
            }
            
            Palselrect(ed,win,&rc);
            FrameRect(hdc,&rc,green_brush);
            
            SelectPalette(hdc,oldpal,1);
            SelectObject(hdc,oldobj);
            
            if(newobj)
                DeleteObject(newobj);
            
            EndPaint(win, &ps);
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(BLKEDIT8*)GetWindowLong(win,GWL_USERDATA);
        Palselrect(ed,win,&rc);
        InvalidateRect(win,&rc,0);
        if(ed->blknum==260) j=ed->psel&0xf0;
        else if(ed->blknum>=256) j=ed->psel&0xfc; else j=ed->psel&0xf8;
        ed->psel=(((short)lparam)<<4)/ed->pwidth+((lparam>>12)/ed->pheight<<4);
        if(ed->blknum==260) i=ed->psel&0xf0,k=15;
        else if(ed->blknum>=256) i=ed->psel&0xfc,k=3; else i=ed->psel&0xf8,k=7;
        if(ed->oed->gfxtmp!=0xff && i!=j) {
            for(j=(ed->size<<6)-1;j>=0;j--) {
                ed->buf[j]&=k;
                if(ed->buf[j]) ed->buf[j]|=i;
            }
            InvalidateRect(ed->blkwnd,0,0);
        }
        Palselrect(ed,win,&rc);
        InvalidateRect(win,&rc,0);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

long CALLBACK blkedit8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    BLKEDIT8*ed;
    PAINTSTRUCT ps;
    HDC hdc;
    switch(msg) {
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

long CALLBACK blksel8proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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
    
    GetClientRect(p_win, &rc);
    
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
                          get_16_le_i(o, k),
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

long CALLBACK
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
        GetClientRect(win,&rc);
        
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
        GetClientRect(hc,&rc);
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
long CALLBACK blksel32proc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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
        GetClientRect(win,&rc);
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
        GetClientRect(win, &rc);
        
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

//aboutfunc#*******************************

BOOL CALLBACK aboutfunc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    if(msg == WM_COMMAND && wparam == IDCANCEL)
        EndDialog(win,0);
    
    return FALSE;
}

//aboutfunc*******************************

BOOL CALLBACK wavesetting(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        SetDlgItemInt(win,IDC_EDIT1,ws_freq,0);
        SetDlgItemInt(win,IDC_EDIT2,ws_bufs,0);
        SetDlgItemInt(win,IDC_EDIT3,ws_len,0);
        CheckDlgButton(win,IDC_CHECK1,ws_flags&1);
        CheckDlgButton(win,IDC_CHECK2,(ws_flags>>1)&1);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ws_freq=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ws_bufs=GetDlgItemInt(win,IDC_EDIT2,0,0);
            ws_len=GetDlgItemInt(win,IDC_EDIT3,0,0);
            ws_flags=IsDlgButtonChecked(win,IDC_CHECK1)
                |(IsDlgButtonChecked(win,IDC_CHECK2)<<1);
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}
BOOL CALLBACK midisetting(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    (void) lparam;
    
    switch(msg) {
    case WM_INITDIALOG:
        SetDlgItemInt(win,IDC_EDIT1,ms_tim1,0);
        SetDlgItemInt(win,IDC_EDIT2,ms_tim2,0);
        SetDlgItemInt(win,IDC_EDIT3,ms_tim3,0);
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            ms_tim1=GetDlgItemInt(win,IDC_EDIT1,0,0);
            ms_tim2=GetDlgItemInt(win,IDC_EDIT2,0,0);
            ms_tim3=GetDlgItemInt(win,IDC_EDIT3,0,0);
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}
int Soundsetting(HWND win,int dev)
{
    if((dev>>16)==1) {
        return ShowDialog(hinstance,(LPSTR)IDD_DIALOG3,win,wavesetting,0);
    } else return ShowDialog(hinstance,(LPSTR)IDD_DIALOG13,win,midisetting,0);
}
BOOL CALLBACK seldevproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j,lp;
    TVINSERTSTRUCT tvi;
    TVITEM*itemstr;
    WAVEOUTCAPS woc;
    MIDIOUTCAPS moc;
    HWND hc;
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,0);
        hc=GetDlgItem(win,IDC_TREE1);
        tvi.hParent=0;
        tvi.hInsertAfter=TVI_LAST;
        tvi.item.mask=TVIF_CHILDREN|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
        tvi.item.stateMask=TVIS_BOLD;
        tvi.item.state=0;
        tvi.item.lParam=0;
        tvi.item.pszText="Wave devices";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Wave mapper";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x10001;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        j=waveOutGetNumDevs();
        if(j>256) j=256;
        tvi.item.pszText=woc.szPname;
        for(i=0;i<j;i++) {
            if(waveOutGetDevCaps(i,&woc,sizeof(woc))) continue;
            tvi.item.lParam=0x10002 + i;
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.hParent=0;
        tvi.item.pszText="Midi devices";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Midi mapper";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x20000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        j=midiOutGetNumDevs();
        if(j>256) j=256;
        tvi.item.pszText=moc.szPname;
        for(i=0;i<j;i++) {
            if(midiOutGetDevCaps(i,&moc,sizeof(moc))) continue;
            tvi.item.lParam=0x20001 + i;
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDOK:
            lp=GetWindowLong(win,DWL_USER);
            if(!lp) break;
            if(!Soundsetting(win,lp)) break;
            Exitsound();
            sounddev=lp;
            Initsound();
            EndDialog(win,1);
            break;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
        break;
    case WM_NOTIFY:
        switch(wparam) {
        case IDC_TREE1:
            switch(((NMHDR*)lparam)->code) {
            case TVN_SELCHANGED:
                itemstr=&(((NMTREEVIEW*)lparam)->itemNew);
                SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_GETITEM,0,(long)itemstr);
                SetWindowLong(win,DWL_USER,itemstr->lParam);
                break;
            }
        }
    }
    return FALSE;
}

typedef struct {
    int sprsetedit;
    int namechg;
    char savespr[0x11c][16];
    int flag;
    int soundvol;
    unsigned short inst[25];
    unsigned short trans[25];
    char saveasmp[MAX_PATH];
} CONFIG;

void Updatesprites(void)
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
            if(win) {
                win=GetDlgItem(GetDlgItem(win,2000),3001);
                oe=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
                if(!(oe->disp&4)) continue;
                k=oe->esize[oe->sprset];
                l=oe->ebuf[oe->sprset];
                for(j=0;j<k;j++) {
                    rc.left=(l[j+1]<<4)-(oe->mapscrollh<<5);
                    rc.top=(l[j]<<4)-(oe->mapscrollv<<5);
                    Getstringobjsize(Getoverstring(oe,5,j),&rc);
                    InvalidateRect(win,&rc,0);
                }
                if(i<128) {
                    k=oe->ssize;
                    l=oe->sbuf;
                    for(j=0;j<k;j++) {
                        m=*(short*)(l+j);
                        rc.left=((m&0x7e)<<3)-(oe->mapscrollh<<5);
                        rc.top=((m&0x1f80)>>3)-(oe->mapscrollv<<5);
                        Getstringobjsize(Getoverstring(oe,10,j),&rc);
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
                if(!(ed->disp&4)) break;
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

BOOL CALLBACK editsprname(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,j;
    static CONFIG*config=0;
    SCROLLINFO si;
    if(msg==WM_HSCROLL) {
        i=soundvol;
        switch(wparam&65535) {
        case SB_LEFT:
            i=0;
            break;
        case SB_RIGHT:
            i=256;
            break;
        case SB_LINELEFT:
            i--;
            break;
        case SB_LINERIGHT:
            i++;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            i=wparam>>16;
            break;
        case SB_PAGELEFT:
            i-=16;
            break;
        case SB_PAGERIGHT:
            i+=16;
        }
        if(i<0) i=0;
        if(i>256) i=256;
        soundvol=i;
        config->flag|=4;
        volflag|=255;
        si.cbSize=sizeof(si);
        si.nPos=i;
        si.fMask=SIF_POS;
        SetScrollInfo((HWND)lparam,SB_CTL,&si,1);
soundvolchg:
        wsprintf(buffer,"%d%%",soundvol*100/256);
        SetDlgItemText(win,IDC_STATIC2,buffer);
    } else if(msg==WM_COMMAND) switch(wparam) {
    case IDC_EDIT1|(EN_CHANGE<<16):
updname:
        i=GetDlgItemInt(win,IDC_EDIT1,0,0);
        if(config->sprsetedit) j=0x1b; else j=255;
        if(i>j || i<0) { SetDlgItemInt(win,IDC_EDIT1,j,0); break; }
        config->namechg=1;
        SetDlgItemText(win,IDC_EDIT2,sprname[i+config->sprsetedit]);
        config->namechg=0;
        break;
    case IDC_EDIT2|(EN_CHANGE<<16):
        if(config->namechg) break;
        i=GetDlgItemInt(win,IDC_EDIT1,0,0);
        config->flag|=1;
        Updatesprites();
        GetDlgItemText(win,IDC_EDIT2,sprname[i+config->sprsetedit],16);
        Updatesprites();
        break;
    case IDC_EDIT3|(EN_CHANGE<<16):
        i=GetDlgItemInt(win,IDC_EDIT3,0,0);
        if(i>24 || i<0) { SetDlgItemInt(win,IDC_EDIT3,24,0); break; }
        config->namechg=1;
        SetDlgItemInt(win,IDC_EDIT4,midi_inst[i]&255,0);
        SetDlgItemInt(win,IDC_EDIT5,midi_inst[i]>>8,0);
        SetDlgItemInt(win,IDC_EDIT6,(char)(midi_trans[i]),1);
        SetDlgItemInt(win,IDC_EDIT8,(char)(midi_trans[i]>>8),1);
        config->namechg=0;
        break;
    case IDC_EDIT4|(EN_CHANGE<<16):
    case IDC_EDIT5|(EN_CHANGE<<16):
    case IDC_EDIT6|(EN_CHANGE<<16):
    case IDC_EDIT8|(EN_CHANGE<<16):
        if(config->namechg) break;
        i=GetDlgItemInt(win,IDC_EDIT3,0,0);
        midi_inst[i]=GetDlgItemInt(win,IDC_EDIT4,0,0)+(GetDlgItemInt(win,IDC_EDIT5,0,0)<<8);
        midi_trans[i]=(GetDlgItemInt(win,IDC_EDIT6,0,1)&255)+(GetDlgItemInt(win,IDC_EDIT8,0,1)<<8);
        config->flag|=8;
        break;
    case IDC_EDIT9|(EN_CHANGE<<16):
        if(config->namechg) break;
        GetDlgItemText(win,IDC_EDIT9,asmpath,MAX_PATH);
        config->flag|=16;
        break;
    case IDC_RADIO1:
        config->sprsetedit=0;
        goto updname;
    case IDC_RADIO3:
        config->sprsetedit=256;
        goto updname;
    case IDCANCEL:
        if(config->flag&1) {
            Updatesprites();
            memcpy(sprname,config->savespr,0x11c0);
            Updatesprites();
        }
        
        strcpy(asmpath,config->saveasmp);
        
        memcpy(midi_inst, config->inst, MIDI_ARR_BYTES);
        memcpy(midi_trans, config->trans, MIDI_ARR_BYTES);
        
        soundvol=config->soundvol;
        free(config);
        EndDialog(win,0);
        
        break;
    
    case IDOK:
        
        cfg_flag |= config->flag;
        
        if(config->flag)
            cfg_modf = 1;
        
        free(config);
        
        EndDialog(win,0);
        
        break;
    }
    else if(msg == WM_INITDIALOG)
    {
        config=malloc(sizeof(CONFIG));
        config->sprsetedit=0;
        config->namechg=1;
        config->soundvol=soundvol;
        config->flag=0;
        memcpy(config->savespr,sprname,0x11c0);
        memcpy(config->inst,midi_inst,100);
        CheckDlgButton(win,IDC_RADIO1,BST_CHECKED);
        SetDlgItemInt(win,IDC_EDIT1,0,0);
        SetDlgItemInt(win,IDC_EDIT3,0,0);
        SetDlgItemText(win,IDC_EDIT9,asmpath);
        strcpy(config->saveasmp,asmpath);
        config->namechg=0;
        si.cbSize=sizeof(si);
        si.fMask=SIF_POS|SIF_RANGE|SIF_PAGE;
        si.nPos=soundvol;
        si.nMin=0;
        si.nMax=256;
        si.nPage=16;
        SetScrollInfo(GetDlgItem(win,IDC_SCROLLBAR1),SB_CTL,&si,0);
        goto soundvolchg;
    }
    return FALSE;
}

// =============================================================================

BOOL CALLBACK
overdlgproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int i, // 
        j, // the overworld area number to load.
        k, // The offset for the 8 byte array of blockset information, for a particular area.
        l, // and auxiliary offset for additional blocktypes that are swapped in sometimes.
        m, // use as the graphics number.
        n,
        p,
        q;
    
    short o[4];
    
    HWND hc;
    OVEREDIT *ed, *oed;
    ZOVER *ov;
    
    unsigned char *rom, *b2;
    
    uint16_t * b4 = 0;
    uint16_t * b5 = 0;
    
    switch(msg)
    {
    
    case WM_MOUSEMOVE:
        
        if(always)
        {
            char handle_text[0x100];
            
            HM_MouseMoveData const d = HM_GetMouseMoveData(win, wparam, lparam);
            
            HWND const child = ChildWindowFromPoint(win, d.m_rel_pos);
            
            sprintf(handle_text,
                    "hwnd: %p, x: %d, y: %d",
                    child,
                    d.m_screen_pos.x,
                    d.m_screen_pos.y);
             
            SetDlgItemText(win, SD_OverWindowFocus, handle_text);
        }
        
        break;
    
    case WM_INITDIALOG:
        
        SetWindowLong(win,DWL_USER,lparam);
        
        ed = (OVEREDIT*) lparam;
        ed->hpal = 0;
        
        j = ed->ew.param;
        rom = ed->ew.doc->rom;
        
        ed->gfxtmp = (j & 0x40) ? 0x21 : 0x20;
        
        if(j == 0x88 || j == 0x93)
            ed->gfxtmp = 0x24;
        
        k = 0x6073 + (ed->gfxtmp << 3);
        m = rom[0x7c9c + j];
        
        if(j == 0x95)
            m = 0x22;
        else if(j == 0x96)
            m = 0x3b;
        else if(j == 0x88 || j == 0x93)
            m = 0x51;
        else if(j == 0x9e)
            m = 0x21;
        else if(j == 0x9c)
            m = 0x41;
        else if(j > 0x7f)
            m = 0x2f;
        
        ed->gfxnum = m;
        l = 0x5d97 + (m << 2);
        
        if(j < 0x80)
            ed->mapsize = rom[0x12844 + (j & 0x3f)];
        else if(j == 0x81)
            ed->mapsize = 0x20;
        else
            ed->mapsize = 0;
        
        EnableWindow( GetDlgItem(win, 3007), j < 0x80); //graphics number
        EnableWindow( GetDlgItem(win, 3010), j < 0x80); //palette number
        
        if(j < 0x80)
        {
            // Determines whether the up/down/left/right arrows are grayed out or not.
            EnableWindow(GetDlgItem(win, 3029), j & 7);
            EnableWindow(GetDlgItem(win, 3030), (j + (ed->mapsize ? 2 : 1)) & 7);
            EnableWindow(GetDlgItem(win, 3031), j & 56);
            EnableWindow(GetDlgItem(win, 3032), (j + (ed->mapsize ? 16 : 8)) & 56);
        }
        else
        {
            //Disable the directional arrows if it's an overlay or special area.
            EnableWindow(GetDlgItem(win, 3029), 0);
            EnableWindow(GetDlgItem(win, 3030), 0);
            EnableWindow(GetDlgItem(win, 3031), 0);
            EnableWindow(GetDlgItem(win, 3032), 0);
        }
        
        //Tell the window to set an appropriate graphic for each arrow button.
        for(i = 0; i < 4; i++)
            SendDlgItemMessage(win, 3029 + i, BM_SETIMAGE, IMAGE_BITMAP, (int) arrows_imgs[i]);
        
        //The GFX# box is set here.
        SetDlgItemInt(win, 3007, m, 0);
        
        //Write down the first eight blocksets this area will use
        for(i = 0; i < 8; i++)
            ed->blocksets[i] = rom[k++];
        
        // rewrite some of these blockset entries with the ones located at $5D97 + (4*gfxnumber)
        for(i = 3; i < 7; i++)
        {
            m = rom[l++];
            
            if(m != 0)
                ed->blocksets[i] = m;
        }
        
        // Tells us which area it is, without regard to light / dark world.
        m = j & 0x3f;
        
        // In certain cases, the 8th blockset is 0x58 or 0x5A
        if((m >= 3 && m <= 7) || j == 0x95)
            ed->blocksets[8] = 0x58, l = 2;
        else
            ed->blocksets[8] = 0x5a, l = 0;
        
        // more blockset crap... blah blah.
        ed->blocksets[9] = ed->blocksets[8] + 1;
        ed->blocksets[10] = (j & 0x40) ? 126 : 116;
        
        // If we be in da dark world...
        if(j >= 0x40)
        {
            ed->sprgfx[2] = ed->sprgfx[1] = ed->sprgfx[0] = rom[0x7ac1 + j];
            ed->sprpal[2] = ed->sprpal[1] = ed->sprpal[0] = rom[0x7bc1 + j];
        }
        else
        {
            ed->sprgfx[0] = rom[0x7a41 + j];
            ed->sprgfx[1] = rom[0x7a81 + j];
            ed->sprgfx[2] = rom[0x7ac1 + j];
            ed->sprpal[0] = rom[0x7b41 + j];
            ed->sprpal[1] = rom[0x7b81 + j];
            ed->sprpal[2] = rom[0x7bc1 + j];
        }
        
        k = 0x5b57 + (ed->sprgfx[1] << 2);
        
        for(i = 0; i < 4; i++)
            ed->blocksets[i + 11] = rom[k + i] + 0x73;
        
        for(i = 0; i < 15; i++)
            Getblocks(ed->ew.doc, ed->blocksets[i]);
        
        Getblocks(ed->ew.doc, 0x79);
        Getblocks(ed->ew.doc, 0x7a);
        
        if( (j & 0x40) || j == 0x96)
            l++;
        
        if(j == 0x88 || j == 0x93)
            l = 4;
        
        Loadpal(ed, rom, 0x1be6c8 + (( (unsigned short*) (rom + 0xdec3b))[l]), 0x21, 7, 5);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd290 : 0x1bd218, 0x91, 15, 4);
        Loadpal(ed, rom, 0x1bd660, 0, 16, 2);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd3c8 : 0x1bd3ac, 0x81, 7, 1);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd4c4 : 0x1bd4a8, 0x89, 7, 1);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd4b6 : 0x1bd49a, 0xe9, 7, 1);
        
        m = ed->sprpal[1];
        
        b2 = rom + 0x7d580 + (m << 1);
        
        Loadpal(ed, rom, 0x1bd4e0 + (( (unsigned short*) (rom + 0xdebd6) )[b2[0]]), 0xd1, 7, 1);
        Loadpal(ed, rom, 0x1bd4e0 + (( (unsigned short*) (rom + 0xdebd6) )[b2[1]]), 0xe1, 7, 1);
        Loadpal(ed, rom, 0x1bd308, 0xf1, 15, 1);
        Loadpal(ed, rom, 0x1bd648, 0xdc, 4, 1);
        Loadpal(ed, rom, 0x1bd630, 0xd9, 3, 1);
        
        if(j < 0x88)
            m = rom[0x7d1c + j];
        // special case palette #s.
        else if(j == 0x95)
            m = 7;
        else if(j == 0x96)
            m = 18;
        else if(j == 0x9d)
            m = 6;
        else if(j == 0x97 || j == 0x9e)
            m = 7;
        else if(j == 0x9c)
            m = 23;
        else
            m = 0;
        
        b2 = rom + 0x75504 + (m << 2);
        
        // Set the palette number, visible in the dialog's edit box.
        SetDlgItemInt(win, 3010, m, 0);
        
        if(b2[0] < 128)
            Loadpal(ed,rom,0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[0]]), 0x29, 7, 3);
        
        if(b2[1] < 128)
            Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[1]]), 0x59, 7, 3);
        
        if(b2[2] < 128)
            Loadpal(ed, rom, 0x1be604 + (((unsigned char*)(rom + 0xdebc6))[b2[2]]), 0x71, 7, 1);
        
        // \task SePH asked about this part where the backdrop is handled.
        // See if it can be fixed.
        if(j & 0x40)
            ed->pal[0] = darkcolor;
        else
            ed->pal[0] = greencolor;
        
        if(j == 0x5b || j == 0x88)
            ed->pal[0] = blackcolor;
        
        m = j & 0x3f;
        
        if(m == 3 || m == 5 || m == 7 || j == 0x95)
            ed->pal[0] = deathcolor;
        
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        ov = ed->ew.doc->overworld + j;
        ov->buf = malloc(0xa00);
        ov->modf = 0;
        
        ed->ov = ov;
        
        if(ed->mapsize)
            l = 4;
        else
            l = 1;
        
        for(k = 0; k < l; k++)
        {
            loadovermap(ov->buf + map_ofs[k], j + map_ind[k], 0, rom);
        }
        
        loadovermap(ov->buf + 1024, getbgmap(ed, j, 1), 1, rom);
        
        if(j < 0x80)
        {
            ed->ovlenb = loadoverovl(ed->ew.doc,ed->ovlmap,j);
            
            if(ed->ew.doc->o_loaded == 2)
                EnableWindow(GetDlgItem(win, 3037), 0);
        }
        
        ed->ovlmodf = 0;
        memcpy(ed->undobuf, ed->ov->buf, 0x800);
        
        ed->undomodf = 0;
        ed->bmih = zbmih;
        ed->sel_scroll = 0;
        ed->sel_select = 0;
        ed->selblk = 0;
        ed->mapscrollh = 0;
        ed->mapscrollv = 0;
        ed->selflag = 0;
        ed->selobj = -1;
        ed->anim = 0;
        ed->disp = 4;
        ed->schflag = 0;
        
        for(i = 0; i < 0x3aa; i++)
            ed->selsch[i] = 0;
        
        for( ; i < 0x57f; i++)
            ed->selsch[i] = 0xff;
        
        for(i = 3000; i < 3002; i++)
        {
            hc = GetDlgItem(win, i);
            
            SetWindowLong(hc, GWL_USERDATA, (int) ed);
            ShowWindow(hc, SW_SHOW);
            Updatesize(hc);
        }
        
        ed->bs.ed = ed;
        ed->bs.scroll = 0;
        ed->bs.sel = 0;
        
        hc = GetDlgItem(win, 3038);
        SetWindowLong(hc,GWL_USERDATA,(int)&(ed->bs));
        Updatesize(hc);
        
        CheckDlgButton(win,3003,BST_CHECKED);
        CheckDlgButton(win,3019,BST_CHECKED);
        ed->tool = 1;
        ed->dtool = 0;
        ed->sprset = 1;
        
        hc = GetDlgItem(win, 3020);
        
        if(j < 0x90)
        {
            ed->e_modf[0] = 0;
            ed->e_modf[1] = 0;
            ed->e_modf[2] = 0;
            ed->ecopy[0] = -1;
            ed->ecopy[1] = -1;
            ed->ecopy[2] = -1;
            l = j >> 7;
            
            for(i = l; i < 3; i++)
            {
                o[i] = *(short*)(rom + sprset_loc[i] + j*2);
                
                for(k = l; k < i; k++)
                {
                    if(o[i] == o[k])
                    {
                        ed->ebuf[i] = ed->ebuf[k];
                        ed->esize[i] = ed->esize[k];
                        ed->ecopy[i] = k;
                        
                        if(i == 1)
                            ed->sprset = 0;
                    }
                }
                
                b2 = rom + 0x50000 + o[i];
                
                for(k = 0; ; k += 3)
                {
                    if(b2[k] == 0xff)
                        break;
                }
                
                ed->esize[i] = k;
                ed->ebuf[i] = malloc(k);
                memcpy(ed->ebuf[i], b2, k);
                
                SendMessage(hc,CB_ADDSTRING,0,(long)(sprset_str[i]));
            }
            
            SendMessage(hc, CB_SETCURSEL, ed->sprset - l, 0);
            
            if(j < 0x80)
            {
                b2 = rom + 0xE0000 + *(short*)(rom + 0xDC2F9 + j*2);
                
                for(k = 0; ; k += 3)
                {
                    if(b2[k] == 0xff)
                        break;
                }
                
                ed->ssize = k;
                ed->sbuf = malloc(k);
                memcpy(ed->sbuf, b2, k);
            }
        }
        else
        {
            ShowWindow(hc,SW_HIDE);
            ShowWindow(GetDlgItem(win,3016),SW_HIDE);
        }
        
        if(j >= 0x80)
        {
            ShowWindow(GetDlgItem(win,3027),SW_HIDE);
            ShowWindow(GetDlgItem(win,3037),SW_HIDE);
        }
        
        EnableWindow(GetDlgItem(win,3018),j<0x80);
        EnableWindow(GetDlgItem(win,3034),j<0x80);
        
        SetDlgItemInt(win,3018,ed->sprgfx[ed->sprset],0);
        SetDlgItemInt(win,3034,ed->sprpal[ed->sprset],0);
        
        Addgraphwin((DUNGEDIT*)ed,1);
        
        break;
    
    case 4002:
        
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        
        break;
    
    case 4000:
        
        SetDlgItemInt(win,3005,wparam,0);
        
        break;
    
    case WM_COMMAND:
        
        ed = (OVEREDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed)
            break;
        
        switch(wparam)
        {
        case 3002:
            
            Overtoolchg(ed, 0, win);
            
            break;
        
        case 3003:
            
            Overtoolchg(ed, 1, win);
            
            break;
        
        case 3004:
            
            Overtoolchg(ed, 2, win);
            
            break;
        
        case 3005 | (EN_CHANGE << 16):
            
            if(ed->disp & 8)
            {
                i = GetDlgItemInt(win, 3005, 0, 0);
                
                if(i != ed->bs.sel)
                {
                    ed->selblk = i;
                    
                    SetBS16(&(ed->bs), i, GetDlgItem(win, 3038));
                }
            }
            else
            {
                i = GetDlgItemInt(win, 3005, 0, 0);
                
                if(i < 0)
                    SetDlgItemInt(win, 3005, 0, 0);
                else if(i > 0x22a7)
                    SetDlgItemInt(win, 3005, 0x22a7, 0);
                else
                    Changeselect(GetDlgItem(win,3000),GetDlgItemInt(win,3005,0,0));
            }
            
            break;
        
        case 3007 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            m = GetDlgItemInt(win, 3007, 0, 0);
            
            if(ed->gfxnum != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, 3007, 79, 0);
                    break;
                }
                
                rom[0x7c9c + ed->ew.param] = m;
                
                ed->gfxnum = m;
                
                l = 0x5d97 + (m << 2);
                k = 0x6073 + (ed->gfxtmp << 3);
                
                for(i = 3; i < 7; i++)
                {
                    Releaseblks(ed->ew.doc, ed->blocksets[i]);
                    
                    m = rom[l++];
                    
                    if(m == 0)
                        m = rom[k + i];
                    
                    ed->blocksets[i] = m;
                    
                    Getblocks(ed->ew.doc,m);
                }
updscrn:
                InvalidateRect(GetDlgItem(win,3000),0,0);
updmap:
                InvalidateRect(GetDlgItem(win,3001),0,0);
            }
            
            break;
        
        case 3008:
            
            ed->anim++;
            
            if(ed->anim == 3)
                ed->anim = 0;
            
            wsprintf(buffer,"Frame %d",ed->anim+1);
            SetWindowText((HWND)lparam,buffer);
            
            goto updscrn;
        
        case 3013:
            
            Overtoolchg(ed,3,win);
            
            break;
        
        case 3011:
            ed->disp &= -2;
            ed->disp |= IsDlgButtonChecked(win, 3011);
            
            goto updmap;
        
        case SD_OverGrid32CheckBox:
            
            ed->disp &= -3;
            ed->disp |= IsDlgButtonChecked(win, SD_OverGrid32CheckBox) << 1;
            
            goto updscrn;
        
        case 3019:
            
            ed->disp &= -5;
            ed->disp |= IsDlgButtonChecked(win, 3019) << 2;
            
            goto updscrn;
        
        case 3037:
            
            ed->selblk = 0;
            ed->disp &= -9;
            ed->disp |= IsDlgButtonChecked(win, 3037) << 3;
            
            ShowWindow(GetDlgItem(win,3000),(ed->disp&8)?SW_HIDE:SW_SHOW);
            ShowWindow(GetDlgItem(win,3038),(ed->disp&8)?SW_SHOW:SW_HIDE);
            
            goto updmap;
        
        case 3010 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            b2 = rom + 0x75504 + ((rom[0x7d1c + ed->ew.param] = GetDlgItemInt(win, 3010, 0, 0)) << 2);
            
            if(b2[0] < 128)
                Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[0]]), 0x29, 7, 3);
            
            if(b2[1] < 128)
                Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[1]]), 0x59, 7, 3);
            
            if(b2[2] < 128)
                Loadpal(ed, rom, 0x1be604 + (((unsigned char*)(rom + 0xdebc6))[b2[2]]), 0x71, 7, 1);
            
            Updpal(ed);
            
            break;
        
        case 3014:
            
            if(ed->selflag)
            {
                free(ed->selbuf);
                ed->selflag = 0;
                
                goto updmap;
            }
            
            b4 = (uint16_t*) malloc(0x800);
            
            memcpy(b4,ed->ov->buf,0x800);
            memcpy(ed->ov->buf,ed->undobuf,0x800);
            memcpy(ed->undobuf,b4,0x800);
            
            free(b4);
            
            i = ed->undomodf;
            ed->undomodf = ed->ov->modf;
            ed->ov->modf = i;
            
            goto updmap;
        
        case 3015:
            
            Overtoolchg(ed,4,win);
            
            break;
        
        case 3016:
            
            Overtoolchg(ed, 5, win);
            
            break;
        
        case 3018 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            m = GetDlgItemInt(win, 3018, 0, 0);
            
            if(ed->ew.param>=0x40)
                n = 2;
            else
                n = ed->sprset;
            
            if(ed->sprgfx[n] != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, 3018, 79, 0);
                    break;
                }
                
                ed->ov->modf = 1;
                ed->sprgfx[n] = m;
                
updsprgfx:
                
                rom = ed->ew.doc->rom;
                k = 0x5b4c + (ed->sprgfx[ed->sprset] << 2);
                
                for(i = 11; i < 15; i++)
                {
                    m = rom[k + i] + 0x73;
                    
                    Releaseblks(ed->ew.doc, ed->blocksets[i]);
                    ed->blocksets[i] = m;
                    
                    Getblocks(ed->ew.doc,m);
                }
                
                m = ed->sprpal[n];
                
                goto updsprpal;
            }
            
            break;
        
        case 3034 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            m = GetDlgItemInt(win, 3034, 0, 0);
            
            if(ed->ew.param >= 0x40)
                n = 2;
            else
                n = ed->sprset;
            
            if(ed->sprpal[n] != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, 3034, 79, 0);
                    break;
                }
                
                ed->ov->modf = 1;
                ed->sprpal[n] = m;
                
updsprpal:
                
                b2 = rom + 0x7d580 + (m << 1);
                
                Loadpal(ed, rom, 0x1bd4e0 + (((unsigned short*)(rom + 0xdebd6))[b2[0]]), 0xd1, 7, 1);
                Loadpal(ed, rom, 0x1bd4e0 + (((unsigned short*)(rom + 0xdebd6))[b2[1]]), 0xe1, 7, 1);
                
                goto updscrn;
            }
            
            break;
        
        case 3020 | (CBN_SELCHANGE << 16):
            
            if((n = ed->ecopy[ed->sprset]) != -1)
            {
                ed->esize[n] = ed->esize[ed->sprset];
                ed->e_modf[n] = ed->e_modf[ed->sprset];
                ed->ebuf[n] = ed->ebuf[ed->sprset];
            }
            
            n = SendMessage((HWND)lparam,CB_GETCURSEL,0,0)+(ed->ew.param>>7);
            
            if(ed->ecopy[n] != -1)
            {
                wsprintf(buffer, "The sprites are the same as in the %s. Do you want to modify only this set?",sprset_str[ed->ecopy[n]]);
                
                if(MessageBox(framewnd,buffer,"Hyrule Magic",MB_YESNO)==IDYES)
                {
                    ed->esize[n] = ed->esize[ed->ecopy[n]];
                    ed->ebuf[n] = malloc(ed->esize[n]);
                    memcpy(ed->ebuf[n], ed->ebuf[ed->ecopy[n]], ed->esize[n]);
                    ed->ecopy[n] = -1;
                    ed->e_modf[n]=1;
                    ed->ov->modf=1;
                }
            }
            
            ed->sprset = n;
            
            if((m = ed->ecopy[n]) != -1)
            {
                ed->esize[n] = ed->esize[m];
                ed->e_modf[n] = ed->e_modf[m];
                ed->ebuf[n] = ed->ebuf[m];
            }
            
            if(ed->tool == 5)
                ed->selobj = -1;
            
            loadovermap(ed->ov->buf + 1024, getbgmap(ed,ed->ew.param,n), 1, ed->ew.doc->rom);
            
            SetDlgItemInt(win,3018,ed->sprgfx[(ed->ew.param>=0x40)?2:n],0);
            SetDlgItemInt(win,3034,ed->sprpal[(ed->ew.param>=0x40)?2:n],0);
            
            m = ed->sprgfx[n];
            
            goto updsprgfx;
        
        case 3021:
            
            if(ed->selflag)
            {
                if(copybuf)
                    free(copybuf);
                
                copy_w = ed->rectright - ed->rectleft;
                copy_h = ed->rectbot-ed->recttop;
                copybuf = malloc(copy_w * copy_h << 1);
                
                k = ed->rectleft + (ed->recttop << 5);
                l = 0;
                b4 = ed->ov->buf;
                
                for(j = 0; j < copy_h; j++)
                {
                    for(i = 0; i < copy_w; i++)
                        copybuf[l++] = b4[i + k];
                    
                    k += 32;
                }
            }
            else
                MessageBox(framewnd,"Nothing is selected","Bad error happened",MB_OK);
            
            break;
        
        case 3022:
            
            Overtoolchg(ed,6,win);
            
            break;
        
        case 3023:
            
            Overtoolchg(ed,7,win);
            
            break;
        
        case 3024:
            
            oved = ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG11,framewnd,editovprop,0);
            
            break;
        
        case 3025:
            
            Overtoolchg(ed,8,win);
            
            break;
        
        case 3026:
            
            Overtoolchg(ed,9,win);
            
            break;
        
        case 3027:
            
            Overtoolchg(ed,10,win);
            
            break;
        
        case 3028:
            ov = ed->ew.doc->overworld;
            j = ed->ew.param^0x40;
            
            if(j >= 0x80)
                break;
            
            goto overlaunch;
        
        case 3029:
        case 3030:
        case 3031:
        case 3032:
            
            ov = ed->ew.doc->overworld;
            j = ed->ew.param;
            
            if(j >= 0x80)
                break;
            
overlaunch:
            
            rom = ed->ew.doc->rom;
            wparam = (wparam - 3029) ^ 1;
            
            if(ed->mapsize)
                if(wparam == 0)
                    j++;
                else if(wparam == 2)
                    j += 8;
            
            if(wparam < 10)
            {
                j += rom[0x12834 + (wparam << 1)] >> 1;
            }
            
            j = ed->ew.doc->rom[0x125ec + (j & 0x3f)] | (j & 0x40);
            
            if(ov[j].win)
            {
                hc = ov[j].win;
                SendMessage(clientwnd,WM_MDIACTIVATE,(int)hc,0);
                
                oed = (OVEREDIT*) GetWindowLong(hc, GWL_USERDATA);
            }
            else
            {
                wsprintf(buffer, "Area %02X", j);
                ov[j].win = Editwin(ed->ew.doc,"ZEOVER",buffer,j,sizeof(OVEREDIT));
            }
            
            break;
        
        case 3035:
            
            if(ed->schflag)
            {
                SetWindowText((HWND)lparam,"Search");
                ShowWindow(GetDlgItem(win,3036),SW_HIDE);
                
                ed->schflag=0;
                free(ed->schbuf);
                
                goto updsel32;
            }
            else
            {
                
search2:
                
                if(GetKeyState(VK_CONTROL) & 128)
                {
                    // \task Test this out. What does it do?
                    ed->schbuf = (uint16_t*) malloc(0x4550);
                    SetCursor(wait_cursor);
                    rom=ed->ew.doc->rom;
                    ed->schflag=1;
                    ov=ed->ew.doc->overworld;
                    
                    b2 = (uint8_t*) malloc(0x455);
                    
                    memset(b2,0,0x455);
                    
                    for(i = 0; i < 160; i++, ov++)
                    {
                        if(ov->win)
                        {
                            b5 = ov->buf;
                            
                            for(j = 0; j < 1008; j++)
                            {
                                if(j & 16)
                                    j += 16;
                                
                                l = (unsigned short) b5[j];
                                
                                if(l < 0x22a8)
                                    b2[l >> 3] |= 1 << (l & 7);
                            }
                        }
                        else
                        {
                            // Allocate a buffer large enough for a 512x512
                            // pixel map of map32 tiles.
                            
                            b5 = (uint16_t*) malloc(512);
                            
                            loadovermap(b5,i,1,rom);
                            for(j=0;j<512;j++)
                                if((l=(unsigned short)b5[j])<0x22a8) b2[l>>3]|=1<<(l&7);
                            free(b5);
                        }
                    }
                    
                    for(i = k = l = 0, j = 1; i < 0x22a8; i++)
                    {
                        if(!(b2[l] & j))
                            ed->schbuf[k++] = i;
                        
                        if(j == 128)
                            j = 1, l++;
                        else j <<= 1;
                    }
                    
                    free(b2);
                    
                    goto schok;
                }
                
                i = ShowDialog(hinstance,
                               (LPSTR) IDD_DIALOG19,
                               framewnd,
                               findblks,
                               (int) ed);
                
                if(i != 0)
                {
                    ed->schflag = 1;
                    ed->schbuf = malloc(0x4550);
                    SetCursor(wait_cursor);
                    
                    k = 0;
                    rom = ed->ew.doc->rom;
                    p = 0;
                    
                    for(l = 0; l < 4; l++)
                        if(ed->schyes[l] != -1)
                            p |= 1 << l;
                    
                    for(i = 0; i < 0x22a8; i++)
                    {
                        Getblock32(rom, i, o);
                        n = 0;
                        q = 0;
                        
                        for(j = 0; j < 4; j++)
                        {
                            m = o[j] + 0xea8;
                            
                            if(ed->selsch[m >> 3] & (1 << (m & 7)))
                                goto contsch;
                            
                            if(ed->selsch[ (m + 0xea8) >> 3 ] & (1 << (m & 7)))
                                q = 1;
                            
                            for(l = 0; l < 4; l++)
                            {
                                m = ed->schyes[l];
                                
                                if(m == -1)
                                    break;
                                if(m == o[j])
                                    n |= 1 << l;
                            }
                        }
                        
                        if(n == p && q)
                            ed->schbuf[k++] = i;
contsch:;
                    }
schok:
                    
                    if(wparam == 3036)
                    {
                        j = m = 0;
                        
                        for(i = 0; i < k; i++)
                        {
                            l = ed->schbuf[i];
                            
                            while(b4[j] < l)
                            {
                                if(j == ed->schnum)
                                    goto sc2done;
                                
                                j++;
                            }
                            
                            if(b4[j] == l)
                                ed->schbuf[m++] = l;
                        }
sc2done:
                        k = m;
                        
                        free(b4);
                    }
                    
                    ed->schnum = k;
                    SetCursor(normal_cursor);
                    
                    if(wparam == 3035)
                    {
                        SetWindowText((HWND)lparam, "Show all");
                        ShowWindow(GetDlgItem(win,3036), SW_SHOW);
                    }
updsel32:
                    hc = GetDlgItem(win, 3000);
                    InvalidateRect(hc, 0, 1);
                    Updatesize(hc);
                }
            }
            
            break;
        
        case 3036:
            
            b4 = ed->schbuf;
            goto search2;
        }
        
        break;
        
    case WM_DESTROY:
        
        ed = (OVEREDIT*) GetWindowLong(win, DWL_USER);
        
        Delgraphwin((DUNGEDIT*) ed);
        
        free(ed->ov->buf);
        
        for(i = 0; i < 15; i++)
            Releaseblks(ed->ew.doc, ed->blocksets[i]);
        
        Releaseblks(ed->ew.doc, 0x79);
        Releaseblks(ed->ew.doc, 0x7a);
        
        ed->ov->win = 0;
        
        free(ed);
        
        break;
    }
    
    return FALSE;
}

BOOL CALLBACK trackdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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
    {"BUTTON","Overlay",328,48,70,20,3037,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Exit",0,72,60,20,3023,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Properties",256,0,60,20,3024,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Hole",128,72,60,20,3025,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Transport",192,72,60,20,3026,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Item",256,72,60,20,3027,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","",400,0,20,20,3029,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",425,0,20,20,3030,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",450,0,20,20,3031,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",475,0,20,20,3032,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",329,24,70,80,3020,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"EDIT","0",56,0,0,20,3005,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,3},
    {"STATIC","GFX#:",0,24,40,20,3006,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",50,24,30,20,3007,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Palette:",90,24,40,20,3009,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",130,24,30,20,3010,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr GFX#:",165,24,55,20,3017,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",220,24,30,20,3018,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr pal:",255,24,45,20,3033,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",300,24,25,20,3034,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Search",56,20,0,20,3035,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BUTTON","Search2",56,42,0,20,3036,WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BLKSEL16","",152,70,0,0,3038,WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,11},
    
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
    {"EDIT","",54,72,30,52, ID_DungEntrRoomNumber, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Y:",4,48,15,28, ID_DungStatic2, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",29,48,40,28, ID_DungEntrYCoord, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","X:",4,24,15,4, ID_DungStatic3, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",29,24,40,4, ID_DungEntrXCoord, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","Y scroll:",79,48,40,28, ID_DungStatic4, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",129,48,40,28, ID_DungEntrYScroll, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","X scroll:",79,24,40,4, ID_DungStatic5, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",129,24,40,4, ID_DungEntrXScroll, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","CX:",173,24,20,4, ID_DungStatic17, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",193,24,40,4, ID_DungEntrCameraX, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"STATIC","CY:",239,24,20,4, ID_DungStatic18, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",259,24,40,4, ID_DungEntrCameraY, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
    {"BUTTON","More",301,24,36,4, ID_DungEntrProps, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"STATIC","Blockset:",104,72,45,52, ID_DungStatic8, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,12},
    {"EDIT","",154,72,40,52, ID_DungEntrTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,12},
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
    {"EDIT","",264,0,30,20, ID_DungFloor1, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Floor 2:",204,24,55,20, ID_DungStatic7, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",264,24,30,20, ID_DungFloor2, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Blockset:",298,0,55,20, ID_DungStatic13, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",360,0,30,20, ID_DungTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","EnemyBlk:",395,0,55,20, ID_DungStatic16, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",450,0,30,20, ID_DungSprTileSet, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Palette:",298,24,55,20, ID_DungStatic14, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",360,24,30,20, ID_DungPalette, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Collision:",395,24,50,20, ID_DungStatic15, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",450,24,90,120, ID_DungCollSettings, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"BUTTON","Exit",550,24,40,24, ID_DungExit, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"STATIC","Layout:",0,24,40,20, ID_DungStatic11, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","More",490,0,30,24, ID_DungEditHeader, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",40,24,30,20, ID_DungLayout, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
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
    {"STATIC","Edit:",0,48,60,20,3000,WS_VISIBLE|WS_CHILD,0,0},
    {"EDIT","",64,48,40,20,3001,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"SAMPEDIT","",0,96,0,0,3002,WS_VISIBLE|WS_CHILD|WS_HSCROLL|WS_TABSTOP,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Copy of:",0,72,60,20,3003,WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,0,0},
    {"EDIT","",64,72,40,20,3004,WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Copy",110,48,50,20,3005,WS_VISIBLE|WS_CHILD,0,0},
    {"BUTTON","Paste",110,72,50,20,3006,WS_VISIBLE|WS_CHILD,0,0},
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

BOOL CALLBACK sampdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SAMPEDIT*ed;
    HWND hc;
    HGLOBAL hgl;
    char*b,*dat;
    int i = 0, j = 0, k = 0;
    
    ZWAVE*zw,*zw2;
    ZINST*zi;
    WAVEFORMATEX*wfx;
    
    const static int wavehdr[] =
    {
        0x46464952,0,0x45564157,0x20746d66,16,0x10001,
        11025,22050,0x100002,0x61746164
    };
    
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        ed=(SAMPEDIT*)lparam;
        ed->dlg=win;
        if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
        ed->init=1;
        ed->editsamp=0;
        ed->editinst=0;
        ed->zoom=65536;
        ed->scroll=0;
        ed->flag=0;
        SetDlgItemInt(win,3001,0,0);
        SetWindowLong(GetDlgItem(win,3002),GWL_USERDATA,(int)ed);
        SetDlgItemInt(win,3012,0,0);
        Loadeditinst(win,ed);
chgsamp:
        i=ed->editsamp;
        zw=ed->ew.doc->waves+i;
        ed->zw=zw;
updcopy:
        if(zw->copy!=-1) {
            CheckDlgButton(win,3003,BST_CHECKED);
            ShowWindow(GetDlgItem(win,3004),SW_SHOW);
            SetDlgItemInt(win,3004,zw->copy,0);
            EnableWindow(GetDlgItem(win,3006),0);
            EnableWindow(GetDlgItem(win,3008),0);
        } else {
            CheckDlgButton(win,3003,BST_UNCHECKED);
            ShowWindow(GetDlgItem(win,3004),SW_HIDE);
            EnableWindow(GetDlgItem(win,3006),1);
            EnableWindow(GetDlgItem(win,3008),1);
        }
        SetDlgItemInt(win,3008,zw->end,0);
        SetDlgItemInt(win,3010,zw->lopst,0);
        if(zw->lflag)
        {
            CheckDlgButton(win,3009,BST_CHECKED);
            ShowWindow(GetDlgItem(win,3010),SW_SHOW);
        }
        else
        {
            CheckDlgButton(win,3009,BST_UNCHECKED);
            ShowWindow(GetDlgItem(win,3010),SW_HIDE);
        }
        ed->sell=0;
        ed->selr=0;
upddisp:
        ed->init=0;
        if(ed->sell>=zw->end) ed->sell=ed->selr=0;
        if(ed->selr>=zw->end) ed->selr=zw->end;
        hc=GetDlgItem(win,3002);
        Updatesize(hc);
        InvalidateRect(hc,0,1);
        break;
    case WM_COMMAND:
        
        ed=(SAMPEDIT*)GetWindowLong(win,DWL_USER);
        
        if(ed->init)
            break;
        
        switch(wparam)
        {
        
        case 3003:
            zw=ed->zw;
            if(zw->copy!=-1) {
                zw->copy=-1;
                zw->end=zw->lflag=0;
                
                zw->buf = (short*) calloc(1, sizeof(short));
                
                ShowWindow(GetDlgItem(win,3004),SW_HIDE);
            }
            else
            {
                j=ed->ew.doc->numwave;
                zw2=ed->ew.doc->waves;
                for(i=0;i<j;i++) {
                    if(zw2->copy==-1 && i!=ed->editsamp) goto chgcopy;
                    zw2++;
                }
                CheckDlgButton(win,3003,BST_UNCHECKED);
chgcopy:
                zw->copy=i;
                free(zw->buf);
                zw->end=zw2->end;
                zw->lopst=zw2->lopst;
                
                zw->buf = (int16_t*) calloc(zw->end + 1, sizeof(int16_t));
                
                i = (zw->end + 1) * sizeof(int16_t);
                
                memcpy(zw->buf, zw2->buf, i);
            }
            
            goto updcopy;
        
        case 3005:
            
            zw = ed->zw;
            
            i = (ed->selr - ed->sell) << 1;
            j = ed->sell;
            
            if(i == 0)
                i = zw->end << 1, j = 0;
            
            hgl=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,44+i);
            b=GlobalLock(hgl);
            memcpy(b,wavehdr,40);
            *(int*)(b+4)=36+i;
            *(int*)(b+40)=i;
            memcpy(b+44,zw->buf+j,i);
            GlobalUnlock(hgl);
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_WAVE,hgl);
            CloseClipboard();
            break;
        case 3006:
            OpenClipboard(0);
            hgl=GetClipboardData(CF_WAVE);
            if(!hgl) {
                MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
                CloseClipboard();
                break;
            }
            b=GlobalLock(hgl);
            if((*(int*)b!=0x46464952) || *(int*)(b+8)!=0x45564157) {
error:
                MessageBox(framewnd,"This is not a wave.","Bad error happened",MB_OK);
noclip:
                GlobalUnlock(hgl);
                CloseClipboard();
                break;
            }
            
            j = *(int*) (b + 4);
            b += 8;
            dat = 0;
            wfx = 0;
            
            for(i = 4; i < j;)
            {
                switch(*(int*)(b+i))
                {
                
                case 0x20746d66:
                    wfx = (WAVEFORMATEX*) (b + i + 8);
                    
                    break;
                
                case 0x61746164:
                    dat = b + i + 8;
                    
                    k = *(int*)(b + i + 4);
                    
                    if(wfx)
                        goto foundall;
                    
                    break;
                }
                
                i += 8 + *(int*) ( b + i + 4);
            }
            if((!wfx)||!dat) goto error;
foundall:
            if(wfx->wFormatTag!=1) {
                MessageBox(framewnd,"The wave is not PCM","Bad error happened",MB_OK);
                goto noclip;
            }
            if(wfx->nChannels!=1) {
                MessageBox(framewnd,"Only mono is allowed.","Bad error happened",MB_OK);
                goto noclip;
            }
            
            if(wfx->wBitsPerSample == 16)
                k >>= 1;
            
            // \task Fixing a bug around here when you paste beyond the range
            // of the current sample.
            zw = ed->zw;
            
            zw->end += k - (ed->selr - ed->sell);
            
            if(k > (ed->selr - ed->sell) )
            {
                // Resize the sample buffer if the size of the data being
                // pasted in exceeds the size of the currently selected
                // region.
                zw->buf = (short*) realloc(zw->buf, (zw->end + 1) << 1);
            }
            
            // Move the part of the sample that is to the right of the selection
            // region in such a way that there is just enough room to copy
            // the new data in.
            memmove(zw->buf + ed->sell + k,
                    zw->buf + ed->selr,
                    (zw->end - k - ed->sell) << 1);
            
            if(k < (ed->selr - ed->sell) )
            {
                // Shrink the sample buffer if the pasted in data is smaller
                // than the selection region.
                zw->buf = (short*) realloc(zw->buf, ( (zw->end + 1) << 1 ) );
            }
            
            if(zw->lopst >= ed->selr)
                zw->lopst += ed->sell + k - ed->selr;
            
            if(zw->lopst >= zw->end)
                zw->lflag = 0, zw->lopst = 0;
            
            if(zw->lflag)
                zw->buf[zw->end] = zw->buf[zw->lopst];
            
            ed->selr = ed->sell + k;
            
            if(wfx->wBitsPerSample == 16)
                memcpy(zw->buf+ed->sell,dat,k<<1);
            else
            {
                j = ed->sell;
                
                for(i = 0; i < k; i++)
                    zw->buf[j++] = ( (dat[i] - 128) << 8 );
            }
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            GlobalUnlock(hgl);
            CloseClipboard();
            Modifywaves(ed->ew.doc,ed->editsamp);
            ed->init=1;
            if(!zw->lflag) {
                CheckDlgButton(win,3009,BST_UNCHECKED);
                ShowWindow(GetDlgItem(win,3010),SW_HIDE);
            }
            SetDlgItemInt(win,3008,zw->end,0);
            SetDlgItemInt(win,3010,zw->lopst,0);
            goto upddisp;
        case 3009:
            zw=ed->zw;
            if(!zw->lflag) {
                zw->lflag=1;
                ShowWindow(GetDlgItem(win,3010),SW_SHOW);
                zw->buf[zw->end]=zw->buf[zw->lopst];
            } else {
                zw->lflag=0;
                ShowWindow(GetDlgItem(win,3010),SW_HIDE);
            }
            Modifywaves(ed->ew.doc,ed->editsamp);
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            break;
        case 3021:
            if(sounddev>=0x20000) {
                MessageBox(framewnd,"A wave device must be selected","Bad error happened",MB_OK);
                break;
            }
            if(sndinit) Stopsong();
            else Initsound();
            playedsong=0;
            sounddoc=ed->ew.doc;
            globvol=65535;
            zwaves->wnum=ed->editsamp;
            zwaves->vol1=zwaves->vol2=127;
            zwaves->sus=7;
            zwaves->rel=0;
            zwaves->atk=31;
            zwaves->dec=0;
            zwaves->envs=zwaves->envclk=zwaves->envclklo=zwaves->pos=0;
            zwaves->freq=1781;
            zwaves->pflag=1;
            break;
        case 3022:
            if(sndinit) Stopsong();
            break;
        case 0x02000bb9:
            if(ed->flag&1) {
                i=GetDlgItemInt(win,3001,0,0);
                if(i<0) i=0;
                if(i>=ed->ew.doc->numwave) i=ed->ew.doc->numwave-1;
                ed->flag&=-2;
                ed->init=1;
                SetDlgItemInt(win,3001,i,0);
                ed->editsamp=i;
                goto chgsamp;
            }
            break;
        case 0x02000bbc:
            if(ed->flag&2) {
                ed->flag&=-3;
                zw=ed->zw;
                j=zw->copy;
                i=GetDlgItemInt(win,3004,0,0);
                k=zw->end;
                if(i<0) i=0;
                if(i>=ed->ew.doc->numwave) i=ed->ew.doc->numwave-1;
                zw2=ed->ew.doc->waves+i;
                if(zw2->copy!=-1) i=j;
                ed->init=1;
                SetDlgItemInt(win,3004,i,0);
                ed->init=0;
                if(i==j) break;
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                goto chgcopy;
            }
            break;
        case 0x02000bc0:
            if(ed->flag&16) {
                ed->flag&=-17;
                zw=ed->zw;
                i=GetDlgItemInt(win,3008,0,0);
                k=zw->end;
                if(i<0) {
                    i=0;
chglen:
                    SetDlgItemInt(win,3008,i,0);
                    ed->init=0;
                    break;
                }
                if(i>65536) { i=65536; goto chglen; }
                b = realloc(zw->buf, (i + 1) << 1);
                if(!b) {
                    MessageBox(framewnd,"Not enough memory","Bad error happened",MB_OK);
                    i=k;
                    ed->init=1;
                    goto chglen;
                } else zw->buf=(short*)b;
                zw->end=i;
                for(j=k;j<i;j++) zw->buf[j]=0;
                if(zw->lopst!=-1) zw->buf[zw->end]=zw->buf[zw->lopst];
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                goto upddisp;
            }
            break;
        case 0x02000bc2:
            if(ed->flag&32) {
                ed->flag&=-33;
                zw=ed->zw;
                j=i=GetDlgItemInt(win,3010,0,0);
                if(i<0) i=0;
                else if(i>=zw->end) i=zw->end-1;
                if(i!=j) {
                    ed->init=1;
                    SetDlgItemInt(win,3010,i,0);
                    ed->init=0;
                }
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
                zw->lopst=i;
                zw->buf[zw->end]=zw->buf[zw->lopst];
            }
            break;
        case 0x02000bc4:
            if(ed->flag&64) {
                ed->flag&=-65;
                i=GetDlgItemInt(win,3012,0,0);
                if(i<0) i=0;
                else if(i>=ed->ew.doc->numinst) i=ed->ew.doc->numinst-1;
                else goto nochginst;
                ed->init=1;
                SetDlgItemInt(win,3012,i,0);
                ed->init=0;
nochginst:
                ed->editinst=i;
                Loadeditinst(win,ed);
            }
            break;
        case 0x02000bc6:
        case 0x02000bc8:
        case 0x02000bca:
        case 0x02000bcc:
            if(ed->flag&128) {
                ed->flag&=-129;
                zi=ed->ew.doc->insts+ed->editinst;
                i=GetDlgItemInt(win,3014,0,0);
                zi->multlo=i;
                zi->multhi=i>>8;
                GetDlgItemText(win,3016,buffer,5);
                i=strtol(buffer,0,16);
                zi->sr=i;
                zi->ad=i>>8;
                
                GetDlgItemText(win,3018,buffer,3);
                
                zi->gain = (uint8_t) strtol(buffer, 0, 16);
                
                zi->samp=GetDlgItemInt(win,3020,0,0);
                ed->ew.doc->m_modf=1;
                ed->ew.doc->w_modf=1;
            }
            break;
        case 0x03000bb9:
            ed->flag|=1;
            break;
        case 0x03000bbc:
            ed->flag|=2;
            break;
        case 0x03000bc0:
            ed->flag|=16;
            break;
        case 0x03000bc2:
            ed->flag|=32;
            break;
        case 0x03000bc4:
            ed->flag|=64;
            break;
        case 0x03000bc6:
        case 0x03000bc8:
        case 0x03000bca:
        case 0x03000bcc:
            ed->flag|=128;
            break;
        }
    }
    return FALSE;
}

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

long CALLBACK overproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    OVEREDIT * const ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
    
    switch(msg)
    {
    
    deflt:
    default:
        return DefMDIChildProc(win, msg, wparam, lparam);
    
    case WM_CLOSE:
        
        if(ed->selflag) Overselectwrite(ed);
        
        if(ed->ew.doc->modf==2) goto deflt;
        
        if(ed->ov->modf)
        {
            wsprintf(buffer,"Confirm modification of area %02X?",ed->ew.param);
            switch(MessageBox(framewnd,buffer,"Overworld editor",MB_YESNOCANCEL)) {
            case IDYES:
                Savemap(ed);
                goto deflt;
            case IDCANCEL:
                break;
            case IDNO:
                goto deflt;
            }
        }
        else
            goto deflt;
        
        break;
    
    case WM_MDIACTIVATE:
        
        if(ed)
        {
            HM_MdiActivateData d = HM_GetMdiActivateData(wparam, lparam);
            
            if(d.m_activating != win)
            {
                break;
            }
            
            activedoc = ed->ew.doc;
            
            Setdispwin((DUNGEDIT*) ed);
        }
        
        break;
    
    case WM_GETMINMAXINFO:
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        
        if(always)
        {
            CREATESTRUCT const * const cs = (CREATESTRUCT*) lparam;
            
            MDICREATESTRUCT const * const
            mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
            
            OVEREDIT * const new_ed = (OVEREDIT*) (mdi_cs->lParam);
            
            SetWindowLong(win, GWL_USERDATA, (long) new_ed);
            
            ShowWindow(win, SW_SHOW);
            
            new_ed->dlg = CreateSuperDialog(&overdlg,
                                            win,
                                            0,
                                            0,
                                            0,
                                            0,
                                            (long) new_ed);
        }
        
        break;
    }
    
    return 0;
}

LRESULT CALLBACK
sampdispproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SAMPEDIT*ed;
    ZWAVE*zw;
    PAINTSTRUCT ps;
    HDC hdc,oldobj;
    SCROLLINFO si;
    int i,j,k,l;
    RECT rc,rc2;
    switch(msg) {
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    case WM_SIZE:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(!ed) break;
        i=ed->height;
        ed->width=(short)lparam;
        ed->height=lparam>>16;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax=ed->zw->end;
        ed->page=si.nPage=(ed->width<<16)/ed->zoom;
        SetScrollInfo(win,SB_HORZ,&si,1);
        if(i!=ed->height) InvalidateRect(win,0,1);
        break;
    case WM_HSCROLL:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->scroll=Handlescroll(win,wparam,ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
        break;
    
    case WM_KEYDOWN:
        
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case VK_DELETE:
            zw=ed->zw;
            if(zw->copy!=-1) break;
            
            if(ed->sell==ed->selr)
            {
                // \task This logic deletes the whole sample if there is
                // is not a selected region of width greater than zero.
                // Is this really intuitive? Seems like a great way to lose
                // work.
                
                zw->end=0;
                zw->buf=realloc(zw->buf,2);
                zw->buf[0]=0;
                zw->lopst=0;
                ed->sell=0;
            }
            else
            {
                memcpy(zw->buf+ed->sell,zw->buf+ed->selr,zw->end-ed->selr);
                
                zw->end += ed->sell-ed->selr;
                
                zw->buf = realloc(zw->buf, (zw->end + 1) << 1);
                
                if(zw->lopst >= ed->selr)
                    zw->lopst += ed->sell - ed->selr;
                
                zw->buf[zw->end]=zw->buf[zw->lopst];
            }
            ed->selr=ed->sell;
            ed->init=1;
            SetDlgItemInt(ed->dlg,3008,zw->end,0);
            SetDlgItemInt(ed->dlg,3010,zw->lopst,0);
            InvalidateRect(win,0,1);
            ed->init=0;
            ed->ew.doc->m_modf=1;
            ed->ew.doc->w_modf=1;
            break;
        }
        break;
    case WM_CHAR:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        switch(wparam) {
        case 1:
            ed->sell=0;
            ed->selr=ed->zw->end;
            InvalidateRect(win,0,1);
            break;
        case 3:
            sampdlgproc(ed->dlg,WM_COMMAND,3005,0);
            break;
        case 22:
            sampdlgproc(ed->dlg,WM_COMMAND,3006,0);
            break;
        }
        
        break;
    
    case WM_LBUTTONDOWN:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetFocus(win);
        SetCapture(win);
        ed->flag|=4;
        ed->flag&=-9;
        i=((short)lparam+ed->scroll)*ed->zoom>>16;
        GetClientRect(win,&rc);
        if(wparam&MK_SHIFT) {
            if(i<ed->sell) ed->flag|=8;
            goto selectmore;
        }
        Getsampsel(ed,&rc);
        InvalidateRect(win,&rc,1);
        ed->selr=ed->sell=i;
        Getsampsel(ed,&rc);
        InvalidateRect(win,&rc,1);
        break;
    case WM_LBUTTONUP:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->flag&=-5;
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->flag&4) {
            GetClientRect(win,&rc);
selectmore:
            rc2=rc;
            Getsampsel(ed,&rc);
            InvalidateRect(win,&rc,1);
            i=((short)lparam+ed->scroll)*ed->zoom>>16;
            if(i>ed->selr && (ed->flag&8)) {
                ed->flag&=-9;
                ed->sell=ed->selr;
            }
            if(i<ed->sell && !(ed->flag&8)) {
                ed->flag|=8;
                ed->selr=ed->sell;
            }
            if(ed->flag&8) ed->sell=i;
            else ed->selr=i;
            if(ed->sell<0) ed->sell=0;
            if(ed->selr>ed->zw->end) ed->selr=ed->zw->end;
            Getsampsel(ed,&rc);
            InvalidateRect(win,&rc,1);
            if((short)lparam>=rc2.right) ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(((short)lparam+ed->scroll-ed->page)<<16),ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
            if((short)lparam<rc2.left) ed->scroll=Handlescroll(win,SB_THUMBPOSITION|(((short)lparam+ed->scroll)<<16),ed->scroll,ed->page,SB_HORZ,ed->zw->end*ed->zoom>>16,1);
        }
        break;
    case WM_PAINT:
        ed=(SAMPEDIT*)GetWindowLong(win,GWL_USERDATA);
        hdc=BeginPaint(win,&ps);
        zw=ed->ew.doc->waves+ed->editsamp;
        oldobj=SelectObject(hdc,green_pen);
        l=0;
        for(i=ps.rcPaint.left-1;i<=ps.rcPaint.right;i++) {
            j=((i+ed->scroll)*ed->zoom>>16);
            if(j<0) continue;
            if(j>=zw->end) break;
            k=((int)(zw->buf[j])+32768)*ed->height>>16;
            if(!l) MoveToEx(hdc,i,k,0),l=1; else LineTo(hdc,i,k);
        }
        SelectObject(hdc,oldobj);
        Getsampsel(ed,&rc);
        if(rc.left<ps.rcPaint.left) rc.left=ps.rcPaint.left;
        if(rc.right>ps.rcPaint.right) rc.right=ps.rcPaint.right;
        if(rc.left<rc.right) {
            rc.top=ps.rcPaint.top;
            rc.bottom=ps.rcPaint.bottom;
            InvertRect(hdc,&rc);
        }
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

long CALLBACK wmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    WMAPEDIT *ed;
    switch(msg) {
    case WM_CLOSE:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        if(ed->selflag) Wmapselectwrite(ed);
        if(ed->modf) {
            wsprintf(buffer,"Confirm modification of world map %d?",ed->ew.param);
            switch(MessageBox(framewnd,buffer,"World map editor",MB_YESNOCANCEL)) {
            case IDYES:
                Saveworldmap(ed);
                break;
            case IDCANCEL:
                return 1;
            }
        }
        goto deflt;
        break;
    case WM_MDIACTIVATE:
        
        if((HWND)lparam != win)
            break;
        
        ed = ((WMAPEDIT*)GetWindowLong(win,GWL_USERDATA));
        activedoc=ed->ew.doc;
        Setdispwin((DUNGEDIT*)ed);
        
        break;
    
    case WM_GETMINMAXINFO:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(WMAPEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        ed=(WMAPEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        ed->dlg=CreateSuperDialog(&wmapdlg,win,0,0,0,0,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

long CALLBACK lmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

//dungproc#***************************************

// This is a child frame window, but the "view" is the 'ed' window.
long CALLBACK dungproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    DUNGEDIT *ed;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        
        if((HWND)lparam != win)
            break;
        
        ed = ((DUNGEDIT*) GetWindowLong(win,GWL_USERDATA));
        
        activedoc = ed->ew.doc;
        
        Setdispwin(ed);
        
        goto deflt;
    
    case WM_GETMINMAXINFO:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        DefMDIChildProc(win,msg,wparam,lparam);
        
        if(!ed)
            goto deflt;
        
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        SetWindowPos(ed->dlg,
                     0,
                     0,
                     0,
                     LOWORD(lparam),
                     HIWORD(lparam),
                     SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        
        goto deflt;
    
    case WM_CLOSE:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        
        if(!Closeroom(ed))
        {
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            DestroyWindow(ed->dlg);
            Releaseblks(ed->ew.doc,0x79);
            Releaseblks(ed->ew.doc,0x7a);
            
            goto deflt;
        }
        
        break;
    
    case WM_DESTROY:
        
        ed = (DUNGEDIT*) GetWindowLong(win,GWL_USERDATA);
        Delgraphwin(ed);
        free(ed);
        
        break;
    
    case WM_CREATE:
        
        ed = (DUNGEDIT*) (((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        
        ed->ew.doc->ents[ed->ew.param] = win;
        
        if( CreateSuperDialog(&dungdlg,win,0,0,0,0, (LPARAM) ed) == 0)
        {
            MessageBox(win, "adsfasdf", "adfasdfads!!!!!!", MB_YESNOCANCEL);
            
            ed->ew.doc->ents[ed->ew.param] = 0;
            
            return FALSE;
        }
        
        hc = GetDlgItem(ed->dlg, ID_DungEditWindow);
        
        SetWindowLong(hc, GWL_USERDATA, (long) ed);
        
        Updatesize(hc);
        InvalidateRect(hc, 0, 0);
        Dungselectchg(ed, hc, 1);
        
deflt:
        
    default:
        
        return DefMDIChildProc(win, msg, wparam, lparam);
    }
    
    return 0;
}

//dungproc**********************************************

long CALLBACK perspproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
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

long CALLBACK texteditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TEXTEDIT*ed;
    switch(msg) {
    case WM_MDIACTIVATE:
        activedoc=((TEXTEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(TEXTEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(TEXTEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(TEXTEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->t_wnd=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(TEXTEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        CreateSuperDialog(&textdlg,win,0,0,0,0,(long)ed);
deflt:
    default:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

long CALLBACK musbankproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    MUSEDIT *ed;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        
        activedoc=((DUNGEDIT*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        
        goto deflt;
    
    case WM_GETMINMAXINFO:
        
        ed=(MUSEDIT*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        
        if(!ed)
            goto deflt;
        
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    
    case WM_SIZE:
        
        ed=(MUSEDIT*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        
        goto deflt;
    
    case WM_DESTROY:
        
        ed=(MUSEDIT*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->mbanks[ed->ew.param]=0;
        
        if(sndinit && ed->ew.doc == sounddoc && !playedsong)
            zwaves->pflag=0;
        
        free(ed);
        
        break;
    case WM_CREATE:
        
        ed = (MUSEDIT*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        CreateSuperDialog((ed->ew.param==3)?&sampdlg:&musdlg,win,0,0,0,0,(long)ed);
        
deflt:
    default:
        
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

long CALLBACK overmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    SCROLLINFO si;
    
    int i = 0; int j = 0; int k = 0; int l = 0;
    int m = 0; int n = 0; int o = 0; int p = 0;
    int q = 0; int r = 0;
    
    POINT pt;
    PAINTSTRUCT ps;
    HDC hdc;
    HPALETTE oldpal;
    RECT rc = {0, 0, 0, 0};
    HMENU menu,menu2;
    HGDIOBJ oldobj,oldobj2;
    
    const static int whirl_ofs[8]={
        0x16b29,0x16b4b,0x16b6d,0x16b8f,0x16ae5,0x16b07,0x16bb1,0x16bd3
    };
    
    uint16_t * b2 = 0;
    uint16_t * b3 = 0;
    uint16_t * b4 = 0;
    uint16_t * b5 = 0;
    
    unsigned char*b6 = 0,*rom = 0;
    
    OVEREDIT * const ed = (OVEREDIT*) GetWindowLong(win, GWL_USERDATA);
    
    switch(msg)
    {
    
    case WM_SIZE:
        
        if(!ed) break;
        si.cbSize=sizeof(si);
        si.fMask=SIF_RANGE|SIF_PAGE;
        si.nMin=0;
        si.nMax = ( ed->mapsize ? 32 : 16 ) - 1;
        si.nPage=lparam>>21;
        ed->mappagev=si.nPage;
        SetScrollInfo(win,SB_VERT,&si,1);
        si.nPage=(lparam&65535)>>5;
        ed->mappageh=si.nPage;
        SetScrollInfo(win,SB_HORZ,&si,1);
        ed->mapscrollv=Handlescroll(win,-1,ed->mapscrollv,ed->mappagev,SB_VERT,si.nMax,32);
        ed->mapscrollh=Handlescroll(win,-1,ed->mapscrollh,ed->mappageh,SB_HORZ,si.nMax,32);
        
        break;
    
    case WM_MOUSEWHEEL:
        
        {
            HM_MouseWheelData d = HM_GetMouseWheelData(wparam, lparam);
            
            unsigned const is_horiz = (d.m_control_key);
            
            unsigned which_sb = (is_horiz) ? SB_HORZ : SB_VERT;
            
            int const which_scroll = (is_horiz) ? ed->mapscrollh : ed->mapscrollv;
            int const which_page   = (is_horiz) ? ed->mappageh : ed->mappagev;
            
            int * const which_return = (is_horiz) ? &ed->mapscrollh : &ed->mapscrollv;
            
            (*which_return) = Handlescroll(win,
                                           (d.m_distance > 0) ? 0 : 1,
                                           which_scroll,
                                           which_page,
                                           which_sb,
                                           ed->mapsize ? 32 : 16, 32);
        }
        
        break;
    
    case WM_VSCROLL:
        
        ed->mapscrollv = Handlescroll(win,wparam,ed->mapscrollv,ed->mappagev,SB_VERT,ed->mapsize?32:16,32);
        
        break;
    
    case WM_HSCROLL:
        
        ed->mapscrollh = Handlescroll(win,wparam,ed->mapscrollh,ed->mappageh,SB_HORZ,ed->mapsize?32:16,32);
        
        break;
    
    case WM_RBUTTONDOWN:
        
        l=ed->tool;
        if(l==6 || l==4) break;
        if(l>2) {
            menu=CreatePopupMenu();
            switch(l) {
            case 3:
                AppendMenu(menu,MF_STRING,2,"Insert entrance");
                break;
            case 5:
                if(ed->ew.param<0x90)
                AppendMenu(menu,MF_STRING,3,"Insert sprite");
                break;
            case 7:
                AppendMenu(menu,MF_STRING,5,"Insert exit");
                break;
            case 8:
                AppendMenu(menu,MF_STRING,6,"Insert hole");
                break;
            case 9:
                AppendMenu(menu,MF_STRING,7,"Insert bird location");
                AppendMenu(menu,MF_STRING,8,"Insert whirlpool");
                break;
            case 10:
                if(ed->ew.param<0x80) AppendMenu(menu,MF_STRING,4,"Insert item");
                break;
            }
            if(ed->selobj!=-1) AppendMenu(menu,MF_STRING,1,"Remove");
            if(ed->ew.param<0x90 && l==5) {
                j=0,k=1;
                for(i=ed->ew.param>>7;i<3;i++,k<<=1) {
                    if(i==ed->sprset || ed->ecopy[i]!=-1 || i==ed->ecopy[ed->sprset]) continue;
                    j|=k;
                }
                if(j) {
                    k=1;
                    menu2=CreatePopupMenu();
                    for(i=0;i<3;i++) {
                        if(k&j) AppendMenu(menu2,MF_STRING,i+9,sprset_str[i]);
                        k<<=1;
                    }
                    AppendMenu(menu,MF_STRING|MF_POPUP,(int)menu2,"Use same sprites as");
                }
            }
            if(ed->disp&8) AppendMenu(menu,MF_STRING,12,"Clear overlay");
            GetCursorPos(&pt);
            k=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY,pt.x,pt.y,0,win,0);
            DestroyMenu(menu);
            i=(lparam&65535)+(ed->mapscrollh<<5);
            j=(lparam>>16)+(ed->mapscrollv<<5);
            rom=ed->ew.doc->rom;
            switch(k) {
            case 1:
                Overselchg(ed,win);
                switch(l) {
                case 3:
                    ((short*)(rom + 0xdb96f))[ed->selobj]=-1;
                    break;
                case 5:
                    b6=ed->ebuf[ed->sprset];
                    ed->esize[ed->sprset]-=3;
                    memcpy(b6+ed->selobj,b6+ed->selobj+3,ed->esize[ed->sprset]-ed->selobj);
                    ed->ebuf[ed->sprset]=realloc(b6,ed->esize[ed->sprset]);
                    ed->e_modf[ed->sprset]=1;
                    ed->ov->modf=1;
                    break;
                case 7:
                    rom[0x15e28 + ed->selobj]=255;
                    ((short*)(rom + 0x15d8a))[ed->selobj]=-1;
                    break;
                case 8:
                    ((short*)(rom + 0xdb826))[ed->selobj]=-1;
                    break;
                case 9:
                    ((short*)(rom + 0x16ae5))[ed->selobj]=-1;
                    break;
                case 10:
                    ed->ssize-=3;
                    b6=ed->sbuf;
                    memcpy(b6+ed->selobj,b6+ed->selobj+3,ed->ssize-ed->selobj);
                    ed->sbuf=realloc(b6,ed->ssize);
                    ed->ov->modf=1;
                    break;
                }
                ed->selobj=-1;
                break;
            case 2:
                for(m=0;m<129;m++)
                {
                    if( is16b_neg1_i(rom + 0xdb96f, m) )
                    {
                        Overselchg(ed,win);
                        ((short*)(rom + 0xdb96f))[m]=ed->ew.param;
                        ((short*)(rom + 0xdba71))[m]=((j<<3)&0x1f80)+((i>>3)&0x7e);
                        rom[0xdbb73 + m]=0;
                        ed->selobj=m;
                        ed->objx=i&0xfff0;
                        ed->objy=j&0xfff0;
                        Overselchg(ed,win);
                        return 0;
                    }
                }
                MessageBox(framewnd,"Can't add anymore entrances.","Bad error happened",MB_OK);
                break;
            case 3:
                m=ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,0);
                Overselchg(ed,win);
                if(m==-1) break;
                if(ed->selobj==-1) ed->selobj=ed->esize[ed->sprset];
                ed->esize[ed->sprset]+=3;
                b6=ed->ebuf[ed->sprset]=realloc(ed->ebuf[ed->sprset],ed->esize[ed->sprset]);
                memmove(b6+ed->selobj+3,b6+ed->selobj,ed->esize[ed->sprset]-ed->selobj-3);
                b6[ed->selobj]=j>>4;
                b6[ed->selobj+1]=i>>4;
                b6[ed->selobj+2]=m;
                ed->e_modf[ed->sprset]=1;
                ed->ov->modf=1;
                ed->objx=i&0xfff0;
                ed->objy=j&0xfff0;
                Overselchg(ed,win);
                break;
            case 4:
                Overselchg(ed,win);
                b6=ed->sbuf=realloc(ed->sbuf,ed->ssize+=3);
                if(ed->selobj==-1) ed->selobj=ed->ssize-3;
                memmove(b6+ed->selobj+3,b6+ed->selobj,ed->ssize-ed->selobj-3);
                *(short*)(b6+ed->selobj)=((i>>3)&0x7e)+((j<<3)&0x1f80);
                ed->objx=i&0xfff0;
                ed->objy=j&0xfff0;
                b6[ed->selobj+2]=0;
                ed->ov->modf=1;
                Overselchg(ed,win);
                break;
            
            case 5: // add exit command?
                
                for(n = 0; n < 79; n++)
                    if(rom[0x15e28 + n] == 255)
                    {
                        rom[0x15e28 + n] = ed->ew.param;
                        
                        ((short*) (rom + 0x15d8a))[n] = 0;
addexit:
                        
                        l = i - 128;
                        m = j - 112;
                        q = ed->mapsize ? 1024 : 512;
                        
                        if(l < 0)
                            l = 0;
                        
                        if(l > q - 256)
                            l = q - 256;
                        
                        if(m < 0)
                            m = 0;
                        
                        if(m > q - 224)
                            m = q - 224;
                        
                        l += (ed->ew.param & 7) << 9;
                        m += (ed->ew.param & 56) << 6;
                        
                        Overselchg(ed,win);
                        
                        ed->selobj = n;
                        ed->objx = i;
                        ed->objy=j;
                        i += (ed->ew.param & 7) << 9;
                        j += (ed->ew.param & 56) << 6;
                        o = ((l & 0xfff0) >> 3) + ((m & 0xfff0) << 3);
                        
                        if(k == 5)
                        {
                            ((short*)(rom + 0x160ef))[n]=i;
                            ((short*)(rom + 0x16051))[n]=j;
                            ((short*)(rom + 0x15fb3))[n]=l;
                            ((short*)(rom + 0x15f15))[n]=m;
                            ((short*)(rom + 0x1622b))[n]=l+128;
                            ((short*)(rom + 0x1618d))[n]=m+112;
                        }
                    else
                    {
                        ((short*)(rom + 0x16b8f))[n]=i;
                        ((short*)(rom + 0x16b6d))[n]=j;
                        ((short*)(rom + 0x16b4b))[n]=l;
                        ((short*)(rom + 0x16b29))[n]=m;
                        ((short*)(rom + 0x16bd3))[n]=l+128;
                        ((short*)(rom + 0x16bb1))[n]=m+112;
                    }
                    Overselchg(ed,win);
                    return 0;
                }
                MessageBox(framewnd,"You can't add anymore exits.","Bad error happened",MB_OK);
                break;
            case 6:
                
                for(n=0;n<19;n++)
                    if( is16b_neg1_i(rom + 0xdb826, n) )
                    {
                        Overselchg(ed,win);
                        ((short*)(rom + 0xdb826))[n]=ed->ew.param;
                        ((short*)(rom + 0xdb800))[n]=((j<<3)&0x1f80)+((i>>3)&0x7e) - 0x400;
                        rom[0xdb84c + n]=0;
                        ed->selobj=n;
                        ed->objx=i&0xfff0;
                        ed->objy=j&0xfff0;
                        Overselchg(ed,win);
                        return 0;
                    }
                
                MessageBox(framewnd,"You can't add anymore holes.","Bad error happened",MB_OK);
                break;
            case 7:
                
                for(n=0;n<9;n++)
                    if( is16b_neg1_i(rom + 0x16ae5, n) )
                    {
                        ((short*)(rom + 0x16ae5))[n]=ed->ew.param;
                        goto addexit;
                    }
                
                MessageBox(framewnd,"You can't add anymore bird locations.","Bad error happened",MB_OK);
                break;
            case 8:
                
                for(n=9;n<17;n++)
                {
                    if( is16b_neg1_i(rom + 0x16ae5, n) )
                    {
                        ((short*)(rom + 0x16ae5))[n]=ed->ew.param;
                        ((short*)(rom + 0x16ce6))[n]=0;
                        goto addexit;
                    }
                }
                
                MessageBox(framewnd,"You can't add anymore whirlpools.","Bad error happened",MB_OK);
                break;
            case 9: case 10: case 11:
                
                i = ed->sprset;
                
                if(ed->ecopy[i] == -1)
                    free(ed->ebuf[i]);
                
                k-=9;
                ed->ebuf[i]=ed->ebuf[k];
                ed->esize[i]=ed->esize[k];
                ed->e_modf[i]=1;
                if(k>i) { i=k; k=ed->sprset; }
                ed->ecopy[i]=k;
                ed->ecopy[k]=-1;
                InvalidateRect(win,0,0);
            }
        } else {
            if(ed->disp&8) {
                l=(lparam>>20)+(ed->mapscrollv<<1);
                k=((lparam&65535)>>4)+(ed->mapscrollh<<1);
                if(ed->mapsize) q=63; else q=31;
                if(k>q || l>q) break;
                SetDlgItemInt(ed->dlg,3005,ed->ovlmap[(ed->disp&1)?((k&31)+((l&31)<<5) + 0x1000):k+(l<<6)],0);
            } else {
                l=(lparam>>21)+ed->mapscrollv;
                k=((lparam&65535)>>5)+ed->mapscrollh;
                if(ed->mapsize) q=31; else q=15;
                if(k>q || l>q) break;
                SetDlgItemInt(ed->dlg,3005,ed->ov->buf[k+(l<<5)],0);
            }
        }
        break;
    case WM_GETDLGCODE:
        return DLGC_WANTCHARS;
    case WM_CHAR:
        
        if(wparam==26) {overdlgproc(GetParent(win),WM_COMMAND,3014,0);break;}
        i=ed->selobj;
        
        if(i == -1)
            break;
        
        rom=ed->ew.doc->rom;
        switch(ed->tool) {
        case 3:
            j=rom[0xdbb73 + i];
            break;
        case 5:
            m=ed->sprset;
            b6=ed->ebuf[m];
            j=b6[i+2];
            break;
        case 7:
            j=((unsigned short*)(rom + 0x15d8a))[i];
            break;
        case 8:
            j=rom[0xdb84c + i];
            break;
        case 9:
            if(i>8) j=((unsigned short*)(rom + 0x16ce6))[i];
            else j=i;
            break;
        case 10:
            j=ed->sbuf[i+2];
            if(j>=128) j=(j>>1)-41;
            if(j==26) ((short*)(rom + 0x16dc5))[ed->ew.param]=0;
            break;
        default:
            return 0;
        }
        Overselchg(ed,win);
        if(wparam>=64) wparam&=0xdf;
        switch(wparam) {
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
            j<<=4;
            j+=wparam-'0';
            goto updent;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            j<<=4;
            j+=wparam-'A'+10;
            goto updent;
        case 'N':
            k=-1;
            goto updadd;
        case 'M':
            k=1;
            goto updadd;
        case 'J':
            k=-16;
            goto updadd;
        case 'K':
            k=16;
updadd:
            j+=k;
updent:
            switch(ed->tool) {
            case 3:
                rom[0xdbb73 + i]=j;
                ed->ew.doc->modf=1;
                break;
            case 5:
                Overselchg(ed,win);
                b6[i+2]=j;
                ed->ov->modf=1;
                ed->e_modf[ed->sprset]=1;
                break;
            case 7:
                ((unsigned short*)(rom + 0x15d8a))[i]=j;
                ed->ew.doc->modf=1;
                break;
            case 8:
                rom[0xdb84c + i]=j;
                break;
            case 9:
                if(i>8) rom[0x16cec + i*2]=j;
                else {
                    j--;
                    j&=15;
                    if(j>8) break;
                    for(k=0;k<8;k++) {
                        l=((short*)(rom+whirl_ofs[k]))[i];
                        ((short*)(rom+whirl_ofs[k]))[i]=((short*)(rom+whirl_ofs[k]))[j];
                        ((short*)(rom+whirl_ofs[k]))[j]=l;
                    }
                    l=rom[0x16bf5 + i];
                    rom[0x16bf5 + i]=rom[0x16bf5 + j];
                    rom[0x16bf5 + j]=l;
                    l=rom[0x16c17 + i];
                    rom[0x16c17 + i]=rom[0x16c17 + j];
                    rom[0x16c17 + j]=l;
                    Overselchg(ed,win);
                    ed->selobj=j;
                }
                break;
            case 10:
                if(j>27) j-=28;
                if(j<0) j+=28;
                if(j==26) {
                    if(((short*)(rom + 0x16dc5))[ed->ew.param]) if(k) j+=k; else return 0;
                    else ((short*)(rom + 0x16dc5))[ed->ew.param]=*(short*)(ed->sbuf+i);
                }
                if(j>22) j=(j+41)<<1;
                ed->sbuf[i+2]=j;
            }
            Overselchg(ed,win);
        }
        break;
    case WM_LBUTTONDBLCLK:
        
        if(ed->selobj==-1) break;
        switch(ed->tool) {
        case 3:
            SendMessage(ed->ew.doc->editwin,4000,0x30000 + ed->ew.doc->rom[0xdbb73 + ed->selobj],0);
            break;
        case 5:
            m=ed->sprset;
            b6=ed->ebuf[m];
            j=b6[ed->selobj+2];
            i=ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,j);
            if(i==-1) break;
            n=ed->mapscrollh<<5;
            o=ed->mapscrollv<<5;
            ed->ov->modf=1;
            ed->e_modf[ed->sprset]=1;
            rc.left=ed->objx-n;
            rc.top=ed->objy-o;
            wsprintf(buffer,"%02X-%s",j,sprname[j]);
            Getstringobjsize(buffer,&rc);
            InvalidateRect(win,&rc,0);
            wsprintf(buffer,"%02X-%s",i,sprname[i]);
            Getstringobjsize(buffer,&rc);
            InvalidateRect(win,&rc,0);
            b6[ed->selobj+2]=i;
            break;
        case 7:
            oved=ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG10,framewnd,editexit,0);
            break;
        case 9:
            oved=ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG12,framewnd,editwhirl,0);
            break;
        }
        break;
    
    case WM_LBUTTONDOWN:
        
        if(ed->tool == 4)
        {
            l = ( ( (lparam >> 21) + ed->mapscrollv ) << 8)
              | ( ( ( ( (short) lparam) >> 5) + ed->mapscrollh ) << 2)
              | 0x7e2000;
            
            wsprintf(buffer,"Position %04X,%04X:\n%06X %06X\n%06X %06X",
                     ((short)lparam) + (ed->mapscrollh << 5) + ((ed->ew.param & 7) << 9),
                     (lparam >> 16) + (ed->mapscrollv << 5) + ((ed->ew.param & 56) << 6),
                     l,
                     l +    2,
                     l + 0x80,
                     l + 0x82);
            
            MessageBox(framewnd,buffer,"Address calculator",MB_OK);
            
            break;
        }
        if(ed->dtool) break;
        if(ed->tool!=1 && ed->selflag) {
            Overselectwrite(ed);
            Getselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
        }
        ed->dtool=ed->tool+1;
        if(ed->tool!=6) SetCapture(win);
        SetFocus(win);
        goto mousemove;
    
    mousemove:
    case WM_MOUSEMOVE:
        
        {
            HWND const par = GetParent(win);
            
            RECT const par_rect = HM_GetWindowRect(par);
            
            HM_MouseMoveData const d =
            HM_GetMouseMoveData(win, wparam, lparam);
            
            POINT const rel_pos =
            {
                d.m_screen_pos.x - par_rect.left,
                d.m_screen_pos.y - par_rect.top
            };
            
            LPARAM const new_lp = MAKELPARAM(rel_pos.x, rel_pos.y);
            
            PostMessage(GetParent(win), msg, wparam, new_lp);
        }
        
        if(!ed->dtool)
        {
            break;
        }
        
        n=ed->mapscrollh<<5;
        o=ed->mapscrollv<<5;
        l=(lparam>>16)+o;
        k=((short)lparam)+n;
        if(ed->mapsize) q=0x3ff; else q=0x1ff;
        if(k>q) k=q-1;
        if(l>q) l=q-1;
        if(k<0) k=0;
        if(l<0) l=0;
        switch(ed->dtool) {
        case 1:
            if(ed->disp&8) {
                k>>=4;
                l>>=4;
                rc.left=(k<<4)-n;
                rc.right=rc.left+17;
                rc.top=(l<<4)-o;
                rc.bottom=rc.top+17;
                InvalidateRect(win,&rc,0);
                if(ed->disp&1) {
                    ed->ovlmap[(k&31)+((l&31)<<5) + 0x1000]=ed->selblk;
                    rc.left=(k^16<<4)-n;
                    rc.right=rc.left+17;
                    InvalidateRect(win,&rc,0);
                    rc.top=(l^16<<4)-o;
                    rc.bottom=rc.top+17;
                    InvalidateRect(win,&rc,0);
                    rc.left=(k<<4)-n;
                    rc.right=rc.left+17;
                    InvalidateRect(win,&rc,0);
                } else ed->ovlmap[k+(l<<6)]=ed->selblk;
                ed->ovlmodf=1;
                ed->ov->modf=1;
                break;
            }
            if(msg==WM_LBUTTONDOWN) {
                memcpy(ed->undobuf,ed->ov->buf,0x800);
                ed->undomodf=ed->ov->modf;
            }
            k>>=5;
            l>>=5;
            ed->ov->buf[k+(l<<5)]=ed->selblk;
            rc.left=(k<<5)-n;
            rc.right=rc.left+32;
            rc.top=(l<<5)-o;
            rc.bottom=rc.top+32;
            InvalidateRect(win,&rc,0);
            if((ed->ew.param==136) && k>=8 && l<8) {
                rc.left-=256;
                rc.right-=256;
                InvalidateRect(win,&rc,0);
            } else if(ed->ew.param==128 && k>=8 && l>=8 && l<16) {
                rc.top-=256;
                rc.bottom-=256;
                InvalidateRect(win,&rc,0);
            }
            ed->ov->modf=1;
            break;
        case 2:
        case 3:
            k>>=5;
            l>>=5;
            if(msg==WM_LBUTTONDOWN) {
                if(ed->selflag) if(k>=ed->rectleft && k<ed->rectright && l>=ed->recttop && l<ed->rectbot) {
                    ed->selx=k;
                    ed->sely=l;
                } else {
                    Overselectwrite(ed);
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    goto newsel;
                } else newsel: ed->rectleft=ed->rectright=k,ed->recttop=ed->rectbot=l;
            } else {
                if(ed->selflag) {
                    if(k==ed->selx && l==ed->sely) break;
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectleft+=k-ed->selx;
                    ed->rectright+=k-ed->selx;
                    ed->recttop+=l-ed->sely;
                    ed->rectbot+=l-ed->sely;
                    ed->selx=k;
                    ed->sely=l;
                } else {
                    if(k>=ed->rectleft) k++;
                    if(l>=ed->recttop) l++;
                    if(ed->rectright==k && ed->rectbot==l) break;
                    Getselectionrect(ed,&rc);
                    InvalidateRect(win,&rc,0);
                    ed->rectright=k,ed->rectbot=l;
                }
                Getselectionrect(ed,&rc);
                InvalidateRect(win,&rc,0);
            }
            break;
        case 4: case 6: case 8: case 9: case 10: case 11:
            rom=ed->ew.doc->rom;
            if(ed->disp&4) {
                if(msg==WM_LBUTTONDOWN) {
                    Overselchg(ed,win);
                    switch(ed->tool) {
                    case 3:
                        b4 = (uint16_t*) (rom + 0xdb96f);
                        b5 = (uint16_t*) (rom + 0xdba71);
                        i=128;
                        m=0;
searchtile:
                        k&=0xfff0;
                        l&=0xfff0;
                        for(;i>=0;i--) {
                            if(b4[i]==ed->ew.param) {
                                j=b5[i]+m;
                                if(((j&0x7e)<<3)==k && ((j&0x1f80)>>3)==l) {
                                    ed->objx=k;
                                    ed->objy=l;
foundobj:
                                    ed->selx=k;
                                    ed->sely=l;
                                    ed->selobj=i;
                                    Overselchg(ed,win);
                                    return 0;
                                }
                            }
                        }
                        break;
                    case 5:
                        k&=0xfff0;
                        l&=0xfff0;
                        m=ed->sprset;
                        b6=ed->ebuf[ed->sprset];
                        for(i=ed->esize[m]-3;i>=0;i-=3) {
                            rc.left=b6[i+1]<<4;
                            rc.top=b6[i]<<4;
                            Getstringobjsize(Getoverstring(ed,5,i),&rc);
                            if(k>=rc.left && l>=rc.top && k<rc.right && l<rc.bottom) {
                                ed->selobj=i;
                                ed->selx=k;
                                ed->sely=l;
                                
                                ed->objx = (short) rc.left;
                                ed->objy = (short) rc.top;
                                
                                rc.left-=n;
                                rc.top-=o;
                                rc.right-=n;
                                rc.bottom-=o;
                                InvalidateRect(win,&rc,0);
                                return 0;
                            }
                        }
                        break;
                    case 7:
                        b4=(uint16_t*)(rom + 0x16051);
                        b5=(uint16_t*)(rom + 0x160ef);
                        j=k+((ed->ew.param&7)<<9);
                        m=l+((ed->ew.param&56)<<6);
                        for(i=78;i>=0;i--) {
                            if(rom[0x15e28 + i]==ed->ew.param) {
                                if(j>=b5[i] && j<b5[i]+16 && m>=b4[i] && m<b4[i]+16) {
                                    ed->objx=b5[i]-j+k;
                                    ed->objy=b4[i]-m+l;
                                    goto foundobj;
                                }
                            }
                        }
                        break;
                    case 8:
                        
                        b4 = (uint16_t*) (rom + 0xdb826);
                        b5 = (uint16_t*) (rom + 0xdb800);
                        i=18;
                        m=0x400;
                        goto searchtile;
                    case 9:
                        
                        b4 = (uint16_t*) (rom + 0x16b8f);
                        b5 = (uint16_t*) (rom + 0x16b6d);
                        
                        j=k+((ed->ew.param&7)<<9);
                        m=l+((ed->ew.param&56)<<6);
                        for(i=16;i>=0;i--) {
                            if(((short*)(rom + 0x16ae5))[i]==ed->ew.param) {
                                if(j>=b4[i] && j<b4[i]+16 && m>=b5[i] && m<b5[i]+16) {
                                    ed->objx=b4[i]-j+k;
                                    ed->objy=b5[i]-m+l;
                                    goto foundobj;
                                }
                            }
                        }
                        break;
                    case 10:
                        k&=0xfff0;
                        l&=0xfff0;
                        m=ed->sprset;
                        b6=ed->sbuf;
                        for(i=ed->ssize-3;i>=0;i-=3) {
                            j=*(short*)(b6+i);
                            rc.left=(j&0x7e)<<3;
                            rc.top=(j&0x1f80)>>3;
                            
                            if(k>=rc.left && k<rc.left+16 && l>=rc.top && l<rc.top+16)
                            {
                                ed->objx = (short) rc.left;
                                ed->objy = (short) rc.top;
                                ed->ov->modf=1;
                                goto foundobj;
                            }
                        }
                        break;
                    }
                    ed->selobj=-1;
                    ed->dtool=0;
                    ReleaseCapture();
                } else {
                    switch(ed->tool) {
                    case 3:
                        b5=(uint16_t*)(ed->ew.doc->rom + 0xdba71);
                        m=0;
movetile:
                        k&=0xfff0;
                        l&=0xfff0;
                        if(k==ed->selx && l==ed->sely) return 0;
                        rc.left=ed->objx-n;
                        rc.top=ed->objy-o;
                        Getstringobjsize("00",&rc);
                        InvalidateRect(win,&rc,0);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        
                        b5[ed->selobj] = ((ed->objy << 3) | (ed->objx >> 3)) - m;
                        
                        ed->selx=k;
                        rc.left=k-n;
                        ed->sely=l;
                        rc.top=l-o;
                        Getstringobjsize("00",&rc);
                        InvalidateRect(win,&rc,0);
                        break;
                    case 5:
                        k&=0xfff0;
                        l&=0xfff0;
                        if(k==ed->selx && l==ed->sely) return 0;
                        m=ed->sprset;
                        b6=ed->ebuf[ed->sprset];
                        rc.left=ed->objx-n;
                        rc.top=ed->objy-o;
                        Getstringobjsize(Getoverstring(ed,5,ed->selobj),&rc);
                        InvalidateRect(win,&rc,0);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        b6[ed->selobj]=ed->objy>>4;
                        b6[ed->selobj+1]=ed->objx>>4;
                        rc.right+=k-ed->selx;
                        rc.bottom+=l-ed->sely;
                        rc.left=ed->objx-n;
                        rc.top=ed->objy-o;
                        ed->selx=k;
                        ed->sely=l;
                        InvalidateRect(win,&rc,0);
                        ed->e_modf[m]=1;
                        ed->ov->modf=1;
                        break;
                    case 7:
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        changeposition(rom,0x160ef,0x16051,0x15fb3,0x15f15,0x1622b,0x1618d,k-ed->selx,l-ed->sely,0x15e77,ed->ew.param,ed->selobj,ed->mapsize?768:256,0x16367,0x16405);
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    case 8:
                        b5 = (uint16_t*)(ed->ew.doc->rom + 0xdb800);
                        m=0x400;
                        goto movetile;
                    case 9:
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        changeposition(rom,0x16b8f,0x16b6d,0x16b4b,0x16b29,0x16bd3,0x16bb1,k-ed->selx,l-ed->sely,0x16b07,ed->ew.param,ed->selobj,ed->mapsize?768:256,0,0);
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    case 10:
                        k&=0xfff0;
                        l&=0xfff0;
                        if(k==ed->selx && l==ed->sely) return 0;
                        Overselchg(ed,win);
                        ed->objx+=k-ed->selx;
                        ed->objy+=l-ed->sely;
                        i=ed->selobj;
                        j=*(short*)(ed->sbuf+i)=(ed->objx>>3)+(ed->objy<<3);
                        if(ed->sbuf[i+2]==0x86 && ed->ew.param<0x80) ((short*)(rom + 0x16dc5))[ed->ew.param]=j;
                        ed->selx=k;
                        ed->sely=l;
                        Overselchg(ed,win);
                        break;
                    }
                    ed->ew.doc->modf=1;
                }
            }
            break;
        case 7:
            ReleaseCapture();
            if(!copybuf) {
                MessageBox(framewnd,"You must copy before you can paste.","Bad error happened",MB_OK);
                ed->dtool=0;
                break;
            }
            k>>=5;
            l>>=5;
            ed->rectleft=k;
            ed->recttop=l;
            ed->rectright=k+copy_w;
            ed->rectbot=l+copy_h;
            ed->stselx=-1;
            ed->stsely=-1;
            q=copy_w*copy_h<<1;
            ed->selbuf=malloc(q);
            ed->selflag=1;
            memcpy(ed->selbuf,copybuf,q);
            Getselectionrect(ed,&rc);
            InvalidateRect(win,&rc,0);
            ed->dtool=0;
            ed->ov->modf=1;
        }
        break;
    
    case WM_LBUTTONUP:
        
        if(ed->dtool > 1 && ed->dtool < 4)
        {
            Getselectionrect(ed, &rc);
            
            InvalidateRect(win, &rc, 0);
            
            if(ed->rectleft < ed->rectright)
            {
                i = ed->rectleft;
                j = ed->rectright;
            }
            else
            {
                i = ed->rectright;
                j = ed->rectleft;
            }
            
            if(ed->recttop < ed->rectbot)
            {
                k = ed->recttop;
                l = ed->rectbot;
            }
            else
            {
                k = ed->rectbot;
                l = ed->recttop;
            }
            
            if(ed->dtool == 3)
            {
                ed->undomodf=ed->ov->modf;
                memcpy(ed->undobuf,ed->ov->buf,0x800);
                
                for(m = i; m < j; m++)
                    for(n = k; n < l; n++)
                        ed->ov->buf[m + (n << 5)] = ed->selblk;
                
                ed->ov->modf = 1;
            }
            else if(!ed->selflag)
            {
                ed->rectleft = i;
                ed->rectright = j;
                ed->recttop = k;
                ed->rectbot = l;
                ed->selflag = 1;
                ed->stselx = i;
                ed->stsely = k;
                j-=i;
                l-=k;
                l<<=5;
                i+=k<<5;
                n=0;
                b4=ed->ov->buf;
                ed->selbuf=malloc((ed->rectright-ed->rectleft)*(ed->rectbot-ed->recttop)<<1);
                for(o=0;o<l;o+=32) for(m=0;m<j;m++) ed->selbuf[n++]=b4[m+i+o];
            };
        }
        
        if(ed->dtool)
            ReleaseCapture();
        
        ed->dtool=0;
        
        break;
    
    case WM_PAINT:
        
        hdc=BeginPaint(win,&ps);
        oldpal=SelectPalette(hdc,ed->hpal,1);
        RealizePalette(hdc);
        k=((ps.rcPaint.right+31)&0xffffffe0);
        l=((ps.rcPaint.bottom+31)&0xffffffe0);
        n=ed->mapscrollh<<5;
        o=ed->mapscrollv<<5;
        if(ed->dtool==2 || ed->dtool==3 || ed->selflag) Getselectionrect(ed,&rc);
        b4=ed->ov->buf;
        if(ed->ew.param==128) p=8; else if(ed->ew.param==136) p=0x20; else p=0x10;
        oldobj=SelectObject(hdc,white_pen);
        if(ed->mapsize) q=0x400; else q=0x200;
        for(j=ps.rcPaint.top&0xffffffe0;j<l;j+=32) {
            if(j+o>=q) break;
            i=ps.rcPaint.left&0xffffffe0;
            b2=b4+ed->mapscrollh+(i>>5)+j+o;
            
            if(p == 0x20)
                if(j + o < 256)
                {
                    b3 = b2 + 8;
                    b5 = b4 + j + o;
                }
                else
                {
                    p = 16;
                    goto bgchange;
                }
            else
            {
                
bgchange:
                b5 = b4 + 0x400 + ( (j + o) >> 1);
                b3 = b2 + 0x400 - ( (j + o) >> 1);
                
                if(b5 >= b4 + 0x500)
                    b3-=0x100,b5-=0x100;
            }
            
            for(;i<k;i+=32)
            {
                if(b3 >= b5 + p)
                    if(ed->ew.param==128)
                    {
                        b3 = b2 + 256;
                        b5 = b4 + 256 + j + o;
                    }
                    else
                        b3 -= p;
                
                if(i + n >= q)
                    break;
                
                m = *b2;
                
                if(ed->dtool == 3)
                {
                    if(i>=rc.left && i<rc.right && j>=rc.top && j<rc.bottom) m=ed->selblk;
                }
                else if(ed->selflag)
                {
                    if(i >= rc.left && i < rc.right && j >= rc.top && j < rc.bottom)
                        m = ed->selbuf
                        [
                            ( (i + n) >> 5 )
                          - ed->rectleft
                          + ( ( (j + o) >> 5 ) - ed->recttop )
                          * (ed->rectright - ed->rectleft)
                        ];
                }
                
                if(ed->disp&8)
                {
                    if(ed->disp&1) {
                        b6=map16ofs+4;
                        r=(((i+n)&0x1ff)>>4)+(((j+o)&0x1ff)<<1) + 0x1000;
                        ovlblk[0]=ed->ovlmap[r];
                        ovlblk[1]=ed->ovlmap[r+1];
                        ovlblk[2]=ed->ovlmap[r+32];
                        ovlblk[3]=ed->ovlmap[r+33];
                        Drawblock32(ed,*b3,0);
                    } else {
                        b6=map16ofs;
                        r = ( (i + n) >> 4 ) + ( (j + o) << 2 );
                        ovlblk[0]=ed->ovlmap[r];
                        ovlblk[1]=ed->ovlmap[r+1];
                        ovlblk[2]=ed->ovlmap[r+64];
                        ovlblk[3]=ed->ovlmap[r+65];
                        Drawblock32(ed,m,0);
                    }
                } else {
                if(!(ed->disp&1)) goto noback;
                if((!ed->layering) || (ed->ew.param==128 && ((b3<b5+16 && b3>=b5+8 && b2<b4+256) || (ed->sprset<2 && b2<b4+512 && b3<b5+8))) ||
                    ((ed->ew.param==136) && b2<b5+8 && b2<b4+256)) {
                    Drawblock32(ed,m,0);
                    Drawblock32(ed,*b3,3);
                } else if(ed->layering==1) {
                    Drawblock32(ed,*b3,0);
                    Drawblock32(ed,m,1);
                } else noback: Drawblock32(ed,m,0);
                }
                Paintblocks(&(ps.rcPaint),hdc,i,j,(DUNGEDIT*)ed);
                if(ed->disp&8) {
                    for(m=0;m<4;m++) {
                        if(!(j+o+(m&2))) continue;
                        if(((ed->ovlmap[r+b6[m]]+1)^(ed->ovlmap[r+b6[m]-b6[2]]+1))&65536) {
                            MoveToEx(hdc,i+blkx[m<<2],j+blky[m<<2],0);
                            LineTo(hdc,i+blkx[m<<2]+16,j+blky[m<<2]);
                        }
                    }
                    for(m=0;m<4;m++) {
                        if(!(i+n+(m&1))) continue;
                        if(((ed->ovlmap[r+b6[m]]+1)^(ed->ovlmap[r+b6[m]-1]+1))&65536) {
                            MoveToEx(hdc,i+blkx[m<<2],j+blky[m<<2],0);
                            LineTo(hdc,i+blkx[m<<2],j+blky[m<<2]+16);
                        }
                    }
                }
                if(ed->disp&2) {
                    MoveToEx(hdc,i+31,j,0);
                    LineTo(hdc,i+31,j+31);
                    LineTo(hdc,i-1,j+31);
                    if(ed->disp&8) {
                        MoveToEx(hdc,i,j+16,0);
                        LineTo(hdc,i+31,j+16);
                        MoveToEx(hdc,i+16,j,0);
                        LineTo(hdc,i+16,j+31);
                    }
                }
                b2++;
                b3++;
            }
        }
        SelectObject(hdc,oldobj);
        if(rc.right>q-n) rc.right=q-n;
        if(rc.bottom>q-o) rc.bottom=q-o;
        if(ed->dtool==2) {
            if(rc.right>rc.left && rc.bottom>rc.top) FrameRect(hdc,&rc,white_brush);
        }
        if(ed->selflag) FrameRect(hdc,&rc,green_brush);
        if(ed->disp&4) {
            oldobj2=SelectObject(hdc,trk_font);
            SetBkMode(hdc,TRANSPARENT);
            q=ed->mapsize?1024:512;
            rom=ed->ew.doc->rom;
            oldobj=0;
            for(i=0;i<6;i++) {
                if(tool_ovt[ed->tool]==i) continue;
                oldobj2=Paintovlocs(hdc,ed,i,n,o,q,rom);
                if(!oldobj) oldobj=oldobj2;
            }
            Paintovlocs(hdc,ed,tool_ovt[ed->tool],n,o,q,rom);
            if(ed->selobj!=-1) {
                Getoverstring(ed,ed->tool,ed->selobj);
                rc.left=ed->objx-n;
                rc.top=ed->objy-o;
                Getstringobjsize(buffer,&rc);
                oldobj2=SelectObject(hdc,green_pen);
                SelectObject(hdc,black_brush);
                if(rc.right>q-n) rc.right=q-n;
                if(rc.bottom>q-o) rc.bottom=q-o;
                Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom);
                Paintspr(hdc,rc.left,rc.top,n,o,q);
                SelectObject(hdc,oldobj2);
            }
            SelectObject(hdc,oldobj);
        }
        SelectPalette(hdc,oldpal,1);
        EndPaint(win,&ps);
        break;
    default:
        return DefWindowProc(win,msg,wparam,lparam);
    }
    return 0;
}

long CALLBACK patchproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    PATCHLOAD*ed;
    switch(msg) {
    case WM_MDIACTIVATE:
        activedoc=((PATCHLOAD*)GetWindowLong(win,GWL_USERDATA))->ew.doc;
        goto deflt;
    case WM_GETMINMAXINFO:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!ed) goto deflt;
        return SendMessage(ed->dlg,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(ed->dlg,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_DESTROY:
        ed=(PATCHLOAD*)GetWindowLong(win,GWL_USERDATA);
        ed->ew.doc->hackwnd=0;
        free(ed);
        break;
    case WM_CREATE:
        ed=(PATCHLOAD*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)ed);
        ShowWindow(win,SW_SHOW);
        CreateSuperDialog(&patchdlg,win,0,0,0,0,(long)ed);
    default:
deflt:
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    return 0;
}

void Unloadsongs(FDOC*param)
{
    int i,j,k;
    SONG*s;
    SONGPART*sp;
    if(param->m_loaded) {
        if(sndinit) Stopsong();
        k=param->numsong[0]+param->numsong[1]+param->numsong[2];
        for(i=0;i<k;i++) {
            s=param->songs[i];
            if(!s) continue;
            if(!--s->inst) {
                for(j=0;j<s->numparts;j++) {
                    sp=s->tbl[j];
                    if(!--sp->inst) free(sp);
                }
                free(s->tbl);
                free(s);
            }
        }
        free(param->scmd);
        param->m_loaded=0;
        free(param->waves);
        free(param->insts);
        free(param->sndinsts);
        free(param->snddat1);
        free(param->snddat2);
    }
}


void AddMRU(char *f)
{
    int i;
    
    for(i=0;i<4;i++)
    {
        if(mrulist[i] && !_stricmp(mrulist[i],f))
        {
            f = mrulist[i];
            
            for( ; i; i--)
                mrulist[i] = mrulist[i-1];
            
            mrulist[0] = f;
            
            goto foundmru;
        }
    }
    
    free(mrulist[3]);
    
    for(i = 3; i; i--)
        mrulist[i] = mrulist[i-1];
    
    mrulist[0]=_strdup(f);
    
foundmru:
    
    cfg_flag |= CFG_MRU_LOADED;
    
    cfg_modf = 1;
}

void UpdMRU(void)
{
    int i;
    
    for(i=0;i<4;i++)
    {
        DeleteMenu(filemenu, ID_MRU1 + i, 0);
        
        if(mrulist[i])
        {
            if(!mruload)
                AppendMenu(filemenu,MF_SEPARATOR,0,0);
            
            mruload=1;
            
            wsprintf(buffer,"%s\tCtrl+Alt+%d",mrulist[i],i+1);
            
            AppendMenu(filemenu, MF_STRING, ID_MRU1 + i, buffer);
        }
    }
}

// for copying rooms.
// Duproom (duplicate room)*************************************

BOOL CALLBACK duproom(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i, // source room/map
        j, // destination room/map
        k, // switch for map type (dungeon or overworld)
        l; // max index for each respective type. e.g. we have at most 296 overworld maps.
    
    FDOC *doc;
    
    switch(msg)
    {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        
        // the function apparently
        // passes in the associated rom file's pointer.
        doc = (FDOC*) lparam;
        
        // Fill in the default button.
        // otherwise it comes up blank.
        CheckDlgButton(win,IDC_RADIO1,BST_CHECKED);
        
        break;
    
    case WM_COMMAND:
        switch(wparam)
        {
        
        case IDOK: // they picked the OK button
            
            doc = (FDOC*) GetWindowLong(win,DWL_USER);
            
            i = GetDlgItemInt(win,IDC_EDIT1,0,0); // get the source room/map number
            
            j = GetDlgItemInt(win,IDC_EDIT2,0,0); // get the destination room/map number
            
            k = IsDlgButtonChecked(win, IDC_RADIO1) == BST_CHECKED; //1 = overworld, 0 = dungeon.
            
            // if k = 1 (overworld), l = 160, else l = 296 (dungeon)
            l = k ? 160 : 296;
            
            // "l" looks a lot like the number 1, doesn't it?
            if(i < 0 || j < 0 || i >= l || j >= l || i == j)
            {
                MessageBox(framewnd,
                           "Please enter two different room numbers in the appropriate range.",
                           "Bad error happened",
                           MB_OK);
                
                break;
            }
            
            if(k)
            {
                //overworld
                Changesize(doc, 0x50000 + j, i);
                Changesize(doc, 0x500a0 + j, 160 + i);
            }
            else
                //dungeon
                Changesize(doc,0x50140 + j,320+i);
        
        case IDCANCEL:
            
            // kill the dialog, and do nothing.
            EndDialog(win,0);
        }
    }
    
    return 0;
}

//Duproom*************************************************************

//rompropdlg#***************************************************

BOOL CALLBACK rompropdlg(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    FDOC *doc;
    
    unsigned char *rom;
    
    short i,
          j,
          k,
          l,
          m,
          n;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win,DWL_USER,lparam);
        
        doc = (FDOC*) lparam;
        
        rom = doc->rom;
        
        for(i = 21; i >= 0; i--)
            if(rom[0x7fc0 + i] != 32)
                break;
        
        // Copy the game image's internal title.
        memcpy(buffer, doc->rom + 0x7fc0, i + 1);
        
        buffer[i + 1] = 0;
        
        SetDlgItemText(win, IDC_EDIT1, buffer);
        
        j = k = l = m = n = 0;
        
        // Checking how long it takes us to find an empty pushable block
        // in the dungeon data.
        for(i=0;i<0x18c;i+=4)
            if( get_16_le(rom + 0x271de + i) != -1)
                j++;
        
        // Checking number of entrance markers on overworld.
        for(i=0;i<129;i++)
            if( ! is16b_neg1_i(rom + 0xdb96f, i) )
                k++;
        
        // Checking number of exit to overworld markers (in general, these
        // are used in the ending sequence too to transition to overworld
        // scenes.
        for(i=0;i<79;i++)
            if(rom[0x15e28 + i] != 255)
                l++;
        
        // Checking number of hole markers on overworld.
        for(i = 0; i < 19; i++)
            if( ! is16b_neg1_i(rom + 0xdb826, i) )
                m++;
        
        // Checking number of whirlpool markers on overworld.
        for(i = 0; i < 9; i++)
        {
            char char_i = i;
            
            buffer[768+i] =
            (
                is16b_neg1_i(rom + 0x16ae5, i) ? '-'
                                               : (char_i + '0')
            );
        }
        
        buffer[777] = 0;
        
        for( ; i < 17; i++)
            if(((short*)(rom + 0x16ae5))[i]!=-1)
                n++;
        
        i=*(short*)(rom + 0xdde7);
        
        wsprintf(buffer,
                 "Sprites: OV:%d, D:%d, Free:%d\n"
                 "OV Secrets: %d used, %d free\n"
                 "Dungeon secrets: %d used, %d free\n"
                 "Dungeon maps: %d used, %d free\n"
                 "Graphics: %d used, %d free\n"
                 "Blocks: %d/99\n"
                 "Torches: %d/288\n"
                 "Entrances: %d/129\n"
                 "Exits: %d/79\n"
                 "Holes: %d/19\n"
                 "Bird locations set: %s\n"
                 "Whirlpools: %d/8\n"
                 "Expansion size: %d\n"
                 "D. headers: %d used, %d free",
                 doc->dungspr - 0x4cb41,
                 doc->sprend - doc->dungspr - 0x300,
                 0x4ec9f - doc->sprend,
                 doc->sctend - 0xdc3f9,
                 0xdc894 - doc->sctend,
                 i + 0x2217,
                 -0x1950 - i,
                 doc->dmend - 0x57621,
                 0x57ce0 - doc->dmend,
                 doc->gfxend - 0x8b800,
                 0xc4000 - doc->gfxend,
                 j,
                 get_16_le(rom + 0x88c1),
                 k,
                 l,
                 m,
                 buffer+768,
                 n,
                 doc->mapexp ? (doc->fsize - doc->mapexp)
                             : 0,
                 *(short*) (rom + 0x27780) + 2174,
                 -*(short*)(rom + 0x27780));
        
        SetDlgItemText(win,IDC_STATIC2,buffer);
        
        break;
    
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case IDOK:
            
            if(always)
            {
                size_t i = 0;
                
                doc = (FDOC*) GetWindowLong(win, DWL_USER);
                
                GetDlgItemText(win, IDC_EDIT1, buffer, 22);
                
                for(i = strlen(buffer); i < 22; i++)
                    buffer[i] = 32;
                
                memcpy(doc->rom + 0x7fc0, buffer, 22);
            }
        
        case IDCANCEL:
            
            EndDialog(win,0);
        }
    }
    
    return FALSE;
}

void Updatepals(void)
{
    DUNGEDIT*ed=firstgraph;
    while(ed) {
        SendMessage(ed->dlg,4002,0,0);
        ed=(DUNGEDIT*)(ed->nextgraph);
    }
}

long CALLBACK trackeditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT*ed;
    switch(msg) {
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

// window procedure for the main frame window
long CALLBACK frameproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    CLIENTCREATESTRUCT ccs;
    
    OPENFILENAME ofn;
    
    HWND hc;
    
    FDOC *doc, *doc2;
    
    DUNGEDIT *ed;
    
    unsigned char *rom;
    char *m;
    
    HANDLE h,h2;
    MDICREATESTRUCT mdic;
    
    SAMPEDIT *sed;
    ASMHACK *mod;
    FILETIME tim;
    
    int i = 0,
        j = 0,
        k = 0,
        l = 0;
    
    DWORD read_size  = 0;
    DWORD write_size = 0;
    
    unsigned char *buf;
    
    switch(msg)
    {
    
    case WM_DISPLAYCHANGE:
        
        if(always)
        {
            HDC const dc = GetDC(framewnd);
            
            i = palmode;
            
            palmode = 0;
            
            if( GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE )
                palmode = 1;
            
            ReleaseDC(framewnd, dc);
        }
        
        if(palmode > i)
        {
            ed = firstgraph;
            
            while(ed)
            {
                Setpalmode(ed);
                ed = ed->nextgraph;
            }
        }
        else if(palmode < i)
        {
            ed = firstgraph;
            
            while(ed)
            {
                Setfullmode(ed);
                ed = ed->nextgraph;
            }
        }
        
        break;
    
    case WM_QUERYNEWPALETTE:
        if(!dispwnd)
            break;
        
        Setpalette(win,dispwnd->hpal);
        
        return 1;
    
    case WM_PALETTECHANGED:
        Updatepals();
        
        break;
    
/*  case WM_ACTIVATE:
        if(disppal && (wparam&65535)) {
            InvalidateRect(dispwnd,0,0);
//          hc=GetDC(dispwnd);
//          SelectPalette(hc,disppal,1);
//          RealizePalette(hc);
//          ReleaseDC(dispwnd,hc);
        }
        break;*/
    
    case WM_CREATE:
        ccs.hWindowMenu=GetSubMenu(GetMenu(win),2);
        ccs.idFirstChild=3600;
        clientwnd=CreateWindowEx(0,"MDICLIENT",0,MDIS_ALLCHILDSTYLES|WS_VSCROLL|WS_HSCROLL|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,0,0,0,0,win,(HMENU)3599,hinstance,(LPSTR)&ccs);
        
        break;
    
    case WM_CLOSE:
        goto fileexit;
    
    case WM_COMMAND:
        switch(wparam&65535)
        {
        
        case ID_MRU1:
        case ID_MRU2:
        case ID_MRU3:
        case ID_MRU4:
            wparam = (wparam & 0xffff) - ID_MRU1;
            
            if(!mrulist[wparam])
                break;
            
            doc = (FDOC*) malloc(sizeof(FDOC));
            strcpy(doc->filename,mrulist[wparam]);
            
            goto openrom;
        
        case ID_Z3_ABOUT:
            ShowDialog(hinstance,MAKEINTRESOURCE(IDD_DIALOG1),framewnd,aboutfunc,0);
            
            break;
        
        case ID_Z3_HELP:
            SetCurrentDirectory(currdir);
            WinHelp(framewnd,"Z3ED.HLP",HELP_CONTENTS,0);
            
            break;
        
fileexit:
        case ID_Z3_EXIT:
            doc=firstdoc;
            
            while(doc)
            {
                hc=doc->mdiwin;
                doc2=doc->next;
                
                if(SendMessage(hc,WM_CLOSE,0,0))
                    return 0;
                
                doc=doc2;
            }
            
            PostQuitMessage(0);
            
            break;
        
        case ID_Z3_SAVEAS:
            if(!activedoc)
                break;
            
saveas:
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="SNES roms\0*.SMC;*.SFC\0All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=activedoc->filename;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Save game";
            ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt="smc";
            ofn.lpfnHook=0;
            
            if(!GetSaveFileName(&ofn))
                return 1;
        
        case ID_Z3_SAVE:
            
            if(!activedoc)
                break;
            
            if(!*activedoc->filename)
                goto saveas;
            
            for(i=0;i<226;i++)
            {
                j = activedoc->blks[i].count;
                
                if(j != 0)
                {
                    activedoc->blks[i].count=1;
                    Releaseblks(activedoc,i);
                    Getblocks(activedoc,i);
                    activedoc->blks[i].count=j;
                }
            }
            
            for(i=0;i<160;i++)
            {
                if(activedoc->overworld[i].win)
                    Savemap((OVEREDIT*)GetWindowLong(activedoc->overworld[i].win,GWL_USERDATA));
            }
            
            for(i=0;i<168;i++)
            {
                if(activedoc->ents[i])
                    Saveroom((DUNGEDIT*)GetWindowLong(activedoc->ents[i],GWL_USERDATA));
            }
            
            for(i=0;i<128;i++)
            {
                if(activedoc->pals[i])
                    Savepal((PALEDIT*)GetWindowLong(activedoc->pals[i],GWL_USERDATA));
            }
            
            for(i=0;i<2;i++)
            {
                if(activedoc->wmaps[i])
                    Saveworldmap((WMAPEDIT*)GetWindowLong(activedoc->wmaps[i],GWL_USERDATA));
            }
            
            for(i=0;i<14;i++)
            {
                if(activedoc->dmaps[i])
                    Savedungmap((LMAPEDIT*)GetWindowLong(activedoc->dmaps[i],GWL_USERDATA));
            }
            
            for(i=0;i<11;i++)
            {
                if(activedoc->tmaps[i])
                    Savetmap((TMAPEDIT*)GetWindowLong(activedoc->tmaps[i],GWL_USERDATA));
            }
            
            if(activedoc->perspwnd)
                Savepersp((PERSPEDIT*) GetWindowLong(activedoc->perspwnd,GWL_USERDATA));
            
            Savesongs(activedoc);
            Savetext(activedoc);
            SaveOverlays(activedoc);
            
            if(activedoc->m_modf || activedoc->t_modf || activedoc->o_modf)
                return 1;
            
            rom = activedoc->rom;
            
            *(int*)(rom + 0x64118) = activedoc->mapend2;
            *(int*)(rom + 0x6411c) = activedoc->mapend;
            *(int*)(rom + 0x17f80) = activedoc->roomend;
            *(int*)(rom + 0x17f84) = activedoc->roomend2;
            *(int*)(rom + 0x17f88) = activedoc->roomend3;
            *(int*)(rom + 0x17f8c) = 0x4445335A;
            *(int*)(rom + 0x17f90) = 962;
            *(int*)(rom + 0x17f94) = 3;
            *(int*)(rom + 0x17f98) = activedoc->gfxend;
            *(short*)(rom + 0x4c298) = *(int*)(rom + 0x17f9c)=activedoc->dungspr;
            *(int*)(rom + 0x17fa0) = activedoc->sprend;
            
            if(activedoc->nummod)
            {
                if(activedoc->p_modf)
                {
updatemods:
                    if(Buildpatches(activedoc))
                        return 1;
                }
                else
                {
                    mod = activedoc->modules;
                    j = activedoc->nummod;
                    
                    for(i=0;i<j;i++,mod++)
                    {
                        h = CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
                        GetFileTime(h,0,0,&tim);
                        CloseHandle(h);
                        
                        __asm
                        {
                            mov eax,tim.dwLowDateTime
                            mov edx,tim.dwHighDateTime
                            mov ebx,activedoc
                            sub eax,[ebx+FDOC.lastbuild]
                            sbb edx,[ebx+FDOC.lastbuild+4]
                            jnc updatemods
                        }
                        
                        // if(tim.dwLowDateTime>activedoc->lastbuild.dwLowDateTime || tim.dwHighDateTime>activedoc->lastbuild.dwHighDateTime) goto updatemods;
                    }
                }
            }
            else
                Removepatches(activedoc);
            
            put_32_le(rom + 0x17fa8, activedoc->sctend);
            
            put_16_le(rom + 0x17fac,
                      (short) (activedoc->dmend - 0x8000) );
            
            put_32_le(rom + 0x17fae, activedoc->ovlend);
            put_32_le(rom + 0x17fb2, activedoc->tmend1);
            put_32_le(rom + 0x17fb6, activedoc->tmend2);
            put_32_le(rom + 0x17fba, activedoc->tmend3);
            put_32_le(rom + 0x17fbe, activedoc->oolend);
            
            if(activedoc->mapexp)
                put_32_le(rom + 0x100000, activedoc->mapexp);
            
            h = CreateFile(activedoc->filename,
                           GENERIC_WRITE,
                           0,
                           0,
                           OPEN_ALWAYS,
                           FILE_FLAG_SEQUENTIAL_SCAN,
                           0);
            
            if(h == INVALID_HANDLE_VALUE)
            {
saveerror:
                MessageBox(framewnd,"Unable to save","Bad error happened",MB_OK);
                
                return 1;
            }
            
            m = strrchr(activedoc->filename,'.');
            
            if
            (
                ( ! m )
             || strrchr(activedoc->filename, '\\') > m
            )
            {
                // \task I think the warnings are right. This should
                // probably be activedoc, not doc.
                m = doc->filename + strlen(activedoc->filename);
            }
            
            l = *(int*)m;
            
            // Appears to attempt to append ".HDM" to the file path.
            // \task Should probably fix this as it's endianness dependent.
            *(int*)m=0x444d482e;
            
            if(activedoc->nummod)
            {
                h2=CreateFile("HMTEMP.DAT",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,0);
                
                if(h2==(HANDLE)-1)
                {
                    *(int*)m=l;
                    CloseHandle(h);
                    
                    goto saveerror;
                }
                
                i='HMD0';
                WriteFile(h2,&i,4,&write_size,0);
                WriteFile(h2,&(activedoc->nummod),14,&write_size,0);
                
                for(i = 0; i < activedoc->numpatch; i++)
                {
                    WriteFile(h2,activedoc->patches+i,8,&write_size,0);
                    
                    WriteFile(h2,
                              activedoc->patches[i].pv,
                              activedoc->patches[i].len,
                              &write_size,
                              0);
                }
                
                WriteFile(h2,activedoc->segs,activedoc->numseg<<2,&write_size,0);
                mod = activedoc->modules;
                
                for(i=0;i<activedoc->nummod;i++,mod++)
                {
                    j = strlen(mod->filename);
                    ((int*)buffer)[0] = j;
                    ((int*)buffer)[1] = activedoc->modules[i].flag;
                    
                    WriteFile(h2,buffer,8,&write_size,0);
                    WriteFile(h2,mod->filename,j,&write_size,0);
                }
                
                CloseHandle(h2);
                
                if(!(rom[0x17fa4]&1))
                {
                    if(!MoveFile("HMTEMP.DAT",activedoc->filename))
                    {
                        if(GetLastError() == ERROR_ALREADY_EXISTS)
                        {
                            wsprintf(buffer,"%s exists already. Do you want to overwrite it?",activedoc->filename);
                            
                            if(MessageBox(framewnd,buffer,"Gigasoft Hyrule Magic",MB_YESNO)==IDYES)
                                goto okay;
                            else
                            {
othersaveerror:
                                *(int*)m=l;
                                DeleteFile("HMTEMP.DAT");
                                CloseHandle(h);
                                
                                return 1;
                            }
                        }
                        else
                            goto othersaveerror;
                    }
                    
                    goto okay2;
                }
                else
                {
okay:
                    DeleteFile(activedoc->filename);
                    
                    if(!MoveFile("HMTEMP.DAT",activedoc->filename))
                    {
                        MessageBox(framewnd,"Unable to save hack database","Bad error happened",MB_OK);
                        
                        goto othersaveerror;
                    }
                }
okay2:
                rom[0x17fa4]|=1;
            }
            else
            {
                if(rom[0x17fa4]&1)
                    DeleteFile(activedoc->filename);
                
                rom[0x17fa4]&=-2;
            }
            
            *(int*)m=l;
            
            if(activedoc->withhead)
            {
                ZeroMemory(buffer,512);
                
                buffer[0] = -128;
                buffer[8] = -86;
                buffer[9] = -69;
                buffer[10] = 4;
                
                WriteFile(h,buffer,512,&write_size,0);
            }
            
            WriteFile(h, activedoc->rom, activedoc->fsize, &write_size, 0);
            
            SetEndOfFile(h);
            CloseHandle(h);
            activedoc->modf=0;
            SetWindowText(activedoc->mdiwin,activedoc->filename);
            AddMRU(activedoc->filename);
            UpdMRU();
            DrawMenuBar(framewnd);
            
            return 0;
        
        case ID_Z3_OPEN:
            
            // check if the shift key is down for some reason.
            if(GetKeyState(VK_SHIFT) & 128)
            {
                // if there is no active document, stop.
                if(!activedoc)
                    break;
                
                // If the music data is loaded...
                if(activedoc->m_loaded)
                {
                    // save the songs in the current document.
                    Savesongs(activedoc);
                    
                    // if the music has been modified, get out...
                    if(activedoc->m_modf)
                        break;
                    
                    Unloadsongs(activedoc);
                    Loadsongs(activedoc);
                    
                    // if the HWND handle for the third music bank is valid.
                    if(activedoc->mbanks[3])
                    {
                        sed = ((SAMPEDIT*)GetWindowLong(activedoc->mbanks[3],GWL_USERDATA));
                        sed->zw = activedoc->waves + sed->editsamp;
                    }
                }
                
                // if overlays are loaded
                if(activedoc->o_loaded)
                {
                    // save the overlays of the active rom.
                    SaveOverlays(activedoc);
                    
                    // if the overlays have been modified, exit.
                    if(activedoc->o_modf)
                    {
                        // what this generally means is that the overlays didn't save
                        // properly
                        break;
                    }
                    
                    Unloadovl(activedoc);
                    LoadOverlays(activedoc);
                }
                
                break;
            }
            
            // allocate enough memory for the object.
            doc = (FDOC*) malloc(sizeof(FDOC));
            
            if(!doc) // not enough memory.
            {
                MessageBox(framewnd,"No memory at all","Bad error happened",MB_OK);
                
                break;
            }
            
            // ofn = open file name
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = win;
            ofn.hInstance = hinstance;
            ofn.lpstrFilter = "SNES roms\0*.SMC;*.SFC\0All files\0*.*\0";
            ofn.lpstrCustomFilter = 0;
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = doc->filename;
            doc->filename[0] = 0;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFileTitle = 0;
            ofn.lpstrInitialDir = 0;
            ofn.lpstrTitle = "Open game";
            ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt = "smc";
            ofn.lpfnHook = 0;
            
            if(!GetOpenFileName(&ofn))
            {
//              i=CommDlgExtendedError();
//              if(i) {
//                  wsprintf(buffer,"GUI error. Error: %08X",i);
//                  MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
//              }
                free(doc); break;
            }
openrom:
            
            // handles duplicate files already being open, by checking pointer values.
            for(doc2 = firstdoc; doc2; doc2 = doc2->next)
            {
                if(!_stricmp(doc2->filename,doc->filename))
                {
                    free(doc);
                    SendMessage(clientwnd,WM_MDIACTIVATE,(long)(doc2->mdiwin),0);
                    
                    return 0;
                }
            }
            
            // load the rom file
            h = CreateFile(doc->filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
            
            if(h == INVALID_HANDLE_VALUE)
            {
                MessageBox(framewnd,"Unable to open file","Bad error happened",MB_OK);
                free(doc);
                
                break;
            }
            
            // j is the size of the file, passing the handle of the file, 
            j = GetFileSize(h,0);
            
            //read the first four bytes of the file.
            ReadFile(h,&i,4,&read_size,0);
            
            //these first four bytes have to match, apparently. 
            //kind of primitive error checking...eh?
            //for the curious, that is SEI, STZ $4200 in hex, backwards.
            if(i != 0x42009c78)
            {
                //move the file's base offset up by 512 bytes if we detect a header.
                SetFilePointer(h,512,0,0);
                
                // the filesize is now regarded as (size - header).
                j -= 512;
                
                // as above, we note there is a header.
                doc->withhead = 1;
            }
            else
            {
                // else leave it as is.
                SetFilePointer(h,0,0,0);
                
                // and the file has no header.
                doc->withhead = 0;
            }
            
            // allocate a rom (buffer) the same size as the file.
            rom = doc->rom = malloc(j);
            
            if(!rom)
            {
                MessageBox(framewnd,"Not enough memory.","Bad error happened",MB_OK);
                CloseHandle(h);
                free(doc);
                
                break;
            }
            
            // set the internal file size variable (in an FDOC)
            doc->fsize = j;
            
            // dump the file into the variable "rom".
            ReadFile(h, rom, j, &read_size, 0);
            CloseHandle(h);
            
            //this is LDA $2801, BEQ... another primitive file check (as though romhackers 
            //would never change such things... >_> <_<
            if( get_32_le(rom + 0x200) != 0xf00128ad)
            {
                MessageBox(framewnd,
                           "Hey! This is not Zelda 3.",
                           "Bad error happened",
                           MB_OK);
error:
                free(rom);
                free(doc);
                
                break;
            }
            
            // these tell us where certain data structures end.
            doc->mapend = 0x6410c;
            doc->mapend2 = 0x5fe5e;
            doc->roomend = 0xfff4e;
            doc->roomend2 = 0x5372a;
            doc->roomend3 = 0x1fff5;
            doc->dungspr = 0x4d62e;
            doc->sprend = 0x4ec9f;
            doc->sctend = 0xdc894;
            doc->ovlend = 0x271de;
            doc->dmend = 0x57ce0;
            doc->tmend1 = 0x66d7a;
            doc->tmend2 = 0x75d31;
            doc->oolend = 0x77b64;
            
            i = *(int*) (rom + 0x17f8c);
            
            if(i == 0x4445335A)
            {
                i = *(int*) (rom + 0x17f94);
                k = *(int*) (rom + 0x17f90);
            }
            else
            {
                i = 0;
                k = 0;
            }
            
            if(k == 92)
            {
                rom[0x9bad] = 0x7e;
                *(short*) (rom + 0x9bb2) = 0x229f;
            }
            
            if(i > 3)
            {
                vererror_str[90] = ((k & 0xe000) >> 13) + 48;
                wsprintf(buffer, vererror_str, k >> 16, k & 8191);
                MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
                
                goto error;
            }
            
            if( i >= 3 ) if( rom[0x17fa4] & 1 )
            {
                // truncate to the last '.' (period) in the filename.
                m = strrchr(doc->filename, '.');
                
                // comment later.
                if( (!m) || strrchr(doc->filename, '\\') > m)
                    //go to the end of the filenamme.
                    m = doc->filename + strlen(doc->filename);
                
                l = *(int*)m; // what is the int value there?
                *(int*)m = 0x444d482e; // set it manually to a HMD0 file.
                
                h = CreateFile(doc->filename,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               0,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               0);
                
                if(h == (HANDLE) -1)
                {
                    wsprintf(buffer,"A required file, %s, is missing",doc->filename);
errormsg:
                    MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
                    
                    goto error;
                }
                
                ReadFile(h,&i,4,&read_size,0);
                
                if(i != 'HMD0')
                {
                    wsprintf(buffer,"%s has an invalid format.",doc->filename);
                    
                    goto errormsg;
                }
                
                ReadFile(h,&(doc->nummod),14,&read_size,0);
                
                doc->patches = malloc(doc->numpatch*sizeof(PATCH));
                
                for(i = 0; i < doc->numpatch; i++)
                {
                    // read in the patches
                    ReadFile(h,doc->patches+i,8,&read_size,0);
                    doc->patches[i].pv=malloc(doc->patches[i].len);
                    ReadFile(h,doc->patches[i].pv,doc->patches[i].len,&read_size,0);
                }
                
                ReadFile(h,doc->segs,doc->numseg<<2,&read_size,0);
                
                mod = doc->modules = malloc(doc->nummod * sizeof(ASMHACK));
                
                for(i = 0; i < doc->nummod; i++, mod++)
                {
                    ReadFile(h,doc->modules+i,sizeof(ASMHACK),&read_size,0);
                    
                    k = (int) mod->filename;
                    mod->filename = malloc(k + 1);
                    
                    ReadFile(h,mod->filename,k,&read_size,0);
                    mod->filename[k]=0;
                }
                
                CloseHandle(h);
                
                *(int*)m = l;
            }
            else
            {
nomod:
                doc->lastbuild.dwLowDateTime=0;
                doc->lastbuild.dwHighDateTime=0;
                doc->numseg=doc->numpatch=doc->nummod=0;
                doc->patches = 0;
                doc->modules = 0;
            }
            else
            {
                *(int*) (rom + 0x17fa4) = 0;
                
                goto nomod;
            }
            
            // if the file size is larger or equal to...
            if(j >= 0x100004)
                // the map expansion value is...
                doc->mapexp = get_32_le(rom + 0x100000);
            else
                // otherwise, there is no expansion.
                doc->mapexp = 0;
            
            i = *(int*) (rom + 0x6411c);
            
            // if this has been modified...
            if(i > 0)
                // assign the map ending bound.
                doc->mapend = i;
            
            i = *(int*) (rom + 0x64118);
            
            if(i > 0)
                doc->mapend2 = i; // similarly...
            
            i = *(int*) (rom + 0x17f80);
            
            if(i > 0)
                doc->roomend = i;
            
            i = *(int*) (rom + 0x17f84);
            
            if(i > 0)
                doc->roomend2 = i;
            
            i = *(int*) (rom + 0x17f88);
            
            if(i > 0)
                doc->roomend3 = i;
            
            i = *(int*) (rom + 0x17f9c);
            
            if(i > 0)
            {
                doc->dungspr = *(int*) (rom + 0x17f9c);
                doc->sprend = *(int*) (rom + 0x17fa0);
            }
            
            // bank + HByte + LByte for something...
            i = romaddr((rom[0x505e] << 16) + (rom[0x513d] << 8) + rom[0x521c]);
            
            for(;;)
            {
                j = rom[i++];
                
                if(j == 0xff)
                    break;
                
                if(j >= 0xe0)
                {
                    k = ((j & 3) << 8) + rom[i++];
                    l = (j & 28) >> 2;
                }
                else
                {
                    k = j & 31;
                    l = j >> 5;
                }
                
                switch(l)
                {
                case 0:
                    i += k + 1;
                    
                    break;
                
                case 1: case 3:
                    i++;
                    
                    break;
                
                default:
                    i += 2;
                }
            }
            
            doc->gfxend = i;
            
            i = *(int*)(rom + 0x17fa8);
            
            if(i > 0)
                doc->sctend = i;
            
            i = *(short*)(rom + 0x17fac);
            
            if(i != -1)
                doc->dmend = i + 0x58000;
            
            i = *(int*) (rom + 0x17fae);
            
            if(i > 0)
                doc->ovlend = i;
            
            i = *(int*) (rom + 0x17fb2);
            
            if(i > 0)
            {
                doc->tmend1 = i;
                doc->tmend2 = *(int*) (rom + 0x17fb6);
                doc->tmend3 = *(short*) (rom + 0x17fba) + 0x60000;
            }
            else
            {
                buf = malloc(0x100d);
                
                memcpy(buf, rom + 0x66359,0xfd);
                memcpy(buf + 0xfd, rom + 0x661c8,0xe0);
                memcpy(buf + 0x1dd, rom + 0x6653f,0xfd);
                memcpy(buf + 0x2da, rom + 0x6668d,0x132);
                memcpy(buf + 0x40c, rom + 0x65d6d,0x45b);
                memcpy(buf + 0x867, rom + 0x662a8,0xb1);
                memcpy(buf + 0x918, rom + 0x66456,0xe9);
                memcpy(buf + 0xa01, rom + 0x6663c,0x51);
                memcpy(buf + 0xa52, rom + 0x667bf,0x5bb);
                memcpy(rom + 0x65d6d, buf, 0x40c);
                memcpy(rom + 0x66179, buf + 0x40c, 0xc01);
                
                *(USHORT*)(rom + 0x64ed0) = 0xdd6c;
                *(USHORT*)(rom + 0x64e5f) = 0xde6a;
                *(USHORT*)(rom + 0x654c0) = 0xdf49;
                *(USHORT*)(rom + 0x65148) = 0xe047;
                *(USHORT*)(rom + 0x65292) = 0xe0f4;
                
                // \task look into these 32-bit constants.
                *(int*)(rom + 0x137d) = 0xd4bf1b79;
                *(USHORT*)(rom + 0x1381) = 0x856e;
                *(int*)(rom + 0x1386) = 0xe5e702e1;
                *(USHORT*)(rom + 0x138a)=0xe6e7;
                
                free(buf);
                
                doc->tmend3 = 0x66179;
            }
            
            i = *(int*)(rom + 0x17fbe);
            
            if(i > 0)
                doc->oolend = i;
            
            if(doc->mapexp == 0x100004)
                if(!Changesize(doc,4097,0))
                    goto error;
            
            if(doc->mapexp && doc->mapexp != 0x100080)
            {
                MessageBox(framewnd,"This ROM is corrupt.","Bad error happened",MB_OK);
                goto error;
            }
            
            for(j=0;j<5;j++)
            {
                for(i=0;i<64;i++)
                {
                    if(rom[0x125ec + i]!=i) Savesprites(doc,sprs[j]+i + 0x10000,0,0);
                }
            }
            
            mdic.szClass = "ZEDOC";
            mdic.szTitle = doc->filename;
            mdic.hOwner = hinstance;
            mdic.x = mdic.y = mdic.cx=mdic.cy=CW_USEDEFAULT;
            mdic.style = WS_OVERLAPPEDWINDOW|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
            mdic.lParam = (long)doc;
            hc = (HWND)SendMessage(clientwnd,WM_MDICREATE,0,(long)&mdic);
            
            if(!firstdoc)
                firstdoc = doc;
            
            if(lastdoc)
                lastdoc->next = doc;
            
            doc->prev = lastdoc;
            doc->next = 0;
            
            lastdoc = doc;
            
            doc->modf = 0;
            
            SendMessage(clientwnd,WM_MDIACTIVATE,(long)hc,0);
            SendMessage(clientwnd,WM_MDIREFRESHMENU,0,0);
            
            doc->mdiwin=hc;
            
            for(i=0;i<226;i++)
                doc->blks[i].count = 0,doc->blks[i].buf = 0;
            
            for(i=0;i<160;i++)
                doc->overworld[i].win = 0, doc->overworld[i].modf = 0;
            
            for(i=0;i<168;i++)
                doc->ents[i] = 0;
            
            for(i=0;i<0x128;i++)
                doc->dungs[i] = 0;
            
            for(i=0;i<PALNUM;i++)
                doc->pals[i] = 0;
            
            for(i=0;i<14;i++)
                doc->dmaps[i] = 0;
            
            for(i=0;i<11;i++)
                doc->tmaps[i] = 0;
            
            for(i=0;i<4;i++)
                doc->mbanks[i] = 0;
            
            doc->m_loaded = 0;
            doc->t_loaded = 0;
            doc->o_loaded = 0;
            doc->t_wnd = 0;
            doc->m_modf = 0;
            doc->t_modf = 0;
            doc->o_modf = 0;
            doc->wmaps[0] = doc->wmaps[1] = 0;
            doc->perspwnd = 0;
            doc->hackwnd = 0;
            doc->p_modf = 0;
            
            if(always)
            {
                HMENU const menu = GetMenu(framewnd);
                
                HMENU const submenu =
                GetSubMenu(menu,
                           GetMenuItemCount(menu) == 5 ? 0 : 1);
                
                EnableMenuItem(submenu, ID_Z3_SAVE, MF_ENABLED);
                EnableMenuItem(submenu, ID_Z3_SAVEAS, MF_ENABLED);
                EnableMenuItem(menu, GetMenuItemCount(menu) == 5 ? 1 : 2,
                               MF_ENABLED | MF_BYPOSITION);
            }
            
            AddMRU(doc->filename);
            UpdMRU();
            
            DrawMenuBar(framewnd);
            
            break;
        
        case ID_FILE_SPRNAME:
            
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG5,framewnd,editsprname,0);
            
            break;
        
        case ID_WINDOW_MDITILE:
            SendMessage(clientwnd,WM_MDITILE,0,0);
            break;
        
        case ID_WINDOW_MDICASCADE:
            SendMessage(clientwnd,WM_MDICASCADE,0,0);
            
            break;
        
        case ID_WINDOW_MDIARRANGEICONS:
            SendMessage(clientwnd,WM_MDIICONARRANGE,0,0);
            
            break;
        
        case ID_OPTION_DEVICE:
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG2,framewnd,seldevproc,0);
            
            break;
        
        case ID_OPTION_CLOSESOUND:
            Exitsound();
            
            break;
        
        case ID_OPTION_CHANNEL1:
        case ID_OPTION_CHANNEL2:
        case ID_OPTION_CHANNEL3:
        case ID_OPTION_CHANNEL4:
        case ID_OPTION_CHANNEL5:
        case ID_OPTION_CHANNEL6:
        case ID_OPTION_CHANNEL7:
        case ID_OPTION_CHANNEL8:
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                i = 1 << (wparam - ID_OPTION_CHANNEL1);
                
                activeflag ^= i;
                
                CheckMenuItem
                (
                    GetSubMenu
                    (
                        GetSubMenu
                        (
                            menu,
                            GetMenuItemCount(menu) == 5 ? 3 : 4
                        ),
                        2
                    ),
                    wparam,
                    (activeflag & i) ? MF_CHECKED : MF_UNCHECKED
                );
                
                if((!(activeflag & i)) && sounddev >= 0x20000)
                    midinoteoff(zchans + ((wparam & 65535) - ID_OPTION_CHANNEL1));
            }
            
            break;
        
        case ID_OPTION_SNDINTERP:
            
            sndinterp = !sndinterp;
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                CheckMenuItem(GetSubMenu(menu,
                                         GetMenuItemCount(menu) == 5 ? 3 : 4),
                                         ID_OPTION_SNDINTERP,
                                         sndinterp ? MF_CHECKED : MF_UNCHECKED);
            }
            
            break;
        
        // \note Unused ID not currently referenced in menus or elsewhere.
        case ID_OPTION_GBTNT:
            
            gbtnt = !gbtnt;
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                CheckMenuItem(GetSubMenu(menu,
                                         GetMenuItemCount(menu) == 5 ? 3 : 4),
                                         ID_OPTION_GBTNT,
                                         gbtnt ? MF_CHECKED : MF_UNCHECKED);
            }
            
            break;
            
        case ID_Z3_CUT:
            SendMessage(GetFocus(),WM_CUT,0,0);
            
            break;
            
        case ID_Z3_COPY:
            SendMessage(GetFocus(),WM_COPY,0,0);
            
            break;
            
        case ID_Z3_PASTE:
            SendMessage(GetFocus(),WM_PASTE,0,0);
            
            break;
            
        case ID_EDITING_DELOVERITEM:
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 128; i++)
                ((unsigned short*) (rom + 0xdc2f9))[i] = 0xc3f9;
            
            *(short*) (rom + 0xdc3f9) = -1;
            activedoc->sctend=0xdc3fb;
            
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELDUNGITEM:
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x140;i++)
                ((unsigned short*) (rom + 0xdb69))[i] = 0xdde9;
            
            *(short*)(rom + 0xdde9)=-1;
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_DELOVERSPR:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x160;i++)
                ((unsigned short*) (rom + 0x4c881))[i] = 0xcb41;
            
            rom[0x4cb41] = 0xff;
            
            memcpy(rom + 0x4cb42,
                   rom + activedoc->dungspr,
                   activedoc->sprend - activedoc->dungspr);
            
            for(i = 0; i < 0x128; i++)
            {
                // \task Probably want something like add_le_16
                // and add_le16_i().
                short val = get_16_le_i(rom + 0x4cb42, i)
                          + (short) ( 0x4cb42 - activedoc->dungspr );
                
                put_16_le_i(rom + 0x4cb42,
                            i,
                            val);
            }
            
            activedoc->sprend += 0x4cb42 - activedoc->dungspr;
            activedoc->dungspr = 0x4cb42;
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELDUNGSPR:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            j = activedoc->dungspr;
            
            for(i = 0; i < 0x128; i++)
            {
                put_16_le_i(rom + j,
                            i,
                            (short) (j + 0x300));
            }
            
            put_16_le(rom + j + 0x300, 0xff00);
            
            activedoc->sprend = j + 0x302;
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_DELBLOCKS:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x18c;i+=4)
                *((short*) (rom + 0x271de + i)) = -1;
            
            activedoc->modf=1;
            
            for(i=0;i<0x128;i++)
            {
                if(activedoc->dungs[i])
                {
                    hc = GetDlgItem(activedoc->dungs[i], ID_DungEditWindow);
                    Updatemap((DUNGEDIT*)GetWindowLong(hc,GWL_USERDATA));
                    InvalidateRect(hc,0,0);
                }
            }
            
            break;
        
        case ID_EDITING_DELTORCH:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            // \task Is this correct? I'm suspicious of writing a 32-bit value
            // to the rom directly.
            put_32_le(rom + 0x2736a, u32_neg1);
            
            put_16_le(rom + 0x88c1, 4);
            
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELENT:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 129; i++)
            {
                put_16_le_i(rom + 0xdb96f, i, -1);
            }
            
        upddisp:
            
            activedoc->modf=1;
            
            for(i = 0; i < 160; i++)
            {
                if(activedoc->overworld[i].win)
                {
                    InvalidateRect(GetDlgItem(GetDlgItem(activedoc->overworld[i].win,2000),3001),0,0);
                }
            }
            
            break;
        
        case ID_EDITING_DELEXIT:
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<79;i++)
            {
                rom[0x15e28 + i] = 255;
                
                put_16_le_i(rom + 0x15d8a, i, -1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELFLY:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 9; i++)
            {
                put_16_le_i(rom + 0x16ae5, i, -1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELWHIRL:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=9;i<17;i++)
            {
                put_16_le_i(rom + 0x16ae5, i, -1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELHOLES:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            memset(rom + 0xdb826,255,19);
            
            goto upddisp;
        
        case ID_EDITING_DELPITS:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            memset(rom + 0x190c,255,114);
            
            break;
        
        case ID_EDIT_ROMPROPERTIES:
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG18,framewnd,rompropdlg,(long)activedoc);
            
            break;
        
        case ID_EDITING_CLEARDUNGS:
            
            for(i=0;i<320;i++)
            {
                door_ofs = 6;
                buf = activedoc->rom+Changesize(activedoc,i+320,8);
                
                if(buf)
                {
                    *(int*)buf=-65535;
                    *(int*)(buf+4)=-1;
                }
            }
            
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_REPLACE:
            
            // we pass it this program, a pointer to the duplicator dialog, pass framewnd as the 
            // owner window. duproom is the dialogue handling procedure, and active doc is
            // a pointer to the currently active rom file.
            ShowDialog(hinstance, (LPSTR) IDD_DIALOG20, framewnd, duproom, (long) activedoc);
            
            break;
        }
        
    default:
        
        return DefFrameProc(win,clientwnd,msg,wparam,lparam);
    }
    
    return 0;
}

long CALLBACK docproc(HWND win,UINT msg, WPARAM wparam,LPARAM lparam)
{
    FDOC *param;
    
    int i;
    
    HANDLE h,h2;
    
    switch(msg)
    {
    case WM_MDIACTIVATE:
        activedoc=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        
        goto deflt;
    
    case WM_CLOSE:
        
        param = (FDOC*) GetWindowLong(win, GWL_USERDATA);
        
        if(param->modf || param->m_modf || param->t_modf || param->p_modf)
            goto save;
        
        for(i = 0; i < 160; i++)
        {
            if(param->overworld[i].modf)
                goto save;
        }
        
        for(i = 0; i < 2; i++)
        {
            if(param->wmaps[i] && ((WMAPEDIT*) (GetWindowLong(param->wmaps[i], GWL_USERDATA)))->modf)
                goto save;
        }
        
        for(i=0;i<168;i++) {
            if(param->ents[i] && ((DUNGEDIT*)(GetWindowLong(param->ents[i],GWL_USERDATA)))->modf) goto save;
        }
        
        for(i=0;i<PALNUM;i++) {
            if(param->pals[i] && ((PALEDIT*)(GetWindowLong(param->pals[i],GWL_USERDATA)))->modf) goto save;
        }
        
        for(i=0;i<14;i++) {
            if(param->dmaps[i] && ((LMAPEDIT*)(GetWindowLong(param->dmaps[i],GWL_USERDATA)))->modf) goto save;
        }
        
        for(i=0;i<11;i++) {
            if(param->tmaps[i] && ((TMAPEDIT*)(GetWindowLong(param->tmaps[i],GWL_USERDATA)))->modf) goto save;
        }
        
        if(param->perspwnd && ((PERSPEDIT*)(GetWindowLong(param->perspwnd,GWL_USERDATA)))->modf) goto save;
        goto dontsave;
save:
        wsprintf(buffer,"Do you want to save the changes to %s?",param->filename);
        i=MessageBox(framewnd,buffer,"Gigasoft Hyrule Magic",MB_YESNOCANCEL);
        if(i==IDYES) if(SendMessage(framewnd,WM_COMMAND,ID_Z3_SAVE,0)) break;
        if(i==IDCANCEL) return 1;
dontsave:
        DestroyWindow(GetDlgItem(win,2000));
        SendMessage(clientwnd,WM_MDIDESTROY,(long)win,0);
        activedoc=0;
        param->modf=2;
        for(i=0;i<160;i++)
        {
            if(param->overworld[i].win) SendMessage(param->overworld[i].win,WM_CLOSE,0,0);
        }
        
        for(i=0;i<168;i++)
        {
            if(param->ents[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->ents[i],0);
        }
        
        for(i=0;i<4;i++)
        {
            if(param->mbanks[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->mbanks[i],0);
        }
        
        for(i=0;i<PALNUM;i++)
        {
            if(param->pals[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->pals[i],0);
        }
        
        for(i=0;i<2;i++)
        {
            if(param->wmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->wmaps[i],0);
        }
        
        for(i=0;i<14;i++)
        {
            if(param->dmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->dmaps[i],0);
        }
        
        for(i=0;i<11;i++)
        {
            if(param->tmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->tmaps[i],0);
        }
        
        if(param->perspwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->perspwnd,0);
        if(param->hackwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->hackwnd,0);
        if(param->m_loaded) {
            for(i=0;i<param->srnum;i++)
                if(param->sr[i].editor) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->sr[i].editor,0);
        }
        if(param->t_wnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->t_wnd,0);
        if(param->perspwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->perspwnd,0);
        if(param->prev) param->prev->next=param->next;
        else firstdoc=param->next;
        if(param->next) param->next->prev=param->prev;
        else lastdoc=param->prev;
        h2=GetMenu(framewnd);
        h=GetSubMenu(h2,GetMenuItemCount(h2)==5?0:1);
        i=firstdoc?MF_ENABLED:MF_GRAYED;
        EnableMenuItem(h,ID_Z3_SAVE,i);
        EnableMenuItem(h,ID_Z3_SAVEAS,i);
        EnableMenuItem(h2,GetMenuItemCount(h2)==5?1:2,i|MF_BYPOSITION);
        DrawMenuBar(framewnd);
        if(sounddoc==param) {
            if(sndinit) {
//              EnterCriticalSection(&cs_song);
                Stopsong();
//              LeaveCriticalSection(&cs_song);
            } else sounddoc=0;
        }
        free(param->rom);
        Unloadsongs(param);
        Unloadovl(param);
        if(param->t_loaded) {
            for(i=0;i<param->t_number;i++) free(param->tbuf[i]);
            free(param->tbuf);
        }
        free(param);
        return 0;
    case WM_GETMINMAXINFO:
        param=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!param) break;
        return SendMessage(param->editwin,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        param=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(param->editwin,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        param=(FDOC*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        SetWindowLong(win,GWL_USERDATA,(long)param);
        ShowWindow(win,SW_SHOW);
        param->editwin=CreateSuperDialog(&z3_dlg,win,0,0,0,0,(long)param);
    default:
deflt:
        
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

int
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
            
            return 1;
        }
        
        name_len = p_buffer[j];
        
        if(name_len > 15)
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "Sprite name too long",
                       "Bad error happened",
                       MB_OK);
            
            return 1;
        }
        else if(name_len == 0)
        {
            MessageBox(framewnd,
                       "Error in configuration file: \n"
                       "Zero length sprite name",
                       "Bad error happened",
                       MB_OK);
            
            return 1;
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
            return 1;
        }
        
        memcpy(sprname[i],
               p_buffer + j,
               name_len);
        
        // Null terminate.
        sprname[i][name_len] = 0;
        
        j += name_len;
    }
    
    return 0;
}

int WINAPI WinMain(HINSTANCE hinst,HINSTANCE pinst,LPSTR cmdline,int cmdshow)
{
    WNDCLASS wc;
    
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
    
    wc.style=0;
    wc.lpfnWndProc=frameproc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=hinst;
    wc.hIcon=LoadIcon(hinstance,MAKEINTRESOURCE(IDI_ICON1));
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
    wc.lpfnWndProc=dungmapproc;
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
    
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=palproc;
    wc.lpszClassName="PALEDIT";
    RegisterClass(&wc);
    
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
    
    // all the windows classes have been registered.
    
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
    blue_pen  = (HPEN) CreatePen(PS_SOLID,0,0xff0000);
    
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
    trk_font = CreateFont(0, 0,
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
            section_size = get_32_le(cfg_desc + 1);
            
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
                
                int spr_load_error = 0;
                
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
                    spr_load_error = Loadsprnames((char const *) section_data,
                                                  section_size);
                    
                    if(spr_load_error == 0)
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
                        soundvol = get_16_le(section_data);
                        
                        l |= CFG_SNDVOL_LOADED;
                    }
                    
                    break;
                
                case 3:
                    
                    // Custom midi configuration of some sort.
                    
                    if(section_size >= 100)
                    {
                        // \task Change this so that it reads things
                        // specifically as little endian
                        memcpy(midi_inst,
                               section_data,
                               MIDI_ARR_BYTES);
                        
                        memcpy(midi_trans,
                               section_data + MIDI_ARR_BYTES,
                               MIDI_ARR_BYTES);
                        
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
    
    if( !(l & CFG_SPRNAMES_LOADED) )
    {
        h = CreateFile("SPRNAME.DAT",
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       0,
                       OPEN_EXISTING,
                       0,
                       0);
        
        if(h == INVALID_HANDLE_VALUE)
        defaultnames:
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
        else
        {
            DWORD bytes_read = 0;
            
            ReadFile(h, buffer, 1, &bytes_read, 0);
            
            if(buffer[0])
            {
                // \task This looks both experimental and potentially buggy.
                
                b = (unsigned char*) malloc(0x1800);
                
                *b = *buffer;
                
                ReadFile(h, b + 1, 0x17ff, &bytes_read, 0);
                
                for(i = 0; i < 256; i++)
                {
                    strcpy(sprname[i], (char const *) b);
                    
                    b += 9;
                }
                
                for(i = 0; i < 28; i++)
                    wsprintf(sprname[i + 256], "S%02X", i);
                
                b -= 0x1800;
                
                free(b);
                
                i = 0;
            }
            else
            {
                uint8_t desc_buffer[4];
                
                char * spr_names = 0;
                
                DWORD num_read     = 0;
                DWORD size_actual  = 0;
                DWORD size_alleged = 0;
                
                // -----------------------------
                
                ReadFile(h, desc_buffer, 4, &num_read, 0);
                
                size_alleged = get_32_le(desc_buffer);
                
                spr_names = (char*) calloc(1, size_alleged);
                
                ReadFile(h, spr_names, size_alleged, &size_actual, 0);
                
                if(size_actual == size_alleged)
                {
                    i = Loadsprnames(spr_names, size_actual);
                }
                
                free(spr_names);
            }
            
            CloseHandle(h);
            
            if(i)
            {
                goto defaultnames;
            }
        }
    }
    
    GetCurrentDirectory(MAX_PATH, currdir);
    
    debug_window = CreateNotificationWindow(framewnd);
    
    while(GetMessage(&msg,0,0,0))
    {
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
            
            put_32_le(b2 + 1, j);
            
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
            
            put_32_le(b2 + 1, j + 1);
            
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
            
            put_32_le(desc + 1, 2);
            
            put_16_le(section, soundvol);
            
            WriteFile(h, desc, 5, &write_bytes, 0);
            WriteFile(h, section, 2, &write_bytes, 0);
        }
        
        if(cfg_flag & CFG_MIDI_LOADED)
        {
            int i = 0;
            
            uint8_t entry[2];
            
            // -----------------------------
            
            desc[0] = 3;
            
            put_32_le(desc + 1, 100);
            
            WriteFile(h, desc, 5, &write_bytes, 0);
            
            for(i = 0; i < MIDI_ARR_WORDS; i += 1)
            {
                put_16_le(entry, midi_inst[i]);
                
                WriteFile(h, entry, 2, &write_bytes, 0);
            }
            
            for(i = 0; i < MIDI_ARR_WORDS; i += 1)
            {
                put_16_le(entry, midi_trans[i]);
                
                WriteFile(h, entry, 2, &write_bytes, 0);
            }
        }
        
        if(cfg_flag & CFG_ASM_LOADED)
        {
            desc[0] = 4;
            
            put_32_le(desc + 1, strlen(asmpath) );
            
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

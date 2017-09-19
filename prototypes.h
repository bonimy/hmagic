
#if ! defined HMAGIC_PROTOTYPES_HEADER_GUARD

    #define HMAGIC_PROTOTYPES_HEADER_GUARD

#include "structs.h"

// Mainly for sprintf
#include "stdio.h"

int
ShowDialog(HINSTANCE hinst,LPSTR id,HWND owner,DLGPROC dlgproc, LPARAM param);

void
Setpalette(HWND const win, HPALETTE const pal);

void
PaintSprName(HDC hdc,
             int x,
             int y,
             int n,
             int o,
             int          const p_clip_width,
             char const * const p_name);

void
Drawblock(OVEREDIT const * const ed,
          int const x,
          int const y,
          int const n,
          int const t);

void
Dungselectchg(DUNGEDIT * ed,
              HWND       hc,
              int        f);

void
fill4x2(uint8_t  const * const rom,
        uint16_t       * const nbuf,
        uint16_t const * const buf);

unsigned char const *
Drawmap(unsigned char  const * const rom,
        unsigned short       * const nbuf,
        unsigned char  const *       map,
        DUNGEDIT             * const ed);

void
getobj(unsigned char const * map);

void
setobj(DUNGEDIT*ed, unsigned char *map);

void
getdoor(uint8_t const *       map,
        uint8_t const * const rom);

void
setdoor(unsigned char *map);


void
Getdungobjsize(int chk,RECT*rc,int n,int o,int p);

void
Updateblk8sel(BLOCKSEL8 *ed, int num);

void
Updatemap(DUNGEDIT *ed);

int
Editblocks(OVEREDIT *ed,
           int num,
           HWND win);

void
Paintblocks(RECT *rc,
            HDC hdc,
            int x,
            int y,
            DUNGEDIT*ed);

int
Handlescroll(HWND win,
             int wparam,
             int sc,
             int page,
             int scdir,
             int size,
             int size2);

char const *
Getsprstring(DUNGEDIT const * const ed,
             int              const i);

char const *
Getsecretstring(uint8_t const * const rom,
                int             const i);

void
LoadHeader(DUNGEDIT * const ed,
           int        const map);

void
InitBlksel8(HWND        hc,
            BLOCKSEL8 * bs,
            HPALETTE    hpal,
            HDC         hdc);

void
Changeblk8sel(HWND        win,
              BLOCKSEL8 * ed);

void
Updatemap(DUNGEDIT *ed);

void
Updpal(void*ed);

void
Loadpal(void          * ed,
        unsigned char * rom,
        int             start,
        int             ofs,
        int             len,
        int             pals);

void
Getblocks(FDOC * doc,
          int    b);

int
Changesize(FDOC * const doc,
           int          num,
           int          size);

void
Releaseblks(FDOC * const doc,
            int          b);

int
Savesprites(FDOC          * const doc,
            int                   num,
            unsigned char *       buf,
            int                   size);

void
Addgraphwin(DUNGEDIT *ed,
            int       t);

int
Closeroom(DUNGEDIT * const ed);

void
Saveroom(DUNGEDIT * const ed);

void
Paintdungeon(DUNGEDIT * const ed,
             HDC hdc,
             RECT *rc,
             int x,int y,
             int k,int l,
             int n,int o,
             unsigned short const *buf);

// =============================================================================

// utility functions.

int
romaddr(int const addr);

// \deprecated
uint16_t
get_16_le(unsigned char const * const p_arr);

/// \deprecated
/// Get little endian 16-bit value from an offset plus an index
/// This saves you from having to multiply an index by two.
uint16_t
get_16_le_i(unsigned char const * const p_arr,
            size_t                const p_index);

uint16_t
ldle16b(uint8_t const * const p_arr);

uint16_t
ldle16b_i(uint8_t const * const p_arr,
          size_t          const p_index);

uint16_t
ldle16h_i(uint16_t const * const p_arr,
          size_t           const p_index);

uint32_t
ldle24b(uint8_t const * const p_arr);

/// "indexed load little endian 24-bit value using a byte pointer"
uint32_t
ldle24b_i(uint8_t const * const p_arr,
          unsigned        const p_index);

uint32_t
ldle32b(uint8_t const * const p_arr);

void
stle16b(uint8_t * const p_arr,
        uint16_t  const p_val);

void
stle16b_i(uint8_t * const p_arr,
          size_t    const p_index,
          uint16_t  const p_val);


void
stle24b(uint8_t * const p_arr,
        uint32_t  const p_value);

void
addle16b(uint8_t * const p_arr,
         uint16_t  const p_addend);

void
addle16b_i(uint8_t * const p_arr,
           size_t    const p_index,
           uint16_t  const p_addend);

int
is16b_neg1(uint8_t const * const p_arr);

int
is16b_neg1_i(uint8_t const * const p_arr,
             size_t          const p_index);

int
is16h_neg1(uint16_t const * const p_arr);

int
is16h_neg1_i(uint16_t const * const p_arr,
             size_t           const p_index);




int
truth(int value);

#endif
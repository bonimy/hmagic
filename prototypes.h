
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
             int w,
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

// =============================================================================

// utility functions.

int
romaddr(int const addr);

uint16_t
ldle16b(uint8_t const * const p_arr);

uint16_t
ldle16h_i(uint16_t const * const p_arr,
          size_t           const p_index);

uint32_t
ldle24b(uint8_t const * const p_arr);

void
stle24b(uint8_t * const p_arr,
        uint32_t  const p_value);

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


#endif
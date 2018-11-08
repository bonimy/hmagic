
#if ! defined HMAGIC_PROTOTYPES_HEADER_GUARD

    #define HMAGIC_PROTOTYPES_HEADER_GUARD

#include "structs.h"

int
ShowDialog(HINSTANCE hinst,LPSTR id,HWND owner,DLGPROC dlgproc, LPARAM param);

void
Setpalette(HWND const win, HPALETTE const pal);

void
PaintSprName(HDC hdc,
             int x,
             int y,
             RECT const * const p_clip,
             char const * const p_name);

void
Drawblock(OVEREDIT const * const ed,
          int const x,
          int const y,
          int const n,
          int const t);

// \param   f Indicates whether to update the object information
// static text control (would typically do this when an object
// is selected and state changes.
void
Dungselectchg(DUNGEDIT * const ed,
              HWND       const hc,
              int        const f);

void
fill4x2(uint8_t  const * const rom,
        uint16_t       * const nbuf,
        uint16_t const * const buf);

void
getobj(uint8_t const * const map);

void
setobj(DUNGEDIT*ed, unsigned char *map);

void
getdoor(uint8_t const *       map,
        uint8_t const * const rom);

void
setdoor(unsigned char *map);

void
Getstringobjsize(char const * str, RECT *rc);

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

void
Updatesize(HWND win);

HWND
CreateSuperDialog(SUPERDLG *dlgtemp,HWND owner,int x,int y,int w,int h,
                  LPARAM lparam);

void
Unloadovl(FDOC *doc);

void
Stopsong(void);

void
Drawdot(HDC hdc, RECT * rc, int q, int n, int o);

/**
    \param p_y  Position of the entity relative to upper bound of the clip
    rectangle. If the upper left coordinate of the entity is above said
    rectangle, this value will be negative.
    
    \param p_window_size this subroutine assumes that the content (paintable)
    dimensions of the window are square. That is, its height is the same
    as its width.
*/
void
Paintspr(HDC const p_dc,
         int const p_x,
         int const p_y,
         int const p_hscroll,
         int const p_vscroll,
         size_t const p_window_size);

// \note Not sure whether to make a task for these or not. While it appears
// at first glance to be dungeon related, this is type punning, and the
// actual object would be chosen from many different types.
void
Setdispwin(DUNGEDIT *ed);

void
Delgraphwin(DUNGEDIT*ed);

void
Setpalmode(DUNGEDIT *ed);

void
Setfullmode(DUNGEDIT *ed);

uint8_t *
Uncompress(uint8_t const *       src,
           int           * const size,
           int             const p_big_endian);


int
Savesecrets(FDOC          * const doc,
            int             const num,
            uint8_t const * const buf,
            int             const size);

uint8_t *
Compress(uint8_t const * const src,
         int             const oldsize,
         int           * const size,
         int             const flag);


void
LoadOverlays(FDOC * const doc);

int
loadoverovl(FDOC *doc, uint16_t *buf, int mapnum);

void
SetBS16(BLOCKSEL16*bs,int i,HWND hc);

HWND
Editwin(FDOC       * const doc,
        char const * const wclass,
        char const * const title,
        LPARAM       const param,
        int          const size);

int
Fixscrollpos(unsigned char*rom,
             int x,int y,
             int sx,int sy,
             int cx,int cy,
             int dp,
             int m,int l,
             int door1,int door2);

// \task These belong in a *Logic.h header flie
void
Savedungmap(LMAPEDIT*ed);
void
Savetmap(TMAPEDIT*ed);
void
Savepersp(PERSPEDIT*ed);
void
Savesongs(FDOC *doc);
void
SaveOverlays(FDOC * const doc);
int
Buildpatches(FDOC * const doc);
void
Removepatches(FDOC * const doc);
void
Updatesprites(void);

void
HM_RegisterClasses(HINSTANCE p_inst);

#endif
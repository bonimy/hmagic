
#if !defined HMAGIC_STRUCTS_HEADER_GUARD

    #define HMAGIC_STRUCTS_HEADER_GUARD

    #define OEMRESOURCE

    // This is legacy code, and I don't want to bother using the MS-specific
    // extensions to the standard library at this time.
    #define _CRT_SECURE_NO_WARNINGS

    // 'nonstandard extension used : non-constant aggregate initializer'
    // This warning is not really relevant since major vendors support this
    // extension and C99 eliminates it completely whenever MS gets around to
    // supporting it.
    #pragma warning(disable:4204)

    // There are just too many of these warnings to address at the moment
    // having to deal with truncation from int to short or char. They should
    // Get tackled eventually (and carefully).
    // \task Do this at some point.
#if 0
    #pragma warning(disable:4242)
    #pragma warning(disable:4244)
#endif

    #pragma warning(push, 0)

    #include <windows.h>
    #include <stdint.h>
    #include <commctrl.h>
    #include <mmsystem.h>
    #include <math.h>

    #pragma warning(pop)


    #include "resource.h"



// =============================================================================

enum
{
    PALNUM = 144
};

// =============================================================================

#pragma pack(push)

// Done so that our structures are not allowed to add padding.
// (which would be harmful for type punning / casting that some of the
// code does. I wouldn't call that best practice either, but it's what
// we have to work with at the moment (e.g. casting one type of edit
// structure to a DUNGEDIT pointer because it has members in common).
#pragma pack(1)

typedef struct
{
    unsigned char *buf;
    int count;
    int modf;
} ZBLOCKS;

typedef struct
{
    HWND win;
    uint16_t *buf;
    int modf;
} ZOVER;

typedef struct
{
    int lopst;
    int end;
    short lflag,copy;
    short*buf;
} ZWAVE;

typedef struct
{
    unsigned char samp;
    unsigned char ad,sr,gain;
    unsigned char multhi,multlo;
} ZINST;

typedef struct tag_ZSFXINT
{
    unsigned char voll,volr;
    short freq;
    unsigned char samp,ad,sr,gain,multhi;
} ZSFXINST;

typedef struct
{
    unsigned short addr;
    short next,prev;
    unsigned char flag,cmd,p1,p2,p3,b1,b2;
    unsigned char tim2;
    unsigned short tim;
} SCMD;

typedef struct
{
    unsigned short start,
                   end;
    
    short first,
          inst,
          bank;
    
    unsigned char endtime,
                  filler;
    
    HWND editor;
    
} SRANGE;

typedef struct
{
    unsigned char flag,inst;
    short tbl[8];
    unsigned short addr;
} SONGPART;

typedef struct
{
    unsigned char flag,inst;
    SONGPART**tbl;
    short numparts;
    short lopst;
    unsigned short addr;
} SONG;

typedef struct
{
    int len;
    int addr;
    unsigned char*pv;
} PATCH;

typedef struct
{
    char*filename;
    int flag;
} ASMHACK;

typedef struct tagFDOC
{
    // FDOC consists of a filename...
    char filename[MAX_PATH];
    
    // A "modified" flag
    int modf;
    
    unsigned char *rom; // Pointer to a rom file.
    int fsize; // the file's size
    
    int mapend, // where overworld maps end, parts one and two.
        mapend2,
        mapexp, // expansion map.
        roomend, // dungeon room end 1
        roomend2,
        roomend3, // through 3...
        ovlend; // end of overlays
    
    
    HWND editwin,
         mdiwin;
    
    ZBLOCKS blks[226]; // A set of 226 blocks.
    
    ZOVER overworld[160]; // 160 Overworld maps
    
    HWND dungs[0x128]; // 0x128 dungeon maps (296)
    HWND ents[0xa8]; // entrances
    
    short m_loaded,m_size,m_free,m_modf; // ???
    
    HWND mbanks[4]; // ???
    
    SRANGE *sr; // deals with music, I guess.
    
    short srnum, srsize;
    short numsong[3]; // ditto
    
    HWND pals[PALNUM]; //collection of windows for displaying palettes
    
    ZWAVE *waves;
    ZINST *insts;
    
    int numwave,numinst,numsndinst;
    
    SONGPART *sp_mark;
    SCMD *scmd;
    
    SONG *songs[128];
    ZSFXINST *sndinsts;
    
    char *snddat1, *snddat2; // more music stuff.
    
    int sndlen1,sndlen2,m_ofs,w_modf;
    
    HWND t_wnd;
    
    short t_loaded,t_number,t_modf,withhead;
    
    // Array of raw binary text data for each message in the game.
    // Once correctly loaded, the first two bytes are a 16-bit value that
    // indicates the length of the buffer, in a way analogous to Pascal
    // strings.
    unsigned char **tbuf;
    
    unsigned short nummod;
    unsigned short numpatch;
    unsigned short numseg;
    
    FILETIME lastbuild;
    ASMHACK *modules;
    
    PATCH *patches;
    
    int segs[92];
    int p_modf;
    
    HWND wmaps[2]; // world maps
    HWND dmaps[14]; // dungeon maps
    HWND tmaps[11]; // tile maps (for intro screen, etc.)
    HWND perspwnd;
    HWND hackwnd;
    
    int gfxend, // graphics end
        dungspr, // dungeon sprite
        sprend, // sprite end
        sctend, // end of overworld secrets data (items)
        dmend, // ???
        tmend1, // treasure map end 1
        tmend2, // treasure map end 2
        tmend3, // treasure map end 3
        oolend; // overworld overlay end
    
    // buffer for doing overlay manipulation
    unsigned short *ovlbuf;
    
    char o_enb[32]; // enabled overlays?
    
    int o_loaded, // flags: overlays have been loaded
        o_modf;   // overlays have been modified
    
    struct tagFDOC *prev, *next; // links to other documents.
    
} FDOC;

typedef struct
{
    FDOC *doc;
    int param;
} EDITWIN;

typedef struct
{
    EDITWIN ew;
    int palw,palh;
    HBRUSH*brush;
    short*pal;
    int modf;
} PALEDIT;

typedef struct
{
    EDITWIN ew;
    HWND dlg;
    int sel_song;
    int init;
} MUSEDIT;

typedef struct
{
    EDITWIN ew;
    HWND dlg;
    unsigned short flag,init,editsamp;
    int width,height,pageh,pagev,zoom,scroll,page,sell,selr;
    int editinst;
    ZWAVE*zw;
} SAMPEDIT;

typedef struct
{
    EDITWIN ew;
    HWND dlg;
    short sel;
    short scroll;
    short len;
    short page;
    short*tbl;
    short withcapt;
    short csel;
    short debugflag;
} TRACKEDIT;

typedef struct
{
    EDITWIN ew;
    HWND dlg;
    short sel;
    short num;
} TEXTEDIT;

/// Dungeon editor structure
typedef struct
{
    EDITWIN ew;
    
    int blocksets[15];
    int anim;
    
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    
    int gfxtmp;
    
    HWND dlg;
    HPALETTE hpal;
    
    int paltype;
    
    void *nextgraph, *prevgraph;
    
    unsigned char disp, layering;
    
    int gfxnum,sprgfx;
    int mapnum;
    
    int palnum;
    
    unsigned char *buf;
    
    unsigned short filler[0x1000];
    unsigned short nbuf[0x2000];
    unsigned short filler2[0x1000];
    
    int mappageh,mappagev;
    int mapscrollh,mapscrollv;
    int selobj;
    
    /// Which "checkbox" is selected. Specifically, this means which radio
    /// button is currently selected, because that determines which "layer"
    /// is a candidate for editing currently. Layers are things like "layer 2",
    /// "item", "sprite", "torch", etc.
    int selchk;
    
    /// Pairs of 16-bit pointers that remember where the object data and
    /// door data, respectively, begin in the data of this room.
    short chkofs[6];
    
    // \note This member acts as a 7th index of the chkofs array in some
    // situations. Along with the packing pragma, it would be a bad idea
    // to rearrange the relative position if this and the chkofs variable.
    short len;
    short chestloc[16];
    
    unsigned char init,chestnum,ischest,withfocus;
    unsigned char coll,modf;
    
    short hmodf;
    short objt,objl;
    short dragx,dragy;
    
    RECT selrect;
    RECT sizerect;
    
    short selcorner;
    
    // Size and buffer for "enemies", or more generally "sprites".
    short esize;
    unsigned char *ebuf;
    
    // Size and buffer for "secrets", aka "items".
    short ssize;
    unsigned char *sbuf;
    
    // Size and buffer for torches.
    short tsize;
    unsigned char *tbuf;
    
    // the header's size.
    int hsize;
    
    // the header buffer.
    unsigned char hbuf[14];
    
    int map_vscroll_delta;
    
    // \task A buffer for all pixels of the map (experimental)
    uint8_t map_bits[512 * 512];
    
} DUNGEDIT;

typedef struct
{
    struct overedit*ed;
    int sel;
    int page;
    int scroll;
} BLOCKSEL16;

typedef struct overedit
{
    EDITWIN ew;
    int blocksets[15];
    int anim;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    int gfxtmp;
    HWND dlg;
    HPALETTE hpal;
    int paltype;
    void*nextgraph,*prevgraph;
    ZOVER*ov;
    int gfxnum,sprgfx[3],sprpal[3];
    int sel_scroll,sel_page,sel_select;
    int mappageh,mappagev;
    int mapscrollh,mapscrollv;
    int rectleft,rectright,recttop,rectbot;
    unsigned char layering,selflag,disp,dtool,sprset,mapsize,tool,s_modf;
    short selobj;
    short objx,objy;
    short selx,sely;
    short stselx,stsely;
    unsigned short undobuf[0x400];
    int undomodf;
    short esize[3];
    unsigned char*ebuf[3];
    unsigned char e_modf[3];
    char ecopy[3];
    short ssize;
    unsigned char*sbuf;
    unsigned short*selbuf;
    unsigned char selsch[0x580];
    unsigned short*schbuf;
    short schyes[4];
    int schpush,schscroll,schpage,schflag,schnum,selblk,schtyped;
    RECT schrc;
    unsigned short ovlmap[0x1400];
    short ovlenb,ovlmodf;
    BLOCKSEL16 bs;
    
} OVEREDIT;

typedef struct
{
    EDITWIN ew;
    int blocksets[15];
    int anim;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    int gfxtmp;
    HWND dlg;
    HPALETTE hpal;
    int paltype;
    void*nextgraph,*prevgraph;
} EDITCOMMON;

typedef struct
{
    OVEREDIT*ed;
    int w,h;
    HDC bufdc;
    HBITMAP bufbmp;
    int sel;
    int flags;
    int scroll;
    int oldflags;
    int dfl;
} BLOCKSEL8;

typedef struct
{
    EDITWIN ew;
    int blocksets[15];
    int anim;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    int gfxtmp;
    HWND dlg;
    HPALETTE hpal;
    int paltype;
    void*nextgraph,*prevgraph;
    unsigned char buf[0x1000];
    int mappageh,mappagev,mapscrollh,mapscrollv;
    BLOCKSEL8 bs;
    short stselx,stsely;
    int rectleft,rectright,recttop,rectbot;
    unsigned char selflag,dtool,tool,modf,undomodf,filler;
    unsigned char*selbuf;
    unsigned char undobuf[0x1000];
    short selx,sely;
    short marknum,selmark;
    short objx,objy;
} WMAPEDIT;

// Title map editor data?
typedef struct
{
    EDITWIN ew;
    int blocksets[15];
    int anim;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    int gfxtmp;
    HWND dlg;
    HPALETTE hpal;
    int paltype;
    void*nextgraph,*prevgraph;
    unsigned char disp,layering;
    BLOCKSEL8 bs;
    unsigned char gfxnum,sprgfx,comgfx,anigfx,pal1,pal2,pal3,pal4,pal5,pal6;
    int modf,datnum;
    int mapscrollh,mapscrollv,mappageh,mappagev;
    unsigned short nbuf[0x3000];
    unsigned char*buf;
    int len,sel,selbg,withfocus,dragx,dragy,tool,selofs,sellen,back1,back2;
    RECT selrect;
    
} TMAPEDIT;

// "Perspective" editor data
typedef struct
{
    EDITWIN ew;
    HWND dlg;
    int objsel;
    char buf[0x74];
    int len;
    int xrot,yrot,zrot,width,height,scalex,scaley,enlarge,tool,selpt;
    char newlen,newptp,newptx,newpty,modf;
    char newface[16];
} PERSPEDIT;

// "Level map" editor data.
typedef struct
{
    EDITWIN ew;
    int blocksets[15];
    int anim;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    int gfxtmp;
    HWND dlg;
    HPALETTE hpal;
    int paltype;
    void*nextgraph,*prevgraph;
    int modf;
    unsigned char floors,basements,bg,level,init;
    char curfloor;
    short bossroom,bosspos,bossofs;
    unsigned char*rbuf;
    unsigned char*buf;
    short len;
    short disp;
    short blkscroll,blkpage,blkrow,maxrow,blksel,tool,sel;
    unsigned short nbuf[0x200];
    
} LMAPEDIT;

// 8x8 tilemap editor data?
typedef struct
{
    OVEREDIT*oed;
    int blknum;
    int zoom;
    int scrollh,scrollv;
    int pageh,pagev;
    int size;
    unsigned char*buf;
    BITMAPINFOHEADER bmih;
    RGBQUAD pal[256];
    HDC bufdc;
    HBITMAP bufbmp;
    HPALETTE hpal;
    int pwidth,pheight;
    HBRUSH brush[256];
    int psel;
    HWND blkwnd;
    
} BLKEDIT8;

typedef struct
{
    EDITWIN ew;
    HWND dlg;
    
} PATCHLOAD;

// Entries that represent GUI controls on a Super Dialog.
typedef struct
{
    char *cname;
    char *caption;
    
    int x, // x position
        y, // y position
        w, // width
        h, // height
        id, // the entry's ID number
        style,
        exstyle,
        flags;
} SD_ENTRY;

// "Super Dialog" data. More conventionally known as a child frame in MDI.
typedef struct
{
    char *title;
    
    DLGPROC proc;
    int style, minw, minh, numsde;
    
    SD_ENTRY *sde;
    
} SUPERDLG;

typedef struct tagSDC
{
    //contains a super dialogue element
    SUPERDLG *dlgtemp;
    
    // the owner window it came from, I guess.
    HWND owner;
    
    // lparam is a multipurpose parameter, w,h = width height?
    int lparam,w,h;
    
    // handle to the dialogue's window
    HWND win;
    
    // linked list to the next/last one.
    struct tagSDC *prev, *next;
    
}SDCREATE;

typedef struct
{
    BLOCKSEL16 bs;
    short blks[4];
    int w,h;
    HDC bufdc;
    HBITMAP bufbmp;
    
} BLOCKEDIT32;

typedef struct
{
    BLOCKSEL8 bs;
    BLOCKEDIT32*be;
    int w,h;
    HDC bufdc;
    HBITMAP bufbmp;
    unsigned short blks[4];
    
} BLOCKEDIT16;

/// Editor data for dungeon object selector dialog.
typedef struct
{
    DUNGEDIT *ed;
    
    int scroll,sel,w,h;
    
    HWND dlg;
    HDC bufdc;
    HBITMAP bufbmp;
    
    short typednum;
    
} CHOOSEDUNG;

typedef struct
{
    int obj;
    int num;
    int sel;
    int w,h;
    BLOCKSEL8 bs;
    char buf[0x342e];
    char dp_w[16];
    char dp_h[16];
    short dp_st[16];
    short flag;
    
} DPCEDIT;

typedef struct
{
    unsigned short start;
    unsigned short len;
    unsigned short relnum,relsz;
    unsigned short*relocs;
    unsigned short bank;
    unsigned short addr;
    unsigned char*buf;
    int flag;
    
} SSBLOCK;

typedef struct
{
    int pflag;
    int wnum;
    int pos;
    char vol1,vol2;
    unsigned char atk,dec,sus,rel,gain,envs;
    unsigned envx,envclk,envclklo;
    unsigned short freq;
    
} ZMIXWAVE;

typedef struct
{
    unsigned char t1,nvol,nlength;
    char note;
    short wnum;
    short pos;
    short tim;
    short ntim;
    unsigned short pan;
    unsigned short vol;
    unsigned short pant,pand,volt,vold;
    int fdlt,freq;
    unsigned char loop,playing,pbt,pbdly,pbtim;
    char pbdlt,vibspd,pbdst,trans;
    unsigned char vibtim,vibt,vibcnt;
    short vibdlt,vibdep;
    short callpos,lopst;
    unsigned short midnote;
    unsigned char midch,midnum;
    short midpb,midpbdlt;
    unsigned char ft,ftcopy;
    
} ZCHANNEL;

#pragma pack(pop)

// =============================================================================

extern SDCREATE *firstdlg, *lastdlg;

extern FDOC *mark_doc;
extern FDOC *firstdoc, *lastdoc, *activedoc;

extern DUNGEDIT * dispwnd;

extern DPCEDIT *dpce;

extern OVEREDIT * oved;

extern SSBLOCK **ssblt;

extern SD_ENTRY over_sd[],
                dung_sd[],
                mus_sd[],
                track_sd[],
                wmap_sd[],
                lmap_sd[],
                tmap_sd[],
                persp_sd[],
                text_sd[],
                samp_sd[],
                patch_sd[],
                z3_sd[];

// Forward declaring these because some of these invoke each other and
// we'd just end up having to forward declare their window procedures which
// would be a lot more text.
extern SUPERDLG overdlg,
                dungdlg, 
                musdlg,
                sampdlg,
                trackdlg,
                textdlg,
                wmapdlg,
                lmapdlg,
                tmapdlg,
                perspdlg,
                patchdlg,
                z3_dlg;

extern HPEN green_pen,
     null_pen,
     white_pen,
     blue_pen;

extern HBRUSH
       black_brush,
       white_brush,
       green_brush,
       yellow_brush,
       purple_brush,
       red_brush,
       blue_brush,
       gray_brush;

extern HGDIOBJ trk_font;

extern HCURSOR normal_cursor,
        forbid_cursor,
        wait_cursor;

extern HBITMAP arrows_imgs[4];

extern HANDLE shade_brush[8];
extern HANDLE sizecsor[5];

// The handle to the program
extern HINSTANCE hinstance;

extern HWND framewnd, clientwnd;

extern short dm_x;
extern short dm_k;

extern unsigned char dm_l, dm_dl;

extern const char obj3_h[248];
extern const char obj3_m[248];
extern const char obj3_w[248];

extern const unsigned char obj3_t[248];

extern char *cur_sec;

extern HWND debug_window;

extern int const always;

extern const short bg3blkofs[4];

extern int palhalf[8];

extern HDC objdc;

extern RECT const empty_rect;

extern DUNGEDIT * dunged;

extern uint8_t drawbuf[0x400];

extern uint16_t *dm_buf;

// =============================================================================

typedef
struct
{
    BOOL m_control_down;

    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseMoveData;

// =============================================================================

/// For use with mouse button down / up messages, click, hover messages, and 
typedef
struct
{
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;

    /// Is the ALT key down?
    BOOL m_alt_key;
    
    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseData;

// =============================================================================

typedef
struct
{
    signed int m_distance;
    
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;

    /// Is the ALT key down?
    BOOL m_alt_key;

    POINT m_screen_pos;

} HM_MouseWheelData;

// =============================================================================

typedef
struct
{
    HWND m_deactivating;
    HWND m_activating;
}
HM_MdiActivateData;

// =============================================================================

#endif
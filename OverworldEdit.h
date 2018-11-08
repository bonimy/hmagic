
#if ! defined HMAGIC_OVERWORLD_EDIT_HEADER_GUARD

    #define HMAGIC_OVERWORLD_EDIT_HEADER_GUARD

    #include "structs.h"

    #include "Wrappers.h"

// =============================================================================

    extern unsigned short ovlblk[4];

    extern const int blkx[16];
    extern const int blky[16];

    extern short const tool_ovt[];

    char const * sprset_str[];

    extern unsigned short copy_w;
    extern unsigned short copy_h;

    extern unsigned short * copybuf;

    extern int const sprset_loc[3];

    extern int const map_ind[4];

    extern int const map_ofs[5];

    extern HM_TextResource area_names;
    
// =============================================================================

    void
    Drawblock32(OVEREDIT * const ed,
                int        const m,
                int        const t);

    void
    Getselectionrect(OVEREDIT const * const ed,
                     RECT           * const rc);



    void
    Overtoolchg(OVEREDIT * const ed,
                int        const i,
                HWND       const win);


    void
    Overselchg(OVEREDIT const * const ed,
               HWND             const win);


    void
    GetOverString(OVEREDIT const * const ed,
                  int              const t,
                  int              const n,
                  text_buf_ty            p_text_buf);

    void
    Overselectwrite(OVEREDIT * const ed);

    void
    Getblock32(uint8_t const * const rom,
               int             const m,
               short         * const l);

    void
    Savemap(OVEREDIT * const ed);


    int
    getbgmap(OVEREDIT * const ed,
             int        const a,
             int        const b);


    extern int
    Savesecrets(FDOC          * const doc,
                int             const num,
                uint8_t const * const buf,
                int             const size);

// =============================================================================

#endif
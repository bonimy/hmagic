
#if ! defined HMAGIC_WORLD_MAP_LOGIC_HEADER_GUARD

    #define HMAGIC_WORLD_MAP_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    extern int const wmmark_ofs[7];

    extern int const wmpic_ofs[7];

    extern int const wmap_ofs[4];

    extern int const sprs[5];

// =============================================================================

    int
    Getwmappos(WMAPEDIT      const * const ed,
               unsigned char const * const rom,
               int                   const i,
               RECT                * const rc,
               int                   const n,
               int                   const o);


    void
    Wmapselectionrect(WMAPEDIT const * const ed,
                      RECT           * const rc);


    void
    Getwmapstring(WMAPEDIT const * const ed,
                  int              const i,
                  text_buf_ty            p_text_buf);

    void
    Wmapselectwrite(WMAPEDIT * const ed);

    void
    Wmapselchg(WMAPEDIT const * const ed,
               HWND             const win,
               int              const c);

    void
    Saveworldmap(WMAPEDIT * const ed);

// =============================================================================

#endif

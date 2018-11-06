
#if ! defined HMAGIC_GRAPHICS_LOGIC_HEADER_GUARD

    #define HMAGIC_GRAPHICS_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    uint8_t *
    Compress(uint8_t const * const src,
             int             const oldsize,
             int           * const size,
             int             const flag);

    uint8_t *
    Uncompress(uint8_t const *       src,
               int           * const size,
               int             const p_big_endian);

    extern unsigned char *
    Make4bpp(unsigned char *buf,int size);

    extern unsigned char *
    Makesnes(unsigned char *b, int size);

    extern unsigned char*
    Make3bpp(unsigned char *b, int size);

    extern unsigned char *
    Make2bpp(unsigned char *b, int size);

    extern unsigned char *
    Unsnes(unsigned char * const buf, int size);

    extern unsigned char *
    GetBG3GFX(unsigned char *buf, int size);

// =============================================================================

#endif
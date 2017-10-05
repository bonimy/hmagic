
#if ! defined HMAGIC_LOGIC_ENUM_HEADER_GUARD

    #define HMAGIC_LOGIC_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    extern char const * text_error;

// =============================================================================

    void
    LoadText(FDOC * const doc);

    void
    Savetext(FDOC * const doc);

    uint8_t *
    Makezeldastring(FDOC const * const doc,
                    char       *       buf);

    void
    Makeasciistring(FDOC          const * const doc,
                    char                * const buf,
                    unsigned char const * const buf2,
                    int                   const bufsize);

// =============================================================================

#endif
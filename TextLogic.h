
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

    extern void
    Makezeldastring(FDOC             const * const p_doc,
                    AsciiTextMessage const * const p_amsg,
                    ZTextMessage           * const p_zmsg);

    extern void
    Makeasciistring
    (
        FDOC             const * const doc,
        ZTextMessage     const * const p_zmsg,
        AsciiTextMessage       * const p_msg
    );

// =============================================================================

    extern int
    AsciiTextMessage_Init(AsciiTextMessage * const p_msg);


    /**
        Initialize and allocate a structure to fix a size that is known
        "a priori".
    */
    extern int
    AsciiTextMessage_InitSized
    (
        AsciiTextMessage * const p_msg,
        size_t             const p_size
    );

    extern int
    AsciiTextMessage_AppendChar
    (
        AsciiTextMessage * const p_msg,
        char               const p_char
    );

    extern int
    AsciiTextMessage_AppendString
    (
        AsciiTextMessage       * const p_msg,
        char             const * const p_str
    );

    extern void
    AsciiTextMessage_Free(AsciiTextMessage * const p_msg);

// =============================================================================

    extern int
    ZTextMessage_Init(ZTextMessage * const p_msg);

    extern void
    ZTextMessage_AppendChar(ZTextMessage * const p_msg,
                            uint8_t        const p_char);

    extern void
    ZTextMessage_AppendStream(ZTextMessage      * const p_msg,
                              uint8_t     const * const p_data,
                              uint16_t            const p_len);

    /**
        Resets the length of the zchar string back to zero.
    */
    extern void
    ZTextMessage_Empty(ZTextMessage * const p_msg);

    extern void
    ZTextMessage_Free(ZTextMessage * const p_msg);

// =============================================================================


#endif

#if ! defined HMAGIC_LOGIC_ENUM_HEADER_GUARD

    #define HMAGIC_LOGIC_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

    #include "Wrappers.h"

// =============================================================================

    extern char const * text_error;

// =============================================================================

    void
    LoadText(FDOC * const doc);

    void
    Savetext(FDOC * const doc);

    extern void
    Makezeldastring(CP2C(FDOC)        p_doc,
                    CP2C(AString)     p_amsg,
                    CP2(ZTextMessage) p_zmsg);

    extern void
    Makeasciistring
    (
        CP2C(FDOC)            doc,
        CP2C(ZTextMessage)    p_zmsg,
        CP2(AString) p_msg
    );

// =============================================================================

    extern int
    AString_Init(CP2(AString) p_msg);


    /**
        Initialize and allocate a structure to fix a size that is known
        "a priori".
    */
    extern int
    AString_InitSized
    (
        CP2(AString)       p_msg,
        size_t       const p_size
    );

    extern int
    AString_InitFromNativeString
    (
        CP2(AString) p_msg,
        CP2C(char)   p_source
    );

    extern int
    AString_InitFormatted
    (
        CP2(AString) p_msg,
        CP2C(char)   p_fmt,
        ...
    );

    extern int
    AString_CopyN
    (
        CP2(AString)        p_dest,
        CP2C(AString)       p_source,
        size_t        const p_count
    );
    
    extern int
    AString_AppendChar
    (
        CP2(AString)       p_msg,
        char         const p_char
    );

    extern int
    AString_AppendString
    (
        CP2(AString) p_msg,
        CP2C(char)   p_str
    );

    extern int
    AString_AppendFormatted
    (
        CP2(AString) p_msg,
        CP2C(char)   p_fmt,
        ...
    );

    extern void
    AString_Free(CP2(AString) p_msg);

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
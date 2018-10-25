
#if ! defined HMAGIC_TEXT_ENUM_HEADER_GUARD

    #define HMAGIC_TEXT_ENUM_HEADER_GUARD

// =============================================================================

    enum
    {
        ID_TextFirst = 3000,
        
        ID_TextEntriesListControl = ID_TextFirst,
        ID_TextEditWindow,
        ID_TextSetTextButton,
        ID_TextEditTextRadio,
        ID_TextEditDictionaryRadio,
        
        ID_TextAfterLast,
        ID_TextNumControls = (ID_TextAfterLast - ID_TextFirst)
    
    };

// =============================================================================



#endif
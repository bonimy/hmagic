
    /**
        The entire purpose of this header is to simulate afxres.h or winres.h
        on build environments that don't have it.
        
        afxres is from the MFC
        (Microsoft Foundation Class) libraries, which are more easily 
        accessible with a professional version of Visual Studio. That can
        be a problem for some users trying to build the program. Most of the
        contents of the file are irrelevant to Hyrule Magic anyway.

        winres.h, if present, is located in an ATLMFC subdirectory in the
        the Windows SDK, but it's possible that a user will still not have this
        file.
        
        Thus, this file is a standin for the useful work that either of the
        two above files would provide, and relies only on core Windows SDK
        headers being present.
    */

    #pragma once


/**
    \note This symbol has to be short because it's used by the resource
    compiler which is limited to preprocessor symbols that are 32 characters
    long. Yeah...
*/
#if ! defined HM_RES_STANDIN_HEADER_GUARD

    #define HM_RES_STANDIN_HEADER_GUARD

    #include <winresrc.h>

// =============================================================================

#if defined IDC_STATIC
    #undef IDC_STATIC
#endif

    #define IDC_STATIC (-1)

// =============================================================================

#endif

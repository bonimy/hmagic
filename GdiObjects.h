
#if ! defined HMAGIC_GDI_OBJECTS_HEADER_GUARD

    #define HMAGIC_GDI_OBJECTS_HEADER_GUARD

// =============================================================================

    #pragma warning(push, 0)
    
    #include "windows.h"
    
    #pragma warning(pop)

// =============================================================================

    extern HPEN green_pen,
         null_pen,
         white_pen,
         blue_pen,
         tint_pen,
         black_pen;
    
    extern HBRUSH
           black_brush,
           white_brush,
           green_brush,
           yellow_brush,
           purple_brush,
           red_brush,
           blue_brush,
           gray_brush,
           tint_br;

    extern HGDIOBJ trk_font;

    extern HCURSOR normal_cursor,
            forbid_cursor,
            wait_cursor;

    extern HBITMAP arrows_imgs[4];

    extern HANDLE shade_brush[8];

    extern HCURSOR sizecsor[5];

    extern BITMAPINFOHEADER zbmih;

    extern RGBQUAD const blackcolor;

// =============================================================================

#endif
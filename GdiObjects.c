
    #include "structs.h"

    #include "Wrappers.h"

// =============================================================================

    HPEN
    green_pen,
    null_pen,
    white_pen,
    blue_pen,
    black_pen,
    gray_pen = 0,
    tint_pen = 0;

// =============================================================================

    HBRUSH
    black_brush,
    white_brush,
    green_brush,
    yellow_brush,
    purple_brush,
    red_brush,
    blue_brush,
    gray_brush,
    tint_br = 0;

    HANDLE
    shade_brush[8];

    HBRUSH
    super_dlg_brush;

// =============================================================================

    HGDIOBJ
    trk_font;

// =============================================================================

    HCURSOR
    normal_cursor,
    forbid_cursor,
    wait_cursor;

    HCURSOR
    sizecsor[5];

    // \note Adding this is a bit of a hack to accomodate the Dungeon map
    // window.
    HCURSOR null_cursor = 0;

// =============================================================================

    HBITMAP
    arrows_imgs[4];

// =============================================================================

    HICON
    main_icon;
    
    HICON
    alt_icon;

// =============================================================================

    HBITMAP
    objbmp;

// =============================================================================

    BITMAPINFOHEADER
    zbmih =
    {
        sizeof(BITMAPINFOHEADER),
        32,
        32,
        1,
        8,
        BI_RGB,
        0,
        0,
        0,
        256,
        0
    };

// =============================================================================

    RGBQUAD const
    blackcolor = { 0, 0, 0, 0 };

// =============================================================================

    extern void
    HM_InitGDI(HINSTANCE const p_inst)
    {
        int i = 0;
        
        // -----------------------------
        
        // Icons
        main_icon = LoadIcon(p_inst, MAKEINTRESOURCE(IDI_ICON1) );
        
        alt_icon = LoadIcon(p_inst, MAKEINTRESOURCE(IDI_ICON2) );
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        // Brushes
        
        black_brush  = (HBRUSH) GetStockObject(BLACK_BRUSH);
        white_brush  = (HBRUSH) GetStockObject(WHITE_BRUSH);
        
        green_brush  = CreateSolidBrush(0x00ff00);
        purple_brush = CreateSolidBrush(0xff00ff);
        yellow_brush = CreateSolidBrush(0x00ffff);
        red_brush    = CreateSolidBrush(0x0000ff);
        blue_brush   = CreateSolidBrush(0xff0000);
        gray_brush   = CreateSolidBrush(0x608060);
        
        for(i = 0; i < MACRO_ArrayLength(shade_brush); i += 1)
        {
            shade_brush[i] = CreateSolidBrush(i * 0x1f1f1f);
        }
        
        super_dlg_brush = (HBRUSH)
        (
            wver ? COLOR_WINDOW + 1
                 : COLOR_BTNFACE + 1
        );
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        // Pens
        
        green_pen = CreatePen(PS_SOLID, 0, 0x00ff00);
        
        null_pen  = (HPEN) GetStockObject(NULL_PEN);
        white_pen = (HPEN) GetStockObject(WHITE_PEN);
        
        blue_pen  = (HPEN) CreatePen(PS_SOLID, 0, 0xff0000);
        
        black_pen = (HPEN) GetStockObject(BLACK_PEN);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        // Bitmaps
        
        arrows_imgs[0] = LoadBitmap(0, (LPSTR) OBM_LFARROW);
        arrows_imgs[1] = LoadBitmap(0, (LPSTR) OBM_RGARROW);
        arrows_imgs[2] = LoadBitmap(0, (LPSTR) OBM_UPARROW);
        arrows_imgs[3] = LoadBitmap(0, (LPSTR) OBM_DNARROW);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        // Cursors
        
        normal_cursor = LoadCursor(0, IDC_ARROW);
        
        wait_cursor   = LoadCursor(0, IDC_WAIT);
        forbid_cursor = LoadCursor(0, IDC_NO);
        
        sizecsor[0] = LoadCursor(0, IDC_SIZENESW);
        sizecsor[1] = LoadCursor(0, IDC_SIZENS);
        sizecsor[2] = LoadCursor(0, IDC_SIZENWSE);
        sizecsor[3] = LoadCursor(0, IDC_SIZEWE);
        sizecsor[4] = normal_cursor;
        
    #if 0
        trk_font = GetStockObject(ANSI_FIXED_FONT);
    #else
        trk_font = CreateFont(16, 0,
                              0, 0,
                              0,
                              FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET,
                              OUT_OUTLINE_PRECIS,
                              CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY,
    #if 0
                              | ANTIALIASED_QUALITY,
    #endif
                              FIXED_PITCH, "Consolas");
    #endif
            
    }

// =============================================================================

    extern void
    HM_TearDownGDI(void)
    {
        int i = 0;
        
        // -----------------------------
        
        DeleteObject(arrows_imgs[0]);
        DeleteObject(arrows_imgs[1]);
        DeleteObject(arrows_imgs[2]);
        DeleteObject(arrows_imgs[3]);
        
        DeleteObject(objdc);
        DeleteObject(objbmp);
        
        DeleteObject(green_brush);
        DeleteObject(purple_brush);
        DeleteObject(yellow_brush);
        DeleteObject(red_brush);
        DeleteObject(blue_brush);
        DeleteObject(gray_brush);
        DeleteObject(green_pen);
        DeleteObject(blue_pen);
        
        for(i = 0; i < 8; i += 1)
        {
            DeleteObject( shade_brush[i] );
        }
        
        DeleteObject(super_dlg_brush);
    }

// =============================================================================

    void
    SetPalette
    (
        HWND     const win,
        HPALETTE const pal)
    {
        HDC const hdc = GetDC(win);
        
        HPALETTE const oldpal = SelectPalette(hdc, pal, 0);
        
        RealizePalette(hdc);
        
        SelectPalette(hdc, oldpal, 1);
        
        ReleaseDC(win, hdc);
        
        return;
    }

// =============================================================================


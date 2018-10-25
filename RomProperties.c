
#include "structs.h"
#include "prototypes.h"

#include "HMagicUtility.h"

// =============================================================================

BOOL
RomProperties_OnInitDialog(FDOC const * const doc,
                           HWND         const win)
{
    char text_buf[0x2000] = { 0 };

    rom_cty const rom = doc->rom;
    
    rom_cty const torch_count = (rom + offsets.dungeon.torch_count);
    
    short i = 0,
          j,
          k,
          l,
          m,
          n;
    
    // -----------------------------
    
    for(i = 21; i >= 0; i--)
        if(rom[0x7fc0 + i] != 32)
            break;
    
    // Copy the game image's internal title.
    memcpy(text_buf, doc->rom + 0x7fc0, i + 1);
    
    text_buf[i + 1] = 0;
    
    SetDlgItemText(win, IDC_EDIT1, text_buf);
    
    j = k = l = m = n = 0;
    
    // Checking how long it takes us to find an empty pushable block
    // in the dungeon data.
    for(i = 0; i < 0x18c; i += 4)
    {
        if( ldle16b(rom + 0x271de + i) != -1)
            j++;
    }
    
    // Checking number of entrance markers on overworld.
    for(i=0;i<129;i++)
    {
        if( ! is16b_neg1_i(rom + 0xdb96f, i) )
            k++;
    }
    
    // Checking number of exit to overworld markers (in general, these
    // are used in the ending sequence too to transition to overworld
    // scenes.
    for(i=0;i<79;i++)
        if(rom[0x15e28 + i] != 255)
            l++;
    
    // Checking number of hole markers on overworld.
    for(i = 0; i < 19; i++)
        if( ! is16b_neg1_i(rom + 0xdb826, i) )
            m++;
    
    // Checking number of whirlpool markers on overworld.
    for(i = 0; i < 9; i++)
    {
        char char_i = i;
        
        text_buf[768+i] =
        (
            is16b_neg1_i(rom + 0x16ae5, i) ? '-'
                                           : (char_i + '0')
        );
    }
    
    text_buf[777] = 0;
    
    for( ; i < 17; i++)
        if(((short*)(rom + 0x16ae5))[i]!=-1)
            n++;
    
    i = *(short*)(rom + 0xdde7);
    
    wsprintf(text_buf,
             "Sprites: OV:%d, D:%d, Free:%d\n"
             "OV Secrets: %d used, %d free\n"
             "Dungeon secrets: %d used, %d free\n"
             "Dungeon maps: %d used, %d free\n"
             "Graphics: %d used, %d free\n"
             "Blocks: %d/99\n"
             "Torches: %d/288\n"
             "Entrances: %d/129\n"
             "Exits: %d/79\n"
             "Holes: %d/19\n"
             "Bird locations set: %s\n"
             "Whirlpools: %d/8\n"
             "Expansion size: %d\n"
             "D. headers: %d used, %d free",
             doc->dungspr - 0x4cb41,
             doc->sprend - doc->dungspr - 0x300,
             0x4ec9f - doc->sprend,
             doc->sctend - 0xdc3f9,
             0xdc894 - doc->sctend,
             i + 0x2217,
             -0x1950 - i,
             doc->dmend - 0x57621,
             0x57ce0 - doc->dmend,
             doc->gfxend - 0x8b800,
             0xc4000 - doc->gfxend,
             j,
             ldle16b(torch_count),
             k,
             l,
             m,
             text_buf + 768,
             n,
             doc->mapexp ? (doc->fsize - doc->mapexp)
                         : 0,
             *(short*) (rom + 0x27780) + 2174,
             -*(short*)(rom + 0x27780));
    
    SetDlgItemText(win, IDC_STATIC2, text_buf);
    
    return TRUE;
}

// =============================================================================

BOOL CALLBACK
rompropdlg(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    FDOC *doc;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        doc = (FDOC*) lparam;
        
        return RomProperties_OnInitDialog(doc, win);
    
    case WM_COMMAND:
        
        switch(wparam)
        {
        
        case IDOK:
            
            if(always)
            {
                char title_buf[0x100] = { 0 };
                
                size_t i = 0;
                
                // -----------------------------
                
                doc = (FDOC*) GetWindowLong(win, DWL_USER);
                
                GetDlgItemText(win, IDC_EDIT1, title_buf, 22);
                
                for(i = strlen(title_buf); i < 22; i++)
                    title_buf[i] = 32;
                
                memcpy(doc->rom + 0x7fc0, title_buf, 22);
            }
        
        case IDCANCEL:
            
            EndDialog(win,0);
        }
    }
    
    return FALSE;
}
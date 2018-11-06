
    // For sprintf()
    #include <stdio.h>

#include "structs.h"
#include "prototypes.h"

#include "Wrappers.h"
#include "GdiObjects.h"
#include "Callbacks.h"

#include "HMagicUtility.h"

#include "OverworldEnum.h"
#include "OverworldEdit.h"

// =============================================================================

SD_ENTRY over_sd[] = 
{
    {"BLKSEL32","",152,70,0,0,3000,WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,11},
    {"OVERWORLD","",0,92,160,0,3001,WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_HSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,10},
    {"BUTTON","Draw",0,0,60,20,3002,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_GROUP|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Select",64,0,60,20,3003,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Rectangle",128,0,60,20,3004,WS_VISIBLE|WS_TABSTOP|WS_CHILD|BS_AUTORADIOBUTTON|BS_PUSHLIKE|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Frame 1",192,0,60,20,3008,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Entrance",64,72,60,20,3013,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Copy",408,48,40,20,3021,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Paste",448,48,40,20,3022,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Warp Swch",335,0,60,20,3028,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Undo",0,48,60,20,3014,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Addr.Calc",64,48,60,20,3015,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Sprite",320,72,60,20,3016,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Background",128,48,80,20,3011,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Grid",208,48,50,20, SD_OverGrid32CheckBox, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Markers",258,48,70,20,3019,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Overlay",328,48,70,20, SD_OverOverlayChkBox, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_AUTOCHECKBOX,0,0},
    {"BUTTON","Exit",0,72,60,20,3023,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Properties",256,0,60,20,3024,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","Hole",128,72,60,20,3025,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Transport",192,72,60,20,3026,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","Item",256,72,60,20,3027,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|BS_PUSHLIKE|BS_AUTORADIOBUTTON,0,0},
    {"BUTTON","",400,0,20,20,3029,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",425,0,20,20,3030,BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",450,0,20,20, SD_OverUpArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"BUTTON","",475,0,20,20, SD_OverDownArrow, BS_BITMAP|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"COMBOBOX","",329,24,70,80,3020,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS|CBS_DROPDOWNLIST|WS_VSCROLL,0,0},
    {"EDIT","0",56,0,0,20,3005,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,3},
    {"STATIC","GFX#:",0,24,40,20,3006,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",50,24,30,20,3007,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Palette:",90,24,40,20,3009,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",130,24,30,20,3010,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr GFX#:",165,24,55,20,3017,WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",220,24,30,20,3018,WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"STATIC","Spr pal:",255,24,45,20, SD_OverSprTileSetStatic, WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,0,0},
    {"EDIT","",300,24,25,20, SD_OverSprTileSetEditCtl, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,0},
    {"BUTTON","Search",56,20,0,20, SD_OverMapSearchBtn, WS_VISIBLE|WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BUTTON","Search2",56,42,0,20, SD_OverAdjustSearchBtn, WS_TABSTOP|WS_CHILD|WS_CLIPSIBLINGS,0,3},
    {"BLKSEL16","",152,70,0,0, SD_OverMap16_Selector, WS_TABSTOP|WS_BORDER|WS_CHILD|WS_VSCROLL|WS_CLIPSIBLINGS,WS_EX_CLIENTEDGE,11},
    
    // For testing window focus
    {"STATIC", "Window Focus: ",
     250, 20,
     0, 20,
     SD_OverWindowFocus,
     WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
     0,
     3},
};

// =============================================================================

RGBQUAD deathcolor={96,96,48,0};

// =============================================================================

void
loadovermap(uint16_t      * const b4,
            int             const m,
            int             const k,
            unsigned char * const rom)
{
    // Index of the current position in the low and upper byte arrays
    // of the decompressed map32 arrays.
    int i = 0;
    
    // Represents the target index in the output buffer of map32.
    int j = 0;
    
    size_t row = 0;
    
    unsigned char *b2, *b3;
    
    if(m > 0x9f)
    {
        // Can't load maps indexed 0xa0 and higher.
        return;
    }
    
    b2 = Uncompress(rom + romaddr(*(int*) (rom + 0x1794d + m * 3)), 0, 1);
    b3 = Uncompress(rom + romaddr(*(int*) (rom + 0x17b2d + m * 3)), 0, 1);
    
    for(row = 0; row < 16; row += 1)
    {
        /// column
        int co = 0;
        
        for(co = 0; co < 16; co++)
        {
            b4[j] = b3[i] + (b2[i] << 8);
            
            i += 1;
            j += 1;
        }
        
        if(k == 0)
        {
            // This to aid in loading 1024x1024 maps, since they consist of four
            // smaller areas pasted together.
            j += 16;
        }
    }
    
    free(b2);
    free(b3);
}

// =============================================================================

void
Changeselect(HWND hc, int sel)
{
    int sc;
    int i;
    
    RECT rc = HM_GetClientRect(hc);
    
    OVEREDIT *ed = (OVEREDIT*) GetWindowLongPtr(hc, GWLP_USERDATA);
    
    rc.top=((ed->sel_select>>2)-ed->sel_scroll)<<5;
    rc.bottom=rc.top+32;
    ed->selblk=sel;
    if(ed->schflag) {
        for(i=0;i<ed->schnum;i++) if(ed->schbuf[i]==sel) {
            sel=i;
            goto foundblk;
        }
        sel=-1;
    }
foundblk:
    ed->sel_select=sel;
    InvalidateRect(hc,&rc,0);
    if(sel==-1) return;
    sel>>=2;
    sc=ed->sel_scroll;
    if(sel>=sc+ed->sel_page) sc=sel+1-ed->sel_page;
    else if(sel<sc) sc=sel;
    SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|(sc<<16),0);
    rc.top=(sel-ed->sel_scroll)<<5;
    rc.bottom=rc.top+32;
    InvalidateRect(hc,&rc,0);
}


// =============================================================================

BOOL CALLBACK
overdlgproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    text_buf_ty text_buf = { 0 };
    
    int i, // 
        j, // the overworld area number to load.
        k, // The offset for the 8 byte array of blockset information, for a particular area.
        l, // and auxiliary offset for additional blocktypes that are swapped in sometimes.
        m, // use as the graphics number.
        n,
        p,
        q;
    
    short o[4];
    
    HWND hc;
    OVEREDIT *ed, *oed;
    ZOVER *ov;
    
    unsigned char *rom, *b2;
    
    uint16_t * b4 = 0;
    uint16_t * b5 = 0;
    
    // Light world default backdrop.
    RGBQUAD lw_default_bd;
    
    // backdrop for extended areas
    RGBQUAD extended_bd;
    
    // Dark world default backdrop color.
    RGBQUAD dw_default_bd;
    
    switch(msg)
    {
    
    case WM_MOUSEMOVE:
        
        if(always)
        {
            char handle_text[0x100];
            
            HM_MouseMoveData const d = HM_GetMouseMoveData(win, wparam, lparam);
            
            HWND const child = ChildWindowFromPoint(win, d.m_rel_pos);
            
            sprintf(handle_text,
                    "hwnd: %p, x: %d, y: %d",
                    child,
                    d.m_screen_pos.x,
                    d.m_screen_pos.y);
             
            SetDlgItemText(win, SD_OverWindowFocus, handle_text);
        }
        
        break;
    
    case WM_INITDIALOG:
        
        SetWindowLong(win,DWL_USER,lparam);
        
        ed = (OVEREDIT*) lparam;
        ed->hpal = 0;
        
        j = ed->ew.param;
        rom = ed->ew.doc->rom;
        
        ed->gfxtmp = (j & 0x40) ? 0x21 : 0x20;
        
        if(j == 0x88 || j == 0x93)
            ed->gfxtmp = 0x24;
        
        k = 0x6073 + (ed->gfxtmp << 3);
        m = rom[0x7c9c + j];
        
        if(j == 0x95)
            m = 0x22;
        else if(j == 0x96)
            m = 0x3b;
        else if(j == 0x88 || j == 0x93)
            m = 0x51;
        else if(j == 0x9e)
            m = 0x21;
        else if(j == 0x9c)
            m = 0x41;
        else if(j > 0x7f)
            m = 0x2f;
        
        ed->gfxnum = m;
        l = 0x5d97 + (m << 2);
        
        if(j < 0x80)
            ed->mapsize = rom[0x12844 + (j & 0x3f)];
        else if(j == 0x81)
            ed->mapsize = 0x20;
        else
            ed->mapsize = 0;
        
        EnableWindow( GetDlgItem(win, 3007), j < 0x80); //graphics number
        EnableWindow( GetDlgItem(win, 3010), j < 0x80); //palette number
        
        if(j < 0x80)
        {
            // Determines whether the up/down/left/right arrows are grayed out or not.
            EnableWindow(GetDlgItem(win, 3029), j & 7);
            EnableWindow(GetDlgItem(win, 3030), (j + (ed->mapsize ? 2 : 1)) & 7);
            EnableWindow(GetDlgItem(win, SD_OverUpArrow), j & 0x38);
            EnableWindow(GetDlgItem(win, SD_OverDownArrow), (j + (ed->mapsize ? 16 : 8)) & 0x38);
        }
        else
        {
            //Disable the directional arrows if it's an overlay or special area.
            EnableWindow(GetDlgItem(win, 3029), 0);
            EnableWindow(GetDlgItem(win, 3030), 0);
            EnableWindow(GetDlgItem(win, SD_OverUpArrow), 0);
            EnableWindow(GetDlgItem(win, SD_OverDownArrow), 0);
        }
        
        //Tell the window to set an appropriate graphic for each arrow button.
        for(i = 0; i < 4; i++)
            SendDlgItemMessage(win, 3029 + i, BM_SETIMAGE, IMAGE_BITMAP, (int) arrows_imgs[i]);
        
        //The GFX# box is set here.
        SetDlgItemInt(win, 3007, m, 0);
        
        //Write down the first eight blocksets this area will use
        for(i = 0; i < 8; i++)
            ed->blocksets[i] = rom[k++];
        
        // rewrite some of these blockset entries with the ones located at $5D97 + (4*gfxnumber)
        for(i = 3; i < 7; i++)
        {
            m = rom[l++];
            
            if(m != 0)
                ed->blocksets[i] = m;
        }
        
        // Tells us which area it is, without regard to light / dark world.
        m = j & 0x3f;
        
        // In certain cases, the 8th blockset is 0x58 or 0x5A
        if((m >= 3 && m <= 7) || j == 0x95)
            ed->blocksets[8] = 0x58, l = 2;
        else
            ed->blocksets[8] = 0x5a, l = 0;
        
        // more blockset crap... blah blah.
        ed->blocksets[9] = ed->blocksets[8] + 1;
        ed->blocksets[10] = (j & 0x40) ? 126 : 116;
        
        // If we be in da dark world...
        if(j >= 0x40)
        {
            ed->sprgfx[2] = ed->sprgfx[1] = ed->sprgfx[0] = rom[0x7ac1 + j];
            ed->sprpal[2] = ed->sprpal[1] = ed->sprpal[0] = rom[0x7bc1 + j];
        }
        else
        {
            ed->sprgfx[0] = rom[0x7a41 + j];
            ed->sprgfx[1] = rom[0x7a81 + j];
            ed->sprgfx[2] = rom[0x7ac1 + j];
            ed->sprpal[0] = rom[0x7b41 + j];
            ed->sprpal[1] = rom[0x7b81 + j];
            ed->sprpal[2] = rom[0x7bc1 + j];
        }
        
        k = 0x5b57 + (ed->sprgfx[1] << 2);
        
        for(i = 0; i < 4; i++)
            ed->blocksets[i + 11] = rom[k + i] + 0x73;
        
        for(i = 0; i < 15; i++)
            Getblocks(ed->ew.doc, ed->blocksets[i]);
        
        Getblocks(ed->ew.doc, 0x79);
        Getblocks(ed->ew.doc, 0x7a);
        
        if( (j & 0x40) || j == 0x96)
            l++;
        
        if(j == 0x88 || j == 0x93)
            l = 4;
        
        Loadpal(ed, rom, 0x1be6c8 + (( (unsigned short*) (rom + 0xdec3b))[l]), 0x21, 7, 5);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd290 : 0x1bd218, 0x91, 15, 4);
        Loadpal(ed, rom, 0x1bd660, 0, 16, 2);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd3c8 : 0x1bd3ac, 0x81, 7, 1);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd4c4 : 0x1bd4a8, 0x89, 7, 1);
        Loadpal(ed, rom, (j & 0x40) ? 0x1bd4b6 : 0x1bd49a, 0xe9, 7, 1);
        
        m = ed->sprpal[1];
        
        b2 = rom + 0x7d580 + (m << 1);
        
        Loadpal(ed, rom, 0x1bd4e0 + (( (unsigned short*) (rom + 0xdebd6) )[b2[0]]), 0xd1, 7, 1);
        Loadpal(ed, rom, 0x1bd4e0 + (( (unsigned short*) (rom + 0xdebd6) )[b2[1]]), 0xe1, 7, 1);
        Loadpal(ed, rom, 0x1bd308, 0xf1, 15, 1);
        Loadpal(ed, rom, 0x1bd648, 0xdc, 4, 1);
        Loadpal(ed, rom, 0x1bd630, 0xd9, 3, 1);
        
        if(j < 0x88)
            m = rom[0x7d1c + j];
        // special case palette #s.
        else if(j == 0x95)
            m = 7;
        else if(j == 0x96)
            m = 18;
        else if(j == 0x9d)
            m = 6;
        else if(j == 0x97 || j == 0x9e)
            m = 7;
        else if(j == 0x9c)
            m = 23;
        else
            m = 0;
        
        b2 = rom + 0x75504 + (m << 2);
        
        // Set the palette number, visible in the dialog's edit box.
        SetDlgItemInt(win, 3010, m, 0);
        
        if(b2[0] < 128)
            Loadpal(ed,rom,0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[0]]), 0x29, 7, 3);
        
        if(b2[1] < 128)
            Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[1]]), 0x59, 7, 3);
        
        if(b2[2] < 128)
            Loadpal(ed, rom, 0x1be604 + (((unsigned char*)(rom + 0xdebc6))[b2[2]]), 0x71, 7, 1);
        
        // \note Right now these are loaded by extracting them from game code
        // this assumes that the game code for loading these values hasn't
        // changed.
        lw_default_bd = HM_RgbFrom5bpc( ldle16b(rom + 0x75625) );
        
        // Slightly different green shade for zora falls / master sword grove.
        extended_bd   = HM_RgbFrom5bpc( ldle16b(rom + 0x75640) );
        
        dw_default_bd = HM_RgbFrom5bpc( ldle16b(rom + 0x7564f) );
        
        // \task SePH asked about this part where the backdrop is handled.
        // See if it can be fixed.
        if(j & 0x40)
        {
            ed->pal[0] = dw_default_bd;
        }
        else
        {
            ed->pal[0] = lw_default_bd;
        }
        
        if(j == 0x5b || j == 0x88)
            ed->pal[0] = blackcolor;
        
        m = j & 0x3f;
        
        {
            if(m == 3 || m == 5 || m == 7 || j == 0x95)
                ed->pal[0] = deathcolor;
            
            for(i = 16; i < 256; i += 16)
                ed->pal[i] = ed->pal[0];
        }
        
        ov = ed->ew.doc->overworld + j;
        ov->buf = malloc(0xa00);
        ov->modf = 0;
        
        ed->ov = ov;
        
        if(ed->mapsize)
            l = 4;
        else
            l = 1;
        
        for(k = 0; k < l; k++)
        {
            loadovermap(ov->buf + map_ofs[k], j + map_ind[k], 0, rom);
        }
        
        loadovermap(ov->buf + 1024, getbgmap(ed, j, 1), 1, rom);
        
        if(j < 0x80)
        {
            ed->ovlenb = loadoverovl(ed->ew.doc,ed->ovlmap,j);
            
            if(ed->ew.doc->o_loaded == 2)
            {
                EnableWindow( GetDlgItem(win, SD_OverOverlayChkBox), 0);
            }
        }
        
        ed->ovlmodf = 0;
        memcpy(ed->undobuf, ed->ov->buf, 0x800);
        
        ed->undomodf = 0;
        ed->bmih = zbmih;
        ed->sel_scroll = 0;
        ed->sel_select = 0;
        ed->selblk = 0;
        ed->mapscrollh = 0;
        ed->mapscrollv = 0;
        ed->selflag = 0;
        ed->selobj = -1;
        ed->anim = 0;
        ed->disp = 4;
        ed->schflag = 0;
        
        for(i = 0; i < 0x3aa; i++)
            ed->selsch[i] = 0;
        
        for( ; i < 0x57f; i++)
            ed->selsch[i] = 0xff;
        
        for(i = 3000; i < 3002; i++)
        {
            hc = GetDlgItem(win, i);
            
            SetWindowLong(hc, GWL_USERDATA, (int) ed);
            ShowWindow(hc, SW_SHOW);
            Updatesize(hc);
        }
        
        ed->bs.ed = ed;
        ed->bs.scroll = 0;
        ed->bs.sel = 0;
        
        hc = GetDlgItem(win, SD_OverMap16_Selector);
        SetWindowLong(hc, GWL_USERDATA, (int)&(ed->bs));
        Updatesize(hc);
        
        CheckDlgButton(win,3003,BST_CHECKED);
        CheckDlgButton(win,3019,BST_CHECKED);
        ed->tool = 1;
        ed->dtool = 0;
        ed->sprset = 1;
        
        hc = GetDlgItem(win, 3020);
        
        if(j < 0x90)
        {
            ed->e_modf[0] = 0;
            ed->e_modf[1] = 0;
            ed->e_modf[2] = 0;
            ed->ecopy[0] = -1;
            ed->ecopy[1] = -1;
            ed->ecopy[2] = -1;
            l = j >> 7;
            
            for(i = l; i < 3; i++)
            {
                o[i] = *(short*)(rom + sprset_loc[i] + j*2);
                
                for(k = l; k < i; k++)
                {
                    if(o[i] == o[k])
                    {
                        ed->ebuf[i] = ed->ebuf[k];
                        ed->esize[i] = ed->esize[k];
                        ed->ecopy[i] = k;
                        
                        if(i == 1)
                            ed->sprset = 0;
                    }
                }
                
                b2 = rom + 0x50000 + o[i];
                
                for(k = 0; ; k += 3)
                {
                    if(b2[k] == 0xff)
                        break;
                }
                
                ed->esize[i] = k;
                ed->ebuf[i] = malloc(k);
                memcpy(ed->ebuf[i], b2, k);
                
                SendMessage(hc,CB_ADDSTRING,0,(long)(sprset_str[i]));
            }
            
            SendMessage(hc, CB_SETCURSEL, ed->sprset - l, 0);
            
            if(j < 0x80)
            {
                b2 = rom + 0xE0000 + *(short*)(rom + 0xDC2F9 + j*2);
                
                for(k = 0; ; k += 3)
                {
                    if(b2[k] == 0xff)
                        break;
                }
                
                ed->ssize = k;
                ed->sbuf = malloc(k);
                memcpy(ed->sbuf, b2, k);
            }
        }
        else
        {
            ShowWindow(hc,SW_HIDE);
            ShowWindow(GetDlgItem(win,3016),SW_HIDE);
        }
        
        if(j >= 0x80)
        {
            ShowWindow(GetDlgItem(win,3027),SW_HIDE);
            ShowWindow(GetDlgItem(win, SD_OverOverlayChkBox), SW_HIDE);
        }
        
        EnableWindow(GetDlgItem(win,3018),j<0x80);
        EnableWindow( GetDlgItem(win, SD_OverSprTileSetEditCtl), j < 0x80);
        
        SetDlgItemInt(win,3018,ed->sprgfx[ed->sprset],0);
        SetDlgItemInt(win, SD_OverSprTileSetEditCtl, ed->sprpal[ed->sprset], 0);
        
        Addgraphwin((DUNGEDIT*)ed,1);
        
        break;
    
    case 4002:
        
        InvalidateRect(GetDlgItem(win,3000),0,0);
        InvalidateRect(GetDlgItem(win,3001),0,0);
        
        break;
    
    case 4000:
        
        SetDlgItemInt(win,3005,wparam,0);
        
        break;
    
    case WM_COMMAND:
        
        ed = (OVEREDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed)
            break;
        
        switch(wparam)
        {
        case 3002:
            
            Overtoolchg(ed, 0, win);
            
            break;
        
        case 3003:
            
            Overtoolchg(ed, 1, win);
            
            break;
        
        case 3004:
            
            Overtoolchg(ed, 2, win);
            
            break;
        
        case 3005 | (EN_CHANGE << 16):
            
            if(ed->disp & 8)
            {
                i = GetDlgItemInt(win, 3005, 0, 0);
                
                if(i != ed->bs.sel)
                {
                    ed->selblk = i;
                    
                    SetBS16(&(ed->bs),
                            i,
                            GetDlgItem(win, SD_OverMap16_Selector));
                }
            }
            else
            {
                i = GetDlgItemInt(win, 3005, 0, 0);
                
                if(i < 0)
                    SetDlgItemInt(win, 3005, 0, 0);
                else if(i > 0x22a7)
                    SetDlgItemInt(win, 3005, 0x22a7, 0);
                else
                    Changeselect(GetDlgItem(win,3000),GetDlgItemInt(win,3005,0,0));
            }
            
            break;
        
        case 3007 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            m = GetDlgItemInt(win, 3007, 0, 0);
            
            if(ed->gfxnum != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, 3007, 79, 0);
                    break;
                }
                
                rom[0x7c9c + ed->ew.param] = m;
                
                ed->gfxnum = m;
                
                l = 0x5d97 + (m << 2);
                k = 0x6073 + (ed->gfxtmp << 3);
                
                for(i = 3; i < 7; i++)
                {
                    Releaseblks(ed->ew.doc, ed->blocksets[i]);
                    
                    m = rom[l++];
                    
                    if(m == 0)
                        m = rom[k + i];
                    
                    ed->blocksets[i] = m;
                    
                    Getblocks(ed->ew.doc,m);
                }
updscrn:
                InvalidateRect(GetDlgItem(win,3000),0,0);
updmap:
                InvalidateRect(GetDlgItem(win,3001),0,0);
            }
            
            break;
        
        case 3008:
            
            ed->anim++;
            
            if(ed->anim == 3)
                ed->anim = 0;
            
            wsprintf(text_buf,
                     "Frame %d",
                     ed->anim + 1);
            
            SetWindowText((HWND) lparam, text_buf);
            
            goto updscrn;
        
        case 3013:
            
            Overtoolchg(ed,3,win);
            
            break;
        
        case 3011:
            ed->disp &= -2;
            ed->disp |= IsDlgButtonChecked(win, 3011);
            
            goto updmap;
        
        case SD_OverGrid32CheckBox:
            
            ed->disp &= -3;
            ed->disp |= IsDlgButtonChecked(win, SD_OverGrid32CheckBox) << 1;
            
            goto updscrn;
        
        case 3019:
            
            ed->disp &= -5;
            ed->disp |= IsDlgButtonChecked(win, 3019) << 2;
            
            goto updscrn;
        
        case SD_OverOverlayChkBox:
            
            ed->selblk = 0;
            ed->disp &= -9;
            ed->disp |= IsDlgButtonChecked(win, SD_OverOverlayChkBox) << 3;
            
            ShowWindow(GetDlgItem(win,3000),(ed->disp&8)?SW_HIDE:SW_SHOW);
            
            ShowWindow(GetDlgItem(win, SD_OverMap16_Selector),
                       (ed->disp & 8) ? SW_SHOW : SW_HIDE);
            
            goto updmap;
        
        case 3010 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            b2 = rom + 0x75504 + ((rom[0x7d1c + ed->ew.param] = GetDlgItemInt(win, 3010, 0, 0)) << 2);
            
            if(b2[0] < 128)
                Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[0]]), 0x29, 7, 3);
            
            if(b2[1] < 128)
                Loadpal(ed, rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[b2[1]]), 0x59, 7, 3);
            
            if(b2[2] < 128)
                Loadpal(ed, rom, 0x1be604 + (((unsigned char*)(rom + 0xdebc6))[b2[2]]), 0x71, 7, 1);
            
            Updpal(ed);
            
            break;
        
        case 3014:
            
            if(ed->selflag)
            {
                free(ed->selbuf);
                ed->selflag = 0;
                
                goto updmap;
            }
            
            b4 = (uint16_t*) malloc(0x800);
            
            memcpy(b4,ed->ov->buf,0x800);
            memcpy(ed->ov->buf,ed->undobuf,0x800);
            memcpy(ed->undobuf,b4,0x800);
            
            free(b4);
            
            i = ed->undomodf;
            ed->undomodf = ed->ov->modf;
            ed->ov->modf = i;
            
            goto updmap;
        
        case 3015:
            
            Overtoolchg(ed,4,win);
            
            break;
        
        case 3016:
            
            Overtoolchg(ed, 5, win);
            
            break;
        
        case 3018 | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            m = GetDlgItemInt(win, 3018, 0, 0);
            
            if(ed->ew.param>=0x40)
                n = 2;
            else
                n = ed->sprset;
            
            if(ed->sprgfx[n] != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, 3018, 79, 0);
                    break;
                }
                
                ed->ov->modf = 1;
                ed->sprgfx[n] = m;
                
updsprgfx:
                
                rom = ed->ew.doc->rom;
                k = 0x5b4c + (ed->sprgfx[ed->sprset] << 2);
                
                for(i = 11; i < 15; i++)
                {
                    m = rom[k + i] + 0x73;
                    
                    Releaseblks(ed->ew.doc, ed->blocksets[i]);
                    ed->blocksets[i] = m;
                    
                    Getblocks(ed->ew.doc,m);
                }
                
                m = ed->sprpal[n];
                
                goto updsprpal;
            }
            
            break;
        
        case SD_OverSprTileSetEditCtl | (EN_CHANGE << 16):
            
            if(ed->ew.param > 0x7f)
                break;
            
            rom = ed->ew.doc->rom;
            m = GetDlgItemInt(win, SD_OverSprTileSetEditCtl, 0, 0);
            
            if(ed->ew.param >= 0x40)
                n = 2;
            else
                n = ed->sprset;
            
            if(ed->sprpal[n] != m)
            {
                if(m > 79 || m < 0)
                {
                    SetDlgItemInt(win, SD_OverSprTileSetEditCtl, 79, 0);
                    break;
                }
                
                ed->ov->modf = 1;
                ed->sprpal[n] = m;
                
updsprpal:
                
                b2 = rom + 0x7d580 + (m << 1);
                
                Loadpal(ed, rom, 0x1bd4e0 + (((unsigned short*)(rom + 0xdebd6))[b2[0]]), 0xd1, 7, 1);
                Loadpal(ed, rom, 0x1bd4e0 + (((unsigned short*)(rom + 0xdebd6))[b2[1]]), 0xe1, 7, 1);
                
                goto updscrn;
            }
            
            break;
        
        case 3020 | (CBN_SELCHANGE << 16):
            
            if((n = ed->ecopy[ed->sprset]) != -1)
            {
                ed->esize[n] = ed->esize[ed->sprset];
                ed->e_modf[n] = ed->e_modf[ed->sprset];
                ed->ebuf[n] = ed->ebuf[ed->sprset];
            }
            
            n = SendMessage((HWND)lparam,CB_GETCURSEL,0,0)+(ed->ew.param>>7);
            
            if(ed->ecopy[n] != -1)
            {
                wsprintf(text_buf,
                         "The sprites are the same as in the %s. "
                         "Do you want to modify only this set?",
                         sprset_str[ed->ecopy[n]]);
                
                if(MessageBox(framewnd, text_buf, "Hyrule Magic", MB_YESNO) == IDYES)
                {
                    ed->esize[n] = ed->esize[ed->ecopy[n]];
                    ed->ebuf[n] = malloc(ed->esize[n]);
                    memcpy(ed->ebuf[n], ed->ebuf[ed->ecopy[n]], ed->esize[n]);
                    ed->ecopy[n] = -1;
                    ed->e_modf[n]=1;
                    ed->ov->modf=1;
                }
            }
            
            ed->sprset = n;
            
            if((m = ed->ecopy[n]) != -1)
            {
                ed->esize[n] = ed->esize[m];
                ed->e_modf[n] = ed->e_modf[m];
                ed->ebuf[n] = ed->ebuf[m];
            }
            
            if(ed->tool == 5)
                ed->selobj = -1;
            
            loadovermap(ed->ov->buf + 1024, getbgmap(ed,ed->ew.param,n), 1, ed->ew.doc->rom);
            
            SetDlgItemInt(win,3018,ed->sprgfx[(ed->ew.param>=0x40)?2:n],0);
            SetDlgItemInt(win, SD_OverSprTileSetEditCtl, ed->sprpal[(ed->ew.param>=0x40)?2:n],0);
            
            m = ed->sprgfx[n];
            
            goto updsprgfx;
        
        case 3021:
            
            if(ed->selflag)
            {
                if(copybuf)
                    free(copybuf);
                
                copy_w = ed->rectright - ed->rectleft;
                copy_h = ed->rectbot-ed->recttop;
                copybuf = malloc(copy_w * copy_h << 1);
                
                k = ed->rectleft + (ed->recttop << 5);
                l = 0;
                b4 = ed->ov->buf;
                
                for(j = 0; j < copy_h; j++)
                {
                    for(i = 0; i < copy_w; i++)
                        copybuf[l++] = b4[i + k];
                    
                    k += 32;
                }
            }
            else
                MessageBox(framewnd,"Nothing is selected","Bad error happened",MB_OK);
            
            break;
        
        case 3022:
            
            Overtoolchg(ed,6,win);
            
            break;
        
        case 3023:
            
            Overtoolchg(ed,7,win);
            
            break;
        
        case 3024:
            
            oved = ed;
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG11,framewnd,editovprop,0);
            
            break;
        
        case 3025:
            
            Overtoolchg(ed,8,win);
            
            break;
        
        case 3026:
            
            Overtoolchg(ed,9,win);
            
            break;
        
        case 3027:
            
            Overtoolchg(ed,10,win);
            
            break;
        
        case 3028:
            ov = ed->ew.doc->overworld;
            j = ed->ew.param^0x40;
            
            if(j >= 0x80)
                break;
            
            goto overlaunch;
        
        case 3029:
        case 3030:
        case SD_OverUpArrow:
        case SD_OverDownArrow:
            
            ov = ed->ew.doc->overworld;
            j = ed->ew.param;
            
            if(j >= 0x80)
                break;
            
overlaunch:
            
            rom = ed->ew.doc->rom;
            wparam = (wparam - 3029) ^ 1;
            
            if(ed->mapsize)
                if(wparam == 0)
                    j++;
                else if(wparam == 2)
                    j += 8;
            
            if(wparam < 10)
            {
                j += rom[0x12834 + (wparam << 1)] >> 1;
            }
            
            j = ed->ew.doc->rom[0x125ec + (j & 0x3f)] | (j & 0x40);
            
            if(ov[j].win)
            {
                hc = ov[j].win;
                SendMessage(clientwnd,WM_MDIACTIVATE,(int)hc,0);
                
                oed = (OVEREDIT*) GetWindowLong(hc, GWL_USERDATA);
            }
            else
            {
                wsprintf(text_buf,
                         "Area %02X - %s",
                         j,
                         area_names.m_lines[j]);
                
                ov[j].win = Editwin(ed->ew.doc, "ZEOVER", text_buf, j, sizeof(OVEREDIT));
            }
            
            break;
        
        case SD_OverMapSearchBtn:
            
            if(ed->schflag)
            {
                SetWindowText((HWND)lparam,"Search");
                ShowWindow(GetDlgItem(win, SD_OverAdjustSearchBtn), SW_HIDE);
                
                ed->schflag=0;
                free(ed->schbuf);
                
                goto updsel32;
            }
            else
            {
                
search2:
                
                if(GetKeyState(VK_CONTROL) & 128)
                {
                    // \task Test this out. What does it do?
                    ed->schbuf = (uint16_t*) malloc(0x4550);
                    SetCursor(wait_cursor);
                    rom=ed->ew.doc->rom;
                    ed->schflag=1;
                    ov=ed->ew.doc->overworld;
                    
                    b2 = (uint8_t*) malloc(0x455);
                    
                    memset(b2,0,0x455);
                    
                    for(i = 0; i < 160; i++, ov++)
                    {
                        if(ov->win)
                        {
                            b5 = ov->buf;
                            
                            for(j = 0; j < 1008; j++)
                            {
                                if(j & 16)
                                    j += 16;
                                
                                l = (unsigned short) b5[j];
                                
                                if(l < 0x22a8)
                                    b2[l >> 3] |= 1 << (l & 7);
                            }
                        }
                        else
                        {
                            // Allocate a buffer large enough for a 512x512
                            // pixel map of map32 tiles.
                            
                            b5 = (uint16_t*) malloc(512);
                            
                            loadovermap(b5,i,1,rom);
                            for(j=0;j<512;j++)
                                if((l=(unsigned short)b5[j])<0x22a8) b2[l>>3]|=1<<(l&7);
                            free(b5);
                        }
                    }
                    
                    for(i = k = l = 0, j = 1; i < 0x22a8; i++)
                    {
                        if(!(b2[l] & j))
                            ed->schbuf[k++] = i;
                        
                        if(j == 128)
                            j = 1, l++;
                        else j <<= 1;
                    }
                    
                    free(b2);
                    
                    goto schok;
                }
                
                i = ShowDialog(hinstance,
                               (LPSTR) IDD_DIALOG19,
                               framewnd,
                               findblks,
                               (int) ed);
                
                if(i != 0)
                {
                    ed->schflag = 1;
                    ed->schbuf = malloc(0x4550);
                    SetCursor(wait_cursor);
                    
                    k = 0;
                    rom = ed->ew.doc->rom;
                    p = 0;
                    
                    for(l = 0; l < 4; l++)
                        if(ed->schyes[l] != -1)
                            p |= 1 << l;
                    
                    for(i = 0; i < 0x22a8; i++)
                    {
                        Getblock32(rom, i, o);
                        n = 0;
                        q = 0;
                        
                        for(j = 0; j < 4; j++)
                        {
                            m = o[j] + 0xea8;
                            
                            if(ed->selsch[m >> 3] & (1 << (m & 7)))
                                goto contsch;
                            
                            if(ed->selsch[ (m + 0xea8) >> 3 ] & (1 << (m & 7)))
                                q = 1;
                            
                            for(l = 0; l < 4; l++)
                            {
                                m = ed->schyes[l];
                                
                                if(m == -1)
                                    break;
                                if(m == o[j])
                                    n |= 1 << l;
                            }
                        }
                        
                        if(n == p && q)
                            ed->schbuf[k++] = i;
contsch:;
                    }
schok:
                    
                    if(wparam == SD_OverAdjustSearchBtn)
                    {
                        j = m = 0;
                        
                        for(i = 0; i < k; i++)
                        {
                            l = ed->schbuf[i];
                            
                            while(b4[j] < l)
                            {
                                if(j == ed->schnum)
                                    goto sc2done;
                                
                                j++;
                            }
                            
                            if(b4[j] == l)
                                ed->schbuf[m++] = l;
                        }
sc2done:
                        k = m;
                        
                        free(b4);
                    }
                    
                    ed->schnum = k;
                    SetCursor(normal_cursor);
                    
                    if(wparam == SD_OverMapSearchBtn)
                    {
                        SetWindowText((HWND)lparam, "Show all");
                        ShowWindow(GetDlgItem(win, SD_OverAdjustSearchBtn), SW_SHOW);
                    }
updsel32:
                    hc = GetDlgItem(win, 3000);
                    InvalidateRect(hc, 0, 1);
                    Updatesize(hc);
                }
            }
            
            break;
        
        case SD_OverAdjustSearchBtn:
            
            b4 = ed->schbuf;
            goto search2;
        }
        
        break;
        
    case WM_DESTROY:
        
        ed = (OVEREDIT*) GetWindowLong(win, DWL_USER);
        
        Delgraphwin((DUNGEDIT*) ed);
        
        free(ed->ov->buf);
        
        for(i = 0; i < 15; i++)
            Releaseblks(ed->ew.doc, ed->blocksets[i]);
        
        Releaseblks(ed->ew.doc, 0x79);
        Releaseblks(ed->ew.doc, 0x7a);
        
        ed->ov->win = 0;
        
        free(ed);
        
        break;
    }
    
    return FALSE;
}
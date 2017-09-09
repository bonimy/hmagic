
    #include "structs.h"

    #include "Callbacks.h"
    #include "prototypes.h"
    #include "wrappers.h"

    #include "DungeonEnum.h"

// =============================================================================

// \task Needs header too
extern HPEN tint_pen;
extern HBRUSH tint_br;

// \task end externs that need header.

// =============================================================================

int szcofs[16] =
{
    4,3,3,3,1,2,0,1,1,0,2,1,1,3,3,3
};

// =============================================================================

// Is this the one that draws the dungeon?
void Updatemap(DUNGEDIT *ed)
{
    int i;
    
    uint16_t *nbuf = ed->nbuf, *buf2;
    
    uint8_t const * buf = ed->buf;
    
    uint8_t const * const rom = ed->ew.doc->rom;
    
    dm_buf = nbuf;
    
    buf2 = (uint16_t*) ( rom + 0x1b52 + ((buf[0] << 4) & 0xf0) );
    fill4x2(rom, ed->nbuf, buf2);
    
    buf2 = (uint16_t*) ( rom + 0x1b52 + (buf[0] & 0xf0) );
    fill4x2(rom, ed->nbuf + 0x1000, buf2);
    
    ed->chestnum = 0;
    
    if(ed->ew.param < 0x8c)
    {
        Drawmap(rom,
                ed->nbuf,
                rom + romaddr(*(int*) (rom + romaddr(*(int*) (rom + 0x882d)) + (buf[1] >> 2) * 3)),
                ed);
        
        buf = Drawmap(rom, ed->nbuf, buf + 2, ed);
        buf = Drawmap(rom, ed->nbuf + 0x1000, buf, ed);
        
        Drawmap(rom, ed->nbuf, buf, ed);
        
        for(i = 0; i < 0x18c; i += 4)
        {
            if(*(short*)(rom + 0x271de + i) == ed->mapnum)
            {
                dm_x = (*(short*)(rom + 0x271e0 + i) & 0x3fff)>>1;
                
                ed->nbuf[dm_x]=0x1922;
                ed->nbuf[dm_x+64]=0x1932;
                ed->nbuf[dm_x+1]=0x1923;
                ed->nbuf[dm_x+65]=0x1933;
            }
        }
        
        for(i = 2; i < ed->tsize; i += 2)
        {
            dm_x = *(short*) (ed->tbuf + i) & 0x1fff;
            dm_x >>= 1;
            ed->nbuf[dm_x] = 0xde0;
            ed->nbuf[dm_x+64] = 0xdf0;
            ed->nbuf[dm_x+1] = 0x4de0;
            ed->nbuf[dm_x+65] = 0x4df0;
        }
    }
    else
        Drawmap(rom,ed->nbuf,buf+2,ed);
}

// =============================================================================

void
PaintDungeonBlocks(RECT       const clip_r,
                   RECT       const data_r,
                   HDC        const p_dc,
                   DUNGEDIT * const ed)
{
    HDC const mem_dc = CreateCompatibleDC(p_dc);
    
    HGDIOBJ tiny_bm = 0; 

    HBITMAP const mem_bm = CreateCompatibleBitmap(p_dc,
                                                  512,
                                                  512);
    
    
    // \task Temporary. We'd prefer to have a const 'ed' object.
    ed->bmih.biWidth  = 512;
    ed->bmih.biHeight = 512;
    
    SetDIBits(mem_dc,
              mem_bm,
              0,
              512,
              ed->map_bits,
              (BITMAPINFO*) &ed->bmih,
              ed->hpal ? DIB_PAL_COLORS : DIB_RGB_COLORS);
    
    tiny_bm = SelectObject(mem_dc, mem_bm);
    
    BitBlt(p_dc,
           clip_r.left,
           clip_r.top,
           clip_r.right - clip_r.left,
           clip_r.bottom - clip_r.top,
           mem_dc,
           data_r.left,
           data_r.top,
           SRCCOPY);
    
    DeleteDC(mem_dc);
    
    DeleteObject(mem_bm);
    DeleteObject(tiny_bm);
}

// =============================================================================

void
DrawDungeonBlock(DUNGEDIT * const ed,
                 int        const x,
                 int        const y,
                 int        const n,
                 int        const t)
{
    register unsigned char *b1 = 0,
                           *b3 = 0,
                           *b4 = 0;
    
    int a,
        b,
        c;
    
    int const scan_size = 512;
    
    // which chr (character / tile) to use.
    int d = (n & 0x03ff);
    
    BOOL const vflip = (n & 0x8000);
    
    unsigned e;
    
    const static char f[14] = {1,1,0,0,0,0,0,0,1,1,1,0,1,1};
    
    uint8_t * const drawbuf = ed->map_bits;
    
    // \task I hate writing code like this. But basically, these bitmaps are
    // drawn upside down.
    uint8_t * b2 = ( drawbuf + (511 * 512) + x - (y * 512) );
    
    int col;
    int mask,tmask;
    
    // -----------------------------
    
    *(char*) &col = *(((char*)&col) + 1) = *(((char*)&col) + 2) = *(((char*)&col) + 3) = ((n & 0x1c00) >> 6);
    
    if((t & 24) == 24)
    {
        // \task Used with only with Link's graphics dialog, seemingly.
        
        b3 = ed->ew.doc->blks[225].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + (n << 6);
        mask = 0xf0f0f0f;
        col = 0;
        b4 = b1 + 0xe000;
    }
    else if(t & 16)
    {
        // \task 2bpp graphics? Not sure.
        // Used with the dungeon map screen (not the maps themselves)
        
        if(d >= 0x180)
            goto noblock;
        
        b3 = ed->ew.doc->blks[bg3blkofs[d >> 7]].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + ((d & 0x7f) << 6);
        mask = 0x3030303;
        col >>= 2;
        
        if(n & 0x4000)
            b1 += 0x4000;
        
        b4 = b1 + 0x2000;
    }
    else if(t & 8)
    {
        // \task Used with a lot of the tilemap screens, title screen in particular.

        if(d >= 0x100)
            goto noblock;
        
        b3 = ed->ew.doc->blks[224].buf;
        
        if(!b3)
            goto noblock;
        
        b1 = b3 + (d << 6);
        mask = 0x3030303;
        col >>= 2;
        
        if(n & 0x4000)
            b1 += 0x8000;
        
        b4 = b1 + 0x4000;
    }
    else if(d >= 0x200)
    {
        if(d < 0x240)
            goto noblock;
        else if(d < 0x280)
        {
            b3 = ed->ew.doc->blks[ed->blocksets[10]].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + ((d - 0x240) << 6);
            mask = 0xf0f0f0f;
        }
        else if(d < 0x300)
        {
            b3 = ed->ew.doc->blks[0x79 + ( (d - 0x280) >> 6)].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + (((d - 0x280) & 63) << 6);
            mask = 0x7070707;
        }
        else
        {
            e = ed->blocksets[(d >> 6) - 1];
            b3 = ed->ew.doc->blks[e].buf;
            
            if(!b3)
                goto noblock;
            
            b1 = b3 + ((d & 0x3f) << 6);
            
            if(e >= 0xc5 && e < 0xd3)
                mask = f[e - 0xc5] ? 0xf0f0f0f : 0x7070707;
            else
                mask = 0x7070707;
        }
        
        if(n & 0x4000)
            b1 += 0x2000;
        
        b4 = b1 + 0x1000;
    }
    else if(ed->gfxtmp == 0xff)
    {
        mask = -1;
        col = 0;
        b1 = ed->ew.doc->blks[223].buf + (n << 6);
    }
    else
    {
        if(ed->gfxtmp >= 0x20)
            mask = palhalf[(n & 0x1c0) >> 6] ? 0xf0f0f0f : 0x7070707;
        else
            mask = (n & 0x100) ? 0x7070707 : 0xf0f0f0f;
        
        if(ed->anim != 3)
        {
            if(ed->gfxtmp >= 0x20)
            {
                e = d - 0x1c0;
                
                if(e < 0x20u)
                    d = (ed->anim << 5) + e + 0x200;
            }
            else
            {
                e = d - 0x1b0;
                if(e<0x20u) {
                    d=(ed->anim<<4)+e + 0x200;
                    if(e>=0x10) d+=0x30;
                }
            }
        }
        
        if(n & 0xff0000)
            b3 = ed->ew.doc->blks[(n >> 16) - 1].buf;
        else
            b3 = ed->ew.doc->blks[ed->blocksets[d >> 6]].buf;
        
        if(!b3)
        {
noblock:
            
            if(t & 1)
                return;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = 0;
                ((int*) b2)[1] = 0;
                
                b2 -= scan_size;
            }
            return;
        }
        
        b1 = b3 + ((d & 0x3f) << 6);
        
        if(n & 0x4000)
            b1 += 0x2000;
        
        b4 = b1 + 0x1000;
    }
    
    if(t & 4)
        col += 0x80808080;
    
    switch(t & 3)
    {
    
    case 0:
        
        if(vflip)
        {
            b2 -= scan_size * 7;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2     = ((*(int*) b1)     & mask) + col;
                ((int*) b2)[1] = ((((int*) b1)[1]) & mask) + col;
                
                b2 += scan_size;
                b1 += 8;
            }
        }
        else
        {
            for(a = 0; a < 8; a++)
            {
                *(int*) b2     = ((*(int*) b1)     & mask) + col;
                ((int*) b2)[1] = ((((int*) b1)[1]) & mask) + col;
                b2 -= scan_size;
                b1 += 8;
            }
        }
        
        break;
    
    case 1:
        
        if(vflip)
        {
            b2 -= scan_size * 7;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0];
                c = ((int*) b4)[1];
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c;
                
                b2 += scan_size;
                b1 += 8;
                b4 += 8;
            }
        }
        else
        {
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0];
                c = ((int*) b4)[1];
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c;
                
                b2 -= scan_size;
                b1 += 8;
                b4 += 8;
            }
        }
        
        break;
    
    case 2:
        
        // \task Seems to be broken?
        if(vflip)
        {
            b2 -= scan_size * 7;
            
            tmask = 0xff00ff00;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = (((*(int*) b1) & mask) + col) & tmask;
                ((int*) b2)[1] = (((((int*) b1)[1]) & mask) + col) & tmask;
                
                b2 += scan_size;
                b1 += 8;
                
                tmask ^= -1;
            }
        }
        else
        {
            tmask = 0xff00ff;
            
            for(a = 0; a < 8; a++)
            {
                *(int*) b2 = (((*(int*) b1) & mask) + col) & tmask;
                ((int*) b2)[1] = (((((int*) b1)[1]) & mask) + col) & tmask;
                
                b2 -= scan_size;
                b1 += 8;
                tmask ^= -1;
            }
        }
        
        break;
    
    case 3:
        
        // \task Seems to be broken?
        if(vflip)
        {            
            tmask = 0xff00ff00;
            
            b2 -= scan_size * 7;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0] | ~tmask;
                c = ((int*) b4)[1] | ~tmask;
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b & tmask;
                ((int*) b2)[1] |= (((*(int*) (b1 + 4)) & mask) + col) & ~c & tmask;
                
                b2 += scan_size;
                b1 += 8;
                b4 += 8;
                
                tmask ^= -1;
            }
        }
        else
        {
            tmask = 0xff00ff;
            
            for(a = 0; a < 8; a++)
            {
                b = ((int*) b4)[0] | ~tmask;
                c = ((int*) b4)[1] | ~tmask;
                
                ((int*) b2)[0] &= b;
                ((int*) b2)[1] &= c;
                
                ((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b & tmask;
                ((int*) b2)[1] |= (((*(int*)(b1 + 4)) & mask) + col) & ~c & tmask;
                
                b2 -= scan_size;
                b1 += 8;
                b4 += 8;
                
                tmask ^= -1;
            }
        }
        
        break;
    }
}

// =============================================================================

static void
DrawDungeonMap8(DUNGEDIT * const p_ed,
                int        const p_x,
                int        const p_y)
{
    uint16_t const * const tilemap = p_ed->nbuf;
    
    int t = (p_x >> 3) | (p_y << 3);
    
    int u = 0;
    
    // -----------------------------
    
    switch(p_ed->layering >> 5)
    {
    
    case 0:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t], u);
        }
        
        break;
    
    case 1:
    case 5:
    case 6:
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t + 0x1000], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t],u);
        }
        
        break;
    
    case 2:
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t + 0x1000], u + 2);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t], u);
        }
        
        break;
    
    case 3:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t + 0x1000], u);
        }
        
        break;
    
    case 4:
    case 7:
        
        if(p_ed->disp & SD_DungShowBG1)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t], u);
            
            u = 1;
        }
        
        if(p_ed->disp & SD_DungShowBG2)
        {
            DrawDungeonBlock(p_ed, p_x, p_y, tilemap[t + 0x1000], u + 2);
        }
        
        break;
    }
}

// =============================================================================

static void
PaintDungeon(DUNGEDIT * const p_ed,
             HDC        const hdc,
             RECT       const clip_r,
             RECT       const data_r)
{
    // loop variable that represents the y coordinate in the tilemap.
    int i = 0;
    
    // Seems to be nonzero if one of the backgrounds is disabled or
    // full of blackness.
    int v;
    
    RECT tile_r =
    {
        data_r.left   & ~0x7,
        data_r.top    & ~0x7,
        (data_r.right + 7) & ~0x7,
        (data_r.bottom + 7) & ~0x7
    };
    
    HGDIOBJ oldobj = 0;
    
    // -----------------------------
    
    if
    (
        ( ! (p_ed->disp & SD_DungShowBothBGs) )
     || ( ( ! (p_ed->disp & SD_DungShowBothBGs) ) && ! (p_ed->layering >> 5) )
    )
    {
        oldobj = SelectObject(hdc, black_brush);
        
        v = 1;
    }
    else
    {
        v = 0;
    }
    
#if 1
    for(i = tile_r.left; i < tile_r.right; i += 8)
#else
    for(i = 0; i < 512; i += 8)
#endif
    {
        // loop variable that represents the current x coordinate in the tilemap.
        // (aligned to a 32 pixel grid though).
        int j = 0;
        
        // -----------------------------
        
#if 1
        for(j = tile_r.top; j < tile_r.bottom; j += 8)
#else
        for(j = 0; j < 512; j += 8)
#endif
        {
            if(v)
            {
                Rectangle(hdc, i, j, i + 8, j + 8);
            }
            else
            {
                DrawDungeonMap8(p_ed, i, j);
            }
        }
    }
    
    PaintDungeonBlocks(clip_r, data_r, hdc, p_ed);
    
    if(oldobj)
    {
        SelectObject(hdc, oldobj);
    }
}


// =============================================================================

void
DungeonMap_OnPaint(DUNGEDIT * const p_ed,
                   HWND       const p_win)
{
    char text_buf[0x200];
    
    uint8_t const * const rom = p_ed->ew.doc->rom;
    
    RECT rc;
    
    PAINTSTRUCT ps;
    
    HDC const hdc = BeginPaint(p_win, &ps);
    
    HPALETTE const oldpal = SelectPalette(hdc, p_ed->hpal, 1);
    
    HGDIOBJ const oldfont = SelectObject(hdc, trk_font);
    
    int const n = p_ed->mapscrollh << 5;
    int const o = (p_ed->mapscrollv * p_ed->map_vscroll_delta);
    
    int k = ((ps.rcPaint.right + 31) & 0xffffffe0);
    int l = ((ps.rcPaint.bottom + 31) & 0xffffffe0);
    
    int so = 0;
    
    RECT clip_r = ps.rcPaint;
    
    RECT data_r =
    {
        ps.rcPaint.left + n,
        ps.rcPaint.top + o,
        ps.rcPaint.right + n,
        ps.rcPaint.bottom + o,
    };
    
    // -----------------------------
    
    if( HM_IsEmptyRect(clip_r) )
    {
        EndPaint(p_win, &ps);

        return;
    }

    
    if( ! tint_pen) { tint_pen = CreatePen(PS_SOLID, 3, RGB(0x90, 0xff, 0x90) ); }
    if( ! tint_br)  { tint_br = CreateSolidBrush( RGB(0x80, 0x80, 0x80) ); }
    
    RealizePalette(hdc);
    
    if(l + o > 0x200)
        l = 0x200 - o;
    
    if(k + n > 0x200)
        k = 0x200 - n;
    
    if
    (
        ( (clip_r.left + n) >= 0x200)
     || ( (clip_r.top + o) >= 0x200)
    )
    {
        EndPaint(p_win, &ps);
        
        return;
    }
    
    if(data_r.right > 0x200)
    {
        data_r.right = 0x200;
    }
    
    if(data_r.bottom > 0x200)
    {
        data_r.bottom = 0x200;
    }
    
    PaintDungeon(p_ed,
                 hdc,
                 clip_r,
                 data_r);
    
    if(p_ed->disp & SD_DungShowMarkers)
    {
        int i = 0;
        
        HGDIOBJ const oldobj2 = SelectObject(hdc,purple_brush);
        
        SetBkMode(hdc, TRANSPARENT);
        
        for(i = 1; i < p_ed->esize; i += 3)
        {
            int const j = ( (p_ed->ebuf[i + 1] & 31) << 4) - n;
            int const k = ( (p_ed->ebuf[i] & 31) << 4) - o;
            
            Ellipse(hdc, j, k, j + 16, k + 16);
            
            strcpy(text_buf, Getsprstring(p_ed, i));
            
            PaintSprName(hdc,j,k,n,o,512, text_buf);
        }
        
        SelectObject(hdc, red_brush);
        
        for(i = 0; i < p_ed->ssize; i += 3)
        {
            int const k_pre = ldle16b(p_ed->sbuf + i);
            
            int const k = ((k_pre & 0x1f80) >> 4) - o;
            int const j = ((k_pre & 0x7e) << 2) - n;
            
            Ellipse(hdc, j, k, j + 16, k + 16);
            
            strcpy(text_buf, Getsecretstring(rom, p_ed->sbuf[i + 2]));
            
            PaintSprName(hdc, j, k, n, o, 512, text_buf);
        }
        
        SelectObject(hdc, oldobj2);
    }
    
    so = p_ed->selobj;
    
    if(so)
    {
        if(p_ed->selchk==9)
        {
            dm_x=(*(short*)(p_ed->tbuf+so)>>1)&0xfff;
        }
        else if(p_ed->selchk==8)
        {
            dm_x=(*(short*)(rom + so + 2)>>1)&0xfff;
        }
        else if(p_ed->selchk==7)
        {
            dm_x=(*(short*)(p_ed->sbuf+so-2)>>1)&0xfff;
            dm_k=p_ed->sbuf[so];
            cur_sec = Getsecretstring(rom, dm_k);              
        }
        else if(p_ed->selchk==6)
        {
            dm_x=((p_ed->ebuf[so+1]&31)<<1)+((p_ed->ebuf[so]&31) << 7);
            dm_k = p_ed->ebuf[so + 2] + ( (p_ed->ebuf[so + 1] >= 224) ? 256 : 0);
        }
        else if(p_ed->selchk & 1)
        {
            getdoor(p_ed->buf+so, rom),
            k = 4,
            l = 4;
        }
        else
            getobj(p_ed->buf+so);
        
        rc = p_ed->selrect;
        
        rc.left-=n;
        rc.top-=o;
        rc.right-=n;
        rc.bottom-=o;
        
        if(rc.right>ps.rcPaint.left && rc.left<ps.rcPaint.right &&
            rc.bottom>ps.rcPaint.top && rc.top<ps.rcPaint.bottom)
        {
            RECT rc2;
            
            if(rc.right+n>512)
                rc2.right=512-n;
            
            if(rc.bottom+o>512)
                rc2.bottom=512-n;
            
            if
            (
                p_ed->selchk >= 6
             && p_ed->selchk < 8
             && (p_ed->disp & SD_DungShowMarkers)
            )
            {
                HGDIOBJ const oldobj2 = SelectObject(hdc, green_pen);
                HGDIOBJ const oldobj3 = SelectObject(hdc, black_brush);
                
                // -----------------------------
                
                HM_DrawRectangle(hdc, rc);
                
                if(p_ed->selchk==6)
                    strcpy(text_buf, Getsprstring(p_ed, so));
                else
                    strcpy(text_buf, Getsecretstring(rom, dm_k));
                
                PaintSprName(hdc,rc.left,rc.top,n,o,512, text_buf);
                SelectObject(hdc,oldobj2);
                SelectObject(hdc,oldobj3);
            }
            else
            {
                
#if 1
                BOOL r = 0;
                
                HDC other_dc = CreateCompatibleDC(hdc);
                
                unsigned width = rc.right - rc.left;
                unsigned height = rc.bottom - rc.top;
                
                HBITMAP other_bm = CreateCompatibleBitmap(hdc,
                                                          width,
                                                          height);
                
                HGDIOBJ const pen = tint_pen;
                HGDIOBJ const br = tint_br;
                
                HGDIOBJ const old_pen = SelectObject(other_dc, pen);
                HGDIOBJ const old_br = SelectObject(other_dc, br);
                
                static int r2_func = 0;
                static int r2_inc  = 0;
                
                BLENDFUNCTION bf = { AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA };
                
                HGDIOBJ tiny_bm = SelectObject(other_dc, other_bm);
                
                RECT rc2 = { 0, 0, width, height };
                
#if 1
                BitBlt(other_dc,
                       rc2.left,
                       rc2.top,
                       width,
                       height,
                       hdc,
                       rc.left,
                       rc.top,
                       SRCCOPY);
                
                r = FrameRect(other_dc, &rc2, green_brush);
#else
                r = Rectangle(other_dc, rc2.left, rc2.top, rc2.right, rc2.bottom);
#endif
                
                
                bf.BlendOp = AC_SRC_OVER;
                bf.BlendFlags = 0;
                bf.SourceConstantAlpha = 0x80;
                bf.AlphaFormat = 0;
                
                r = AlphaBlend(hdc,
                           rc.left,
                           rc.top,
                           width,
                           height,
                           other_dc,
                           0,
                           0, 
                           width,
                           height,
                           bf);
                
                
                SelectObject(other_dc, old_pen);
                SelectObject(other_dc, old_br);
                
                DeleteObject(other_bm);
                DeleteObject(tiny_bm);
                
                DeleteDC(other_dc);
                
                // r = FrameRect(hdc, &rc, blue_brush);
                
#else
                FrameRect(hdc,
                          &rc,
                          (p_ed->withfocus & 1) ? green_brush : gray_brush);
#endif
            }
        }
    }
    
    SelectObject(hdc, oldfont);
    
    SelectPalette(hdc, oldpal, 1);
    
    EndPaint(p_win, &ps);
}

// =============================================================================

void
DungeonMap_OnMouseWheel(DUNGEDIT * const p_ed,
                        MSG        const p_msg)
{
    HM_MouseWheelData const d = HM_GetMouseWheelData(p_msg.wParam,
                                                     p_msg.lParam);
    
    unsigned scroll_type = SB_LINEUP;
    
    WPARAM fake_wp = 0;
    
    HWND const w = p_msg.hwnd;
    
    SCROLLINFO si_v = HM_GetVertScrollInfo(p_msg.hwnd);
    
    // -----------------------------
    
    if(d.m_distance > 0)
    {
        // wheel moving up or left
        if(d.m_control_key)
        {
            scroll_type = SB_PAGEUP;
        }
        else
        {
            scroll_type = SB_LINEUP;
        }
    }
    else
    {
        if(d.m_control_key)
        {
            scroll_type = SB_PAGEDOWN;
        }
        else
        {
            scroll_type = SB_LINEDOWN;
        }
    }             
    
    fake_wp = MAKEWPARAM(scroll_type, 0);
    
    p_ed->mapscrollv = Handlescroll(w,
                                    fake_wp,
                                    p_ed->mapscrollv,
                                    p_ed->mappagev,
                                    SB_VERT,
                                    (si_v.nMax - si_v.nMin) + 1,
                                    p_ed->map_vscroll_delta);
}

// =============================================================================

void
DungeonMap_OnLeftMouseDown(DUNGEDIT * const p_ed,
                           MSG        const p_packed_msg)
{
    // What to do if the left mouse button goes down.
    
    // Obtains the rom and other data structures associated with this room.
    // notes: o is the x coordinate of the click, p is the y coordinate
    // q is the size of the object we're looking for (3 bytes or 2 bytes)
    // m is the number of objects of that type in the array we're looking through
    // n is a counter to step through those objects
    // i is flag variable (bit 0: it's a door (type 2 object)
    
    HM_MouseData const d = HM_GetMouseData(p_packed_msg);
    
    
    
    HWND const w = p_packed_msg.hwnd;
     
    int const o = d.m_rel_pos.x + (p_ed->mapscrollh << 5);
    int const p = d.m_rel_pos.y + (p_ed->mapscrollv * p_ed->map_vscroll_delta);
    
    uint8_t const * const rom = p_ed->ew.doc->rom;
    
    if(p_ed->selcorner)
    {
        p_ed->withfocus |= 8;
        p_ed->dragx = (o + 4) & -8;
        p_ed->dragy = (p + 4) & -8;
        p_ed->sizerect = p_ed->selrect;
        
        SetCapture(w);
    }
    else
    {
        int i = 0;
        int m = 0;
        int n = 0;
        int q = 0;
        
        // -----------------------------
            
        if
        (
            p_ed->selobj
         && o >= p_ed->selrect.left
         && o < p_ed->selrect.right
         && p >= p_ed->selrect.top
         && p < p_ed->selrect.bottom
        )
        {
            goto movesel;
        }
        
        Dungselectchg(p_ed, w, 0);
        
        // If it's a layout, handle only type 1 objects
        if(p_ed->ew.param >= 0x8c)
        {
            i = 0, q = 3;
        }
        else
        {
            i = p_ed->selchk;
            
            if(i < 6)
            {
                i &= 6;
                
                if(p_ed->chkofs[i + 2] != p_ed->chkofs[i + 1])
                    i |= 1;
                
                q = 3 - (i & 1);
            }
        }
        
        if(i == 9)
            n = 2, m = p_ed->tsize - 2, q = 2;
        else if(i == 8)
            n = 0x271de, m = 0x27366, q = 4;
        else if(i == 7)
            n = 0, m = p_ed->ssize - 3, q = 3;
        else if(i == 6)
            n = 1, m = p_ed->esize - 3, q = 3;
        else
            n = p_ed->chkofs[i & 6], m = p_ed->chkofs[i + 1] - 2 - q;
        
        p_ed->selobj=0;
        
        for(;m >= n; m -= q)
        {
            RECT rc;
            
            // -----------------------------
            
            if(i < 6 && (i & 1) && m < p_ed->chkofs[i])
            {
                i--, q = 3, m -= 3;
                
                if(m < n)
                    break;
            }
            
            if(i == 9)
            {
                dm_x = (*(short*) (p_ed->tbuf + m) >> 1) & 0xfff;
            }
            else if(i == 8)
            {
                if(*(short*) (rom + m) != p_ed->mapnum)
                    continue;
                
                dm_x = (*(short*) (rom + m + 2) >> 1) & 0xfff;
            }
            else if(i == 7)
            {
                dm_x = *(short*) (p_ed->sbuf + m) >> 1;
                dm_k = p_ed->sbuf[m + 2];
                cur_sec = Getsecretstring(rom, dm_k);
            }
            else if(i == 6)
            {
                dm_x = ((p_ed->ebuf[m] & 31) << 7) + ((p_ed->ebuf[m + 1] & 31) << 1);
                
                dm_l = ((p_ed->ebuf[m] & 0x60) >> 2) | ((p_ed->ebuf[m + 1] & 0xe0) >> 5);
                dm_k = p_ed->ebuf[m + 2] + (((dm_l & 7) == 7) ? 256 : 0);
            }
            else if(i & 1)
                getdoor(p_ed->buf + m, rom);
            else
                getobj(p_ed->buf + m);
            
            rc.left = ((dm_x & 0x3f) << 3);
            rc.top = ((dm_x >> 6) << 3);
            
            Getdungobjsize(i, &rc, 0, 0, 0);
            
            if(o >= rc.left && o < rc.right && p >= rc.top && p < rc.bottom)
            {
                if(i == 7)
                    m += 2;
                
                p_ed->selobj = m;
                p_ed->selchk = i;
                
            movesel:
                
                p_ed->withfocus |= 2;
                p_ed->dragx = o;
                p_ed->dragy = p;
                
                SetCapture(w);
                
                break;
            }
        }
        
        Dungselectchg(p_ed, w, 1);
    }
    
    SetFocus(w);
}

// =============================================================================

LRESULT CALLBACK
dungmapproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    // Handles the actual dungeon map portion
    // of the dungeon dialog
    
    char text_buf[0x200];
    
    SCROLLINFO si;
    
    int i, j, k, l, m, n, o, p, q = 0;
    
    DUNGEDIT  * const ed = (DUNGEDIT*) GetWindowLong(win, GWL_USERDATA);
    RECT rc;
    HMENU menu,menu2;
    POINT pt;
    
    unsigned char * rom = 0;
    
    MSG const packed_msg = HM_PackMessage(win, msg, wparam, lparam);

    // Some messages require a valid editor pointer to do anything useful.
    switch(msg)
    {
    
    default:
        
        break;
    
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_SIZE:
    case WM_MOUSEWHEEL:
    case WM_VSCROLL:
    case WM_HSCROLL:
    case WM_CHAR:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_PAINT:
        
        if( ! ed )
        {
            return 0;
        }
        
        break;
    }
    
    switch(msg)
    {
    
    case WM_LBUTTONUP:
        
        if(ed->withfocus & 10)
        {
            ReleaseCapture();
            ed->withfocus &= -15;
            ed->selcorner = 16;
            
            goto updcursor;
        }
        
        break;
    
    case WM_MOUSEMOVE:
    updcursor:
        
        {
            HM_MouseMoveData d = HM_GetMouseMoveData(win, wparam, lparam);
            
            char info[0x200];
            
            sprintf(info, "X: %d, Y: %d", d.m_rel_pos.x / 8, d.m_rel_pos.y / 8);
            
            SetDlgItemText(debug_window, IDC_STATIC2, info);
            
            o = d.m_rel_pos.x + (ed->mapscrollh << 5);
            p = d.m_rel_pos.y + (ed->mapscrollv * ed->map_vscroll_delta);
        }
        
        if(ed->withfocus & 2)
        {
            if(o > ed->dragx + 7 || o < ed->dragx || p > ed->dragy+7 || p < ed->dragy)
            {
                if(ed->selchk == 9)
                    dm_x = *(short*)(ed->tbuf+ed->selobj) >> 1;
                else if(ed->selchk == 8)
                    dm_x = *(short*)(ed->ew.doc->rom + ed->selobj + 2) >> 1;
                else if(ed->selchk == 7)
                    dm_x = *(short*)(ed->sbuf + ed->selobj - 2) >> 1;
                else if(ed->selchk == 6)
                {
                    dm_x = ( (ed->ebuf[ed->selobj] & 31) << 7)
                         + ( (ed->ebuf[ed->selobj  + 1] & 31) << 1);
                }
                else if(ed->selchk & 1)
                    break;
                else
                    getobj(ed->buf + ed->selobj);
                
                i = (o - ed->dragx) >> 3;
                j = (p - ed->dragy) >> 3;
                
                if(!(i||j))
                    break;
                
                if(ed->selchk == 6)
                    i &= -2, j &= -2;
                
                k = (dm_x & 0x3f) + i;
                l = ((dm_x >> 6) & 0x3f) + j;
                
                if(k > 0x3f || k < 0 || l > 0x3f || l < 0)
                    ed->withfocus |= 4;
                else
                {
                    Dungselectchg(ed,win,0);
                    dm_x &= 0x6000;
                    dm_x |= k + (l << 6);
                    
                    if(ed->selchk==9)
                    {
                        *(short*)(ed->tbuf+ed->selobj)=dm_x<<1;
                        goto modfx;
                    }
                    else if(ed->selchk == 8)
                    {
                        *(short*)(ed->ew.doc->rom + ed->selobj+2) = dm_x << 1;
                        goto ncursor;
                    }
                    else if(ed->selchk == 7)
                    {
                        *(short*)(ed->sbuf+ed->selobj-2) = dm_x<<1;
                        goto modfx;
                    }
                    else if(ed->selchk & 1)
                        setdoor(ed->buf + ed->selobj), ed->modf = 1;
                    else if(ed->selchk == 6)
                    {
                        if(dm_l >= 0x18 && dm_x > 0xfc0)
                            ed->withfocus |= 4;
                        else
                        {
                            ed->ebuf[ed->selobj] = (ed->ebuf[ed->selobj] & 224) + ((dm_x >> 7) & 31);
                            ed->ebuf[ed->selobj+1] = (ed->ebuf[ed->selobj + 1] & 224) + ((dm_x >> 1) & 31);
modfx:
                            ed->modf = 1;
ncursor:
                            if(ed->withfocus & 4)
                            {
                                SetCursor(normal_cursor);
                                ed->withfocus &= -5;
                            }
                        }
                    }
                    else
upddrag:
                        setobj(ed,ed->buf+ed->selobj);
                }
                
                if(ed->withfocus & 4)
                    SetCursor(forbid_cursor);
                else
                {
                    ed->dragx += i << 3;
                    ed->dragy += j << 3;
                    
                    goto updmap;
                }
            }
        }
        else
        {
            i = 0;
            
            if(ed->withfocus & 8)
            {
                o = (o + 4) & -8;
                p = (p + 4) & -8;
                i = o - ed->dragx;
                j = p - ed->dragy;
                
                if(!(i||j))
                    break;
                
                k = ed->objt;
                l = obj3_t[k];
                n = obj3_m[k];
                rc = ed->sizerect;
                m = ed->selcorner;
                
                if(m & 1)
                    rc.left = o;
                
                if(m & 2)
                    rc.right = o;
                
                if(m & 4)
                    rc.top = p;
                
                if(m & 8)
                    rc.bottom = p;
                
                p = ( (rc.right - rc.left) >> 3) - obj3_w[k];
                q = ( (rc.bottom - rc.top) >> 3) - obj3_h[k];
                
                switch(l & 192)
                {
                case 0:
                    
                    o = p;
                    
                    break;
                
                case 64:
                    
                    o = 0;
                    
                    if(m & 3)
                        o = p;
                    
                    if((m & 12) && q > o)
                        o = q;
                    
                    break;
                
                case 128:
                    
                    o = q;
                    
                    break;
                }
                
                o /= obj3_m[k];
                
                switch(l & 15)
                {
                case 0: case 2:
                    if(o>16) o=0;
                    else if(o<1) o=1;
                    break;
                case 1:
                    if(o>16) o=15;
                    else if(o<1) o=0;
                    else o--;
                    break;
                case 3:
                    if(o>4) o=3;
                    else if(o<1) o=0;
                    else o--;
                    o<<=2;
                    q/=obj3_m[k];
                    if(q>4) o+=3;
                    else if(q>=1) o+=q-1;
                    break;
                case 4:
                    o-=4;
                    o>>=3;
                    if(o<0) o=0;
                    if(o>3) o=3;
                    if(q>32) o+=12;
                    else if(q>24) o+=8;
                    else if(q>20) o+=4;
                    break;
                }
                
                if(o == ed->objl)
                    break;
                
                Dungselectchg(ed,win,0);
                dm_k = k; // assign the object's type.
                dm_l = o; // assign the object's subtype
                rc.left=0;
                rc.top=0;
                Getdungobjsize(0,&rc,0,0,1);
                
                if(l == 20)
                    dm_x = (short) (((m&1)?ed->sizerect.right:(ed->sizerect.left+rc.right-rc.left))>>3)-3;
                else
                    dm_x = (short) (((m&1)?(ed->sizerect.right-rc.right+rc.left):ed->sizerect.left)>>3);
                
                if(l & 32)
                    dm_x += (short)
                    (
                        (
                            (
                                (m & 4) ? ed->sizerect.bottom
                                        : (ed->sizerect.top + rc.bottom - rc.top)
                            ) >> 3
                        )
                      - ( (l & 16) ? 1 : 5 )
                    ) << 6;
                else
                    dm_x += (short) (((m & 4) ? (ed->sizerect.bottom - rc.bottom + rc.top) : ed->sizerect.top)>>3<<6);
                goto upddrag;
            }
            else
            {
                i=0;
                
                if(ed->selchk < 6 && !(ed->selchk&1) && ed->selobj && ed->objt < 0xf8)
                {
                    k = ed->objt;
                    j = obj3_t[k];
                    
                    if(obj3_m[k])
                    {
                        rc = ed->selrect;
                        
                        if(j < 128 && p >= rc.top - 2 && p < rc.bottom + 2)
                        {
                            if(o >= rc.left - 2 && o < rc.left + 2) i |= 1;
                            if(o < rc.right + 2 && o >= rc.right - 2) i |= 2;
                        }
                        if((j>=64 || ((j&15)>=3)) && o>=rc.left-2 && o<rc.right+2) {
                            if(p>=rc.top-2 && p<rc.top+2) i|=4;
                            if(p<rc.bottom+2 && p>=rc.bottom-2) i|=8;
                        }
                    }
                }
//              if(ed->selcorner!=i) {
                    ed->selcorner=i;
                    SetCursor(sizecsor[szcofs[i]]);
//              }
            }
        }
        break;
    
    case WM_LBUTTONDOWN:
        
        DungeonMap_OnLeftMouseDown(ed, packed_msg);
        
        break;
    
    case WM_ACTIVATE:
        
        if(wparam == WA_ACTIVE)
            SetFocus(win);
        
        break;
    
    case WM_SIZE:
        
        si.cbSize = sizeof(si);
        
        si.fMask = SIF_RANGE | SIF_PAGE;
        
        {
            unsigned const height = HIWORD(lparam);
            
            unsigned const divider = 9;
            
            si.nMin = 0;
            si.nMax = ( ( (512 + 1024) / divider ) - 1 );
            
            si.nPage = (height / divider);
            
            ed->mappagev = si.nPage;
            ed->map_vscroll_delta = divider;
        }
        
        SetScrollInfo(win, SB_VERT, &si, 1);
        
        si.nMin = 0;
        si.nMax = 15;
        
        si.nPage = (lparam & 65535) >> 5;
        ed->mappageh = si.nPage;
        
        SetScrollInfo(win, SB_HORZ, &si, 1);
        
        ed->mapscrollv = Handlescroll(win,
                                      -1,
                                      ed->mapscrollv,
                                      ed->mappagev,
                                      SB_VERT,
                                      16,
                                      ed->map_vscroll_delta);
        
        ed->mapscrollh = Handlescroll(win,
                                      -1,
                                      ed->mapscrollh,
                                      ed->mappageh,
                                      SB_HORZ,
                                      16,
                                      32);
        
        break;
    
    case WM_MOUSEWHEEL:
        
        DungeonMap_OnMouseWheel(ed, packed_msg);
        
        break;
    
    case WM_VSCROLL:
        
        {
            SCROLLINFO const si_v = HM_GetVertScrollInfo(win);
            
            ed->mapscrollv = Handlescroll(win,
                                          wparam,
                                          ed->mapscrollv,
                                          ed->mappagev,
                                          SB_VERT,
                                          (si_v.nMax - si_v.nMin) + 1,
                                          ed->map_vscroll_delta);
        }
        
        break;
    
    case WM_HSCROLL:
        
        ed->mapscrollh = Handlescroll(win,
                                      wparam,
                                      ed->mapscrollh,
                                      ed->mappageh,
                                      SB_HORZ,
                                      16,
                                      32);
        
        break;
    
    case WM_GETDLGCODE:
        
        return DLGC_WANTARROWS | DLGC_WANTCHARS;
    
    case WM_CHAR:
        
        if(wparam >= 64)
            wparam &= 223;
        
        if(!ed->selobj)
            break;
        
        if(ed->selchk > 7)
            break;
        
        if(ed->selchk == 7)
            switch(wparam)
            {
            
            case '-':
                
                ed->sbuf[ed->selobj-1]^=32;
                ed->modf=1;
                
                goto updsel;
            }
        else if(ed->selchk == 6)
            switch(wparam)
            {
            
            case ',':
            case '.':
                
                Dungselectchg(ed,win,0);
                dm_l=((ed->ebuf[ed->selobj]&0x60)>>2)|((ed->ebuf[ed->selobj+1]&0xe0)>>5);
                
                if(wparam==',')
                    dm_l--;
                else
                    dm_l++;
                
                i=(ed->ebuf[ed->selobj]&159)|((dm_l<<2)&96);
                
                if(i==255)
                    break;
                
                ed->ebuf[ed->selobj]=i;
                ed->ebuf[ed->selobj+1]=(ed->ebuf[ed->selobj+1]&31)|(dm_l<<5);
                ed->modf=1;
                
                goto updsel;
            
            case '-':
                
                i=ed->ebuf[ed->selobj]^128;
                
                if(i==255)
                    break;
                
                ed->ebuf[ed->selobj]=i;
                ed->modf=1;
                
                goto updsel;
        }
        else if(!(ed->selchk & 1))
            switch(wparam)
            {
            
            case '.':
                
                getobj(ed->buf + ed->selobj);
                
                if(dm_k > 0xff)
                    break;
                
                dm_l++;
                
                goto upd;
            
            case ',':
                
                getobj(ed->buf+ed->selobj);
                
                if(dm_k > 0xff)
                    break;
                
                dm_l--;
                
                goto upd;
            
            case '-':
                
                getobj(ed->buf + ed->selobj);
                
                dm_k ^= 0x100;
                
                if(dm_k >= 0x100)
                    dm_k &= 0x13f;
                else
                    dm_l = 0;
                
                goto upd;
            
            case 2:
                
                j = ed->chkofs[ed->selchk];
            
            if(ed->selobj == j)
                break;
            
            i = *(int*) (ed->buf + ed->selobj);
            
            memmove(ed->buf+j+3,ed->buf+j,ed->selobj-j);
            *(short*)(ed->buf+j)=i;
            ed->buf[j+2]=i>>16;
            ed->selobj=j;
            ed->modf=1;
            
            goto updmap;
        
        case 22:
            
            j=ed->chkofs[ed->selchk+1]-5;
            
            if(ed->selobj == ed->chkofs[ed->selchk + 1] - 5)
                break;
            
            i=*(int*)(ed->buf+ed->selobj);
            memcpy(ed->buf+ed->selobj,ed->buf+ed->selobj+3,j-ed->selobj);
            *(short*)(ed->buf+j)=i;
            
            ed->buf[j+2]=i>>16;
            ed->selobj=j;
            ed->modf=1;
            
            goto updmap;
        
        case 'B':
            
            if(ed->selobj == ed->chkofs[ed->selchk])
                break;
            
            i=*(int*)(ed->buf+ed->selobj-3);
            *(short*)(ed->buf+ed->selobj-3)=*(short*)(ed->buf+ed->selobj);
            
            ed->buf[ed->selobj-1]=ed->buf[ed->selobj+2];
            *(short*)(ed->buf+ed->selobj)=i;
            
            ed->buf[ed->selobj+2]=i>>16;
            ed->selobj-=3;
            ed->modf=1;
            
            goto updmap;
        
        case 'V':
            
            if(ed->selobj == ed->chkofs[ed->selchk + 1] - 5)
                break;
            
            i = *(int*) (ed->buf + ed->selobj + 3);
            *(short*)(ed->buf+ed->selobj+3)=*(short*)(ed->buf+ed->selobj);
            
            ed->buf[ed->selobj+5]=ed->buf[ed->selobj+2];
            *(short*)(ed->buf+ed->selobj)=i;
            
            ed->buf[ed->selobj+2]=i>>16;
            ed->selobj+=3;
            ed->modf=1;
            
            goto updmap;
        
        case '+': // Trying to add an item to a chest.
        case '\\': // Decrements the item value, '+' increments the item value.
            
            rom = ed->ew.doc->rom;
            
            for(k = 0; k < ed->chestnum; k++)
                if(ed->selobj == ed->chestloc[k])
                    break;
            
            if(k == ed->chestnum)
                break;
            
            for(l = 0; l < 0x1f8; l += 3)
            {
                // If the room index in the data matches the room index of the room we're looking at...
                if( (*(short*) (rom + 0xe96e + l) & 0x7fff) == ed->mapnum)
                {
                    k--;
                    
                    if(k < 0)
                    {
                        m = *(char*)(rom + 0xe970 + l);
                        
                        if(wparam == '\\') 
                            m++;
                        else
                            m--;
                        
                        if(m < 0)
                            m = 75;
                        
                        if(m > 75)
                            m = 0;
                        
chestchg:
                        
                        *(char*) (rom + 0xe970 + l) = m;
                        
                        Dungselectchg(ed, win, 1);
                        
                        break;
                    }
                }
            }
            
            // If we got to the end of the array without finding a match...
            if(l == 0x1f8)
            {
                for(l = 0; l < 0x1f8; l += 3)
                {
                    if( is16b_neg1(rom + 0xe96e + l) )
                    {
                        // Note that if it is a big chest (0xFB) we will set the MSBit.
                        *(short*)(rom + 0xe96e + l) = ed->mapnum | ((ed->buf[ed->selobj + 2] == 0xfb) ? 0x8000 : 0);
                        
                        // Give it a default value of zero (sword + blue shield)
                        m = 0;
                        
                        goto chestchg;
                    }
                }
                
                if(l == 0x1f8)
                    MessageBox(framewnd,"You can't add anymore chests.","Bad error happened",MB_OK);
            }
        }
        else
            switch(wparam)
            {
            case 2:
                j=ed->chkofs[ed->selchk];
                if(ed->selobj==j) break;
                i=*(short*)(ed->buf+ed->selobj);
                memmove(ed->buf+j+2,ed->buf+j,ed->selobj-j-2);
                *(short*)(ed->buf+j)=i;
                ed->selobj=j;
                ed->modf=1;
                goto updmap;
            case 22:
                j=ed->chkofs[ed->selchk+1]-4;
                if(ed->selobj==j) break;
                i=*(short*)(ed->buf+ed->selobj);
                memmove(ed->buf+ed->selobj,ed->buf+ed->selobj+2,j-ed->selobj);
                *(short*)(ed->buf+j)=i;
                ed->selobj=j;
                ed->modf=1;
                goto updmap;
            case 'B':
                if(ed->selobj==ed->chkofs[ed->selchk]) break;
                i=*(short*)(ed->buf+ed->selobj-2);
                *(short*)(ed->buf+ed->selobj-2)=*(short*)(ed->buf+ed->selobj);
                *(short*)(ed->buf+ed->selobj)=i;
                ed->selobj-=2;
                ed->modf=1;
                goto updmap;
            case 'V':
                if(ed->selobj==ed->chkofs[ed->selchk+1]-4) break;
                i=*(short*)(ed->buf+ed->selobj+2);
                *(short*)(ed->buf+ed->selobj+2)=*(short*)(ed->buf+ed->selobj);
                *(short*)(ed->buf+ed->selobj)=i;
                ed->selobj+=2;
                ed->modf=1;
                goto updmap;
            }
        break;
    
    case WM_LBUTTONDBLCLK:
    chooseobj:
        
        if(!ed->selobj)
            break;
        
        if(ed->selchk>=7)
            break;
        
        if(ed->selchk==6)
        {
            q=(ed->ebuf[ed->selobj+1]>=224)?768:512;
            i = ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,ed->ebuf[ed->selobj+2]+q);
        }
        else
            i = ShowDialog(hinstance,
                           MAKEINTRESOURCE(IDD_DUNG_CHOOSE_OBJECT),
                           framewnd,
                           choosedung,
                           ed);
        
        if(i == -1)
            break;
        
        Dungselectchg(ed,win,0);
        SetFocus(win);
        if(ed->selchk==6) {
            ed->ebuf[ed->selobj+2]=i;
            if(i&256) ed->ebuf[ed->selobj+1]|=224;
            else ed->ebuf[ed->selobj+1]&=31;
            ed->modf=1;
            goto updsel;
        } else if(ed->selchk&1) {
            getdoor(ed->buf+ed->selobj,ed->ew.doc->rom);
            i-=0x1b8;
            dm_k=i/42;
            dm_l=i%42;
            goto upddr;
        } else {
            getobj(ed->buf+ed->selobj);
            if(i<0x40) dm_k=i + 0x100;
            else if(i<0x138) {
                dm_k=i - 0x40;
                j=obj3_t[dm_k];
                dm_l=(j==0)|(j==2);
            }
            else
            {
                dm_k = ( ( (i - 0x138) >> 4) & 7) + 0xf8;
                dm_l = (i - 0x138) & 15;
            }
            
            goto upd;
        }
        
        break;
    
    case WM_RBUTTONDOWN:
        
        SetFocus(win);
        menu = CreatePopupMenu();
        
        rom = ed->ew.doc->rom;
        
        if(ed->selchk < 6)
        {
            AppendMenu(menu,MF_STRING,1,"Insert an object");
            if(ed->ew.param<0x8c) AppendMenu(menu,MF_STRING,2,"Insert a door");
        }
        else if(ed->selchk == 6)
            AppendMenu(menu, MF_STRING, 4, "Insert an enemy");
        else if(ed->selchk==7)
            AppendMenu(menu,MF_STRING,7,"Insert an item");
        else if(ed->selchk==8)
            AppendMenu(menu,MF_STRING,8,"Insert a block");
        else if(ed->selchk==9)
            AppendMenu(menu,MF_STRING,9,"Insert a torch");
        
        if(ed->selchk < 7 && ed->selobj)
            AppendMenu(menu,MF_STRING,5,"Choose an object");
        
        if(ed->selobj)
            AppendMenu(menu,MF_STRING,3,"Remove");
        
        i = 0;
        
        if(ed->selchk < 6)
            i = ed->chkofs[ed->selchk + 1] - ed->chkofs[ed->selchk] > 2;
        else if(ed->selchk == 6)
            i = ed->esize > 1;
        else if(ed->selchk == 7)
            i = ed->ssize > 0;
        else if(ed->selchk == 8)
        {
            for(i = 0; i < 0x18c; i += 4)
            {
                if( *(short*) (rom + 0x271de + i) == ed->mapnum)
                {
                    i = 1;
                    
                    break;
                }
            }
        }
        else if(ed->selchk==9) i=ed->tsize;
        if(i) AppendMenu(menu,MF_STRING,6,"Remove all");
        AppendMenu(menu,MF_STRING,10,"Discard room header");
        menu2=CreateMenu();
        for(i=0;i<15;i++) {
            wsprintf(text_buf, "%d", i);
            AppendMenu(menu2,MF_STRING,256+i,text_buf);
        }
        AppendMenu(menu2,MF_STRING,271,"Other...");
        AppendMenu(menu,MF_POPUP|MF_STRING,(int)menu2,"Edit blocks");
        GetCursorPos(&pt);
        k=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY,pt.x,pt.y,0,win,0);
        DestroyMenu(menu);
        i=(lparam&65535)+(ed->mapscrollh<<5);
        j=(lparam>>16)+(ed->mapscrollv * ed->map_vscroll_delta);
        if(k>=256) {
            if(k>=264) k+=2;
            Editblocks((OVEREDIT*)ed,k-256,win);
        }
        else
            switch(k)
            {
            case 1:
                m=ed->selchk|1;
                Dungselectchg(ed,win,0);
                ed->selchk&=6;
                
                if(ed->selobj>=ed->chkofs[m] || !ed->selobj)
                    l = ed->chkofs[m] - 2;
                else
                    l = ed->selobj;
                
                o = ShowDialog(hinstance,
                               MAKEINTRESOURCE(IDD_DUNG_CHOOSE_OBJECT),
                               framewnd,
                               choosedung,
                               ed);
                
                if(o==-1) break;
                for(n=m;n<7;n++) ed->chkofs[n]+=3;
                ed->buf=realloc(ed->buf,ed->len);
                memmove(ed->buf+l+3,ed->buf+l,ed->len-l-3);
                ed->selobj=l;
                dm_x=((i>>3)&0x3f)|((j<<3)&0xfc0);
                ed->buf[l]=0;
                ed->buf[l+1]=0;
                ed->buf[l+2]=0;
                if(o<0x40) dm_k=o + 0x100;
                else if(o<0x138) {
                    dm_k=o - 0x40;
                    j=obj3_t[dm_k];
                    dm_l=(j==0)|(j==2);
                }
                else
                {
                    dm_k = ( ( (o - 0x138) >> 4) & 7) + 0xf8;
                    dm_l = (o - 0x138) & 15;
                }
                
                goto upd;
            
            case 2:
            m=(ed->selchk|1)+1;
            Dungselectchg(ed,win,0);
            ed->selchk|=1;
            if(ed->selobj=ed->chkofs[m-1] || !ed->selobj) l=ed->chkofs[m]-2;
            else l=ed->selobj;
            if(l==ed->chkofs[m-1]-2) p=4; else p=2;
            
            o = ShowDialog(hinstance,
                         MAKEINTRESOURCE(IDD_DUNG_CHOOSE_OBJECT),
                         framewnd,choosedung,
                         ed);
            
            if(o == -1)
                break;
            
            for(n=m;n<7;n++)
                ed->chkofs[n]+=p;
            
            ed->buf = realloc(ed->buf,ed->len);
            
            memcpy(ed->buf+l+p,ed->buf+l,ed->len-l-p);
            if(p==4) { *(short*)(ed->buf+l)=-16; l+=2; }
            ed->selobj=l;
            o-=0x1b8;
            dm_k=o/42;
            dm_l=o%42;
            goto upddr;
        case 3:
            Dungselectchg(ed,win,0);
            m=ed->selchk;
            l=ed->selobj;
            if(m==9) {
                memcpy(ed->tbuf+ed->selobj,ed->tbuf+ed->selobj+2,ed->tsize-ed->selobj-2);
                ed->tsize-=2;
                if(ed->tsize==2) ed->tsize=0;
                ed->tbuf=realloc(ed->tbuf,ed->tsize);
                ed->modf=1;
            } else if(m==8) *(short*)(rom+ed->selobj)=-1,ed->ew.doc->modf=1;
            else if(m==7) {
                memcpy(ed->sbuf+ed->selobj-2,ed->sbuf+ed->selobj+1,ed->ssize-ed->selobj-1);
                ed->ssize-=3;
                ed->sbuf=realloc(ed->sbuf,ed->ssize);
                ed->modf=1;
            } else if(m==6) {
                memcpy(ed->ebuf+ed->selobj,ed->ebuf+ed->selobj+3,ed->esize-ed->selobj-3);
                ed->esize-=3;
                ed->ebuf=realloc(ed->ebuf,ed->esize);
                ed->modf=1;
            } else if(m&1) {
                if(ed->chkofs[m+1]==ed->chkofs[m]+4) p=4,l-=2; else p=2;
                for(n=m+1;n<7;n++) ed->chkofs[n]-=p;
                memcpy(ed->buf+l,ed->buf+l+p,ed->len-l);
                ed->buf=realloc(ed->buf,ed->len);
                ed->modf=1;
            } else {
                dm_k=0;
                dm_x=0;
                dm_l=0;
                setobj(ed,ed->buf+l);
                for(n=m+1;n<7;n++) ed->chkofs[n]-=3;
                memcpy(ed->buf+l,ed->buf+l+3,ed->len-l);
                ed->buf=realloc(ed->buf,ed->len);
            }
            ed->selobj=0;
            goto updmap;
        case 4:
            Dungselectchg(ed,win,0);
            o=ShowDialog(hinstance,(LPSTR)IDD_DIALOG9,framewnd,choosesprite,512);
            
            if(o == -1)
                break;
            
            if(!ed->selobj)
                ed->selobj = 1;
            
            ed->esize+=3;
            ed->ebuf=realloc(ed->ebuf,ed->esize);
            memmove(ed->ebuf+ed->selobj+3,ed->ebuf+ed->selobj,ed->esize-ed->selobj-3);
            ed->ebuf[ed->selobj]=j>>4;
            i>>=4;
            if(o&256) i+=224;
            ed->ebuf[ed->selobj+1]=i;
            ed->ebuf[ed->selobj+2]=o;
            ed->modf=1;
            goto updsel;
        case 5:
            goto chooseobj;
        case 6: // delete all objects, sprites, etc. "REMOVE ALL"
            if(ed->selchk<6) {
                k = ed->selchk;
                j = ed->chkofs[k];
                i = ed->chkofs[k + 1] - 2;
                
                if(j == i)
                    break;
                
                if(k & 1)
                    j -= 2;
                else
                {
                    dm_k = 0;
                    dm_x = 0;
                    dm_l = 0;
                    
                    for(l = j; l < i; l += 3)
                        setobj(ed, ed->buf + l);
                }
                
                memcpy(ed->buf + j, ed->buf + i, ed->len - i);
                
                for(k += 1; k < 7; k++)
                    ed->chkofs[k] += j - i;
                
                ed->buf = realloc(ed->buf, ed->len);
                
                Updatemap(ed);
            }
            else if(ed->selchk == 6)
            {
                // remove all sprites
                
                ed->ebuf = realloc(ed->ebuf,1);
                ed->esize = 1;
                ed->modf = 1;
            }
            else if(ed->selchk == 7)
            {
                ed->sbuf=realloc(ed->sbuf,0);
                ed->ssize=0;
                ed->modf=1;
            }
            else if(ed->selchk==8)
            {
                for(i=0;i<0x18c;i+=4)
                    if(*(short*)(rom + 0x271de + i)==ed->mapnum) *(short*)(rom + 0x271de + i)=-1;
                ed->ew.doc->modf=1;
                Updatemap(ed);
            }
            else
            {
                ed->tbuf=realloc(ed->tbuf,0);
                ed->tsize=0;
                ed->modf=1;
                Updatemap(ed);
            }
            InvalidateRect(win,0,0);
            ed->selobj=0;
            goto updsel;
        case 7:
            Dungselectchg(ed,win,0);
            ed->sbuf=realloc(ed->sbuf,ed->ssize+=3);
            if(!ed->selobj) ed->selobj=ed->ssize-1;
            memmove(ed->sbuf+ed->selobj+1,ed->sbuf+ed->selobj-2,ed->ssize-ed->selobj-1);
            *(short*)(ed->sbuf+ed->selobj-2)=((i>>2)&0x7e)+((j<<4)&0x1f80);
            ed->sbuf[ed->selobj]=0;
            ed->modf=1;
            goto updsel;
        case 8:
            Dungselectchg(ed,win,0);
            
            for(k=0;k<0x18c;k+=4)
                if( is16b_neg1(rom + 0x271de + k) )
                {
                    *(short*)(rom + 0x271de + k)=ed->mapnum;
                    *(short*)(rom + 0x271e0 + k)=((i>>2)&0x7e)|((j<<4)&0x1f80);
                    ed->selobj=0x271de + k;
                    break;
                }
            
            if(k==0x18c)
                MessageBox(framewnd,"You can't add anymore blocks","Bad error happened",MB_OK);
            ed->ew.doc->modf=1;
            goto updmap;
        case 9:
            Dungselectchg(ed,win,0);
            k=ed->tsize;
            if(!k) k+=2;
            ed->selobj=k;
            ed->tbuf=realloc(ed->tbuf,k+=2);
            ed->tsize=k;
            *(short*)(ed->tbuf)=ed->mapnum;
            *(short*)(ed->tbuf+k-2)=((i>>2)&0x7e)|((j<<4)&0x1f80);
            ed->modf=1;
            goto updmap;
        case 10:
            i=askinteger(295,"Use another header","Room #");
            if(i<0) break;
            
            LoadHeader(ed, i);
            
            ed->hmodf = i + 2;
            ed->modf  = i + 2;
            
            goto updmap;
        }
        
        break;
    
    case WM_KEYDOWN:
        
        rom = ed->ew.doc->rom;

        if( !(lparam & 0x1000000) )
        {
            // Allows the num pad directional keys to work regardless of 
            // whether numlock is on or not. (This block is entered if numlock
            // is off.)
            
            switch(wparam)
            {
            case VK_RIGHT:
                wparam=VK_NUMPAD6;
                break;
            case VK_LEFT:
                wparam=VK_NUMPAD4;
                break;
            case VK_DOWN:
                wparam=VK_NUMPAD2;
                break;
            case VK_UP:
                wparam=VK_NUMPAD8;
                break;
            }
        }
        
        if(ed->selchk>7)
            break;
        else if(ed->selchk==7) if(wparam==VK_RIGHT)
        {
            if(ed->ssize==0) break;
            Dungselectchg(ed,win,0);
            if(!ed->selobj) ed->selobj=2; else ed->selobj+=3;
            if(ed->selobj>=ed->ssize) ed->selobj=ed->ssize-1;
            goto selchg;
        }
        else if(wparam==VK_LEFT)
        {
            if(ed->ssize<2) break;
            Dungselectchg(ed,win,0);
            ed->selobj-=3;
            if(ed->selobj<1) ed->selobj=2;
            goto selchg;
        }
        else
        {
            if(!ed->selobj) break;
            Dungselectchg(ed,win,0);
            switch(wparam) {
            case VK_NUMPAD4:
                ed->sbuf[ed->selobj-2]-=2;
                ed->modf=1;
                goto updsel;
            case VK_NUMPAD6:
                ed->sbuf[ed->selobj-2]+=2;
                ed->modf=1;
                goto updsel;
            case VK_NUMPAD8:
                *((short*)(ed->sbuf+ed->selobj-2))-=128;
                ed->modf=1;
                goto updsel;
            case VK_NUMPAD2:
                *((short*)(ed->sbuf+ed->selobj-2))+=128;
                ed->modf=1;
                goto updsel;
            case 'N':
                k=-1;
                goto updpot;
            case 'M':
                k=1;
                goto updpot;
            case 'J':
                k=-16;
                goto updpot;
            case 'K':
                k=16;
updpot:
                j=ed->sbuf[ed->selobj];
                if(j>22) j=(j>>1)-41;
                j+=k;
                if(j>27) j-=28;
                if(j<0) j+=28;
                if(j>22) j=(j+41)<<1;
                ed->sbuf[ed->selobj]=j;
                Dungselectchg(ed,win,1);
                ed->modf=1;
                break;
            }
        }
        else if(ed->selchk==6) if(wparam==VK_RIGHT)
        {
            if(ed->esize<2) break;
            Dungselectchg(ed,win,0);
            if(!ed->selobj) ed->selobj=1; else ed->selobj+=3;
            if(ed->selobj>=ed->esize) ed->selobj=ed->esize-3;
            goto selchg;
        }
        else if(wparam == VK_LEFT)
        {
            if(ed->esize<2) break;
            Dungselectchg(ed,win,0);
            ed->selobj-=3;
            if(ed->selobj<1) ed->selobj=1;
            goto selchg;
        }
        else
        {
            if(!ed->selobj)
                break;
            Dungselectchg(ed,win,0);
            
            switch(wparam)
            {
        case VK_NUMPAD6:
            dm_x=ed->ebuf[ed->selobj+1];
            ed->ebuf[ed->selobj+1]=(dm_x&224)|((dm_x+1)&31);
            ed->modf=1;
            goto updsel;
        case VK_NUMPAD4:
            dm_x=ed->ebuf[ed->selobj+1];
            ed->ebuf[ed->selobj+1]=(dm_x&224)|((dm_x-1)&31);
            ed->modf=1;
            goto updsel;
        case VK_NUMPAD8:
            dm_x=ed->ebuf[ed->selobj];
            ed->ebuf[ed->selobj]=(dm_x&224)|((dm_x-1)&31);
            ed->modf=1;
            goto updsel;
        case VK_NUMPAD2:
            dm_x=ed->ebuf[ed->selobj];
            ed->ebuf[ed->selobj]=(dm_x&224)|((dm_x+1)&31);
            ed->modf=1;
            goto updsel;
        case 'N':
            ed->ebuf[ed->selobj+2]--;
            ed->modf=1;
            goto updsel;
        case 'M':
            ed->ebuf[ed->selobj+2]++;
            ed->modf=1;
            goto updsel;
        case 'J':
            ed->ebuf[ed->selobj+2]-=16;
            ed->modf=1;
            goto updsel;
        case 'K':
            ed->ebuf[ed->selobj+2]+=16;
            ed->modf=1;
            goto updsel;
        
        case 'B':
            
            if(ed->selobj == 1)
                break;
            
            // \task Perhaps a swaple24b() is in order.
            i = ldle24b(ed->ebuf + ed->selobj - 3);
            
            stle24b(ed->ebuf + ed->selobj - 3,
                    ldle24b(ed->ebuf + ed->selobj) );
            
            stle24b(ed->ebuf + ed->selobj, i);
            
            ed->selobj -= 3;
            ed->modf = 1;
            
            goto updsel;
        
        case 'V':
            
            // Moves the selected sprite later in the load sequence.
            
            if(ed->selobj == ed->esize - 3)
                break;
            
            i = ldle24b(ed->ebuf + ed->selobj + 3);
            
            stle24b(ed->ebuf + ed->selobj + 3,
                     ldle24b(ed->ebuf + ed->selobj) );
            
            stle24b(ed->ebuf + ed->selobj, i);
            
            ed->selobj += 3;
            ed->modf = 1;
            
            goto updsel;
        } }
        else if(ed->selchk & 1)
            if(wparam == VK_RIGHT)
            {
                if(ed->chkofs[ed->selchk] >= ed->chkofs[ed->selchk + 1] - 2)
                    break;
                
                Dungselectchg(ed,win,0);
                
                if(!ed->selobj)
selfirst:
                    ed->selobj = ed->chkofs[ed->selchk];
                else
                {
                    ed->selobj += 2;
                    
                    if(ed->selobj >= ed->chkofs[ed->selchk + 1] - 2)
                    {
                        ed->selobj = ed->chkofs[ed->selchk + 1] - 4;
                        break;
                    }
                }
                
                goto selchg;
            }
            else if(wparam == VK_LEFT)
            {
                if(ed->chkofs[ed->selchk] == ed->chkofs[ed->selchk + 1] - 2)
                    break;
                
                Dungselectchg(ed,win,0);
                
                if(!ed->selobj)
                    goto selfirst;
                
                ed->selobj -= 2;
                
                if(ed->selobj < ed->chkofs[ed->selchk])
                {
                    if(ed->chkofs[ed->selchk] != ed->chkofs[ed->selchk - 1] + 2)
                    {
                        ed->selchk--;
                        ed->selobj -= 3;
                    }
                    else
                    {
                        ed->selobj = ed->chkofs[ed->selchk];
                        break;
                    }
                }
                
                goto selchg;
            }
            else
            {
                if(!ed->selobj)
                    break;
                
                Dungselectchg(ed,win,0);
                
                switch(wparam)
                {
                case VK_NUMPAD6:
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_dl++;
                    if(dm_dl>11) dm_dl=0;
                    goto upddr;
                case VK_NUMPAD4:
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_dl--;
                    if(dm_dl>11) dm_dl=11;
                    goto upddr;
                case VK_NUMPAD2:
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_k--;
                    goto upddr;
                case VK_NUMPAD8:
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_k++;
                    goto upddr;
                case 'K':
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_l+=0x10;
                    goto upddr;
                case 'J':
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_l-=0x10;
                    goto upddr;
                case 'M':
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_l++;
                    goto upddr;
                case 'N':
                    getdoor(ed->buf+ed->selobj,rom);
                    dm_l--;
                upddr:
                    
                    if(dm_l>41)
                        dm_l-=42;
                    
                    if(dm_l>41)
                        dm_l+=84;
                    setdoor(ed->buf+ed->selobj);
                    ed->modf=1;
                    goto updmap;
                }
            }
            else if(wparam==VK_RIGHT)
            {
                if(ed->chkofs[ed->selchk] == ed->chkofs[ed->selchk+1]-2)
                    break;
                
                Dungselectchg(ed,win,0);
                
                if(!ed->selobj)
                    goto selfirst;
                
                ed->selobj+=3;
                
                if(ed->selobj>=ed->chkofs[ed->selchk+1]-2)
                    if(ed->chkofs[ed->selchk+2]==ed->chkofs[ed->selchk+1])
                    {
                        ed->selobj=ed->chkofs[ed->selchk+1]-5+(ed->selchk&1);
                        break;
                    }
                    else
                        ed->selobj+=2,ed->selchk++;
                    
                    goto selchg;
            }
            else if(wparam == VK_LEFT)
            {
                if(ed->chkofs[ed->selchk]==ed->chkofs[ed->selchk+1]-2) break;
                Dungselectchg(ed,win,0);
                if(!ed->selobj) goto selfirst;
                ed->selobj-=3;
                if(ed->selobj<ed->chkofs[ed->selchk]) {
                    ed->selobj=ed->chkofs[ed->selchk];
                    break;
                }
                
            selchg:
                
                Dungselectchg(ed,win,1);
                
                break;
            }
            else
            {
                if(!ed->selobj) break;
                Dungselectchg(ed,win,0);
                switch(wparam) {
            case VK_NUMPAD6:
                getobj(ed->buf+ed->selobj);
                dm_x++;
                if(dm_k<0x100 && (dm_x&0x3f)==0x3f) dm_x++;
                goto upd;
            case VK_NUMPAD4:
                getobj(ed->buf+ed->selobj);
                dm_x--;
                if(dm_k<0x100 && (dm_x&0x3f)==0x3f) dm_x--;
                goto upd;
            case VK_NUMPAD2:
                getobj(ed->buf+ed->selobj);
                dm_x+=64;
                goto upd;
            case VK_NUMPAD8:
                getobj(ed->buf+ed->selobj);
                dm_x-=64;
                goto upd;
            case 'K':
                getobj(ed->buf+ed->selobj);
                dm_k+=0x10;
                if(dm_k>0xff && dm_k<0x110) dm_k-=0x100;
                if(dm_k>0x13f) dm_k-=0x40;
                goto upd;
            case 'J':
                getobj(ed->buf+ed->selobj);
                dm_k-=0x10;
                if(dm_k>0xef && dm_k<0x100) dm_k+=0x40;
                if(dm_k<0) dm_k+=0x100;
                goto upd;
            case 'M':
                getobj(ed->buf+ed->selobj);
                dm_k++;
                if(dm_k==0x100) dm_k=0;
                if(dm_k==0x140) dm_k=0x100;
                goto upd;
            case 'N':
                getobj(ed->buf+ed->selobj);
                dm_k--;
                if(dm_k==0xff) dm_k=0x13f;
                
                // \task Just a gentle reminder that if we change this to
                // an unsigned type we will probably have problems
                // (due to integer promotion rules).
                if(dm_k==-1) dm_k=0xff;
            upd:
                setobj(ed,ed->buf+ed->selobj);
            updmap:
                Updatemap(ed);
            updsel:
                Dungselectchg(ed,win,1);
            } }
            break;
    
    case WM_SETFOCUS:
        
        ed->withfocus |= 1;
        
        Dungselectchg(ed, win, 0);
        
        break;
    
    case WM_KILLFOCUS:
        
        ed->withfocus &= -2;
        
        Dungselectchg(ed,win,0);
        
        break;
    
    case WM_PAINT:
        
        DungeonMap_OnPaint(ed, win);
        
        break;
    
    default:
        
        return DefWindowProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

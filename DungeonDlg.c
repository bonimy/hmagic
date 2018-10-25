
    // For printf()
    #include <stdio.h>

#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"

#include "Wrappers.h"
#include "Callbacks.h"

#include "HMagicUtility.h"

#include "DungeonEnum.h"
#include "DungeonLogic.h"

// =============================================================================

#if 1

    void
    quick_u16_signedness_test(void)
    {
        uint16_t i = 0;
        
        // unsigned
        uint16_t a_u = 0xffff;
        uint16_t a_n = ~a_u;
        
        // indeterminate? standard probably indicates signed though
        short a_std = 0xffff;
        
        // signed
        int16_t a_s = 0xffff;

        unsigned int b_u = a_std;
        
        int b_s = a_std;

        (void) a_s;
        if( ~a_u == 0 )
        {
            printf("unexpected");
        }

        if( ~a_u == -1 )
        {
            printf("unexpected");
        }
        
        if( a_n == 0 )
        {
            printf("unexpected");
        }


        if(a_u == ~0)
        {
            printf("unexpected");
        }

        if(a_u == -1)
        {
            printf("unexpected");
        }
        
        if(a_u == ~0)
        {
            printf("unexpected");
        }
        
        if( (a_u + 1) != 0 )
        {
            uint16_t c_u = (a_u + 1);
            
            if(c_u != 0)
            {
                printf("unexpected");
            }
            
        }

        if( (a_u + 1) != 0 )
        {
            printf("unexpected");
        }

        if(b_s != -1)
        {
            printf("probably happening");
        }
        
        if(b_s != ~0)
        {
            printf("probably happening");
        }

        if(b_u == -1)
        {
            printf("probably happening");
        }
        
        if(b_u == ~0)
        {
            printf("probably happening");
        }
        
        for(i = 0;  ; i += 1)
        {
            uint16_t j = ~i;
            
            if( (i + j) == i )
            {
                printf("hrm... %d", i);
            }
            
            if(i == 0xffff)
            {
                break;
            }
        }
    }

#endif

// =============================================================================

    /// IDs of dialog controls to hide if the user elects to edit an overlay
    /// or layout.
    unsigned const overlay_hide[] =
    {
        ID_DungRoomNumber,
        ID_DungStatic1,
        ID_DungEntrRoomNumber,
        ID_DungStatic2,
        ID_DungEntrYCoord,
        ID_DungStatic3,
        ID_DungEntrXCoord,
        ID_DungStatic4,
        ID_DungEntrYScroll,
        ID_DungStatic5,
        
        ID_DungEntrXScroll,
        ID_DungStartLocGroupBox,
        ID_DungLeftArrow,
        ID_DungRightArrow,
        ID_DungUpArrow,
        ID_DungDownArrow,
        ID_DungStatic7,
        ID_DungFloor2,
        ID_DungStatic8,
        ID_DungEntrTileSet,
        
        ID_DungStatic9,
        ID_DungEntrSong,
        ID_DungStatic10,
        ID_DungEntrPalaceID,
        ID_DungStatic11,
        ID_DungLayout,
        ID_DungObjLayer1,
        ID_DungObjLayer2,
        ID_DungObjLayer3,
        ID_DungEditGroupBox,
        
        ID_DungStatic12,
        ID_DungBG2Settings,
        ID_DungStatic15,
        ID_DungSprTileSet,
        ID_DungCollSettings,
        ID_DungSprLayer,
        ID_DungChangeRoom,
        ID_DungShowSprites,
        ID_DungStatic16,
        ID_DungStatic17,
        
        ID_DungEntrCameraX,
        ID_DungStatic18,
        ID_DungEntrCameraY,
        ID_DungEntrProps,
        ID_DungEditHeader,
        ID_DungItemLayer,
        ID_DungBlockLayer,
        ID_DungTorchLayer,
        ID_DungSortSprites,
        ID_DungExit
    };

    enum
    {
        ID_DungOverlayHideCount = sizeof(overlay_hide) / sizeof(unsigned),
    };


// =============================================================================

// Specific configurations that are checked involving BG2 settings.
// Dunno what they signify yet.
const static unsigned char bg2_ofs[]={
    0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0, 0x01
};

const static short nxtmap[4]={-1,1,-16,16};

// =============================================================================

// Fix entrance scrolling settings.
// Seems to update those 8 numbers that control scrolls in entrances?
void FixEntScroll(FDOC*doc,int j)
{
    unsigned char *rom = doc->rom;
    HWND win;
    int i,k,l,n,o,p;
    
    const static unsigned char hfl[4]={0xd0,0xb0};
    const static unsigned char vfl[4]={0x8a,0x86};
    
    k=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
    
    win = doc->dungs[k];
    
    if(win)
        n = GetDlgItemInt(win, ID_DungLayout, 0, 0);
    else
        n = rom[romaddr(*(int*)(rom + 0xf8000 + k * 3))+1]>>2;
    
    o = rom[(j >= 0x85 ? 0x15ac7 : 0x14f5a) + (j << 1)] & 1;
    p = rom[(j >= 0x85 ? 0x15ad5 : 0x15064) + (j << 1)] & 1;
    
    i = (((vfl[o] >> n) << 1) & 2) + (((hfl[p] >> n) << 5) & 32);
    
    rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
    rom[(j>=0x85?0x15ba6:0x1569f)+j]=(o<<1)+(p<<4);
    
    i = (k >> 4) << 1;
    l = (k & 15) << 1;
    
    j <<= 3;
    
    if(j >= 0x428)
        j += 0xe37;
    
    rom[0x1491d + j] = i + o;
    rom[0x1491e + j] = i;
    rom[0x1491f + j] = i + o;
    rom[0x14920 + j] = i + 1;
    rom[0x14921 + j] = l + p;
    rom[0x14922 + j] = l;
    rom[0x14923 + j] = l + p;
    rom[0x14924 + j] = l + 1;
}

// =============================================================================

void
Initroom(DUNGEDIT * const ed,
         HWND       const win)
{
    unsigned char *buf2;
    
    int i;
    int j;
    int l;
    int m;
    
    unsigned char *rom = ed->ew.doc->rom;
    
    buf2 = rom + (ed->hbuf[1] << 2) + 0x75460;
    
    // Loadpal(ed,rom,0x1bd734 + *((unsigned short*)(rom + 0xdec4b + buf2[0])),0x21,15,6);
    // I didn't comment the above out, to the best of my knowledge, -MON
    
    j = 0x6073 + (ed->gfxtmp << 3);
    
    ed->palnum = ed->hbuf[1];
    ed->gfxnum = ed->hbuf[2];
    ed->sprgfx = ed->hbuf[3];
    
    // gives an offset (bunched in 4's)
    l = 0x5d97 + (ed->hbuf[2] << 2);
    
    // determine blocsets 0-7
    for(i = 0; i < 8; i++)
        ed->blocksets[i] = rom[j++];
    
    // these are uniquely determined
    ed->blocksets[8] = (rom + 0x1011e)[ed->gfxnum];
    ed->blocksets[9] = 0x5c;
    ed->blocksets[10] = 0x7d;
    
    // rewrite blocksets 3-6, if necessary
    for(i = 3; i < 7; i++)
    {
        m = rom[l++];
        
        if(m)
            ed->blocksets[i] = m;
    }
    
    l = 0x5c57 + (ed->sprgfx << 2);
    
    // determine blocksets 11-14, which covers them all.
    for(i = 0; i < 4; i++)
        ed->blocksets[i + 11] = rom[l + i] + 0x73;
    
    //get the block graphics for our tilesets?
    for(i = 0; i < 15; i++)
        Getblocks(ed->ew.doc, ed->blocksets[i]);
    
    ed->layering = (ed->hbuf[0] & 0xe1);
    
    // take bits 2-4, shift right 2 to get a 3 bit number.
    ed->coll = ((ed->hbuf[0] & 0x1c) >> 2);
    
    // the header is unmodified, nor is the room, so far.
    // if something changes, the flag will be set.
    ed->modf  = 0;
    ed->hmodf = 0;
    
    SetDlgItemInt(win, ID_DungFloor1, ed->buf[0] & 15, 0); // floor 1
    SetDlgItemInt(win, ID_DungFloor2, ed->buf[0] >> 4, 0); // floor 2
    SetDlgItemInt(win, ID_DungTileSet, ed->gfxnum, 0); //blockset
    SetDlgItemInt(win, ID_DungPalette, ed->palnum, 0); //palette
}

// =============================================================================

void
LoadDungeonObjects(DUNGEDIT * const p_ed,
                   unsigned   const p_map)
{
    uint8_t const * const rom = p_ed->ew.doc->rom;
    
    // Get the base address for this room in the rom.
    uint8_t const * const buf = rom
                              + romaddr( ldle24b_i(rom + 0xf8000, p_map) );
    
    // Offset into the raw object data array.
    // The first two bytes are floor tile fill pattern (floor1 / floor2),
    // layout and other information that are irrelevant to loading objects,
    // so skip over them.
    uint16_t i = 2;
    
    // Indicates which layer we're loading from. Even values represent the
    // 3 byte object portion of a layer, odd values represent the 2 byte or
    // door portion. E.g. if the value of this is 5, that represents the
    // door objects of layer 3.
    int j = 0;
    
    // -----------------------------
    
    p_ed->selobj = 0;
    p_ed->selchk = 0;
    
    for(j = 0; j < 6; j++)
    {
        p_ed->chkofs[j] = i;
        
        for( ; ; )
        {
            uint16_t k = ldle16b(buf + i);
            
            // code indicating that this layer has terminated
            if(k == 0xffff)
                break;
            
            // code indicating that we should begin loading door objects.
            if(k == 0xfff0)
            {
                j++;
                
                p_ed->chkofs[j] = i + 2;
                
                for( ; ; )
                {
                    i += 2;
                    
                    k = ldle16b(buf + i);
                    
                    if(k == 0xffff)
                        goto end;
                    
                    if( ! p_ed->selobj )
                    {
                        p_ed->selobj = i;
                        p_ed->selchk = j;
                    }
                }
            }
            else
            {
                i += 3;
            }
            
            // if there is no selected object, pick one.
            if( ! p_ed->selobj )
            {
                p_ed->selobj = i - 3;
                p_ed->selchk = j;
            }
        }
        
        j++;
        
        p_ed->chkofs[j] = i + 2;
end:
        i += 2;
    }
    
    p_ed->len = i; // size of the buffer.
    
    // generate the buffer of size i.
    // copy the data from buf to ed->buf.
    p_ed->buf = (uint8_t*) hm_memdup(buf, i);
}

// =============================================================================

void
Openroom(DUNGEDIT * const ed,
         int        const map)
{
    uint8_t const * const rom = ed->ew.doc->rom;
    
    rom_cty const torches = (rom + offsets.dungeon.torches);
    
    rom_cty const torch_count = (rom + offsets.dungeon.torch_count);
    
    // Get the base address for this room in the rom.
    uint8_t const * buf = 0;

    int i = 0;
    
    int num_torches = 0;
    
    int layer = 0;
    
    HWND win = ed->dlg;
    
    // -----------------------------
    
    ed->mapnum = map;
    
    ed->ew.doc->dungs[map] = win;
    
    LoadDungeonObjects(ed, map);
    
    layer = ed->selchk & ~1;
    
    // do some initial settings, determine which radio button to check first 1, 2, 3,...
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer1, (layer == 0) );
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer2, (layer == 2) );
    HM_BinaryCheckDlgButton(win, ID_DungObjLayer3, (layer == 4) );
    HM_BinaryCheckDlgButton(win, ID_DungSprLayer, (layer == SD_DungSprLayerSelected) );
    HM_BinaryCheckDlgButton(win, ID_DungItemLayer, (layer == SD_DungItemLayerSelected) );
    HM_BinaryCheckDlgButton(win, ID_DungBlockLayer, (layer == SD_DungBlockLayerSelected) );
    HM_BinaryCheckDlgButton(win, ID_DungTorchLayer, (layer == SD_DungTorchLayerSelected) );
    
    // this is the "layout", ranging from 0-7
    SetDlgItemInt(win, ID_DungLayout, ed->buf[1] >> 2, 0);
    
    // load the header information.
    LoadHeader(ed, map);
    
    Initroom(ed,win);
    
    {
        char text_buf[0x100];
        
        // prints the room string in the upper left hander corner.
        wsprintf(text_buf, "Room %d", map);
        
        // ditto.
        SetDlgItemText(win, ID_DungRoomNumber, text_buf);
    }
    
    if(map > 0xff)
    {
        // If we're in the upper range, certain buttons might be grayed out so we can't
        // get back to the lower range
        for(i = 0; i < 4; i++)
        {
            EnableWindow(GetDlgItem(win, ID_DungLeftArrow + i),
                         ((map + nxtmap[i]) & 0xff) < 0x28);
        }
    }
    else
    {
        EnableWindow( GetDlgItem(win, ID_DungLeftArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungRightArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungUpArrow), 1);
        EnableWindow( GetDlgItem(win, ID_DungDownArrow), 1);
    }
    
    CheckDlgButton(win, ID_DungObjLayer1, BST_CHECKED);
    CheckDlgButton(win, ID_DungObjLayer2, BST_UNCHECKED);
    CheckDlgButton(win, ID_DungObjLayer3, BST_UNCHECKED);
    
    for(i = 0; i < 9; i++)
    {
        if(ed->layering == bg2_ofs[i])
        {
            // \task Is this buggy? What if the value is not in this list?
            // Probably not the case in a vanilla rom, but still...
            SendDlgItemMessage(win, ID_DungBG2Settings, CB_SETCURSEL, i, 0);
            
            break;
        }
    }
    
    SendDlgItemMessage(win, ID_DungCollSettings, CB_SETCURSEL, ed->coll, 0);
    SetDlgItemInt(win, ID_DungSprTileSet, ed->sprgfx, 0);
    
    buf = rom + 0x50000 + ((short*)(rom + ed->ew.doc->dungspr))[map];
    
    for(i = 1; ; i += 3)
    {
        if(buf[i] == 0xff)
            break;
    }
    
    ed->esize = i;
    ed->ebuf = (uint8_t*) malloc(i);
    
    memcpy(ed->ebuf,buf,i);
    
    CheckDlgButton(win, ID_DungSortSprites, *ed->ebuf ? BST_CHECKED : BST_UNCHECKED);
    
    // load the secrets for the dungeon room
    buf = rom + 0x10000 + ((short*) (rom + 0xdb69))[map];
    
    for(i = 0; ; i += 3)
        if( is16b_neg1(buf + i) )
            break;
    
    ed->ssize = i;
    ed->sbuf = malloc(i);
    
    memcpy(ed->sbuf,buf,i);
    
    buf = torches;
    
    num_torches = ldle16b(torch_count);
    
    for(i = 0; ; i += 2)
    {
        int j = i;
        
        if(i >= num_torches)
        {
            ed->tsize = 0;
            ed->tbuf = 0;
            
            break;
        }
        
        for( ; ; )
        {
            i += 2;
            
            if( is16b_neg1(buf + i) )
                break;
        }
        
        if( ldle16b(buf + j) == map)
        {
            ed->tbuf = (uint8_t*) malloc(ed->tsize = i - j);
            
            memcpy(ed->tbuf, buf + j, i - j);
            
            break;
        }
    }
}

// =============================================================================

int
Closeroom(DUNGEDIT * const ed)
{
    int i;
    
    if(ed->ew.doc->modf != 2 && ed->modf)
    {
        char text_buf[0x100];
        
        if(ed->ew.param < 0x8c)
            wsprintf(text_buf,
                     "Confirm modification of room %d?",
                     ed->mapnum);
        
        else
            wsprintf(text_buf,
                     "Confirm modification of overlay map?");
        
        switch
        (
            MessageBox(framewnd,
                       text_buf,
                       "Dungeon editor",
                       MB_YESNOCANCEL)
        )
        {
        
        case IDYES:
            Saveroom(ed);
            
            break;
        
        case IDCANCEL:
            return 1;
        }
    }
    
    for(i = 0; i < 15; i++)
        Releaseblks(ed->ew.doc,ed->blocksets[i]);
    
    if(ed->ew.param<0x8c)
        ed->ew.doc->dungs[ed->mapnum] = 0;
    
    return 0;
}

// =============================================================================

void Savedungsecret(FDOC*doc,int num,unsigned char*buf,int size)
{
    int i,j,k;
    int adr[0x140];
    unsigned char*rom=doc->rom;
    for(i=0;i<0x140;i++)
        adr[i]=0x10000 + ((short*)(rom + 0xdb69))[i];
    j=adr[num];
    k=adr[num+1];
    
    if( is16b_neg1(rom + j) )
    {
        if(!size) return;
        j+=size+2;
        adr[num]+=2;
    } else {
        if(!size) {
            if(j>0xdde9) {
                j-=2;
                adr[num]-=2;
            }
        } else j+=size;
    }
    if(*(short*)(rom+k)!=-1) k-=2;
    if(adr[0x13f]-k+j>0xe6b0) {
        MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
        return;
    }
    memmove(rom+j,rom+k,adr[0x13f]+2-k);
    if(size) memcpy(rom+adr[num],buf,size);
    if(j==k) return;
    ((short*)(rom + 0xdb69))[num]=adr[num];
    for(i=num+1;i<0x140;i++) {
        ((short*)(rom + 0xdb69))[i]=adr[i]+j-k;
    }
}

// =============================================================================

void
Saveroom(DUNGEDIT * const ed)
{
    uint8_t * const rom = ed->ew.doc->rom; // get our romfile from the Dungedit struct.
    
    rom_ty const torches = rom + offsets.dungeon.torches;
    
    rom_ty const torch_count = rom + offsets.dungeon.torch_count;
    
    int l;
    int m;
    int n;
    
    // -----------------------------
    
    if( !ed->modf ) // if the dungeon map hasn't been modified, do nothing.
        return;
    
    // if it has... save it.
    if( ed->ew.param >= 0x8c) // if the map is an overlay... 
    {
        int i = Changesize(ed->ew.doc, ed->ew.param + 0x301f5, ed->len - 2);
        
        if(!i)
            return;
        
        memcpy(rom + i, ed->buf + 2, ed->len - 2);
    }
    else
    {
        short i = 0;
        short j = 0;
        short k = 0;
        
        int ret_size = 0;
        
        Savesprites(ed->ew.doc, 0x160 + ed->mapnum, ed->ebuf, ed->esize);
        
        for(i = 5; i >= 1; i -= 2)
        {
            if(ed->chkofs[i] != ed->chkofs[i + 1])
                break;
        }
        
        door_ofs = ed->chkofs[i];
        
        ret_size = Changesize(ed->ew.doc, ed->mapnum + 0x30140, ed->len);
        
        if( !ret_size )
            return;
        
        memcpy(rom + ret_size, ed->buf, ed->len);
        
        Savedungsecret(ed->ew.doc, ed->mapnum, ed->sbuf, ed->ssize);
        
        k = ldle16b(torch_count);
        
        if( is16b_neg1(torches) )
            k = 0;
        
        for(i = 0; i < k; i += 2)
        {
            j = i;
            
            // test block
            quick_u16_signedness_test();
            
            for( ; ; )
            {
                j += 2;
                
                if( is16b_neg1(torches + j) )
                    break;
            }
            
            if( ldle16b(torches + i) == ed->mapnum )
            {
                if(!ed->tsize)
                    j += 2;
                
                if(k + i + ed->tsize - j > 0x120)
                    goto noroom;
                
                memmove(torches + i + ed->tsize, torches + j, k - j);
                memcpy(torches + i, ed->tbuf, ed->tsize);
                
                k += i + ed->tsize - j;
                
                break;
            }
            else
                i = j;
        }
        
        if(ed->tsize && i == k)
        {
            j = ed->tsize + 2;
            
            if(k + j > 0x120)
            noroom:
            {
                MessageBox(framewnd,
                           "Not enough room for torches",
                           "Bad error happened",
                           MB_OK);
            }
            else
            {
                memcpy(torches + k, ed->tbuf, ed->tsize);
                
                stle16b(torches + k + j - 2, u16_neg1);
                
                k += j;
            }
        }
        
        // \note This has the effect of changing the immediate operand of a
        // CPX command (number of bytes worth of torch data) and checking whether
        // there is valid torch data in the first two entries. Effectively if there
        // are two 0xffff words in the torch data and the operand of the instruction
        // is 4, it means no room can load torch data.
        if(!k)
        {
            k = 4;
            
            stle32b(torches, u32_neg1);
        }
        
        stle16b(torch_count, k);
        
        m = ed->hmodf;
        
        // if the headers have been modified, save them.
        if(m != 0)
        {
            // some sort of upper bound.
            int k = 0x28000 + ldle16b(rom + 0x27780);
            
            // some sort of lower bound.
            int i = 0x28000 + ldle16b_i(rom + 0x27502, ed->mapnum);
            
            for(j = 0; j < 0x140; j++)
            {
                // if we hit the map number we're currently on, keep moving.
                if(j == ed->mapnum)
                    continue;
                
                if( 0x28000 + ldle16b_i(rom + 0x27502, j) == i )
                {
                    char text_buf[0x100];
                    
                    if(m > 1)
                        goto headerok;
                    
                    wsprintf(text_buf,
                             "The room header of room %d is reused. Modify this one only?",
                             ed->mapnum);
                    
                    if(MessageBox(framewnd, text_buf, "Bad error happened", MB_YESNO) == IDYES)
                    {
headerok:
                        k = i;
                        
                        goto changeroom;
                    }
                    
                    break;
                }
            }
            
            for(j = 0; j < 0x140; j++)
            {
                l = 0x28000 + ((short*)(rom + 0x27502 ))[j];
                
                if(l > i && l < k)
                    k = l;
            }
changeroom:
            
            if(m > 1)
            {
                ( (short*) ( rom + 0x27502 ) )[ed->mapnum] = ( (short*) ( rom + 0x27502 ) )[m - 2];
                
                n = 0;
            }
            else
            {
                n = ed->hsize;
            }
            
            // \task Should this actually be a signed load?
            if( ldle16b(rom + 0x27780) + (i + n - k) > 0 )
            {
                MessageBox(framewnd,
                           "Not enough room for room header",
                           "Bad error happened",
                           MB_OK);
            }
            else
            {
                short const delta = (short) (i + n - k);
                
                memmove(rom + i + n, rom + k, 0x28000 + *((short*)(rom + 0x27780)) - k);
                
                addle16b(rom + 0x27780, delta);
                
                memcpy(rom + i, ed->hbuf, n);
                
                for(j = 0; j < 0x140; j++)
                {
                    if
                    (
                        j != ed->mapnum
                     && ldle16b_i(rom + 0x27502, j) + 0x28000 >= k
                    )
                    {
                        addle16b_i(rom + 0x27502, j, delta);
                    }
                }
            }
            
            if(n)
            {
                rom[i]   = ed->layering | (ed->coll << 2);
                rom[i+1] = (uint8_t) ed->palnum;
                rom[i+2] = (uint8_t) ed->gfxnum;
                rom[i+3] = (uint8_t) ed->sprgfx;
            }
        }
    }
    
    ed->modf=0;
    ed->ew.doc->modf=1;
}


// =============================================================================

BOOL CALLBACK
editdungprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    int i,
        j,
        k,
        l;
    
    unsigned char *rom;
    
    static int radio_ids[3] = {IDC_RADIO10,IDC_RADIO12,IDC_RADIO13};
    
    (void) lparam;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        rom = dunged->ew.doc->rom;
        j = dunged->ew.param;
        
        if(j < 0x85)
        {
            i = rom[0x15510 + j];
            
            if(i == 2)
                // what exactly is a horizontal entrance doorway? O_o
                CheckDlgButton(win, IDC_RADIO9, BST_CHECKED);
            else if(i)
                CheckDlgButton(win, IDC_RADIO3, BST_CHECKED);
            else
                CheckDlgButton(win, IDC_RADIO1, BST_CHECKED);
        }
        else
        {
            ShowWindow(GetDlgItem(win,IDC_RADIO9),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_RADIO3),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_RADIO1),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
            ShowWindow(GetDlgItem(win,IDC_STATIC3),SW_SHOW);
            ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_SHOW);
            
            SetDlgItemInt(win, IDC_EDIT1, ((short*) (rom + 0x15b36))[j], 0);
        }
        
        i = rom[(j >= 0x85 ? 0x15b98 : 0x15595) + j];
        
        if(i & 0xf)
            CheckDlgButton(win,IDC_RADIO5,BST_CHECKED);
        else
            CheckDlgButton(win,IDC_RADIO4,BST_CHECKED);
        
        SetDlgItemInt(win,IDC_EDIT2,i>>4,0);
        
        i = rom[(j >= 0x85 ? 0x15b9f : 0x1561a) + j];
        
        if(i & 0x20)
            CheckDlgButton(win, IDC_CHECK1, BST_CHECKED);
        
        if(i & 0x02)
            CheckDlgButton(win, IDC_CHECK4, BST_CHECKED);
        
        switch(l = rom[(j >= 0x85 ? 0x15ba6 : 0x1569f) + j])
        {
        
        case 0:
            
            CheckDlgButton(win,IDC_RADIO6,BST_CHECKED);
            
            break;
        
        case 2:
            
            CheckDlgButton(win,IDC_RADIO8,BST_CHECKED);
            
            break;
        
        case 16:
            
            CheckDlgButton(win,IDC_RADIO7,BST_CHECKED);
            
            break;
        
        case 18:
            
            CheckDlgButton(win,IDC_RADIO11,BST_CHECKED);
            
            break;
        }
        
        SetDlgItemInt(win,
                      IDC_EDIT3,
                      (char) rom[(j >= 0x85 ? 0x15b8a : 0x15406) + j], 1);
        
        i = ( (unsigned short*) (rom + (j >= 0x85 ? 0x15b28 : 0x15724) ) )[j];
        
        if(i && i != 0xffff)
        {
            k = (i >> 15) + 1;
            
            SetDlgItemInt(win, IDC_EDIT13, (i & 0x7e) >> 1, 0);
            SetDlgItemInt(win, IDC_EDIT14, (i & 0x3f80) >> 7, 0);
        }
        else
        {
            k = 0;
            
            EnableWindow( GetDlgItem(win, IDC_EDIT13), 0);
            EnableWindow( GetDlgItem(win, IDC_EDIT14), 0);
        }
        
        CheckDlgButton(win, radio_ids[k], BST_CHECKED);
        
        j <<= 3;
        
        if(j >= 0x428)
            j += 0xe37;
        
        SetDlgItemInt(win, IDC_EDIT4, rom[0x1491d + j], 0);
        SetDlgItemInt(win, IDC_EDIT5, rom[0x1491e + j], 0);
        SetDlgItemInt(win, IDC_EDIT22, rom[0x1491f + j], 0);
        SetDlgItemInt(win, IDC_EDIT23, rom[0x14920 + j], 0);
        SetDlgItemInt(win, IDC_EDIT24, rom[0x14921 + j], 0);
        SetDlgItemInt(win, IDC_EDIT25, rom[0x14922 + j], 0);
        SetDlgItemInt(win, IDC_EDIT26, rom[0x14923 + j], 0);
        SetDlgItemInt(win, IDC_EDIT27, rom[0x14924 + j], 0);
        
        break;
    case WM_COMMAND:
        switch(wparam) {
        case IDC_RADIO10:
            EnableWindow(GetDlgItem(win,IDC_EDIT13),0);
            EnableWindow(GetDlgItem(win,IDC_EDIT14),0);
            break;
        case IDC_RADIO12:
        case IDC_RADIO13:
            EnableWindow(GetDlgItem(win,IDC_EDIT13),1);
            EnableWindow(GetDlgItem(win,IDC_EDIT14),1);
            break;
        case IDOK:
            
            rom = dunged->ew.doc->rom;
            
            j = dunged->ew.param;
            
            if(j < 0x85)
            {
                if( IsDlgButtonChecked(win, IDC_RADIO9) )
                    i = 2;
                else if( IsDlgButtonChecked(win, IDC_RADIO3) )
                    i = 1;
                else
                    i = 0;
                
                rom[0x15510 + j] = i;
            }
            else
                ((short*)(rom + 0x15b36))[j]=GetDlgItemInt(win,IDC_EDIT1,0,0);
            
            i = IsDlgButtonChecked(win,IDC_RADIO5);
            
            rom[(j>=0x85?0x15b98:0x15595)+j]=i+(GetDlgItemInt(win,IDC_EDIT2,0,0)<<4);
            i=0;
            if(IsDlgButtonChecked(win,IDC_CHECK4)) i=2;
            if(IsDlgButtonChecked(win,IDC_CHECK1)) i|=32;
            rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
            if(IsDlgButtonChecked(win,IDC_RADIO6)) i=0;
            else if(IsDlgButtonChecked(win,IDC_RADIO8)) i=2;
            else if(IsDlgButtonChecked(win,IDC_RADIO7)) i=16;
            else i=18;
            rom[(j>=0x85?0x15ba6:0x1569f)+j]=i;
            
            rom[(j >= 0x85 ? 0x15b8a : 0x15406) + j] =
            GetDlgItemInt(win, IDC_EDIT3, 0, 1);
            
            if(IsDlgButtonChecked(win,IDC_RADIO10)) i=-1;
            else {
                i=(GetDlgItemInt(win,IDC_EDIT13,0,0)<<1)+(GetDlgItemInt(win,IDC_EDIT14,0,0)<<7);
                if(IsDlgButtonChecked(win,IDC_RADIO13)) i|=0x8000;
            }
            ((short*)(rom+(j>=0x85?0x15b28:0x15724)))[j]=i;
            i=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
            k=(i&496)<<5;
            i=(i&15)<<9;
            j<<=3;
            if(j>=0x428) j+=0xe37;
            rom[0x1491d + j] = GetDlgItemInt(win, IDC_EDIT4, 0, 0);
            rom[0x1491e + j] = GetDlgItemInt(win, IDC_EDIT5, 0, 0);
            rom[0x1491f + j] = GetDlgItemInt(win, IDC_EDIT22, 0, 0);
            rom[0x14920 + j] = GetDlgItemInt(win, IDC_EDIT23, 0, 0);
            rom[0x14921 + j] = GetDlgItemInt(win, IDC_EDIT24, 0, 0);
            rom[0x14922 + j] = GetDlgItemInt(win, IDC_EDIT25, 0, 0);
            rom[0x14923 + j] = GetDlgItemInt(win, IDC_EDIT26, 0, 0);
            rom[0x14924 + j] = GetDlgItemInt(win, IDC_EDIT27, 0, 0);
            dunged->ew.doc->modf=1;
        case IDCANCEL:
            EndDialog(win,0);
            break;
        }
    }
    return FALSE;
}

// =============================================================================

BOOL CALLBACK
dungdlgproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int j, // the "entrance" number. not a room number. Is used to retrieve the room #
        k, // the room number retrieved using j.
        i, // 
        l,
        m,
        n;
    
    HWND hc;
    
    DUNGEDIT *ed;
    
    uint8_t *buf2;
    
    unsigned char *rom;
    
//  const static int inf_ofs[5]={0x14813,0x14f59,0x15063,0x14e4f,0x14d45};
//  const static int inf_ofs2[5]={0x156be,0x15bb4,0x15bc2,0x15bfa,0x15bec};
    
    // music offsets?
    const static unsigned char mus_ofs[] =
    {
        255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,240,241,242,243
    };
    
    const static char *bg2_str[] =
    {
        "Off",
        "Parallaxing",
        "Dark",
        "On top",
        "Translucent",
        "Parallaxing2",
        "Normal",
        "Addition",
        "Dark room"
    };
    
    const static char *coll_str[] =
    {
        "One",
        "Both",
        "Both w/scroll",
        "Moving floor",
        "Moving water"
    };
    
    // message handling for the window.
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        ed = (DUNGEDIT*) lparam;
        
        // get the 'room' we're looking at.
        j = ed->ew.param;
        
        rom = ed->ew.doc->rom;
        
        // the dungedit has been initialized.
        ed->init = 1;
        
        // pass this win to ed's dialogue.
        ed->dlg = win;
        
        // void the HPALETTE in ed.
        ed->hpal = 0;
        
        if(j >= 0x8c)
        {
            // Only certain controls are necessary for working with a dungeon
            // overlay or layout, so hide the rest.
            for(i = 0; i < ID_DungOverlayHideCount; i++)
                ShowWindow( GetDlgItem(win, overlay_hide[i]), SW_HIDE );
            
            if(j < 0x9f)
                buf2 = rom + romaddr( *(int*) (rom + 0x26b1c + j * 3) );
            else if(j < 0xa7)
                buf2 = rom + romaddr( *(int*) (rom + romaddr( *(int*) (rom + 0x882d) ) + (j - 0x9f) * 3 ) );
            else
                buf2 = rom + (rom[0x9c25] << 15) - 0x8000 + *(unsigned short*) (rom + 0x9c2a);
            
            for(i = 0; ; i += 3)
                if( is16b_neg1(buf2 + i) )
                    break;
            
            ed->chkofs[0] = 2;
            ed->len = ed->chkofs[2] = ed->chkofs[1] = i + 4;
            ed->selobj = 2;
            ed->buf = (uint8_t*) malloc(i + 4);
            
            *(short*)(ed->buf)=15;
            
            memcpy(ed->buf+2,buf2,i+2);
            ed->hbuf[0] = 0;
            ed->hbuf[1] = 1;
            ed->hbuf[2] = 0;
            ed->hbuf[3] = 0;
            ed->hbuf[4] = 0;
            ed->gfxtmp = 0;
            
            CheckDlgButton(win, ID_DungShowBG1, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowBG2, BST_CHECKED);
            
            Initroom(ed,win);
        }
        else
        {
            // not an overlay
            
            k = ldle16b_i(rom + (j >= 0x85 ? 0x15b6e : 0x14813),
                          j >= 0x85 ? (j - 0x85) : j );
            
            // read in the room number, put it to screen.
            SetDlgItemInt(win, ID_DungEntrRoomNumber, k, 0);
            
            i = (k & 0x1f0) << 5;
            l = (k & 0x00f) << 9;
            
            if(j >= 0x85)
            {
                // these are the starting location maps
                SetDlgItemInt(win, ID_DungEntrYCoord, ((short*) (rom + 0x15ac6) )[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXCoord, ((short*) (rom + 0x15ad4) )[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrYScroll, ((short*) (rom + 0x15ab8) )[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXScroll, ((short*) (rom + 0x15aaa) )[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrCameraX, ((short*) (rom + 0x15af0) )[j], 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, ((short*) (rom + 0x15ae2) )[j], 0);
            }
            else
            {
                // these are normal rooms
                SetDlgItemInt(win, ID_DungEntrYCoord, ((short*) (rom + 0x14f59))[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXCoord, ((short*) (rom + 0x15063))[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrYScroll, ((short*) (rom + 0x14e4f))[j] - i, 0);
                SetDlgItemInt(win, ID_DungEntrXScroll, ((short*) (rom + 0x14d45))[j] - l, 0);
                SetDlgItemInt(win, ID_DungEntrCameraX, ((short*) (rom + 0x15277))[j], 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, ((short*) (rom + 0x1516d))[j], 0);
            }
            
            // the show BG1, BG2, Sprite, check boxes.
            // make them initially checked.
            CheckDlgButton(win, ID_DungShowBG1, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowBG2, BST_CHECKED);
            CheckDlgButton(win, ID_DungShowSprites, BST_CHECKED);
            
            for(i = 0; i < 4; i++)
                // map in the arrow bitmaps for those buttons at the top l.eft
                SendDlgItemMessage(win, ID_DungLeftArrow + i, BM_SETIMAGE, IMAGE_BITMAP, (int) arrows_imgs[i]);
            
            for(i = 0; i < 40; i++)
                // map in the names of the music tracks
                SendDlgItemMessage(win, ID_DungEntrSong, CB_ADDSTRING, 0, (int) mus_str[i]);
            
            // if j is a starting location, use 0x15bc9, otherwise 0x1582e as an offset.
            l = rom[ (j >= 0x85 ? 0x15bc9 : 0x1582e) + j ];
            
            for(i = 0; i < 40; i++)
            {
                if(mus_ofs[i] == l)
                {
                    SendDlgItemMessage(win, ID_DungEntrSong, CB_SETCURSEL, i, 0);
                    
                    break;
                }
            }
            
            for(i = 0; i < 9; i++)
                SendDlgItemMessage(win, ID_DungBG2Settings, CB_ADDSTRING, 0, (int) bg2_str[i]);
            
            for(i = 0; i < 5; i++)
                SendDlgItemMessage(win, ID_DungCollSettings, CB_ADDSTRING, 0, (int) coll_str[i]);
            
            for(i = 0; i < 15; i++)
            {
                SendDlgItemMessage(win,
                                   ID_DungEntrPalaceID,
                                   CB_ADDSTRING,
                                   0,
                                   (int) level_str[i]);
            }
            
            SendDlgItemMessage(win,
                               ID_DungEntrPalaceID,
                               CB_SETCURSEL,
                               ( (rom[ (j >= 0x85 ? 0x15b91 : 0x1548b) + j] + 2) >> 1) & 0x7f,
                               0);
            
            ed->gfxtmp = rom[(j >= 0x85 ? 0x15b83 : 0x15381) + j];
            
            SetDlgItemInt(win, ID_DungEntrTileSet, ed->gfxtmp, 0);
            
            Openroom(ed, k);
        }
        
        // \task Investigate this. Do all dungeons really set the backdrop color
        // to pure black?
        ed->pal[0] = blackcolor;
        
        // for the first entry of each palette, the first color is "blackcolor"
        for(i = 16; i < 256; i += 16)
            ed->pal[i] = ed->pal[0];
        
        // start at the top left corner of the dungeon map
        ed->mapscrollh = 0;
        ed->mapscrollv = 0;
        
        // it's the bit map information header... whatever that is.
        ed->bmih = zbmih;
        
        ed->bmih.biWidth  = 512;
        ed->bmih.biHeight = 512;
        
        ed->selchk = 0;
        ed->withfocus = 0;
        
        ed->disp = (SD_DungShowBothBGs | SD_DungShowMarkers);
        ed->anim = 0;
        ed->init = 0;
        
        Getblocks(ed->ew.doc, 0x79);
        Getblocks(ed->ew.doc, 0x7a);
        
        Addgraphwin(ed, 1);
        Updatemap(ed);
        
        goto updpal3;
    
    case 4002: // used as a code to update the palette? so it seems
        
        InvalidateRect( GetDlgItem(win, ID_DungEditWindow), 0, 0);
        
        break;
    
    case WM_COMMAND:
        
        ed = (DUNGEDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed || ed->init)
            break;
        
        switch(wparam)
        {
        
        // the arrow buttons
        case ID_DungLeftArrow:
        case ID_DungRightArrow:
        case ID_DungUpArrow:
        case ID_DungDownArrow:
            
            k = ed->mapnum;
            
            k = ( k + nxtmap[wparam - ID_DungLeftArrow] & 0xff) + (k & 0x100);
            
            if(ed->ew.doc->dungs[k])
            {
                MessageBox(framewnd,"The room is already open in another editor","Bad error happened",MB_OK);
                break;
            }
newroom:
            
            if(Closeroom(ed))
                break;
            
            ed->init = 1;
            
            Openroom(ed, k);
            
            ed->init = 0;
            Updatemap(ed);
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed, hc, 1);
            InvalidateRect(hc,0,0);
            
            goto updpal;
        
        case ID_DungChangeRoom:
            
            k = askinteger(295, "Jump to room", "Room #");
            
            if(k == -1)
                break;
            
            goto newroom;
        
        case ID_DungShowBG1:
            
            // \task What's with these bitpatterns? enumerate them?
            ed->disp &= SD_DungHideBG1;
            
            if(IsDlgButtonChecked(win, ID_DungShowBG1) == BST_CHECKED)
                ed->disp |= SD_DungShowBG1;
            
            goto updscrn;
        
        case ID_DungShowBG2:
            
            ed->disp &= SD_DungHideBG2;
            
            if(IsDlgButtonChecked(win, ID_DungShowBG2) == BST_CHECKED)
                ed->disp |= SD_DungShowBG2;
            
            goto updscrn;
        
        case ID_DungShowSprites:
            
            ed->disp &= SD_DungHideMarkers;
            
            if(IsDlgButtonChecked(win, ID_DungShowSprites) == BST_CHECKED)
                ed->disp |= SD_DungShowMarkers;
            
            goto updscrn;
        
        case ID_DungAnimateButton:
            
            ed->anim++;
            
            if(ed->anim == 3)
                ed->anim = 0;
            
            {
                char text_buf[0x100];
                
                wsprintf(text_buf, "Frm%d", ed->anim + 1);
            
                SetWindowText((HWND)lparam, text_buf);
            }
            
            goto updscrn;
        
        case ID_DungObjLayer1:
            
            i = 0;
            
        updchk:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            
            Dungselectchg(ed, hc, 0);
            
            if(ed->chkofs[i + 1] == ed->chkofs[i] + 2)
            {
                i++;
                
                if(ed->chkofs[i + 1] <= ed->chkofs[i] + 2)
                {
                    i--;
                    ed->selobj = 0;
                    
                    goto no_obj;
                }
            }
            
            ed->selobj = ed->chkofs[i];
no_obj:
            ed->selchk = i;
            
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungObjLayer2:
            
            i = 2;
            
            goto updchk;
        
        case ID_DungObjLayer3:
            
            i = 4;
            
            goto updchk;
        
        case ID_DungSprLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            
            Dungselectchg(ed,hc,0);
            
            ed->selchk = SD_DungSprLayerSelected;
            ed->selobj = ed->esize > 2;
            
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungEntrRoomNumber | (EN_CHANGE << 16):
        case ID_DungEntrYCoord | (EN_CHANGE << 16):
        case ID_DungEntrXCoord | (EN_CHANGE << 16):
        case ID_DungEntrYScroll | (EN_CHANGE << 16):
        case ID_DungEntrXScroll | (EN_CHANGE << 16):
        case ID_DungFloor1 | (EN_CHANGE << 16):
        case ID_DungFloor2 | (EN_CHANGE << 16):
        case ID_DungEntrTileSet | (EN_CHANGE << 16):
        case ID_DungLayout | (EN_CHANGE << 16):
        case ID_DungTileSet | (EN_CHANGE << 16):
        case ID_DungPalette | (EN_CHANGE << 16):
        case ID_DungSprTileSet | (EN_CHANGE << 16):
        case ID_DungEntrCameraX | (EN_CHANGE << 16):
        case ID_DungEntrCameraY | (EN_CHANGE << 16):
            
            wparam &= 65535;
            
            j = GetDlgItemInt(win, wparam, 0, 0);
            
            rom = ed->ew.doc->rom;
            
            // \task consider a different check for this. The enumerations
            // could change order.
            if(wparam > ID_DungEntrXScroll)
            {
                switch(wparam)
                {
                
                case ID_DungFloor1:
                    
                    if(j>15)
                    {
                        SetDlgItemInt(win, ID_DungFloor1, 15, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungFloor1, 0, 0);
                        break;
                    }
                    
                    ed->buf[0] &= 0xf0;
                    ed->buf[0] |= j;
                    
                    if(ed->ew.param < 0x8c)
                        ed->modf = 1;
updmap:
                    
                    Updatemap(ed);
                    
updscrn: // update the screen (obviously)
                    
                    hc = GetDlgItem(win, ID_DungEditWindow);
                    
                    InvalidateRect(hc,0,0);
                    
                    break;
                
                case ID_DungFloor2:
                    
                    if(j > 15)
                    {
                        SetDlgItemInt(win, ID_DungFloor2, 15, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungFloor2, 0, 0);
                        break;
                    }
                    
                    ed->buf[0]&=0xf;
                    ed->buf[0]|=j<<4;
                    ed->modf=1;
                    
                    goto updmap;
                
                case ID_DungLayout:
                    
                    if(j > 7)
                    {
                        SetDlgItemInt(win, ID_DungLayout, 7, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungLayout, 0, 0);
                        break;
                    }
                    
                    ed->buf[1] = j << 2;
                    ed->modf = 1;
                    
                    for(i = 0; i < 0x85; i++)
                    {
                        if(((short*)(rom + 0x14813))[i] == ed->mapnum)
                            FixEntScroll(ed->ew.doc,i);
                    }
                    
                    for( ; i < 0x8c; i++)
                    {
                        if(((short*)(rom + 0x15a64))[i] == ed->mapnum)
                            FixEntScroll(ed->ew.doc, i);
                    }
                    
                    goto updmap;
                
                case ID_DungEntrTileSet:
                    
                    if(j > 30)
                    {
                        SetDlgItemInt(win, ID_DungEntrTileSet, 30, 0);
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungEntrTileSet, 0, 0);
                        break;
                    }
                    
                    ed->gfxtmp = j;
                    rom[ed->ew.param + (ed->ew.param >= 0x85 ? 0x15b83 : 0x15381)] = j;
                    ed->ew.doc->modf = 1;
updgfx:
                    
                    for(i = 0; i < 0; i++)
                        Releaseblks(ed->ew.doc,ed->blocksets[i]);
                    
                    j = 0x6073 + (ed->gfxtmp << 3);
                    l = 0x5d97 + (ed->gfxnum << 2);
                    
                    for(i = 0; i < 8; i++)
                        ed->blocksets[i] = rom[j++];
                    
                    for(i = 3; i < 7; i++)
                    {
                        if(m = rom[l++])
                            ed->blocksets[i] = m;
                    }
                    
                    ed->blocksets[8] = rom[0x1011e + ed->gfxnum];
                    
                    for(i=0;i<9;i++)
                        Getblocks(ed->ew.doc,ed->blocksets[i]);
                    
                    goto updscrn;
                
                case ID_DungTileSet:
                    
                    if(j > 23)
                    {
                        SetDlgItemInt(win, ID_DungTileSet, 23, 0);
                        
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungTileSet, 0, 0);
                        
                        break;
                    }
                    
                    ed->gfxnum = j;
                    ed->modf = 1;
                    ed->hmodf = 1;
                    
                    goto updgfx;
                
                case ID_DungSprTileSet:
                    
                    ed->sprgfx = j;
                    ed->modf   = 1;
                    ed->hmodf  = 1;
                    
                    l = 0x5c57 + (j << 2);
                    
                    for(i = 0; i < 4; i++)
                    {
                        Releaseblks(ed->ew.doc, ed->blocksets[i + 11]);
                        
                        ed->blocksets[i+11]=rom[l+i] + 0x73;
                        
                        Getblocks(ed->ew.doc,ed->blocksets[i+11]);
                    }
                    
                    break;
                    
                updpal:
                    
                    rom = ed->ew.doc->rom;
                    
                updpal3:
                    
                    j = ed->palnum;
                    
                    goto updpal2;
                
                case ID_DungPalette:
                    
                    if(j > 71)
                    {
                        SetDlgItemInt(win, ID_DungPalette, 71, 0);
                        
                        break;
                    }
                    else if(j < 0)
                    {
                        SetDlgItemInt(win, ID_DungPalette, 0, 0);
                        
                        break;
                    }
                    
                    ed->palnum = j;
                    
                    if(ed->ew.param < 0x8c)
                    {
                        ed->modf = 1;
                        ed->hmodf = 1;
                    }
                    
updpal2:
                    
                    buf2 = rom + (j << 2) + 0x75460;
                    i = 0x1bd734 + buf2[0]*90;
                    
                    Loadpal(ed, rom, i, 0x21, 15, 6);
                    Loadpal(ed, rom, i, 0x89, 7, 1);
                    Loadpal(ed, rom, 0x1bd39e + buf2[1] * 14, 0x81, 7, 1);
                    Loadpal(ed, rom, 0x1bd4e0 + buf2[2] * 14, 0xd1, 7, 1);
                    Loadpal(ed, rom, 0x1bd4e0 + buf2[3] * 14, 0xe1, 7, 1);
                    Loadpal(ed, rom, 0x1bd308, 0xf1, 15, 1);
                    Loadpal(ed, rom, 0x1bd648, 0xdc, 4, 1);
                    Loadpal(ed, rom, 0x1bd630, 0xd9, 3, 1);
                    Loadpal(ed, rom, 0x1bd218, 0x91, 15, 4);
                    Loadpal(ed, rom, 0x1bd6a0, 0, 16, 2);
                    
                    Loadpal(ed, rom, 0x1bd4d2, 0xe9, 7, 1);
                    
                    if(ed->hbuf[4] == 6)
                        Loadpal(ed, rom, 0x1be22c, 0x7b, 2, 1);
                    
                    Updpal(ed);
                    
                    break;
                
                case ID_DungEntrCameraX:
                    
                    ((short*)(rom+(ed->ew.param>=0x85?0x15af0:0x15277)))[ed->ew.param]=j;
                    ed->ew.doc->modf=1;
                    
                    break;
                
                case ID_DungEntrCameraY:
                    
                    ((short*)(rom+(ed->ew.param>=0x85?0x15ae2:0x1516d)))[ed->ew.param]=j;
                    ed->ew.doc->modf=1;
                    
                    break;
                }
            }
            else
            {
                j = ed->ew.param;
                k = GetDlgItemInt(win, ID_DungEntrRoomNumber, 0, 0);
                
                if((unsigned)k > 295)
                {
                    ed->init = 1;
                    SetDlgItemInt(win, ID_DungEntrRoomNumber, 295, 0);
                    ed->init = 0;
                    k = 295;
                }
                
                ((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j]=k;
                i=(k&496)<<5;
                l=(k&15)<<9;
                
                m = GetDlgItemInt(win, ID_DungEntrYScroll, 0, 0);
                n = GetDlgItemInt(win, ID_DungEntrXScroll, 0, 0);
                
                if(j>=0x85)
                {
                    ((short*)(rom + 0x15ac6))[j]=GetDlgItemInt(win, ID_DungEntrYCoord, 0, 0)+i;
                    ((short*)(rom + 0x15ad4))[j]=GetDlgItemInt(win, ID_DungEntrXCoord, 0, 0)+l;
                    ((short*)(rom + 0x15ab8))[j] = m + i;
                    ((short*)(rom + 0x15aaa))[j] = n + l;
                }
                else
                {
                    ((short*)(rom + 0x14f59))[j]=GetDlgItemInt(win, ID_DungEntrYCoord, 0, 0)+i;
                    ((short*)(rom + 0x15063))[j]=GetDlgItemInt(win, ID_DungEntrXCoord, 0, 0)+l;
                    ((short*)(rom + 0x14e4f))[j] = m + i;
                    ((short*)(rom + 0x14d45))[j] = n + l;
                }
                
                SetDlgItemInt(win, ID_DungEntrCameraX, n + 127, 0);
                SetDlgItemInt(win, ID_DungEntrCameraY, m + 119, 0);
                
                if(wparam < ID_DungStatic4)
                    FixEntScroll(ed->ew.doc, j);
            }
            
            break;
        
        case ID_DungEntrSong | (CBN_SELCHANGE << 16):
        case ID_DungEntrPalaceID | (CBN_SELCHANGE << 16):
        case ID_DungBG2Settings | (CBN_SELCHANGE << 16):
        case ID_DungCollSettings | (CBN_SELCHANGE << 16):
            
            rom = ed->ew.doc->rom;
            
            j = SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
            
            if(j == -1)
                break;
            
            switch(wparam & 65535)
            {
            
            case ID_DungEntrSong:
                
                rom
                [
                    (ed->ew.param >= 0x85 ? 0x15bc9 : 0x1582e)
                  + ed->ew.param
                ] = mus_ofs[j];
                
                ed->ew.doc->modf = 1;
                
                break;
            
            case ID_DungEntrPalaceID:
                
                rom
                [
                    (ed->ew.param >= 0x85 ? 0x15b91 : 0x1548b)
                  + ed->ew.param
                ] = (j << 1) - 2;
                
                ed->ew.doc->modf = 1;
                
                goto updscrn;
            
            case ID_DungBG2Settings:
                
                ed->layering=bg2_ofs[j];
                ed->hmodf=ed->modf=1;
                
                goto updscrn;
            
            case ID_DungCollSettings:
                
                ed->coll=j;
                ed->hmodf=ed->modf=1;
                
                break;
            }
            
            break;
        
        case ID_DungEntrProps:
            
            dunged = ed;
            
            ShowDialog(hinstance,
                       (LPSTR) IDD_DIALOG14,
                       framewnd,
                       editdungprop,
                       0);
            
            break;
        
        case ID_DungEditHeader:
            
            dunged=ed;
            
            ShowDialog(hinstance,
                       (LPSTR) IDD_HEADER,
                       framewnd,
                       editroomprop,
                       0);
            
            goto updpal;
        
        case ID_DungItemLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk = SD_DungItemLayerSelected;
            ed->selobj=(ed->ssize>2)?2:0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungBlockLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk=8;
            ed->selobj=0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungTorchLayer:
            
            hc = GetDlgItem(win, ID_DungEditWindow);
            Dungselectchg(ed,hc,0);
            ed->selchk=9;
            ed->selobj=(ed->tsize>2)?2:0;
            Dungselectchg(ed,hc,1);
            
            break;
        
        case ID_DungSortSprites:
            
            *ed->ebuf^=1;
            ed->modf=1;
            
            break;
        
        case ID_DungExit:
            rom=ed->ew.doc->rom;
            for(i=0;i<79;i++)
            {
                if(((unsigned short*)(rom + 0x15d8a))[i]==ed->mapnum)
                    goto foundexit;
            }
            
            MessageBox(framewnd,"None is set.","Bad error happened",MB_OK);
            
            break;
foundexit:
            
            SendMessage(ed->ew.doc->editwin, 4000, rom[0x15e28 + i] + 0x20000,0);
        }
        
        break;
    }
    
    return FALSE;
}

//dungdlgproc*******************************************



// =============================================================================




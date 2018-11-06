
    #include "structs.h"

    #include "LevelMapLogic.h"

// =============================================================================

char const * level_str[] =
{
    "None",
    "Church",
    "Castle",
    "East",
    "Desert",
    "Agahnim",
    "Water",
    "Dark",
    "Mud",
    "Wood",
    "Ice",
    "Tower",
    "Town",
    "Mountain",
    "Agahnim 2"
};

// =============================================================================

extern void
lmapblkchg(HWND             const win,
           LMAPEDIT const * const ed)
{
    RECT rc;
    rc.left = ed->blksel % ed->blkrow << 4;
    rc.top = (ed->blksel / ed->blkrow - ed->blkscroll) << 4;
    rc.right=rc.left+16;
    rc.bottom=rc.top+16;
    InvalidateRect(win,&rc,0);
}

// =============================================================================

extern void
Paintfloor(LMAPEDIT * const ed)
{
    int i,j,k,l,m,n,o,p,q;
    
    uint16_t * const nbuf = ed->nbuf;
    
    uint8_t const * const rom = ed->ew.doc->rom;
    
    m=ed->ew.param;
    
    for(i=0x6f;i<0x1ef;i+=32)
        nbuf[i+1] = nbuf[i] = 0xf00;
    
    if(ed->curfloor>=0) {
        nbuf[0x1af]=((short*)(rom + 0x564e9))[ed->curfloor]&0xefff;
        nbuf[0x1b0]=0xf1d;
    } else {
        nbuf[0x1af]=0xf1c;
        nbuf[0x1b0]=((short*)(rom + 0x564e9))[ed->curfloor^-1]&0xefff;
    }
    for(i=0;i<4;i++) {
        nbuf[((short*)(rom + 0x56431))[i]>>1]=((short*)(rom + 0x56429))[i]&0xefff;
    }
    for(j=0;j<2;j++) {
        k=((short*)(rom + 0x5643d))[j]>>1;
        for(i=0;i<10;i++) {
            nbuf[(k+i)&0x7ff]=((short*)(rom + 0x56439))[j]&0xefff;
        }
    }
    for(j=0;j<2;j++) {
        k=((short*)(rom + 0x56445))[j]>>1;
        for(i=0;i<0x140;i+=32) {
            nbuf[(k+i)&0x7ff]=((short*)(rom + 0x56441))[j]&0xefff;
        }
    }
    k=(ed->basements+ed->curfloor)*25;
    n=0x92;
    for(o=0;o<25;o+=5) {
        for(i=0;i<5;i++) {
            l=ed->rbuf[k];
            if(l==15) j=0x51; else {
                q=0;
                for(p=0;;p++) {
                    j=ed->rbuf[p];
                    if(j!=15) if(j==l) break; else q++;
                }
                j=ed->buf[q];
            }
            j<<=2;
            l=((short*)(rom + 0x57009))[j];
            nbuf[n]=l;
            l=((short*)(rom + 0x5700b))[j];
            nbuf[n+1]=l;
            l=((short*)(rom + 0x5700d))[j];
            nbuf[n+32]=l;
            l=((short*)(rom + 0x5700f))[j];
            nbuf[n+33]=l;
            n+=2;
            k++;
        }
        n+=54;
    }
}

// =============================================================================


    #include "structs.h"

    #include "prototypes.h"

    #include "OverworldEdit.h"

// =============================================================================

void Updateblk32disp(BLOCKEDIT32 *ed, int num)
{
    int k,p=num<<2;
    unsigned short *o;
    RECT rc;
    rc.left=blkx[p];
    rc.right=rc.left+16;
    rc.top=blky[p];
    rc.bottom=rc.top+16;
    o=(unsigned short*)(ed->bs.ed->ew.doc->rom + 0x78000 + (ed->blks[num]<<3));
    
    for(k=0;k<4;k++)
    {
        Drawblock(ed->bs.ed,blkx[p],blky[p],o[k],0);
        p++;
    }
    
    Paintblocks(&rc,objdc,0,0,(DUNGEDIT*)(ed->bs.ed));
    StretchBlt(ed->bufdc,(num&1)*ed->w>>1,(num&2)*ed->h>>2,ed->w>>1,ed->h>>1,objdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SRCCOPY);
}
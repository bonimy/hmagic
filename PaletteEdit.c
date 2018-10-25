
#include "structs.h"

#include "HMagicUtility.h"

// =============================================================================

const int pal_addr[] =
{
    0x1bd630, 0x1bd648, 0x1bd308, 0x1be6c8,0x1bd218,0x1be86c,0x1be604,0x1bd4e0,
    0x1bd734, 0x1bd660, 0xadb27, 0x1bd39e,0x1bd446,0x1be544,0xcc425,0x1eccd3
};

// =============================================================================

void
Savepal(PALEDIT * const ed)
{
    int k;
    
    short *b;
    
    if( !ed->modf)
    {
        return;
    }
    
    k = ed->palw*ed->palh;
    b = (short*)(ed->ew.doc->rom+romaddr(pal_addr[(ed->ew.param>>10)&63]+(k*(ed->ew.param>>16)<<1)));
    
    memcpy(b,ed->pal,k<<1);
    
    ed->modf = 0;
    ed->ew.doc->modf = 1;
}


#include "structs.h"
#include "prototypes.h"

#include "HMagicUtility.h"

#include "WorldMapLogic.h"


// =============================================================================

int const wmmark_ofs[7] =
{
    0x53de4,0x53e2c,0x53e08,0x53e50,0x53e74,0x53e98,0x53ebc
};

int const wmpic_ofs[7] =
{
    0x53ee0,0x53f04,0x53ef2,0x53f16,0x53f28,0x53f3a,0x53f4c
};

int const wmap_ofs[4] =
{
    0, 32, 2048, 2080
};

int const sprs[5] =
{
    0, 64, 128, 208, 272
};

// =============================================================================

int
Getwmappos(WMAPEDIT      const * const ed,
           unsigned char const * const rom,
           int                   const i,
           RECT                * const rc,
           int                   const n,
           int                   const o)
{
    int a,b;
    
    b = ed->marknum;
    
    if(b == 9)
    {
        rc->left = ( rom[0x53763 + i] + (rom[0x5376b + i] << 8) ) >> 4;
        rc->top  = ( rom[0x53773 + i] + (rom[0x5377b + i] << 8) ) >> 4;
    }
    else
    {
        a=((short*)(rom+wmmark_ofs[i]))[b];
        if(a<0) return 1;
        rc->left=a>>4;
        rc->top=((short*)(rom+wmmark_ofs[i]+18))[b]>>4;
    }
    if(!ed->ew.param) rc->left+=128,rc->top+=128;
    rc->left-=n;
    rc->top-=o;
    return 0;
}

// =============================================================================

void
Wmapselectionrect(WMAPEDIT const * const ed,
                  RECT           * const rc)
{
    int i = ( ed->rectleft - (ed->mapscrollh << 2) ) << 3;
    
    int j = ( (ed->rectright - (ed->mapscrollh << 2) ) << 3);
    
    if(i < j)
    {
        rc->left  = i;
        rc->right = j;
    }
    else
    {
        rc->left  = j;
        rc->right = i;
    }
    
    i = (ed->recttop-(ed->mapscrollv<<2))<<3;
    j = ((ed->rectbot-(ed->mapscrollv<<2))<<3);
    
    if(i < j)
    {
        rc->top    = i;
        rc->bottom = j;
    }
    else
    {
        rc->top    = j;
        rc->bottom = i;
    }
}

// =============================================================================

void
Wmapselchg(WMAPEDIT const * const ed,
           HWND             const win,
           int              const c)
{
    RECT rc;
    
    HWND win2;
    
    WMAPEDIT *ed2;
    
    int a,b;
    
    a=ed->ew.param ? 0 : 128;
    b=a-(ed->mapscrollv << 5);
    
    a-=ed->mapscrollh << 5;
    
    rc.left = ed->objx + a;
    rc.top  = ed->objy + b;
    
    if(ed->tool==4)
    {
        rc.right=rc.left+64;
        rc.bottom=rc.top+64;
    }
    else
    {
        text_buf_ty text_buf;
        
        Getwmapstring(ed,ed->selmark, text_buf); 
        
        Getstringobjsize(text_buf, &rc);
    }
    
    InvalidateRect(win,&rc,0);
    
    if(c)
    {
        win2=ed->ew.doc->wmaps[ed->ew.param^1];
        
        ed->ew.doc->modf=1;
        
        if(!win2)
            return;
        
        ed2=(WMAPEDIT*)GetWindowLong(win2,GWL_USERDATA);
        
        if((ed->tool==4 && ed2->tool!=4) || (ed->tool!=4 && 
            (ed2->marknum!=ed->marknum || ed2->tool==4))) return;
        
        win2 = GetDlgItem(ed2->dlg, 3000);
        
        rc.right-=a;
        rc.bottom-=b;
        
        a=ed2->ew.param?0:128;
        b=a-(ed2->mapscrollv<<5);
        
        a-=ed2->mapscrollh<<5;
        
        rc.left=ed->objx+a;
        rc.top=ed->objy+b;
        rc.right+=a;
        rc.bottom+=b;
        
        InvalidateRect(win2,&rc,0);
    }
}

// =============================================================================

void
Getwmapstring(WMAPEDIT const * const ed,
              int              const i,
              text_buf_ty            p_text_buf)
{
    if(ed->marknum == 9)
    {
        wsprintf(p_text_buf,
                 "%d",
                 i + 1);
    }
    else
    {
        wsprintf(p_text_buf,
                 "%d-%04X",
                 i + 1,
                 ldle16b_i(ed->ew.doc->rom + wmpic_ofs[i], ed->marknum) );
    }
}

// =============================================================================

void
Wmapselectwrite(WMAPEDIT * const ed)
{
    int i,j,k,l,m,n,o,p,q;
    m=ed->ew.param?32:64;
    
    if(ed->stselx!=ed->rectleft || ed->stsely!=ed->recttop)
    {
        ed->undomodf=ed->modf;
        memcpy(ed->undobuf,ed->buf,0x1000);
        if(ed->rectleft<0) i=-ed->rectleft; else i=0;
        if(ed->recttop<0) j=-ed->recttop; else j=0;
        if(ed->rectright>m) k=m; else k=ed->rectright;
        k-=ed->rectleft;
        if(ed->rectbot>m) l=m; else l=ed->rectbot;
        l-=ed->recttop;
        q=ed->rectright-ed->rectleft;
        p=j*q;
        
        o = ( (ed->recttop + j) << 6) + ed->rectleft;
        
        for( ; j < l; j++)
        {
            for(n = i; n < k; n++)
            {
                ed->buf[n+o]=ed->selbuf[n+p];
            }
            o+=64;
            p+=q;
        }
        
        ed->modf=1;
    }
    
    ed->selflag=0;
    free(ed->selbuf);
}

// =============================================================================

void
Saveworldmap(WMAPEDIT * const ed)
{
    int i,j=ed->ew.param?1:4,k;
    int*b,*b2;
    if(!ed->modf) return;
    b=(int*)(ed->ew.doc->rom + 0x54727 + (ed->ew.param<<12));
    for(i=0;i<j;i++) {
        b2=(int*)(ed->buf+wmap_ofs[i]);
        for(k=0;k<32;k++) {
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            *(b++)=*(b2++);
            b2+=8;
        }
    }
    
    ed->modf = 0;
    ed->ew.doc->modf=1;
}

// =============================================================================

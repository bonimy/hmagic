
#include "structs.h"
#include "prototypes.h"

#include "GdiObjects.h"
#include "Wrappers.h"

#include "HMagicUtility.h"

// =============================================================================

HM_TextResource entrance_names;

// =============================================================================

void
fill4x2(uint8_t  const * const rom,
        uint16_t       * const nbuf,
        uint16_t const * const buf)
{
    int i;
    int j;
    int k;
    int l;
    int m;
    
    for(l = 0; l < 4; l++)
    {
        i = ((short*) (rom + 0x1b02))[l] >> 1;
        
        for(m = 0; m < 8; m++)
        {
            for(k = 0; k < 8; k++)
            {
                for(j = 0; j < 2; j++)
                {
                    nbuf[i] = buf[0];
                    nbuf[i+1] = buf[1];
                    nbuf[i+2] = buf[2];
                    nbuf[i+3] = buf[3];
                    nbuf[i+64] = buf[4];
                    nbuf[i+65] = buf[5];
                    nbuf[i+66] = buf[6];
                    nbuf[i+67] = buf[7];
                    i += 128;
                }
                
                i -= 252;
            }
            
            i += 224;
        }
    }
}

void len1(void)
{
    if(!dm_l) dm_l=0x20;
}

void len2(void)
{
    if(!dm_l) dm_l=0x1a;
}

void draw2x2(void)
{
    dm_wr[0]  = dm_rd[0];
    dm_wr[64] = dm_rd[1];
    dm_wr[1]  = dm_rd[2];
    dm_wr[65] = dm_rd[3];
    
    dm_wr += 2;
}
void drawXx4(int x)
{
    while(x--)
    {
        dm_wr[0]   = dm_rd[0];
        dm_wr[64]  = dm_rd[1];
        dm_wr[128] = dm_rd[2];
        dm_wr[192] = dm_rd[3];
        
        dm_rd += 4;
        
        dm_wr++;
    }
}
void drawXx4bp(int x)
{
    while(x--)
    {
        dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
        dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
        dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
        dm_buf[0x10c0 + dm_x] = dm_buf[0xc0 + dm_x] = dm_rd[3];
        
        dm_x++;
        
        dm_rd += 4;
    }
}
void drawXx3bp(int x)
{
    while(x--)
    {
        dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
        dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
        dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
        
        dm_x++;
        
        dm_rd += 3;
    }
}
void drawXx3(int x)
{
    while(x--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[64]=dm_rd[1];
        dm_wr[128]=dm_rd[2];
        dm_rd+=3;
        dm_wr++;
    }
}

void draw3x2(void)
{
    int x=2;
    while(x--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[1];
        dm_wr[2]=dm_rd[2];
        dm_wr[64]=dm_rd[3];
        dm_wr[65]=dm_rd[4];
        dm_wr[66]=dm_rd[5];
    }
}

void draw1x5(void)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[64]=dm_rd[1];
    dm_wr[128]=dm_rd[2];
    dm_wr[192]=dm_rd[3];
    dm_wr[256]=dm_rd[4];
}

void draw8fec(uint16_t n)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[1]=dm_rd[1];
    
    dm_wr[65]=dm_wr[64] = n;
}

void draw9030(uint16_t n)
{
    dm_wr[1]=dm_wr[0]=n;
    dm_wr[64]=dm_rd[0];
    dm_wr[65]=dm_rd[1];
}

void draw9078(uint16_t n)
{
    dm_wr[0]=dm_rd[0];
    dm_wr[64]=dm_rd[1];
    dm_wr[65]=dm_wr[1]=n;
}

void draw90c2(uint16_t n)
{
    dm_wr[64]=dm_wr[0]=n;
    dm_wr[1]=dm_rd[0];
    dm_wr[65]=dm_rd[1];
}

void draw4x2(int x)
{
    while(dm_l--) {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[1];
        dm_wr[2]=dm_rd[2];
        dm_wr[3]=dm_rd[3];
        dm_wr[64]=dm_rd[4];
        dm_wr[65]=dm_rd[5];
        dm_wr[66]=dm_rd[6];
        dm_wr[67]=dm_rd[7];
        dm_wr+=x;
    }
}
void draw2x6(uint16_t * const nbuf)
{
    int m = 6;
    
    dm_wr = nbuf + dm_x;
    
    while(m--)
    {
        dm_wr[0]=dm_rd[0];
        dm_wr[1]=dm_rd[6];
        dm_wr+=64;
        dm_rd++;
    }
}
void drawhole(int l, uint16_t * nbuf)
{
    draw2x6(nbuf);
    dm_x+=2;
    dm_rd+=6;
    while(l--) {
        dm_buf[dm_x]=dm_buf[dm_x+64]=dm_buf[dm_x+128]=
        dm_buf[dm_x+192]=dm_buf[dm_x+256]=dm_buf[dm_x+320]=
        dm_rd[0];
        dm_x++;
    }
    draw2x6(nbuf);
}
void draw4x4X(int n)
{
    int x;
    while(n--) {
        x=2;
        while(x--) {
            dm_wr[0]=dm_rd[0];
            dm_wr[1]=dm_rd[1];
            dm_wr[2]=dm_rd[2];
            dm_wr[3]=dm_rd[3];
            dm_wr[64]=dm_rd[4];
            dm_wr[65]=dm_rd[5];
            dm_wr[66]=dm_rd[6];
            dm_wr[67]=dm_rd[7];
            dm_wr+=128;
        }
        dm_wr-=252;
    }
}

void draw12x12(void)
{
    int m,
        l = 12;
    
    while(l--)
    {
        m = 12;
        
        while(m--)
        {
            dm_buf[dm_x + 0x1000] = dm_rd[0];
            dm_x++;
            dm_rd++;
        }
        
        dm_x+=52;
    }
}

void draw975c(void)
{
    unsigned char m;
    m=dm_l;
    dm_wr[0]=dm_rd[0];
    while(m--) dm_wr[1]=dm_rd[3],dm_wr++;
    dm_wr[1]=dm_rd[6];
    dm_wr[2]=dm_wr[3]=dm_wr[4]=dm_wr[5]=dm_rd[9];
    m=dm_l;
    dm_wr[6]=dm_rd[12];
    while(m--) dm_wr[7]=dm_rd[15],dm_wr++;
    dm_wr[7]=dm_rd[18];
    dm_tmp+=64;
    dm_wr=dm_tmp;
}

void draw93ff(void)
{
    int m;
    uint16_t *tmp;
    m = dm_l;
    
    tmp=dm_wr;
    dm_wr[0] = dm_rd[0];
    
    while(m--)
        dm_wr[1] = dm_rd[1], dm_wr[2] = dm_rd[2], dm_wr += 2;
    
    dm_wr[1] = dm_rd[3];
    tmp += 64;
    dm_wr = tmp;
}

void drawtr(void)
{
    unsigned char n=dm_l,l;
    for(l=0;l<n;l++)
        dm_wr[l] = dm_rd[0];
}

void tofront4x7(int n)
{
    int m=7;
    while(m--) {
        dm_buf[n]|=0x2000;
        dm_buf[n+1]|=0x2000;
        dm_buf[n+2]|=0x2000;
        dm_buf[n+3]|=0x2000;
        n+=64;
    }
}
void tofront5x4(int n)
{
    int m=5;
    while(m--) {
        dm_buf[n]|=0x2000;
        dm_buf[n+64]|=0x2000;
        dm_buf[n+128]|=0x2000;
        dm_buf[n+192]|=0x2000;
        n++;
    }
}
void tofrontv(int n)
{
    int m=n;
    n&=0x783f;
    while(n!=m) {
        dm_buf[n]|=0x2000;
        dm_buf[n+1]|=0x2000;
        dm_buf[n+2]|=0x2000;
        dm_buf[n+3]|=0x2000;
        n+=64;
    }
}
void tofronth(int n)
{
    int m=n;
    n&=0x7fe0;
    while(n!=m) {
        dm_buf[n]|=0x2000;
        dm_buf[n+64]|=0x2000;
        dm_buf[n+128]|=0x2000;
        dm_buf[n+192]|=0x2000;
        n++;
    }
}

void drawadd4(uint8_t const * const rom,
              int             const p_x)
{
    int m = 4;
    int x = p_x;
    
    dm_rd = (uint16_t*) ( rom + 0x1b52 + ldle16b_i(rom + 0x4e06, dm_l) );
    
    while(m--)
    {
        dm_buf[x + 0x1040] = dm_rd[0];
        dm_buf[x + 0x1080] = dm_rd[1];
        dm_buf[x +   0xc0] = dm_rd[2];
        
        dm_rd += 3;
        
        x++;
    }
    
    x -= 4;
    
    tofrontv(x + 0x100);
}

void drawaef0(uint8_t const * const rom,
              int             const p_x)
{
    int m = 2;
    int x = p_x;
    
    dm_rd = (uint16_t*) ( rom + 0x1b52 + ldle16b_i(rom + 0x4ec6, dm_l) );
    
    while(m--)
    {
        dm_buf[x + 0x1001] = dm_rd[0];
        dm_buf[x + 0x1041] = dm_rd[1];
        dm_buf[x + 0x1081] = dm_rd[2];
        dm_buf[x + 0x10c1] = dm_rd[3];
        
        dm_rd += 4;
        
        x++;
    }
    
    dm_buf[x +   1] = dm_rd[0];
    dm_buf[x +  65] = dm_rd[1];
    dm_buf[x + 129] = dm_rd[2];
    dm_buf[x + 193] = dm_rd[3];
    
    x += 2;
    
    while(x & 0x3f)
    {
        dm_buf[x      ] |= 0x2000;
        dm_buf[x +  64] |= 0x2000;
        dm_buf[x + 128] |= 0x2000;
        dm_buf[x + 192] |= 0x2000;
        
        x++;
    }
}

const static char obj_w[64]={
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,4,4,4,4,
    2,2,2,2,4,2,2,2,2,2,4,4,4,4,2,2,4,4,4,2,6,4,4,4,
    4,4,4,4,2,4,4,10,4,4,4,4,24,3,6,1
};
const static char obj_h[64]={
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,
    2,2,2,2,4,3,2,2,2,3,5,3,4,4,3,2,5,4,2,3,3,4,4,4,
    4,4,4,4,2,2,2,4,3,3,3,3,6,3,3,7
};
const static char obj2_w[128]={
    4,4,4,1,1,1,1,1,1,1,1,1,1,16,1,1,
    2,2,5,2,4,10,2,16,2,2,2,4,4,4,4,4,
    4,4,2,2,2,2,4,4,4,4,44,2,4,14,28,2,
    2,4,4,4,3,3,6,6,3,3,4,4,6,6,2,2,
    2,2,2,2,2,2,2,4,4,2,2,8,6,6,4,2,
    2,2,2,2,14,3,2,2,2,2,4,3,6,6,2,2,
    3,3,22,2,2,2,4,4,4,3,3,4,4,4,3,3,
    4,8,10,64,8,2,8,8,8,4,4,20,2,2,2,1
};
const static char obj2_h[128]={
    3,5,7,1,1,1,1,1,1,1,1,1,1,4,1,1,
    2,2,8,2,3,8,2,4,2,2,2,4,4,4,4,4,
    4,4,2,2,2,2,4,4,4,4,44,2,4,14,25,2,
    2,3,3,4,2,2,3,3,2,2,6,6,4,4,2,2,
    2,2,2,2,2,2,2,4,4,2,2,3,8,3,3,2,
    2,2,2,2,14,5,2,2,2,2,2,5,4,3,2,2,
    6,6,13,2,2,2,4,3,3,4,4,4,3,3,4,4,
    10,8,8,64,8,2,3,3,8,3,4,8,2,2,2,1
};

const char obj3_w[248]={
    0,0,0,0,0,-4,-4,0,0,5,5,5,5,5,5,5,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    5,2,3,2,2,2,2,2,2,2,2,2,2,2,2,13,
    13,1,1,0,3,1,-2,-2,-2,-4,-4,-4,-2,-4,-12,2,
    2,2,2,2,2,2,2,2,2,0,0,-12,2,2,2,2,
    1,2,2,0,1,-8,-8,1,1,1,1,2,2,5,-2,22,
    2,4,4,4,4,4,4,2,2,1,1,1,2,2,1,1,
    4,1,1,4,4,2,3,3,2,1,1,2,1,2,1,2,
    2,3,3,3,3,3,3,2,2,2,1,1,1,1,1,2,
    4,4,2,2,4,2,2,1,1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    7,7,0,2,2,0,0,0,0,0,0,-2,0,0,1,1,
    0,12,0,0,0,0,0,0,0,0,0,1,1,3,3,1,
    1,0,0,1,1,1,1,0,4,0,4,0,8,2,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
};

const char obj3_h[248]={
    2,4,4,4,4,4,4,2,2,9,9,9,9,9,9,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,3,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,4,1,1,4,4,3,4,3,3,8,4,2,1,
    1,1,1,1,1,1,1,5,3,2,2,2,3,4,4,4,
    1,3,3,2,1,2,2,1,1,1,1,3,3,3,2,1,
    0,0,0,0,0,-4,-4,0,0,3,0,0,13,13,1,1,
    0,3,1,-2,-2,-2,-4,-4,-12,0,0,-12,1,0,1,-8,
    -8,-4,-4,-4,-4,2,2,-2,5,-2,22,7,7,0,0,3,
    0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,1,4,1,1,4,4,4,2,2,4,2,2,2,1,1,
    0,6,0,0,0,0,0,0,0,0,0,1,1,0,0,1,
    1,0,0,1,1,1,1,0,4,0,4,0,5,2,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
};

const char obj3_m[248]={
    2,2,2,2,2,6,6,2,2,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,4,1,0,6,6,4,6,8,8,4,6,14,1,
    1,1,1,1,1,1,1,2,2,4,4,14,2,2,2,2,
    1,2,2,2,0,12,12,0,0,0,0,2,2,1,4,1,
    2,2,2,2,2,6,6,2,2,1,1,1,1,1,0,0,
    4,1,0,6,6,6,8,8,14,1,1,14,1,2,0,12,
    12,8,8,8,8,2,2,6,1,4,1,1,1,1,1,2,
    2,2,2,2,4,2,2,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
    1,1,4,1,1,2,2,2,2,2,4,4,2,2,0,0,
    4,2,4,3,4,4,4,4,4,4,4,0,0,1,1,0,
    0,4,4,0,0,0,0,3,4,4,4,4,2,2,2,4,
    4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

const unsigned char obj3_t[248]=
{
    0,2,2,1,1,1,1,1,1,97,65,65,97,97,65,65,
    97,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,
    97,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    128,130,130,129,129,129,129,129,129,129,129,129,129,129,129,129,
    129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
    129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
    130,130,128,128,129,129,129,129,129,129,129,129,129,129,129,129,
    65,65,65,113,65,65,65,65,113,65,65,65,113,129,129,129,
    1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,3,1,1,20,4,1,
    1,3,3,1,1,1,1,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

// =============================================================================

void Getstringobjsize(char const * str, RECT *rc)
{
    SIZE s;
    
    GetTextExtentPoint32(objdc,str,strlen(str), &s);
    
    rc->bottom = s.cy;
    rc->right = s.cx;
    
    if(rc->bottom < 16)
        rc->bottom = 16;
    
    if(rc->right < 16)
        rc->right = 16;
    
    rc->bottom += rc->top;
    rc->right  += rc->left;
}

// =============================================================================

const static char obj4_h[4]={5,7,11,15};
const static char obj4_w[4]={8,16,24,32};

// \task This can probably be moved to DungeonMap.c
void Getdungobjsize(int chk,RECT*rc,int n,int o,int p)
{
    // Loads object sizes
    int a = 0,
        b = 0,
        c = 0,
        d = 0,
        e = 0;
    
    char*f;
    switch(chk) {
    case 0: case 2: case 4:
        if(dm_k >= 0x100)
            rc->right=obj_w[dm_k - 0x100]<<3,rc->bottom=obj_h[dm_k - 0x100]<<3;
        else if(dm_k >= 0xf8)
        {
            rc->right = obj2_w[ ( (dm_k - 0xf8) << 4) + dm_l] << 3;
            rc->bottom = obj2_h[ ( (dm_k - 0xf8) << 4) + dm_l] << 3;
        }
        else {
            c=0;
            d=dm_l;
            e=obj3_t[dm_k];
            switch(e&15) {
            case 0:
                len1();
                break;
            case 1:
                dm_l++;
                break;
            case 2:
                len2();
                break;
            case 3:
                c=((dm_l&3)+1)*obj3_m[dm_k];
                dm_l>>=2;
                dm_l++;
                break;
            
            case 4:
                
                c = ( obj4_h[dm_l >> 2] + 3 ) << 1;
                
                dm_l = obj4_w[dm_l & 3];
                
                if(e & 16)
                    rc->left -= dm_l << 3;
                
                break;
            }
            
            dm_l*=obj3_m[dm_k];
            switch(e&192) {
            case 0:
                a=dm_l;
                b=c;
                break;
            case 64:
                a=dm_l;
                b=dm_l;
                break;
            case 128:
                a=0;
                b=dm_l;
                break;
            }
            
            if(e & 32)
                rc->top -= ( b + ( (e & 16) ? 2 : 4) ) << 3;
            
            a += obj3_w[dm_k];
            b += obj3_h[dm_k];
            
            rc->right = a << 3;
            rc->bottom = b << 3;
            
            dm_l=d;
        }
        if(dm_k==0xff) {if(dm_l==8) rc->left-=16; else if(dm_l==3) rc->left=-n,rc->top=-o;}
        else if(dm_k==0xfa) {
            if(dm_l==14) rc->left+=16,rc->top+=32;
            else if(dm_l==10) rc->left=80-n,rc->top=64-o;
        }
        break;
    case 1: case 3: case 5:
        switch(dm_k) {
        case 0:
            if(dm_l==24) {
                rc->right=176;
                rc->bottom=96;
            } else if(dm_l==25) rc->right=rc->bottom=32; else if(dm_dl>8) {
                rc->top-=120;
                rc->bottom=144;
                rc->right=32;
            } else if(dm_dl>5) {
                rc->top-=72;
                rc->bottom=96;
                rc->right=32;
            } else rc->right=32,rc->bottom=24;
            break;
        case 1:
            if(dm_l==5 || dm_l==6) {
                rc->right=96;
                rc->bottom=64;
                rc->left-=32;
                rc->top-=32;
            } else if(dm_l==2 || dm_l==7 || dm_l==8) {
                rc->right=rc->bottom=32;
            } else rc->right=32,rc->bottom=24,rc->top+=8;
            break;
        case 2:
            rc->bottom=32;
            if(dm_dl>8) {
                rc->left-=104;
                rc->right=128;
            } else if(dm_dl>5) {
                rc->left-=56;
                rc->right=80;
            } else rc->right=24;
            break;
        case 3:
            rc->right=24,rc->bottom=32,rc->left+=8;
        }
        break;
    case 8: case 9:
        rc->right=16;
        rc->bottom=16;
        break;
    case 7:
        Getstringobjsize(cur_sec, rc);
        goto blah2;
    case 6:
        if(dm_k>=0x11c) f="Crash"; else f=sprname[dm_k];
        Getstringobjsize(f,rc);
blah2:

// \note Disabled because it prevents us from extending marker text
// outside of the de facto map area.
#if 0
        if(rc->right>512-n) rc->right=512-n;
        if(rc->bottom>512-o) rc->bottom=512-o;
#endif
        goto blah;
    }
    rc->right+=rc->left;
    rc->bottom+=rc->top;
blah:
    if(!p)
    {
        if(rc->left < -n)
        {
            rc->left = -n;
            rc->top -= (-rc->left - n) >> 9 << 3;
            rc->right = 512 - n;
        }
        
        if(rc->top<-o)
        {
            rc->top = -o;
            rc->bottom = 512 - o;
        }
        
// \note Disabled because it prevents us from extending marker text
// outside of the de facto map area.
#if 0
        if(rc->right > 512 - n)
        {
            rc->left = -n;
            rc->bottom += (rc->right + n) >> 9 << 3;
            rc->right = 512 - n;
        }
        
        if(rc->bottom > 512 - o)
        {
            rc->top    = -o;
            rc->bottom = 512 - o;
        }
#endif
    }
}

void
setobj(DUNGEDIT*ed, unsigned char *map)
{
    unsigned char c=0;
    short k,l,m,n,o;
    unsigned char*rom;
    dm_x&=0xfff;
    dm_l&=0xf;
    o=map[2];
    if(dm_k>0xff) {
        if((dm_x&0xf3f)==0xf3f) goto invalobj;
        map[0]=0xfc + ((dm_x>>4)&3);
        map[1]=(dm_x<<4)+(dm_x>>8);
        map[2]=(dm_x&0xc0)|dm_k;
    } else {
        if((dm_x&0x3f)==0x3f) goto invalobj;
        if(dm_k<0xf8) {
            if((dm_l==3||!dm_l) && dm_x==0xffc) {
invalobj:
                if(ed->withfocus&10) ed->withfocus|=4;
                else MessageBox(framewnd,"You cannot place that object there.","No",MB_OK);
                getobj(map);
                return;
            }
            map[0]=(dm_x<<2)|(dm_l>>2);
            map[1]=((dm_x>>4)&0xfc)|(dm_l&3);
        } else {
            if((dm_l==12||!dm_l) && dm_x==0xffc) goto invalobj;
            if((dm_k==0xf9 && dm_l==9) || (dm_k==0xfb && dm_l==1)) c=1;
            map[0]=(dm_x<<2)|(dm_l&3);
            map[1]=((dm_x>>4)&0xfc)|(dm_l>>2);
        }
        map[2]=(unsigned char)dm_k;
    }
    if(c && !ed->ischest)
    {
        rom = ed->ew.doc->rom;
        
        m = 0;
        
        for(l=0;l<0x1f8;l+=3)
            if( is16b_neg1(rom + 0xe96e + l) )
                break;
        
        if(l!=0x1f8)
        {
            for(k=0;k<0x1f8;k+=3)
            {
                n = ldle16b(rom + 0xe96e + l);
                
                if(n == ed->mapnum)
                {
                    if(ed->chestloc[m++]>map-ed->buf) {
                        if(l<k)
                            MoveMemory(rom + 0xe96e + l, rom + 0xe96e + l + 3,
                                       k - l - 3);
                        else
                            MoveMemory(rom + 0xe96e + k + 3, rom + 0xe96e + k, l - k - 3);
setchest:
                        *(short*)(rom + 0xe96e + k) = ed->mapnum | ((dm_k == 0xfb) ? 0x8000 : 0);
                        rom[0xe970 + k] = 0;
                        break;
                    }
                }
            }
            if(k==0x1f8) {k=l; goto setchest; }
        }
    } else if(ed->ischest && (map[2]!=o || !c)) {
        for(k=0;k<ed->chestnum;k++) if(map-ed->buf==ed->chestloc[k]) break;
        for(l=0;l<0x1f8;l+=3) {
            if((*(short*)(ed->ew.doc->rom + 0xe96e + l)&0x7fff)==ed->mapnum) {
                k--;
                if(k<0) {
                    *(short*)(ed->ew.doc->rom + 0xe96e + l)=c?(ed->mapnum+((o==0xf9)?32768:0)):-1;
                    break;
                }
            }
        }
    }
    
    if(ed->withfocus&4) {SetCursor(normal_cursor);ed->withfocus&=-5;}
    
    ed->modf=1;
}

void getdoor(unsigned char const *       map,
             unsigned char const * const rom)
{
    unsigned short i = *(unsigned short*) map;
    
    dm_dl = (i & 0xf0) >> 4;
    dm_l = i >> 9;
    dm_k = i & 3;
    
    if(dm_l == 24 && !dm_k)
        dm_x = ( ldle16b_i(rom + 0x19de, dm_dl) ) >> 1;
    else
        dm_x = ( ldle16b_i(rom + 0x197e, dm_dl + dm_k * 12) ) >> 1;
}

void
setdoor(unsigned char *map)
{
    dm_k&=3;
    map[0]=dm_k+(dm_dl<<4);
    map[1]=dm_l<<1;
}

unsigned char const *
Drawmap(unsigned char  const * const rom,
        unsigned short       * const nbuf,
        unsigned char  const *       map,
        DUNGEDIT             * const ed)
{
    unsigned short i;
    unsigned char l,m,o;
    short n;
    unsigned char ch = ed->chestnum;
    
    uint16_t *dm_src, *tmp;
    
    for( ; ; )
    {
        i=*(unsigned short*)map;
        if(i==0xffff) break;
        if(i==0xfff0) {
            for(;;) {
                
                map+=2;
                
                i=*(unsigned short*)(map);
                
                if(i == 0xffff)
                    goto end;
                
                getdoor(map, rom);
                dm_wr=nbuf+dm_x;
                switch(dm_k) {
                case 0:
                    switch(dm_l) {
                    case 24:
                        dm_l=42;
                        dm_rd = (uint16_t*) (rom + 0x1b52 + ((unsigned short*)(rom + 0x4e06))[dm_l]);
                        drawhole(18,nbuf);
                        dm_rd = (uint16_t*) (rom + 0x1b52 + ((unsigned short*)(rom + 0x4d9e))[dm_l]);
                        dm_x+=364;
                        drawhole(18,nbuf);
                        break;
                    case 11: case 10: case 9:
                        break;
                    case 25:
                        dm_rd = (uint16_t*) (rom + 0x22dc);
                        drawXx4(4);
                        break;
                    case 3:
                        tofrontv(dm_x+64);
                        break;
                    case 1:
                        tofront4x7(dm_x&0x783f);
                    default:
                        if(dm_l<0x20) {
                            if(dm_dl>5) {
                                dm_wr=nbuf+((((unsigned short*)(rom + 0x198a))[dm_dl])>>1);
                                
                                if(dm_l==1)
                                    tofront4x7(dm_x+256);
                                
                                dm_rd = (uint16_t*)
                                ( rom + 0x1b52 + ldle16b_i(rom + 0x4e06, dm_l) );
                                
                                m=4;
                                while(m--) {
                                    dm_wr[64]=dm_rd[0];
                                    dm_wr[128]=dm_rd[1];
                                    dm_wr[192]=dm_rd[2];
                                    dm_rd+=3;
                                    dm_wr++;
                                }
                            }
                            
                            n = 0;
                            
                            dm_rd = (uint16_t*)
                            (
                                rom + 0x1b52
                              + ldle16b_i(rom + 0x4d9e, dm_l)
                            );
                            
                            dm_wr=nbuf+dm_x;
                            m=4;
                            while(m--) {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_rd+=3;
                                dm_wr++;
                            }
                        } else {
                            n=dm_x;
                            if(dm_dl>5 && dm_l!=0x23) {
                                dm_x=(((unsigned short*)(rom + 0x198a))[dm_dl])>>1;
                                drawadd4(rom,dm_x);
                            }
                            
                            dm_x = n;
                            
                            dm_rd = (uint16_t*)
                            (
                                rom + 0x1b52
                              + ldle16b_i(rom + 0x4d9e, dm_l)
                            );
                            
                            m=4;
                            while(m--) {
                                dm_buf[dm_x]=dm_rd[0];
                                dm_buf[dm_x + 0x1040]=dm_rd[1];
                                dm_buf[dm_x + 0x1080]=dm_rd[2];
                                dm_rd+=3;
                                dm_x++;
                            }
                            dm_x-=4;
                            if(dm_l!=0x23) tofrontv(dm_x);
                        }
                    }
                    break;
                case 1:
                    switch(dm_l) {
                    case 3:
                        while(dm_x&0x7c0) {
                            dm_buf[dm_x]|=0x2000;
                            dm_buf[dm_x+1]|=0x2000;
                            dm_buf[dm_x+2]|=0x2000;
                            dm_buf[dm_x+3]|=0x2000;
                            dm_x+=64;
                        }
                        break;
                    case 11: case 10: case 9:
                        break;
                    case 5: case 6:
                        dm_rd = (uint16_t*)(rom + 0x41a8);
                        o=10;
                        l=8;
                        dm_wr-=0x103;
                        tmp=dm_wr;
                        while(l--) {
                            m=o;
                            while(m--) *(dm_wr++)=*(dm_rd++);
                            tmp+=64;
                            dm_wr=tmp;
                        }
                        break;
                    case 2: case 7: case 8:
                        tofront4x7(dm_x + 0x100);
                        dm_rd = (uint16_t*)(rom + 0x4248);
                        drawXx4(4);
                        dm_x+=188;
                        m=4;
                        while(m--) dm_buf[dm_x++]|=0x2000;
                        break;
                    case 1:
                        tofront4x7(dm_x);
                    
                    // \task falls through. Intentional?
                    default:
                        
                        dm_rd = (uint16_t*)
                        ( rom + 0x1b52 + ldle16b_i(rom + 0x4e06, dm_l) );
                        
                        if(dm_l<0x20) {
                            m=4;
                            while(m--) {
                                dm_wr[64]=dm_rd[0];
                                dm_wr[128]=dm_rd[1];
                                dm_wr[192]=dm_rd[2];
                                dm_rd+=3;
                                dm_wr++;
                            }
                        } else drawadd4(rom,dm_x);
                    }
                    break;
                case 2:
                    switch(dm_l) {
                    case 10: case 11:
                        break;
                    case 3:
                        tofronth(dm_x+1);
                        break;
                    case 1:
                        tofront5x4(dm_x&0x7fe0);
                    default:
                        if(dm_l<0x20) {
                            if(dm_dl > 5)
                            {
                                dm_wr = nbuf
                                      + ( ( ldle16b_i(rom + 0x19ba, dm_dl) ) >> 1);
                                
                                dm_rd = (uint16_t*)
                                (
                                    rom + 0x1b52
                                  + ldle16b_i(rom + 0x4ec6, dm_l)
                                );
                                
                                m = 3;
                                
                                dm_wr++;
                                
                                while(m--)
                                {
                                    dm_wr[0]   = dm_rd[0];
                                    dm_wr[64]  = dm_rd[1];
                                    dm_wr[128] = dm_rd[2];
                                    dm_wr[192] = dm_rd[3];
                                    dm_rd+=4;
                                    dm_wr++;
                                }
                            }
                            n=0;
                            
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + ldle16b_i(rom + 0x4e66, dm_l) );
                            
                            dm_wr=nbuf+dm_x;
                            m=3;
                            while(m--) {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_wr[192]=dm_rd[3];
                                dm_rd+=4;
                                dm_wr++;
                            }
                        }
                        else
                        {
                            if(dm_dl > 5)
                                drawaef0(rom,(((uint16_t*)(rom + 0x19ba))[dm_dl])>>1);
                            
                            m = 2;
                            
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + ldle16b_i(rom + 0x4e66, dm_l) );
                            
                            dm_buf[dm_x      ] = dm_rd[0];
                            dm_buf[dm_x +  64] = dm_rd[1];
                            dm_buf[dm_x + 128] = dm_rd[2];
                            dm_buf[dm_x + 192] = dm_rd[3];
                            
                            while(m--) {
                                dm_buf[dm_x + 0x1001]=dm_rd[4];
                                dm_buf[dm_x + 0x1041]=dm_rd[5];
                                dm_buf[dm_x + 0x1081]=dm_rd[6];
                                dm_buf[dm_x + 0x10c1]=dm_rd[7];
                                dm_rd+=4;
                                dm_x++;
                            }
                        }
                    }
                    break;
                case 3:
                    switch(dm_l) {
                    case 10: case 11:
                        break;
                    case 3:
                        dm_x+=2;
                        while(dm_x&0x3f) {
                            dm_buf[dm_x]|=0x2000;
                            dm_buf[dm_x+64]|=0x2000;
                            dm_buf[dm_x+128]|=0x2000;
                            dm_buf[dm_x+192]|=0x2000;
                            dm_x++;
                        }
                        break;
                    case 1:
                        tofront5x4(dm_x+4);
                    default:
                        
                        if(dm_l < 0x20)
                        {
                            dm_rd = (uint16_t*)
                            ( rom + 0x1b52 + ldle16b_i(rom + 0x4ec6, dm_l) );
                            
                            dm_wr++;
                            
                            m = 3;
                            
                            while(m--)
                            {
                                dm_wr[0]=dm_rd[0];
                                dm_wr[64]=dm_rd[1];
                                dm_wr[128]=dm_rd[2];
                                dm_wr[192]=dm_rd[3];
                                dm_rd+=4;
                                dm_wr++;
                            }
                        }
                        else
                            drawaef0(rom, dm_x);
                    }
                }
            }
        }
        
        getobj(map);
        
        map += 3;
        
        dm_wr=nbuf+dm_x;
        
        if(dm_k >= 0x100)
        {
            dm_src = dm_rd = (uint16_t*)
            ( rom + 0x1b52 + ldle16b_i(rom + 0x81f0, dm_k) );
            
            switch(dm_k - 0x100) {
            case 59:
//              dm_rd=(short*)(rom + 0x2ce2);
                goto d43;
            case 58:
//              dm_rd=(short*)(rom + 0x2cca);
                goto d43;
            case 57:
//              dm_rd=(short*)(rom + 0x2cb2);
                goto d43;
            case 56:
//              dm_rd=(short*)(rom + 0x2c9a);
d43:
                drawXx3(4);
                break;
            case 45: case 46: case 47:
            case 50: case 51:
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            case 36: case 37: case 41: case 28:
                drawXx4(4);
                break;
            case 48: case 49:
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
                drawXx4bp(4);
                break;
            case 16: case 17: case 18: case 19:
                drawXx4bp(3);
                break;
            case 20: case 21: case 22: case 23:
                drawXx3bp(4);
                break;
            case 52:
            case 24: case 25: case 26: case 27: case 30: case 31: case 32:
            case 39:
                draw2x2();
                break;
            case 29: case 33: case 38: case 43:
                drawXx3(2);
                break;
            case 34: case 40:
                dm_l=5;
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[1]=dm_rd[1];
                    dm_wr[2]=dm_rd[2];
                    dm_wr[3]=dm_rd[3];
                    dm_rd+=4;
                    dm_wr+=64;
                }
                break;
            case 35:
                drawXx3(4);
                break;
            case 42: case 53:
                dm_l=1;
                draw4x2(1);
                break;
            case 62:
            case 44:
                drawXx3(6);
                break;
            case 54:
                
                dm_rd = (uint16_t*) (rom + 0x2c5a);
                
                dm_l = 1;
                
                goto case99;
            
            case 55:
                drawXx4(10);
                break;
            case 60:
                n=*dm_rd;
                dm_l=6;
                while(dm_l--) {
                    dm_buf[dm_x+1]=dm_buf[dm_x+5]=dm_buf[dm_x+9]=
                    dm_buf[dm_x+15]=dm_buf[dm_x+19]=dm_buf[dm_x+23]=
                    (dm_buf[dm_x]=dm_buf[dm_x+4]=dm_buf[dm_x+8]=
                    dm_buf[dm_x+14]=dm_buf[dm_x+18]=dm_buf[dm_x+22]=dm_rd[0])|0x4000;
                    dm_buf[dm_x+3]=dm_buf[dm_x+7]=dm_buf[dm_x+17]=
                    dm_buf[dm_x+21]=
                    (dm_buf[dm_x+2]=dm_buf[dm_x+6]=dm_buf[dm_x+16]=
                    dm_buf[dm_x+20]=dm_rd[6])|0x4000;
                    dm_rd++;
                    dm_x+=64;
                }
                dm_rd+=6;
                dm_wr+=10;
                drawXx3(4);
                break;
            case 61:
                drawXx3(3);
                break;
            case 63:
                dm_l=8;
                while(dm_l--) {
                    dm_buf[dm_x]=dm_rd[0];
                    dm_buf[dm_x + 0x40]=dm_rd[1];
                    dm_buf[dm_x + 0x80]=dm_rd[2];
                    dm_buf[dm_x + 0xc0]=dm_rd[3];
                    dm_buf[dm_x + 0x100]=dm_rd[4];
                    dm_buf[dm_x + 0x140]=dm_rd[5];
                    dm_buf[dm_x + 0x180]=dm_rd[6];
                    dm_rd+=7;
                }
                break;
            }
        }
        else
        {
            if(dm_k >= 0xf8)
            {
                dm_k &= 7;
                dm_k <<= 4;
                dm_k |= dm_l;
                
                dm_src = dm_rd = (uint16_t*)
                (rom + 0x1b52 + ldle16b_i(rom + 0x84f0, dm_k) );
                
                switch(dm_k)
                {
                case 0:
                    m=3;
rows4:
                    while(m--) {
                        dm_wr[0]=dm_rd[0];
                        dm_wr[1]=dm_rd[1];
                        dm_wr[2]=dm_rd[2];
                        dm_wr[3]=dm_rd[3];
                        dm_rd+=4;
                        dm_wr+=64;
                    }
                    break;
                case 1:
                    m=5;
                    goto rows4;
                case 2:
                    m=7;
                    goto rows4;
                case 3: case 14:
                case 4: case 5: case 6: case 7: case 8: case 9:
                case 10: case 11: case 12: case 15:
                    
                    dm_wr[0]=dm_rd[0];
                    
                    break;
                case 13:
                    dm_rd = (uint16_t*) (rom + 0x2fda);
                case 23:
                    m=5;
                    tmp=dm_wr;
                    while(m--) {
                        // \task Endianness issues with derived pointers like
                        // this. These will be tough to track down.
                        dm_wr[2]=dm_wr[9]=dm_rd[1];
                        dm_wr[73]=(dm_wr[66]=dm_rd[2])|0x4000;
                        dm_wr[137]=(dm_wr[130]=dm_rd[4])|0x4000;
                        dm_wr[201]=(dm_wr[194]=dm_rd[5])|0x4000;
                        dm_wr++;
                    }
                    dm_wr=tmp;
                    dm_wr[15]=(dm_wr[0]=dm_rd[0])|0x4000;
                    dm_wr[14]=dm_wr[8]=dm_wr[7]=dm_wr[1]=dm_rd[1];
                    dm_wr[142]=(dm_wr[129]=dm_rd[3])|0x4000;
                    break;
                case 25:
                    if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
                case 16: case 17: case 19: case 34: case 35: case 36:
                case 37:
                case 22: case 24: case 26: case 47: case 48: case 62:
                case 63: case 64: case 65: case 66: case 67: case 68: case 69:
                case 70: case 73: case 74: case 79: case 80: case 81: case 82:
                case 83: case 86: case 87: case 88: case 89: case 94:
                case 95: case 99: case 100: case 101: case 117: case 124:
                case 125: case 126:
                    draw2x2();
                    break;
                case 18:
                    m=3;
                    while(m--) {
                        dm_wr[384]=dm_wr[192]=dm_wr[0]=dm_rd[0];
                        dm_wr[448]=dm_wr[256]=dm_wr[64]=dm_rd[1];
                        dm_wr+=2;
                    }
                    break;
                case 20:
                    drawXx3(4);
                    break;
                case 21: case 114:
                    o=10;
                    l=8;
                    tmp=dm_wr;
                    while(l--) {
                        m=o;
                        while(m--) *(dm_wr++)=*(dm_rd++);
                        tmp+=64;
                        dm_wr=tmp;
                    }
                    break;
                case 27: case 28: case 30: case 31: case 32:
                case 33:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_buf[dm_x]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_buf[dm_x + 0x40]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_buf[dm_x + 0x80]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_buf[dm_x + 0xc0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 112:
                    drawXx4(4);
                    dm_wr+=124;
                    dm_rd-=16;
                    drawXx4(4);
                    dm_wr+=252;
                    drawXx4(4);
                    break;
                case 113:
                    drawXx4(4);
                    drawXx4(4);
                    dm_wr+=248;
                    drawXx4(4);
                    drawXx4(4);
                    break;
                case 120:
                    drawXx4(4);
                    dm_wr+=250;
                    drawXx4(4);
                    dm_rd-=16;
                case 29: case 102: case 107:
                case 122:
                    drawXx4(4);
                    break;
                case 38: case 39:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_buf[dm_x]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 40: case 41:
                    m=4;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_buf[dm_x + 0xc0]=dm_rd[3];
                        dm_x++;
                        dm_rd+=4;
                    }
                    break;
                case 42:
                    dm_rd+=0xb6e;
                    dm_x=0x20a;
                    draw12x12();
                    dm_rd-=3;
                    dm_x=0x22a;
                    draw12x12();
                    dm_rd--;
                    dm_x=0xa0a;
                    draw12x12();
                    dm_rd-=5;
                    dm_x=0xa2a;
                    draw12x12();
                    break;
                case 44:
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    dm_wr+=124;
                    dm_rd+=4;
                    draw2x2();
                    dm_rd+=4;
                case 43:
                    draw2x2();
                    break;
                case 45:
                    dm_rd+=0xda5;
                    m=14;
                    while(m--) {
                        dm_wr[13]=(dm_wr[0]=dm_rd[0])|0x4000;
                        dm_wr[12]=dm_wr[11]=
                        (dm_wr[2]=dm_wr[1]=dm_rd[14])^0x4000;
                        dm_wr[10]=(dm_wr[3]=dm_rd[28])^0x4000;
                        dm_wr[9]=(dm_wr[4]=dm_rd[42])^0x4000;
                        dm_wr[8]=(dm_wr[5]=dm_rd[56])^0x4000;
                        dm_wr[7]=(dm_wr[6]=dm_rd[70])^0x4000;
                        dm_wr+=64;
                        dm_rd++;
                    }
                    break;
                case 46:
                    n=dm_x;
                    m=6;
                    dm_rd+=0xdf9;
                    while(m--) {
                        dm_buf[dm_x + 0x107]=dm_buf[dm_x + 0x10d]=dm_buf[dm_x + 0x113]=dm_rd[0];
                        dm_buf[dm_x + 0x147]=dm_buf[dm_x + 0x14d]=dm_buf[dm_x + 0x153]=dm_rd[1];
                        dm_buf[dm_x + 0x187]=dm_buf[dm_x + 0x18d]=dm_buf[dm_x + 0x193]=dm_rd[2];
                        dm_buf[dm_x + 0x1c7]=dm_buf[dm_x + 0x1cd]=dm_buf[dm_x + 0x1d3]=dm_rd[3];
                        dm_rd+=4;
                        dm_x++;
                    }
                    m=5;
                    dm_x=n;
                    while(m--)
                    {
                        dm_buf[dm_x  + 0x117]=dm_buf[dm_x + 0x158]=dm_buf[dm_x + 0x199]=
                        dm_buf[dm_x  + 0x1da]=dm_buf[dm_x + 0x21b]=dm_buf[dm_x + 0x25c]=dm_buf[dm_x + 0x29d]=
                        (dm_buf[dm_x + 0x282]=dm_buf[dm_x + 0x243]=dm_buf[dm_x + 0x204]=
                        dm_buf[dm_x  + 0x1c5]=dm_buf[dm_x + 0x186]=dm_buf[dm_x + 0x147]=dm_buf[dm_x + 0x108]=dm_rd[0])|0x4000;
                        dm_rd++;
                        dm_x+=64;
                    }
                    m=6;
                    dm_x=n;
                    while(m--) {
                         dm_buf[dm_x + 0x2dd] = dm_buf[dm_x + 0x45d]=dm_buf[dm_x + 0x5dd]=
                        (dm_buf[dm_x + 0x2c2] = dm_buf[dm_x + 0x442]=dm_buf[dm_x + 0x5c2]=dm_rd[0])|0x4000;
                         dm_buf[dm_x + 0x2dc] = dm_buf[dm_x + 0x45c]=dm_buf[dm_x + 0x5dc]=
                        (dm_buf[dm_x + 0x2c3] = dm_buf[dm_x + 0x443]=dm_buf[dm_x + 0x5c3]=dm_rd[1])|0x4000;
                         dm_buf[dm_x + 0x2db] = dm_buf[dm_x + 0x45b]=dm_buf[dm_x + 0x5db]=
                        (dm_buf[dm_x + 0x2c4] = dm_buf[dm_x + 0x444]=dm_buf[dm_x + 0x5c4]=dm_rd[2])|0x4000;
                         dm_buf[dm_x + 0x2da] = dm_buf[dm_x + 0x45a]=dm_buf[dm_x + 0x5da]=
                        (dm_buf[dm_x + 0x2c5] = dm_buf[dm_x + 0x445]=dm_buf[dm_x + 0x5c5]=dm_rd[3])|0x4000;
                        dm_rd+=4;
                        dm_x+=64;
                    }
                    m=6;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x24c]=dm_buf[dm_x + 0x252]=dm_rd[0];
                        dm_buf[dm_x + 0x28c]=dm_buf[dm_x + 0x292]=dm_rd[6];
                        dm_rd++;
                        dm_x++;
                    }
                    dm_rd+=6;
                    m=6;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x387]=dm_buf[dm_x + 0x507]=dm_rd[0];
                        dm_buf[dm_x + 0x388]=dm_buf[dm_x + 0x508]=dm_rd[1];
                        dm_rd+=2;
                        dm_x+=64;
                    }
                    m=5;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x247]=dm_rd[0];
                        dm_buf[dm_x + 0x287]=dm_rd[1];
                        dm_buf[dm_x + 0x2c7]=dm_rd[2];
                        dm_buf[dm_x + 0x307]=dm_rd[3];
                        dm_buf[dm_x + 0x347]=dm_rd[4];
                        dm_rd+=5;
                        dm_x++;
                    }
                    m=4;
                    dm_x=n;
                    while(m--) {
                        dm_buf[dm_x + 0x70e]|=0x2000;
                        dm_buf[dm_x + 0x74e]|=0x2000;
                        dm_x++;
                    }
                    break;
                case 49:
                    if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
                case 50: case 103: case 104: case 121:
                    drawXx3(4);
                    break;
                case 51: case 72:
                    drawXx4(4);
                    break;
                case 52: case 53: case 56: case 57:
                    draw3x2();
                    break;
                case 54: case 55: case 77: case 93:
                    drawXx3(6);
                    break;
                case 58: case 59:
                    drawXx3(4);
                    dm_wr+=188;
                    drawXx3(4);
                    break;
                case 78:
                    drawXx3(4);
                    break;
                case 60: case 61: case 92:
                    drawXx4(6);
                    break;
                case 71:
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    dm_rd+=4;
                    dm_wr+=124;
                    draw2x2();
                    dm_rd+=4;
                    draw2x2();
                    break;
                case 75: case 118: case 119:
                    drawXx3(8);
                    break;
                case 76:
                    l=8;
                    while(l--) {
                        m=6;
                        while(m--) {
                            dm_wr[0]=dm_rd[0];
                            dm_wr++;
                            dm_rd++;
                        }
                        dm_wr+=58;
                    }
                    break;
                case 84:
                    m=6;
                    tmp=dm_wr;
                    while(m--) {
                        dm_wr[1]=dm_wr[2]=dm_wr[65]=dm_wr[66]=dm_rd[0];
                        dm_wr[130]=(dm_wr[129]=dm_rd[1])|0x4000;
                        dm_wr+=2;
                    }
                    dm_wr=tmp;
                    m=3;
                    while(m--) {
                        dm_wr[0xc1]=dm_wr[0xc3]=dm_wr[0xcb]=dm_wr[0xcd]=
                        (dm_wr[0xc0]=dm_wr[0xc2]=dm_wr[0xca]=dm_wr[0xcc]=dm_rd[2])|0x4000;
                        dm_wr[0xc5]=dm_wr[0xc7]=dm_wr[0xc9]=
                        (dm_wr[0xc4]=dm_wr[0xc6]=dm_wr[0xc8]=dm_rd[5])|0x4000;
                        dm_rd++;
                        dm_wr+=64;
                    }
                    dm_wr=tmp;
                    dm_wr[77]=dm_wr[13]=(dm_wr[64]=dm_wr[0]=dm_rd[5])|0x4000;
                    dm_wr[141]=(dm_wr[128]=dm_rd[6])|0x4000;
                    m=4;
                    dm_wr=tmp;
                    dm_rd=dm_src;
                    while(m--) {
                        dm_wr[0x28a]=(dm_wr[0x283]=dm_rd[10])|0x4000;
                        dm_wr[0x289]=(dm_wr[0x284]=dm_rd[14])|0x4000;
                        dm_wr[0x288]=(dm_wr[0x285]=dm_rd[18])|0x4000;
                        dm_wr[0x287]=(dm_wr[0x286]=dm_rd[22])|0x4000;
                        dm_rd++;
                        dm_wr+=64;
                    }
                    break;
                case 85: case 91:
                    m=3;
                    dm_wr[0]=dm_rd[0];
                    dm_wr[1]=dm_rd[1];
                    dm_wr[2]=dm_rd[2];
                    while(m--) {
                        dm_wr[64]=dm_rd[3];
                        dm_wr[65]=dm_rd[4];
                        dm_wr[66]=dm_rd[5];
                        dm_wr+=64;
                    }
                    dm_wr[64]=dm_rd[6];
                    dm_wr[65]=dm_rd[7];
                    dm_wr[66]=dm_rd[8];
                    break;
                case 90:
                    m=2;
                    while(m--) {
                        dm_wr[0]=dm_rd[0];
                        dm_wr[1]=dm_rd[1];
                        dm_wr[2]=dm_rd[2];
                        dm_wr[3]=dm_rd[3];
                        dm_rd+=4;
                        dm_wr+=64;
                    }
                    break;
                case 96: case 97:
                    drawXx3(3);
                    dm_wr+=189;
                    drawXx3(3);
                    break;
                case 98:
                    m=22;
                    while(m--) {
                        dm_buf[dm_x + 0x1000]=dm_rd[0];
                        dm_buf[dm_x + 0x1040]=dm_rd[1];
                        dm_buf[dm_x + 0x1080]=dm_rd[2];
                        dm_buf[dm_x + 0x10c0]=dm_rd[3];
                        dm_buf[dm_x + 0x1100]=dm_rd[4];
                        dm_buf[dm_x + 0x1140]=dm_rd[5];
                        dm_buf[dm_x + 0x1180]=dm_rd[6];
                        dm_buf[dm_x + 0x11c0]=dm_rd[7];
                        dm_buf[dm_x + 0x1200]=dm_rd[8];
                        dm_buf[dm_x + 0x1240]=dm_rd[9];
                        dm_buf[dm_x + 0x1280]=dm_rd[10];
                        dm_rd+=11;
                        dm_x++;
                    }
                    dm_x-=22;
                    m=3;
                    while(m--) {
                        dm_buf[0x12c9 + dm_x] = dm_rd[0];
                        dm_buf[0x1309 + dm_x] = dm_rd[3];
                        dm_rd++;
                        dm_x++;
                    }
                    break;
                case 105: case 106: case 110: case 111:
                    drawXx4(3);
                    break;
                case 109: case 108:
                    drawXx3(4);
                    break;
                case 115:
                    fill4x2(rom,nbuf,dm_rd + 0x70);
                    break;
                case 123:
                    tmp=dm_wr;
                    draw4x4X(5);
                    tmp+=256;
                    dm_wr=tmp;
                    draw4x4X(5);
                    break;
                case 116:
                    drawXx4(4);
                    drawXx4(4);
                    dm_wr+=248;
                    drawXx4(4);
                    drawXx4(4);
                    break;
                }
                continue;
            }
            
            dm_src = dm_rd = (uint16_t*)
            ( rom + 0x1b52 + ldle16b_i(rom + 0x8000, dm_k) );
            
            switch(dm_k)
            {
            
            case 0:
                len1();
                while(dm_l--) draw2x2();
                break;
            
            case 1: case 2:
                len2();
                while(dm_l--) drawXx4(2),dm_rd=dm_src;
                break;
            
            case 3: case 4:
                dm_l++;
                while(dm_l--) {
                    dm_buf[0x1000 + dm_x] = dm_buf[     + dm_x] = dm_rd[0];
                    dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[1];
                    dm_buf[0x1080 + dm_x] = dm_buf[0x80 + dm_x] = dm_rd[2];
                    dm_buf[0x10c0 + dm_x] = dm_buf[0xc0 + dm_x] = dm_rd[3];
                    dm_buf[0x1001 + dm_x] = dm_buf[0x1  + dm_x] = dm_rd[4];
                    dm_buf[0x1041 + dm_x] = dm_buf[0x41 + dm_x] = dm_rd[5];
                    dm_buf[0x1081 + dm_x] = dm_buf[0x81 + dm_x] = dm_rd[6];
                    dm_buf[0x10c1 + dm_x] = dm_buf[0xc1 + dm_x] = dm_rd[7];
                    
                    dm_x += 2;
                }
                break;
            case 5: case 6:
                dm_l++;
                while(dm_l--) drawXx4(2),dm_rd=dm_src,dm_wr+=4;
                break;
            case 7: case 8:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 9: case 12: case 13: case 16: case 17: case 20:
                dm_l+=6;
                while(dm_l--) draw1x5(),dm_wr-=63;
                break;
            case 21: case 24: case 25: case 28: case 29: case 32:
                n=-63;
case25:
                dm_l+=6;
                while(dm_l--) {
                    dm_buf[dm_x + 0x1000] = dm_buf[dm_x]=dm_rd[0];
                    dm_buf[dm_x + 0x1040] = dm_buf[dm_x + 0x40]=dm_rd[1];
                    dm_buf[dm_x + 0x1080] = dm_buf[dm_x + 0x80]=dm_rd[2];
                    dm_buf[dm_x + 0x10c0] = dm_buf[dm_x + 0xc0]=dm_rd[3];
                    dm_buf[dm_x + 0x1100] = dm_buf[dm_x + 0x100]=dm_rd[4];
                    dm_x+=n;
                }
                break;
            case 23: case 26: case 27: case 30: case 31:
                n=65;
                goto case25;
            case 10: case 11: case 14: case 15: case 18: case 19: case 22:
                dm_l+=6;
                while(dm_l--) draw1x5(),dm_wr+=65;
                break;
            case 33:
                dm_l=dm_l*2+1;
                drawXx3(2);
                while(dm_l--) dm_rd-=3,drawXx3(1);
                drawXx3(1);
                break;
            case 34:
                dm_l+=2;
case34b:
                if(*dm_wr!=0xe2) *dm_wr=*dm_rd;
case34:
                dm_wr++;
                dm_rd++;
                while(dm_l--) *(dm_wr++)=*dm_rd;
                dm_rd++;
                *dm_wr=*dm_rd;
                break;
            case 35: case 36: case 37: case 38: case 39: case 40: case 41:
            case 42: case 43: case 44: case 45: case 46: case 179: case 180:
                dm_l++;
                n=(*dm_wr)&0x3ff;
                if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc))
                    *dm_wr=*dm_rd;
                goto case34;
            case 47:
                dm_l+=10;
                n=*(dm_rd++);
                if(((*dm_wr)&0x3ff)!=0xe2) draw8fec(n);
                dm_wr+=2;
                dm_rd+=2;
                while(dm_l--) {
                    dm_wr[0]=*dm_rd;
                    dm_wr[64]=n;
                    dm_wr++;
                }
                dm_rd++;
                draw8fec(n);
                break;
            case 48:
                dm_l+=10;
                n=*(dm_rd++);
                if(((dm_wr[64])&0x3ff)!=0xe2) draw9030(n);
                dm_wr+=2;
                dm_rd+=2;
                while(dm_l--) {
                    dm_wr[0]=n;
                    dm_wr[64]=*dm_rd;
                    dm_wr++;
                }
                dm_rd++;
                draw9030(n);
                break;
            case 51:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4);
                break;
            case 52:
                dm_l+=4;
                n=*dm_rd;
                while(dm_l--) *(dm_wr++)=n;
                break;
            case 53:
                break;
            case 54: case 55:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=2;
                break;
            case 56:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx3(2),dm_wr+=2;
                break;
            case 61:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
                break;
            case 57:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
                break;
            case 58: case 59:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx3(4),dm_wr+=4;
                break;
            case 60:
                dm_l++;
                while(dm_l--) {
                    dm_rd=dm_src,draw2x2();
                    dm_wr+=0x17e;
                    dm_rd+=4;
                    draw2x2();
                    dm_wr-=0x17e;
                }
                break;
            case 62: case 75:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=12;
                break;
            case 63: case 64: case 65: case 66: case 67: case 68: case 69:
            case 70:
                dm_l++;
                n=(*dm_wr)&0x3ff;
                if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc)) *dm_wr=*dm_rd;
                dm_rd++;
                dm_wr++;
                n=*(dm_rd++);
                while(dm_l--) *(dm_wr++)=n;
                *dm_wr=*dm_rd;
                break;
            case 71:
                dm_l++;
                dm_l<<=1;
                draw1x5();
                dm_rd+=5;
                dm_wr++;
                while(dm_l--) draw1x5(),dm_wr++;
                dm_rd+=5;
                draw1x5();
                break;
            case 72:
                dm_l++;
                dm_l<<=1;
                drawXx3(1);
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[64]=dm_rd[1];
                    dm_wr[128]=dm_rd[2];
                    dm_wr++;
                }
                dm_rd+=3;
                drawXx3(1);
                break;
            case 73: case 74:
                dm_l++;
                draw4x2(4);
                break;
            case 76:
                dm_l++;
                dm_l<<=1;
                drawXx3(1);
                while(dm_l--) drawXx3(1),dm_rd-=3;
                dm_rd+=3;
                drawXx3(1);
                break;
            case 77: case 78: case 79:
                dm_l++;
                drawXx4(1);
                while(dm_l--) drawXx4(2),dm_rd-=8;
                dm_rd+=8;
                drawXx4(1);
                break;
            case 80:
                dm_l+=2;
                n=*dm_rd;
                while(dm_l--) *(dm_wr++)=n;
                break;
            case 81: case 82: case 91: case 92:
                drawXx3(2);
                while(dm_l--) drawXx3(2),dm_rd-=6;
                dm_rd+=6;
                drawXx3(2);
                break;
            case 83:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 85: case 86:
                dm_l++;
                draw4x2(12);
                break;
            case 93:
                dm_l+=2;
                drawXx3(2);
                while(dm_l--) {
                    dm_wr[0]=dm_rd[0];
                    dm_wr[64]=dm_rd[1];
                    dm_wr[128]=dm_rd[2];
                    dm_wr++;
                }
                dm_rd+=3;
                drawXx3(2);
                break;
            case 94:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=2;
                break;
            case 95:
                dm_l+=21;
                goto case34b;
            case 96: case 146: case 147:
                len1();
                while(dm_l--) draw2x2(),dm_wr+=126;
                break;
            case 97: case 98:
                len2();
                draw4x2(0x80);
                break;
            case 99: case 100:
                dm_l++;
case99:
                while(dm_l--) {
                    dm_buf[0x1000 + dm_x] = dm_buf[       dm_x] = dm_rd[0];
                    dm_buf[0x1001 + dm_x] = dm_buf[1    + dm_x] = dm_rd[1];
                    dm_buf[0x1002 + dm_x] = dm_buf[2    + dm_x] = dm_rd[2];
                    dm_buf[0x1003 + dm_x] = dm_buf[3    + dm_x] = dm_rd[3];
                    dm_buf[0x1040 + dm_x] = dm_buf[0x40 + dm_x] = dm_rd[4];
                    dm_buf[0x1041 + dm_x] = dm_buf[0x41 + dm_x] = dm_rd[5];
                    dm_buf[0x1042 + dm_x] = dm_buf[0x42 + dm_x] = dm_rd[6];
                    dm_buf[0x1043 + dm_x] = dm_buf[0x43 + dm_x] = dm_rd[7];
                    
                    dm_x+=128;
                }
                break;
            case 101: case 102:
                dm_l++;
                draw4x2(0x180);
                break;
            case 103: case 104:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=126;
                break;
            case 105:
                dm_l+=2;
                if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
                dm_wr+=64;
                while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
                *dm_wr=dm_rd[2];
                break;
            case 106: case 107: case 121: case 122:
                dm_l++;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 108:
                dm_l+=10;
                n=*(dm_rd++);
                if((*dm_wr&0x3ff)!=0xe3) draw9078(n);
                dm_rd+=2;
                dm_wr+=128;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr[1]=n,dm_wr+=64;
                dm_rd++;
                draw9078(n);
                break;
            case 109:
                dm_l+=10;
                n=*(dm_rd++);
                if((dm_wr[1]&0x3ff)!=0xe3) draw90c2(n);
                dm_rd+=2;
                dm_wr+=128;
                while(dm_l--) *dm_wr=n,dm_wr[1]=*dm_rd,dm_wr+=64;
                dm_rd++;
                draw90c2(n);
                break;
            case 112:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=252;
                break;
            case 113:
                dm_l+=4;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 115: case 116:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0x17c;
                break;
            case 117:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
                break;
            case 118: case 119:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
                break;
            case 120: case 123:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x37e;
                break;
            case 124:
                dm_l+=2;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 125:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x7e;
                break;
            case 127: case 128:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x2fe;
                break;
            case 129: case 130: case 131: case 132:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
                break;
            case 133: case 134:
                draw3x2(),dm_wr+=128,dm_rd+=6;
                while(dm_l--) draw3x2(),dm_wr+=128;
                dm_rd+=6;
                draw3x2();
                break;
            case 135:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
                break;
            case 136:
                dm_l++;
                draw2x2(),dm_wr+=0x7e;
                dm_rd+=4;
                while(dm_l--) dm_wr[0]=dm_rd[0],dm_wr[1]=dm_rd[1],dm_wr+=64;
                dm_rd+=2;
                drawXx3(2);
                break;
            case 137:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0xfe;
                break;
            case 138:
                dm_l+=21;
                if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
                dm_wr+=64;
                while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
                *dm_wr=dm_rd[2];
                break;
            case 140: case 139:
                dm_l+=8;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 141: case 142:
                dm_l++;
                while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
                break;
            case 143:
                dm_l+=2;
                dm_l<<=1;
                dm_wr[0]=dm_rd[0];
                dm_wr[1]=dm_rd[1];
                while(dm_l--) dm_wr[64]=dm_rd[2],dm_wr[65]=dm_rd[3],dm_wr+=64;
                break;
            case 144: case 145:
                len2();
                draw4x2(0x80);
                break;
            case 148:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0xfc;
                break;
            case 149: case 150:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=0x7e;
                break;
            case 160: case 169:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    n=dm_l;
                    drawtr();
                    dm_wr+=64;
                }
                break;
            case 161: case 166: case 170:
                dm_l+=4;
                m=0;
                while(dm_l--) {
                    m++;
                    n=m;
                    for(l=0;l<n;l++) dm_wr[l]=*dm_rd;
                    dm_wr+=64;
                }
                break;
            case 162: case 167: case 171:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    drawtr();
                    dm_wr+=65;
                }
                break;
            case 163: case 168: case 172:
                dm_l+=4;
                for(;dm_l;dm_l--) {
                    drawtr();
                    dm_wr-=63;
                }
                break;
            case 164:
                dm_l+=4;
                l=dm_l;
                tmp=dm_wr;
                
                while(l--)
                {
                    drawtr();
                    dm_src = dm_wr;
                    dm_wr += 64;
                }
                
                // \task We can track down endianness issues by searching
                // for like, "dm_rd[" and "dm_wr" perhaps.
                dm_rd = (uint16_t*) (rom + 0x218e);
                
                m = 2;
                
                dm_wr = tmp;
                
                while(m--) {
                    l=dm_l-2;
                    dm_wr[0]=dm_rd[0];
                    while(l--) dm_wr[1]=dm_rd[1],dm_wr++;
                    dm_wr[1]=dm_rd[2];
                    dm_rd+=3;
                    dm_wr=dm_src;
                }
                dm_wr=tmp+64;
                m=dm_l-1;
                l=m-1;
                dm_wr+=m;
                dm_src=dm_wr;
                dm_wr=tmp+64;
                m=2;
                
                dm_rd = (uint16_t*) (rom + 0x219a);
                
                while(m--) {
                    n=l;
                    while(n--)
                        // \task This look an error in the context of using
                        // comma delimited statements. What gets assigned to
                        // dm_wr at the end?
                        *dm_wr = *dm_rd, dm_wr += 64;
                    dm_wr=dm_src;
                    dm_rd++;
                }
                break;
            case 165:
                dm_l+=4;
                for(;dm_l;dm_l--) drawtr(),dm_wr+=64;
                break;
            case 176: case 177:
                dm_l+=8;
                drawtr();
                break;
            
            case 178:
                
                dm_l++;
                
                while(dm_l--)
                {
                    dm_rd = dm_src;
                    
                    drawXx4(4);
                }
                
                break;
            
            case 181:
                dm_l++; goto c182;
            case 182: case 183:
                len2();
c182:
                while(dm_l--) drawXx4(2),dm_rd-=8;
                break;
            case 184: // yeah!
                len2();
                while(dm_l--) draw2x2();
                break;
            case 185:
                len1();
                while(dm_l--) draw2x2();
                break;
            case 186:
                dm_l++;
                while(dm_l--) dm_rd=dm_src,drawXx4(4);
                break;
            case 187:
                dm_l++;
                while(dm_l--) draw2x2(),dm_wr+=2;
                break;
            case 188: case 189:
                dm_l++;
                while(dm_l--) draw2x2();
                break;
            case 192: case 194:
                
                l = (dm_l & 3) + 1;
                
                dm_l >>= 2;
                dm_l++;
                
                while(l--)
                {
                    m=dm_l;
                    dm_src=dm_wr;
                    
                    while(m--)
                    {
                        dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[3]=
                        dm_wr[64]=dm_wr[65]=dm_wr[66]=dm_wr[67]=
                        dm_wr[128]=dm_wr[129]=dm_wr[130]=dm_wr[131]=
                        dm_wr[192]=dm_wr[193]=dm_wr[194]=dm_wr[195] = dm_rd[0];
                        
                        dm_wr += 4;
                    }
                    
                    dm_wr = dm_src + 256;
                }
                break;
            case 193:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l+=4;
                tmp=dm_wr;
                m=dm_l;
                drawXx3(3);
                while(m--) {
                    drawXx3(2);
                    dm_rd-=6;
                }
                dm_rd+=6;
                drawXx3(3);
                dm_src=dm_rd;
                tmp+=0xc0;
                o=l;
                while(o--) {
                    dm_wr=tmp;
                    dm_rd=dm_src;
                    m=dm_l;
                    draw3x2();
                    dm_rd+=6;
                    dm_wr+=3;
                    while(m--) draw2x2();
                    dm_rd+=4;
                    draw3x2();
                    tmp+=0x80;
                }
                dm_rd+=6;
                dm_wr=tmp;
                m=dm_l;
                drawXx3(3);
                while(m--) {
                    drawXx3(2);
                    dm_rd-=6;
                }
                dm_rd+=6;
                drawXx3(3);
                tmp-=(1+l)<<6;
                dm_l+=2;
                dm_wr=tmp+dm_l;
                
                dm_rd = (uint16_t*) (rom + 0x20e2);
                
                draw2x2();
                
                break;
            case 195: case 215:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l++;
                while(l--) {
                    tmp=dm_wr;
                    m=dm_l;
                    while(m--) {
                        dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[64]=dm_wr[65]=dm_wr[66]=
                        dm_wr[128]=dm_wr[129]=dm_wr[130]=*dm_rd;
                        dm_wr+=3;
                    }
                    dm_wr = tmp + 0xc0;
                }
                break;
            case 196:
                dm_rd+=(ed->buf[0]&15)<<3;
            case 200: case 197: case 198: case 199: case 201: case 202:
                l=(dm_l&3);
                dm_l>>=2;
                dm_l++;
                l++;
                goto grid4x4;
            case 205:
                l=dm_l&3;
                dm_l>>=2;
                m=(unsigned char)(((short*)(rom + 0x1b0a))[dm_l]);
                n=((short*)(rom + 0x1b12))[l];
                dm_src=dm_wr;
                dm_wr-=n;
                
                tmp = dm_wr;
                
                dm_rd = (uint16_t*) (rom + 0x1f2a);
                
                while(n--) {
                    dm_wr[0]=dm_rd[0];
                    o=(m<<1)+4;
                    while(o--) {
                        dm_wr[64]=dm_rd[1];
                        dm_wr+=64;
                    }
                    dm_wr[64]=dm_rd[2];
                    tmp++;
                    dm_wr=tmp;
                }
                dm_wr=dm_src;
                dm_x=dm_src-nbuf-1;
                o=dm_x&31;
                if(dm_x&32) o|=0x200;
                o|=0x800;
                dm_wr = dm_src;
                dm_rd = (uint16_t*) (rom + 0x227c);
                
                drawXx3(3);
                dm_wr += 0xbd;
                
                while(m--)
                    draw3x2(), dm_wr += 128;
                
                dm_rd+=6;
                drawXx3(3);
                break;
            case 206:
                
                dm_rd = (uint16_t*) (rom + 0x22ac);
                
                tmp=dm_wr;
                drawXx3(3);
                l=dm_l&3;
                dm_l>>=2;
                o=m=(unsigned char)(((short*)(rom + 0x1b0a))[dm_l]);
                n=((short*)(rom + 0x1b12))[l];
                dm_wr=tmp + 0xc0;
                while(o--) draw3x2(),dm_wr+=128;
                dm_rd+=6;
                drawXx3(3);
                tmp+=3;
                
                dm_rd = (uint16_t*) (rom + 0x1f2a);
                
                m <<= 1;
                m+=4;
                while(n--)
                {
                    dm_wr = tmp;
                    dm_wr[0] = dm_rd[0];
                    
                    o = m;
                    
                    while(o--)
                    {
                        dm_wr[64] = dm_rd[1];
                        dm_wr += 64;
                    }
                    
                    dm_wr[64] = dm_rd[2];
                    tmp++;
                }
                break;
            
            case 209: case 210: case 217: case 223: case 224: case 225:
            case 226: case 227: case 228: case 229: case 230: case 231:
            case 232:
                l=(dm_l&3);
                dm_l>>=2;
                dm_l++;
                l++;
grid4x4:
                while(l--) {
                    tmp=dm_wr;
                    draw4x4X(dm_l);
                    dm_wr = tmp + 0x100;
                }
                break;
            case 216: case 218:
                l=dm_l&3;
                dm_l>>=2;
                dm_l=(unsigned char)(((unsigned short*)(rom + 0x1b3a))[dm_l]);
                l=(unsigned char)(((unsigned short*)(rom + 0x1b3a))[l]);
                dm_rd=(unsigned short*)(rom + 0x1c62);
                goto grid4x4;
                break;
            case 219:
                l=(dm_l&3)+1;
                dm_l>>=2;
                dm_l++;
                dm_rd+=(ed->buf[0]&240)>>1;
                goto grid4x4;
                break;
            case 220:
                l=((dm_l&3)<<1)+5;
                dm_l=(dm_l>>2)+1;
                dm_tmp=dm_wr;
                while(l--) {
                    draw975c();
                }
                dm_rd++;
                draw975c();
                dm_rd++;
                draw975c();
                break;
            case 221:
                
                l = ((dm_l&3)<<1)+1;
                dm_l = (dm_l>>2)+1;
                tmp = dm_wr;
                
                draw93ff();
                
                dm_rd += 4;
                
                while(l--)
                {
                    draw93ff();
                }
                
                dm_rd+=4;
                
                draw93ff();
                
                dm_rd+=4;
                
                draw93ff();
                
                break;
            case 222: // Yup. It's 222!
                
                l=(dm_l&3)+1;
                dm_l=(dm_l>>2)+1;
                while(l--) {
                    m=dm_l;
                    tmp=dm_wr;
                    while(m--) draw2x2();
                    tmp+=128;
                    dm_wr=tmp;
                }
                
                break;
            }
        }
    }
end:
    ed->chestnum = ch;
    return map+2;
}

// =============================================================================

void
LoadHeader(DUNGEDIT * const ed,
           int        const map)
{
    // we are passed in a dungeon editing window, and a map number (room number)
    
    uint8_t const * const rom = ed->ew.doc->rom;
    
    /// address of the header of the room indicated by parameter 'map'.
    uint16_t i = 0;
    
    // upper limit for the header offset.
    uint16_t m = 0;
    
    /// counter variable for looping through all dungeon rooms.
    int j = 0;
    
    // size of the header
    int l = 0;
    
    // -----------------------------
    
    i = ldle16b_i(rom + 0x27502, map);
    
    l = 14;
    
    // sort through all the other header offsets
    for(j = 0; j < 0x140; j++)
    {
        // m gives the upper limit for the header.
        // if is less than 14 bytes from i.
        m = ldle16b_i(rom + 0x27502, j);
        
        // \task When merging with other branches, note that
        // m and i are compared. If one is 16-bit, for example,
        // and the other is 32-bit, that is a big potential
        // problem.
        if( (m > i) && (m < (i + 14) ) )
        {
            l = (m - i);
            
            break;
        }
    }
    
    // determine the size of the header
    ed->hsize = l;
    
    // copy 14 bytes from the i offset.
    memcpy(ed->hbuf, rom + rom_addr_split(0x04, i), 14);
}

// =============================================================================


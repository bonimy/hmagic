
    #include "structs.h"

    #include "HMagicUtility.h"

// =============================================================================

extern uint8_t *
Compress(uint8_t const * const src,
         int             const oldsize,
         int           * const size,
         int             const flag)
{
    unsigned char *b2 = (unsigned char*) malloc(0x1000); // allocate a 2^12 sized buffer
    
    int i, j, k, l, m = 0,
        n,
        o = 0,
        bd = 0, p, q = 0,r;
    
    for(i = 0; i < oldsize; )
    {
        l=src[i]; // grab a char from the buffer.
        
        k=0;
        
        r=!!q; // r = the same logical value (0 or 1) as q, but not the same value necesarily.
        
        for(j = 0; j < i - 1; j++)
        {
            if(src[j] == l)
            {
                m = oldsize - j;
                
                for(n = 0; n < m; n++)
                    if(src[n+j]!=src[n+i])
                        break;
                
                if(n > k)
                    k=n,o=j;
            }
        }
        
        for(n = i + 1; n < oldsize; n++)
        {
            if(src[n] != l)
            {
                // look for chars identical to the first one. 
                // stop if we can't find one.
                // n will reach i+k+1 for some k >= 0.
                
                break;
            }
        }
        
        n -= i; // offset back by i. i.e. n = k+1 as above.
        
        if(n > 1 + r)
            p=1;
        else
        {
            m=src[i+1];
            
            for(n=i+2;n<oldsize;n++)
            {
                if(src[n]!=l)
                    break;
                
                n++;
                
                if(src[n]!=m)
                    break;
            }
            
            n-=i;
            
            if(n>2+r)
                p=2;
            else
            {
                m=oldsize-i;
                
                for(n=1;n<m;n++)
                    if(src[i+n]!=l+n)
                        break;
                
                if(n>1+r)
                    p=3;
                else
                    p=0;
            }
        }
        
        if(k>3+r && k>n+(p&1))
            p=4,n=k;
        
        if(!p)
            q++,i++;
        else
        {
            if(q)
            {
                q--;
                
                if(q>31)
                {
                    b2[bd++] = (unsigned char) ( 224 + (q >> 8) );
                }
                
                b2[bd++] = (unsigned char) q;
                q++;
                
                memcpy(b2+bd,src+i-q,q);
                
                bd += q;
                q = 0;
            }
            
            i+=n;
            n--;
            
            if(n>31)
            {
                b2[bd++] = (unsigned char) ( 224 + (n >> 8) + (p << 2) );
                b2[bd++] = (unsigned char) n;
            }
            else
                b2[bd++] = (unsigned char) ( (p << 5) + n );
            
            switch(p)
            {
            
            case 1: case 3:
                b2[bd++] = (unsigned char) l;
                break;
            
            case 2:
                b2[bd++] = (unsigned char) l;
                b2[bd++] = (unsigned char) m;
                
                break;
            
            case 4:
                if(flag)
                {
                    b2[bd++] = (unsigned char) (o >> 8);
                    b2[bd++] = (unsigned char) o;
                }
                else
                {
                    b2[bd++] = (unsigned char) o;
                    b2[bd++] = (unsigned char) (o >> 8);
                }
            }
            
            continue;
        }
    }
    
    if(q)
    {
        q--;
        
        if(q>31)
        {
            b2[bd++] = (unsigned char) ( 224 + (q >> 8) );
        }
        
        b2[bd++] = (unsigned char) q;
        q++;
        
        memcpy(b2 + bd,
               src + i - q,
               q);
        
        bd += q;
    }
    
    b2[bd++]=255;
    b2 = (unsigned char*) realloc(b2, bd);
    *size=bd;
    
    return b2;
}

// =============================================================================

uint8_t *
Uncompress(uint8_t const *       src,
           int           * const size,
           int             const p_big_endian)
{
    unsigned char *b2 = (unsigned char*) malloc(1024);
    
    int bd = 0, bs = 1024;
    
    unsigned char a;
    unsigned char b;
    unsigned short c, d;
    
    for( ; ; )
    {
        // retrieve a uchar from the buffer.
        a = *(src++);
        
        // end the decompression routine if we encounter 0xff.
        if(a == 0xff)
            break;
        
        // examine the top 3 bits of a.
        b = (a >> 5);
        
        if(b == 7) // i.e. 0b 111
        {
            // get bits 0b 0001 1100
            b = ( (a >> 2) & 7 );
            
            // get bits 0b 0000 0011, multiply by 256, OR with the next byte.
            c  = ( (a & 0x0003) << 8 );
            c |= *(src++);
        }
        else
            // or get bits 0b 0001 1111
            c = (uint16_t) (a & 31);
        
        c++;
        
        if( (bd + c) > (bs - 512) )
        {
            // need to increase the buffer size.
            bs += 1024;
            b2 = (uint8_t*) realloc(b2,bs);
        }
        
        // 7 was handled, here we handle other decompression codes.
        
        switch(b)
        {
        
        case 0: // 0b 000
            
            // raw copy
            
            // copy info from the src buffer to our new buffer,
            // at offset bd (which we'll be increasing;
            memcpy(b2+bd,src,c);
            
            // increment the src pointer accordingly.
            src += c;
            
            bd += c;
            
            break;
        
        case 1: // 0b 001
            
            // rle copy
            
            // make c duplicates of one byte, inc the src pointer.
            memset(b2+bd,*(src++),c);
            
            // increase the b2 offset.
            bd += c;
            
            break;
        
        case 2: // 0b 010
            
            // rle 16-bit alternating copy
            
            d = ldle16b(src);
            
            src += 2;
            
            while(c > 1)
            {
                // copy that 16-bit number c/2 times into the b2 buffer.
                stle16b(b2 + bd, d);
                
                bd += 2;
                c -= 2; // hence c/2
            }
            
            if(c) // if there's a remainder of c/2, this handles it.
                b2[bd++] = (char) d;
            
            break;
        
        case 3: // 0b 011
            
            // incrementing copy
            
            // get the current src byte.
            a = *(src++);
            
            while(c--)
            {
                // increment that byte and copy to b2 in c iterations.
                // e.g. a = 4, b2 will have 4,5,6,7,8... written to it.
                b2[bd++] = a++;
            }
            
            break;
        
        default: // 0b 100, 101, 110
            
            // lz copy
            
            if(p_big_endian)
            {
                d = (*src << 8) + src[1];
            }
            else
            {
                d = ldle16b(src);
            }
            
            while(c--)
            {
                // copy from a different location in the buffer.
                b2[bd++] = b2[d++];
            }
            
            src += 2;
        }
    }
    
    b2 = (unsigned char*) realloc(b2,bd);
    
    if(size)
        (*size) = bd;
    
    // return the unsigned char* buffer b2, which contains the uncompressed data.
    return b2;
}

// =============================================================================

extern unsigned char *
Make4bpp(unsigned char *buf,int size)
{
    short *b1 = malloc(size);
    short *a;
    
    unsigned char *b;
    short *c;
    
    int d,e;
    
    // looks at the uchar buffer in terms of shorts.
    a = (short*) buf;
    
    //cast again on (a+8). 
    //Note that this is 16 bytes ahead, not 8.
    //this is because it was incremented 8 shorts, then typecast
    //as char. 8 shorts = 16 bytes.
    b = (unsigned char*) (a + 8);
    
    c = b1;
    
    // e is the size divided by 32.
    for(e = size >> 5; e; e--)
    {
        for(d = 8; d; d--)
        {
            // looks complicated. OR's the lower bytes of *a and *b, shifts them left 8 bits.
            // (*a | *b) = temp. AND by 0xFF00, so we have only bits 8-15 being occupied.
            // next, OR with *a, so we get *a's upper and lower bytes mapped in again.
            // Also, OR in *b
            
            *c = *a;
            c[8] = ( ( ( *a | ( ( *a | *b) << 8) ) & 0xff00 ) | *b );
            
            a++; // inc by 1 short
            b++; // inc by 1 char
            c++; // inc by 1 short
        }
        
        a = (short*) b;
        b += 16;
        c += 8;
    }
    
    return (unsigned char*) b1;
}

//Make4bpp*********************************

extern unsigned char *
Makesnes(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size << 1);
    
    int a, e = 0;
    
    unsigned char h, c, d, f[4];
    
    short g;
    
    for(a = size >> 5; a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 16; h <<= 1, g++)
            {
                c = ( ( b[7] & h ) >> g );
                
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                
                f[g]=c; // this is the algorithm that turns a bitmap on it side, so it seems.
            }
            
            b += 8;
            
            buf[e+d+d]=f[0];
            
            buf[e+d+d+1]=f[1];
            
            buf[e+d+d+16]=f[2];
            
            buf[e+d+d+17]=f[3];
        }
        
        e+=32;
    }
    
    return buf;
}

// =============================================================================

extern unsigned char*
Make3bpp(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size * 3 >> 3);
    int a, e = 0;
    unsigned char h,c,d,f[3];
    short g;
    
    for(a = (size >> 6); a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 8; h <<= 1, g++)
            {
                c = ((b[7] & h) >> g);
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                f[g] = c;
            }
            
            b += 8;
            buf[e + d + d] = f[0];
            buf[e + d + d + 1] = f[1];
            buf[e + d + 16] = f[2];
        }
        
        e += 24;
    }
    
    return buf;
}

// =============================================================================

extern unsigned char *
Make2bpp(unsigned char *b, int size)
{
    unsigned char *buf = (unsigned char*) malloc(size >> 1);
    int a, e = 0;
    unsigned char h, c, d, f[2];
    short g;
    
    for(a = size >> 5; a; a--)
    {
        for(d = 0; d < 8; d++)
        {
            for(h = 1, g = 0; h < 4; h <<= 1, g++)
            {
                c = (( b[7] & h ) >> g);
                c |= (b[6] & h) >> g << 1;
                c |= (b[5] & h) >> g << 2;
                c |= (b[4] & h) >> g << 3;
                c |= (b[3] & h) >> g << 4;
                c |= (b[2] & h) >> g << 5;
                c |= (b[1] & h) >> g << 6;
                c |= (b[0] & h) >> g << 7;
                f[g] = c;
            }
            
            b += 8;
            buf[e + d + d] = f[0];
            buf[e + d + d + 1] = f[1];
        }
        
        e+=16;
    }
    
    return buf;
}

// =============================================================================

// Think this converts a bitplaned tile to a bitmap
extern unsigned char *
Unsnes(unsigned char * const buf, int size)
{
    int i,
        j,
        k = 0,
        l,
        m,
        n = size << 1,
        o = (size << 2) + 7;
    
    unsigned char * const b2 = (unsigned char*) malloc(size << 3);
    
    // -----------------------------
    
    for(j = 0; j < size; j += 16)
    {
        for(m = 8; m; m--, j += 2)
        {
            for(i = 7, l = 0x80; l; )
            {
                b2[k^o] = b2[k] =
                (
                    (
                        ( buf[j] & l )
                      + ( (buf[j + 1]  & l) << 1 )
                      + ( (buf[j + 16] & l) << 2 )
                      + ( (buf[j + 17] & l) << 3 )
                    ) >> i
                );
                
                b2[(k^o) + n] = b2[k + n] = masktab[ b2[k] ];
                k++;
                l >>= 1;
                i--;
            }
        }
    }
    
    return b2;
}

// =============================================================================

extern unsigned char *
GetBG3GFX(unsigned char *buf, int size)
{
    int c,d;
    
    unsigned char *buf2 = malloc(size);
    
    size >>= 1;
    
    for(d = 0; d < size; d += 16)
    {
        for(c = 0; c < 16; c++)
            buf2[d+d+c]=buf[d+c];
        
        for(c = 0; c < 16; c++)
        {
            // interesting, it's interlaced with a row of zeroes every
            //other line.
            buf2[d+d+c+16]=0;
        }
    }
    
    return buf2;
}

// =============================================================================
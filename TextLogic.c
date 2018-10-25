
#include "structs.h"
#include "prototypes.h"

#include "HMagicUtility.h"

// =============================================================================

const char z_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "!?-.,\0>()@£${}\""
    "\0\0\0\0'\0\0\0\0\0\0\0 <\0\0\0\0";

char const * const tsym_str[] =
{
    "Up",
    "Down",
    "Left",
    "Right",
    0,
    "1HeartL",
    "1HeartR",
    "2HeartL",
    "3HeartL",
    "3HeartR",
    "4HeartL",
    "4HeartR",
    0,0,
    "A",
    "B",
    "X",
    "Y"
};

char const * const tcmd_str[] =
{
    "NextPic",
    "Choose",
    "Item",
    "Name",
    "Window",
    "Number",
    "Position",
    "ScrollSpd",
    "Selchg",
    "Crash",
    "Choose3",
    "Choose2",
    "Scroll",
    "1",
    "2",
    "3",
    "Color",
    "Wait",
    "Sound",
    "Speed",
    "Mark",
    "Mark2",
    "Clear",
    "Waitkey",
};

char const * text_error = 0;

// =============================================================================

void
LoadText(FDOC * const doc)
{
    enum
    {
        /// Default size of a buffer and the amount by which it increases when
        /// reallocation is necessary.
        TEXT_GROW_SIZE = 128,
        COUNT_GROW_SIZE = 64,
    };
    
    int data_pos = 0xe0000;
    
    int msg_count     = 0;
    int max_msg_count = 0x200;
    
    unsigned char a;
    
    unsigned char *rom = doc->rom;
    
    // list of pointers for the game's text messages
    doc->tbuf = (uint8_t**) calloc(max_msg_count, sizeof(uint8_t*) );
    
    for( ; ; )
    {
        // current maximum size for the buffer (can change as needed)
        size_t max_msg_size = TEXT_GROW_SIZE;
        
        // the size of the useful data in the message buffer
        size_t msg_size = 2;
        
        // buffer for the current message
        uint8_t * msg = (uint8_t*) calloc(1, TEXT_GROW_SIZE);
        
        // -----------------------------
        
        for( ; ; )
        {
            a = rom[data_pos];
            
            if(a == 0x80)
            {
                // 0x80 tells us to go to the second data set 
                data_pos = 0x75f40;
            }
            else if(a == 0xff)
            {
                // 0xff is the terminator byte for all text data, period
                
                doc->t_number = msg_count;
                doc->t_loaded = 1; // indicate that text is loaded and return
                
                return;
            }
            
            else if(a < 0x67)
            {
                // if it's a character, just copy verbatim to the destination
                // buffer.
                data_pos++;
                msg[msg_size++] = a;
            }
            else if(a >= 0x88)
            {
                uint8_t const adj_code = (a - 0x88);
                
                // use the dictionary to load the characters up
                uint16_t const l = ldle16b(rom + 0x74703 + (adj_code << 1) );
                uint16_t const k = ldle16b(rom + 0x74705 + (adj_code << 1) );
                
                uint32_t src_addr = romaddr(0xe0000 + k);
                
                size_t len = (k - l);
                size_t i   = 0;
                
                for( ; i < len; i += 1)
                {
                    msg[msg_size++] = rom[src_addr + i];
                }
                
                data_pos++;
            }
            else if(a == 0x7f)
            {
                // 0x7f is a terminator byte for each message
                data_pos++;
                
                break;
            }
            else
            {
                // 0x7536B is a length indicator for each byte code
                int l = rom[0x7536b + a];
                
                while(l--)
                    msg[msg_size++] = rom[data_pos++];
            }
            
            if(msg_size >= (max_msg_size - 64) )
            {
                // if the text data won't fit into the current buffer, reallocate
                msg = (uint8_t*) recalloc(msg,
                                          max_msg_size + TEXT_GROW_SIZE,
                                          max_msg_size,
                                          sizeof(uint8_t) );
                
                max_msg_size += TEXT_GROW_SIZE;
            }
        }
        
        stle16b(msg, msg_size);
        
        msg = (uint8_t*) recalloc(msg,
                                  msg_size,
                                  max_msg_size,
                                  sizeof(uint8_t) );
        
        if(msg_count == max_msg_count)
        {
            doc->tbuf = (uint8_t**)
            recalloc(doc->tbuf,
                     max_msg_count + COUNT_GROW_SIZE,
                     max_msg_count,
                     sizeof(uint8_t*) );
            
            max_msg_count += COUNT_GROW_SIZE;
        }
        
        doc->tbuf[msg_count++] = msg;
    }
}

// =============================================================================

void
Savetext(FDOC * const doc)
{
    int i,bd,j,k;
    short l,m,n,o,p,q,r,v,t,u,w;
    
    // \task Can b2 overflow or is this just... fantasy?
    unsigned char*b, b2[2048];
    unsigned char*rom=doc->rom;
    
    size_t write_pos = 0xe0000;
    
    if(!doc->t_modf)
        return;
    
    doc->t_modf=0;
    
    w = ( ldle16b(rom + 0x74703) - 0xc705 ) >> 1;
    
    for(i=0;i<doc->t_number;i++) {
        b=doc->tbuf[i];
        m=bd=0;
        k=*(short*)b;
        j=2;
        r=w;
        for(;j<k;)
        {
            q=b2[bd++]=b[j++];
            if(q>=0x67) {
                l=rom[0x7536b + q] - 1;
                while(l--) b2[bd++]=b[j++];
                m=bd;
            }
            if(bd>m+1) {
                o=*(short*)(rom + 0x74703);
                v=255;
                t=w;
                for(l=0;l<w;l++) {
                    n=o;
                    o=*(short*)(rom + 0x74705 + (l<<1));
                    p=o-n-bd+m;
                    if(p>=0) if(!memcmp(rom + 0x78000 + n,b2+m,bd-m)) {
                        if(p<v) t=l,v=p;
                        if(!p) {r=t;u=j;break;}
                    }
                }
                if(t==w || b[j] >= 0x67) {
                    if(r!=w) {
                        b2[m]=r + 0x88;
                        m++;
                        j=u;
                        bd=m;
                        r=w;
                    } else m=bd-1;
                }
            }
        }
        
        write_pos += bd;
        
        // The subtraction of two accounts for the need for a message
        // terminator code (0x7f) and the potential need for a master
        // terminator code for all of the text data (0xff).
        if( (write_pos < 0xe0000) && (write_pos > (0x77400 - 2) ) )
        {
            doc->t_modf = 1;
            
            MessageBox(framewnd,
                       "Not enough space for monologue.",
                       "Bad error happened",
                       MB_OK);
            
            return;
        }
        
        // Check if writing this message would put us past the end of the first
        // bank where text data can reside. The subtraction of two is to
        // account for a message terminator code (0x7f) as well as a possible
        // additional bank switch code (0x80).
        if( write_pos > (0xe8000 - 2) )
        {
            rom[write_pos - bd] = 0x80;
            write_pos = 0x75f40 + bd;
        }
        
        memcpy(rom + write_pos - bd, b2, bd);
        
        rom[write_pos++] = 0x7f;
    }
    
    rom[write_pos] = 0xff;
    
    doc->modf = 1;
}

// =============================================================================

uint8_t *
Makezeldastring(FDOC const * const doc,
                char       *       buf)
{
    text_buf_ty text_buf = { 0 };
    
    uint8_t * b2 = (uint8_t*) malloc(128);
    char *n;
    
    int bd = 2, bs = 128;
    
    short j,l,m,k;
    
    for(;;)
    {
        j = *(buf++);
        
        // look for a [ character
        if(j == '[')
        {
            // m is the distance to the ] character
            m = strcspn(buf," ]");
            
            for(l = 0; l < 18; l++)
                if(tsym_str[l] && (!tsym_str[l][m]) && !_strnicmp(buf,tsym_str[l],m))
                    break;
            
            // the condition l == 18 means it did not find any string in the
            // special symbol strings list to match this one
            if(l == 18)
            {
                for(l = 0; l < 24; l++)
                    if((!tcmd_str[l][m]) && !_strnicmp(buf,tcmd_str[l],m))
                        break;
                
                // if this condition is true it means we didn't find a match in the 
                // command strings either
                if(l == 24)
                {
                    // strtol converts a string to a long data type
                    j = (short) strtol(buf, &n, 16);
                    
                    // k is the distance from the start of the command to the 
                    k = n - buf;
                    
                    // if the string doesn't match the pattern [XX] fogedda boud it                 
                    if(k > 2 || k < 1)
                    {
                        buf[m] = 0;
                        wsprintf(text_buf, "Invalid command \"%s\"", buf);
                        
error:
                        
                        MessageBox(framewnd, text_buf, "Bad error happened", MB_OK);
                        free(b2);
                        text_error = buf;
                        
                        return 0;
                    };
                    
                    m = k;
                    b2[bd++] = (char) j;
                    l = 0;
                }
                else
                    b2[bd++] = l + 0x67, l = doc->rom[0x753d2 + l] - 1;
            }
            else
                b2[bd++] = l + 0x4d, l = 0;
            
            buf += m;
            
            while(l--)
            {
                if(*buf!=' ')
                {
syntaxerror:
                    wsprintf(text_buf,"Syntax error: '%c'",*buf);
                    
                    goto error;
                }
                
                buf++;
                
                j = (short) strtol(buf,&n,16);
                m = n - buf;
                
                if(m > 2 || m < 1)
                {
                    wsprintf(text_buf,"Invalid number");
                    goto error;
                };
                
                buf += m;
                b2[bd++] = (char) j;
            }
            
            if(*buf!=']')
                goto syntaxerror;
            
            buf++;
        }
        else
        {
            if(!j)
                break;
            
            for(l = 0; l < 0x5f; l++)
                if(z_alphabet[l] == j)
                    break;
            
            if(l == 0x5f)
            {
                wsprintf(text_buf,"Invalid character '%c'",j);
                goto error;
            }
            
            b2[bd++] = (char) l;
        }
        
        if(bd > bs - 64)
        {
            bs += 128;
            b2 = (uint8_t*) realloc(b2, bs);
        }
    }
    
    // \task Shouldn't we just structurize this stuff and store the length
    // as a member? for pete's sake...
    *(unsigned short*) b2 = bd;
    
    return b2;
}

// =============================================================================

void
Makeasciistring(FDOC          const * const doc,
                char                * const buf,
                unsigned char const * const buf2,
                int                   const bufsize)
{
    text_buf_ty text_buf = { 0 };
    
    int i;
    
    short j,k,l,m,n;
    
    j = *(short*) buf2;
    l = 2;
    
    // -----------------------------
    
    for(i=0;i<bufsize-1;) {
        if(l>=j) break;
        k=buf2[l++];
        if(k<0x5f)
        {
            if(!z_alphabet[k])
            {
                if(k==0x43) m=wsprintf(text_buf,"...");
                else m=wsprintf(text_buf,"[%s]",tsym_str[k - 0x4d]);
                goto longstring;
            }
            else
            {
                buf[i++]=z_alphabet[k];
            }
        }
        else if(k >= 0x67 && k < 0x7f)
        {
            m=wsprintf(text_buf,"[%s",tcmd_str[k - 0x67]);
            n=doc->rom[0x7536b + k] - 1;
            while(n--) m+=wsprintf(text_buf+m," %02X",buf2[l++]);
            text_buf[m++]=']';
longstring:
            n = 0;
            
            while(m--)
            {
                buf[i++] = text_buf[n++];
                
                if(i == bufsize - 1)
                    break;
            }
        }
        else
        {
            m = wsprintf(text_buf, "[%02X]",k);
            
            goto longstring;
        }
    }
    
    buf[i] = 0;
}

// =============================================================================


    #include "structs.h"
    #include "prototypes.h"

    #include "Wrappers.h"

    #include "HMagicUtility.h"

    #include "TextLogic.h"

// =============================================================================

const char z_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "!?-.,\0>()@£${}\""
    "\0\0\0\0'\0\0\0\0\0\0\0 <\0\0\0\0";


enum
{
    NUM_Zchars = sizeof(z_alphabet) / sizeof(const char)
};

// =============================================================================

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
    0,
    0,
    "A",
    "B",
    "X",
    "Y"
};

enum
{
    NUM_Tsym = sizeof(tsym_str) / sizeof(char const *)
};

// =============================================================================

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


enum
{
    NUM_CmdStrings = sizeof(tcmd_str) / sizeof(char const *)
};

// =============================================================================

char const * text_error = 0;

// =============================================================================

    enum
    {
        /// Default size of a buffer and the amount by which it increases when
        /// reallocation is necessary.
        COUNT_GROW_SIZE = 64,
        TEXT_GROW_SIZE = 128,
    };
    
// =============================================================================

void
LoadText(FDOC * const doc)
{
    int data_pos = 0xe0000;
    
    int msg_count     = 0;
    int max_msg_count = 0x200;
    
    unsigned char *rom = doc->rom;
    
    // list of pointers for the game's text messages
    doc->text_bufz = (ZTextMessage*) calloc(max_msg_count,
                                            sizeof(ZTextMessage) );
    
    for( ; ; )
    {
        // buffer for the current message
        ZTextMessage msg = { 0 };
        
        // -----------------------------
        
        ZTextMessage_Init(&msg);
        
        for( ; ; )
        {
            // Current zchar to convert to ascii.
            uint8_t a = rom[data_pos];
            
            // -----------------------------
            
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
                
                ZTextMessage_AppendChar(&msg, a);
            }
            else if(a >= 0x88)
            {
                uint8_t const adj_code = (a - 0x88);
                
                // use the dictionary to load the characters up
                uint16_t const l = ldle16b_i(rom + 0x74703, adj_code);
                uint16_t const k = ldle16b_i(rom + 0x74705, adj_code);
                
                uint32_t src_addr = romaddr(0xe0000 + l);
                
                size_t len = (k - l);
                size_t i   = 0;
                
                for( ; i < len; i += 1)
                {
                    ZTextMessage_AppendChar(&msg, rom[src_addr + i]);
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
                {
                    ZTextMessage_AppendChar(&msg,
                                            rom[data_pos]);
                    
                    data_pos += 1;
                }
            }
        }
        
        if(msg_count == max_msg_count)
        {
            doc->text_bufz = (ZTextMessage*)
            recalloc(doc->text_bufz,
                     max_msg_count + COUNT_GROW_SIZE,
                     max_msg_count,
                     sizeof(ZTextMessage) );
            
            max_msg_count += COUNT_GROW_SIZE;
        }
        
        doc->text_bufz[msg_count] = msg;
        
        msg_count += 1;
    }
}

// =============================================================================

void
Savetext(FDOC * const doc)
{
    int i,bd,j,k;
    short l,m,n,o,p,q,r,v,t,u,w;
    
    // \task Can b2 overflow or is this just... fantasy?
    unsigned char b2[2048];
    unsigned char*rom=doc->rom;
    
    size_t write_pos = 0xe0000;
    
    ZTextMessage * msg;
    
    // -----------------------------
    
    if(!doc->t_modf)
        return;
    
    doc->t_modf=0;
    
    w = ( ldle16b(rom + 0x74703) - 0xc705 ) >> 1;
    
    for(i=0;i<doc->t_number;i++)
    {
        msg = &doc->text_bufz[i];
        
        m=bd=0;
        
        k = msg->m_len;
        
        j = 0;
        r = w;
        
        for( ; j < k; )
        {
            q = b2[bd] = msg->m_text[j];
            
            bd += 1;
            j += 1;
            
            if(q >= 0x67)
            {
                l=rom[0x7536b + q] - 1;
                
                for
                (
                    ;
                    l--;
                    bd += 1, j += 1
                )
                {
                    b2[bd] = msg->m_text[j];
                }
                
                m = bd;
            }
            
            if(bd > m + 1)
            {
                o=*(short*)(rom + 0x74703);
                
                v = 255;
                
                t = w;
                
                for(l = 0; l < w; l++)
                {
                    n = o;
                    o = *(short*)(rom + 0x74705 + (l<<1));
                    p = o - n - bd + m;
                    
                    if(p >= 0)
                        if( ! memcmp(rom + 0x78000 + n,b2+m,bd-m) )
                        {
                            if(p < v)
                            {
                                t = l, v = p;
                            }
                            
                            if(!p)
                            {
                                r=t;
                                u=j;
                                break;
                            }
                    }
                }
                
                if(t==w || msg->m_text[j] >= 0x67)
                {
                    if(r != w)
                    {
                        b2[m] = r + 0x88;
                        m++;
                        j=u;
                        bd = m;
                        r = w;
                    }
                    else
                    {
                        m = (bd - 1);
                    }
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

// Bidirectional, case insensitive, string comparison of up to n characters
int
bi_strnistr(char const * const p_lhs,
            char const * const p_rhs,
            size_t       const p_lhs_size,
            size_t       const p_rhs_size)
{
    size_t i = 0;
    
    // Take the minimum of the two to avoid buffer overflows.
    size_t const limit = (p_lhs_size < p_rhs_size)
                       ? p_lhs_size
                       : p_rhs_size;
    
    // -----------------------------
    
    // We are looking for an exact length match in both directions,
    // in addition to character by character match.
    if(p_lhs_size != p_rhs_size)
    {
        return 0;
    }
    
    for(i = 0; i < limit; i += 1)
    {
        char const a = toupper( p_lhs[i] );
        char const b = toupper( p_rhs[i] );
        
        // -----------------------------
        
        if(a != b)
        {
            return 0;
        }
    }
    
    return 1;
}

// =============================================================================

/// Converts ASCII text to the game's custom text format.
extern void
Makezeldastring(FDOC             const * const p_doc,
                AsciiTextMessage const * const p_amsg,
                ZTextMessage           * const p_zmsg)
{
    char * text_buf = NULL;
    
    char const * const abuf = p_amsg->m_text;
    
    int i = 0;
    
    short j,l,m,k;
    
    // -----------------------------
    
    for( ; ; )
    {
        j = abuf[i];
        
        i += 1;
        
        // look for a [ character
        if(j == '[')
        {
            // m is the distance to the ] character
            m = strcspn(abuf + i, " ]");
            
            for(l = 0; l < NUM_Tsym; l += 1)
            {
                if
                (
                    ( tsym_str[l] )
                 && (
                        bi_strnistr
                        (
                            abuf + i,
                            tsym_str[l],
                            m,
                            strlen(tsym_str[l])
                        )
                    )
                )
                {
                    break;
                }
            }
            
            // Means it did not find any string in the special symbol strings
            // list to match this one
            if(l == NUM_Tsym)
            {
                for(l = 0; l < NUM_CmdStrings; l += 1)
                {
                    if
                    (
                        ( ! tcmd_str[l][m] )
                     && (
                            bi_strnistr
                            (
                                abuf + i,
                                tcmd_str[l],
                                m,
                                strlen(tcmd_str[l])
                            )
                        )
                    )
                    {
                        break;
                    }
                }
                
                // if this condition is true it means we didn't find a match in
                // the command strings either
                if(l == NUM_CmdStrings)
                {
                    char * n = NULL;
                    
                    // -----------------------------
                    
                    // strtol converts a string to a long data type
                    j = (short) strtol(abuf + i, &n, 16);
                    
                    // k is the distance from the start of the command to the 
                    k = n - (abuf + i);
                    
                    // if the string doesn't match the pattern [XX] fogedda boud it                 
                    if(k > 2 || k < 1)
                    {
                        asprintf(&text_buf,
                                 "Invalid command \"%s\"",
                                 abuf + i);
                        
error:
                        
                        MessageBox(framewnd, text_buf, "Bad error happened", MB_OK);

                        ZTextMessage_Free(p_zmsg);
                        
                        text_error = (abuf + i);
                        
                        free(text_buf);

                        return;
                    };
                    
                    m = k;
                    
                    ZTextMessage_AppendChar(p_zmsg, j);
                    
                    l = 0;
                }
                else
                {
                    ZTextMessage_AppendChar(p_zmsg, l + 0x67);
                    
                    l = p_doc->rom[0x753d2 + l] - 1;
                }
            }
            else
            {
                ZTextMessage_AppendChar(p_zmsg, l + 0x4d);
                
                l = 0;
            }
            
            i += m;
            
            while(l--)
            {
                char * n = NULL;
                
                // -----------------------------
                
                if( abuf[i] != ' ')
                {
syntaxerror:
                    asprintf(&text_buf,
                             "Syntax error: '%c'",
                             abuf[i]);
                    
                    goto error;
                }
                
                i += 1;
                
                j = (short) strtol(abuf + i, &n, 16);
                
                m = n - (abuf + i);
                
                if(m > 2 || m < 1)
                {
                    asprintf(&text_buf,
                             "Invalid number");
                    
                    goto error;
                };
                
                i += m;
                
                ZTextMessage_AppendChar(p_zmsg, j);
            }
            
            if( abuf[i] != ']')
            {
                goto syntaxerror;
            }
            
            i += 1;
        }
        else
        {
            if( ! j )
            {
                break;
            }
            
            for(l = 0; l < NUM_Zchars; l++)
            {
                if(z_alphabet[l] == j)
                    break;
            }
            
            if(l == NUM_Zchars)
            {
                asprintf(&text_buf,
                         "Invalid character '%c'",
                         j);
                
                goto error;
            }
            
            ZTextMessage_AppendChar(p_zmsg, l);
        }
    }
}

// =============================================================================

    extern int
    AsciiTextMessage_Init(AsciiTextMessage * const p_msg)
    {
        if(p_msg == NULL)
        {
            return 0;
        }
        
        if(p_msg->m_text)
        {
            p_msg->m_len = 0;
            
            /// Fill with zeroes, so reset it, essentially.
            memset(p_msg->m_text, 0, p_msg->m_capacity + 1);
        }
        else
        {
            p_msg->m_capacity = 2048;
            p_msg->m_len      = 0;
            p_msg->m_text     = (char*) calloc(1, p_msg->m_capacity + 1);
        }
        
        return (p_msg->m_text != NULL);
    }

// =============================================================================

    extern int
    AsciiTextMessage_InitSized
    (
        AsciiTextMessage * const p_msg,
        size_t             const p_size
    )
    {
        if(p_msg == NULL)
        {
            return 0;
        }
        
        if(p_msg->m_text != NULL)
        {
            p_msg->m_len = 0;
            
            free(p_msg->m_text);
            
            p_msg->m_text = NULL;
        }
    
        p_msg->m_capacity = p_size;
        p_msg->m_len      = p_size;
        p_msg->m_text     = (char*) calloc(1,
                                            p_msg->m_capacity + 1);
        
        return (p_msg->m_text != NULL);
    }

// =============================================================================

    extern int
    AsciiTextMessage_AppendChar(AsciiTextMessage * const p_msg,
                                char               const p_char)
    {
        int i = p_msg->m_len;
        
        // -----------------------------
        
        // Check if the buffer needs expanding.
        if(p_msg->m_len == p_msg->m_capacity)
        {
            p_msg->m_text = (char*) recalloc
            (
                p_msg->m_text,
                p_msg->m_capacity + 2048 + 1,
                p_msg->m_capacity + 1,
                sizeof(char)
            );
            
            if(p_msg->m_text == NULL)
            {
                return 0;
            }
            
            p_msg->m_capacity += 2048;
        }
        
        p_msg->m_text[i]     = p_char;
        p_msg->m_text[i + 1] = '\0';
        
        p_msg->m_len += 1;
        
        return 1;
    }

// =============================================================================

    extern void
    AsciiTextMessage_Free(AsciiTextMessage * const p_msg)
    {
        if(p_msg)
        {
            if(p_msg->m_text)
            {
                free(p_msg->m_text);
                
                p_msg->m_text = NULL;
                
                p_msg->m_capacity = 0;
                p_msg->m_len      = 0;
            }
        }
    }

// =============================================================================

    extern int
    ZTextMessage_Init(ZTextMessage * const p_msg)
    {
        p_msg->m_len      = 0;
        p_msg->m_capacity = TEXT_GROW_SIZE;
        
        p_msg->m_text = (uint8_t*) calloc(p_msg->m_capacity,
                                          sizeof(uint8_t) );
        
        if(p_msg->m_text == NULL)
        {
            return 0;
        }
        
        return 1;
    }

// =============================================================================

    extern void
    ZTextMessage_AppendChar(ZTextMessage * const p_msg,
                            uint8_t        const p_char)
    {
        if(p_msg)
        {
            uint16_t const i = p_msg->m_len;
            
            // -----------------------------
            
            if(p_msg->m_len == p_msg->m_capacity)
            {
                uint16_t const old_capacity = p_msg->m_capacity;
                
                // -----------------------------
                
                p_msg->m_capacity += TEXT_GROW_SIZE;
                
                p_msg->m_text = (uint8_t*) recalloc
                (
                    p_msg->m_text,
                    p_msg->m_capacity,
                    old_capacity,
                    sizeof(uint8_t)
                );
            }
            
            p_msg->m_text[i] = p_char;
    
            p_msg->m_len += 1;
        }
    }

// =============================================================================

    extern void
    ZTextMessage_AppendStream(ZTextMessage       * const p_msg,
                              uint8_t      const * const p_data,
                              uint16_t             const p_len)
    {
        if(p_msg && p_data && (p_len > 0) )
        {
            uint16_t const i       = p_msg->m_len;
            uint16_t const new_len = (i + p_len);
            
            // -----------------------------
            
            if(new_len > p_msg->m_capacity)
            {
                uint16_t const new_capacity = new_len + TEXT_GROW_SIZE
                                            - (new_len % TEXT_GROW_SIZE);  
                
                p_msg->m_text = (uint8_t*) recalloc
                (
                    p_msg->m_text,
                    new_capacity,
                    p_msg->m_capacity,
                    sizeof(uint8_t)
                );
                
                p_msg->m_capacity = new_capacity;
            }
            
            memcpy(p_msg->m_text + i,
                   p_data,
                   p_len);
            
            p_msg->m_len = new_len;
        }
    }

// =============================================================================

extern void
ZTextMessage_Free(ZTextMessage * const p_msg)
{
    if(p_msg != NULL)
    {
        if(p_msg->m_text)
        {
            free(p_msg->m_text);
            
            p_msg->m_text = NULL;
        }
        
        p_msg->m_len      = 0;
        p_msg->m_capacity = 0;
    }
}

// =============================================================================

extern void
Makeasciistring
(
    FDOC             const * const doc,
    ZTextMessage     const * const p_zmsg,
    AsciiTextMessage       * const p_msg
)
{
    text_buf_ty text_buf = { 0 };
    
    int i;
    
    short k,m;
    
    uint16_t const zchar_len = p_zmsg->m_len;
    
    // Index into the zchar buffer.
    size_t z_i = 0;
    
    // -----------------------------
    
    AsciiTextMessage_Init(p_msg);
    
    for(i = 0; ; )
    {
        if(z_i >= zchar_len)
            break;
        
        k = p_zmsg->m_text[z_i];
        
        z_i += 1;
        
        if(k < NUM_Zchars)
        {
            if( ! z_alphabet[k] )
            {
                if(k == 0x43)
                {
                    m = wsprintf(text_buf,"...");
                }
                else
                {
                    m = wsprintf(text_buf,
                                 "[%s]",
                                 tsym_str[k - 0x4d]);
                }
                
                goto longstring;
            }
            else
            {
                AsciiTextMessage_AppendChar(p_msg,
                                            z_alphabet[k]);
            }
        }
        else if(k >= 0x67 && k < 0x7f)
        {
            size_t n = 0;
            
            m = wsprintf(text_buf,"[%s",tcmd_str[k - 0x67]);
            n = doc->rom[0x7536b + k] - 1;
            
            while(n--)
            {
                m += wsprintf(text_buf + m,
                              " %02X",
                              p_zmsg->m_text[z_i]);
                
                z_i += 1;
            }
            
            text_buf[m] = ']';
            
            m += 1;
            
        longstring:
            
            n = 0;
            
            while(m--)
            {
                AsciiTextMessage_AppendChar(p_msg,
                                            text_buf[n]);
                
                n += 1;
            }
        }
        else
        {
            m = wsprintf(text_buf, "[%02X]",k);
            
            goto longstring;
        }
    }
}

// =============================================================================

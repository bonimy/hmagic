
    #include "structs.h"
    #include "prototypes.h"

    #include "Wrappers.h"

    #include "HMagicUtility.h"

    #include "TextLogic.h"

// =============================================================================

char const * z_alphabet[] =
{
    // codes 0x00 and up
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    
    // codes 0x3E and up
    "!", "?", "-", ".", ",",
    
    // codes 0x43 and up
    "[...]", ">", "(", ")",
    
    // codes 0x47 and up
    "[Ankh]", "[Waves]", "[Snake]", "[LinkL]", "[LinkR]",
    "\"", "[Up]", "[Down]", "[Left]", "[Right]", "'",
    
    // codes 0x52 and up
    "[1HeartL]", "[1HeartR]", "[2HeartL]", "[3HeartL]",
    "[3HeartR]", "[4HeartL]", "[4HeartR]",
    
    " ", "<", "[A]", "[B]", "[X]", "[Y]",
    
    // \note These are invalid characters that shouldn't be encountered,
    // however, I've decided to make them two characters each, just so we have
    // a basic assumption that each of the strings in this array have
    // a minimum of two characters allocated.
    "\0\0", "\0\0", "\0\0", "\0\0", "\0\0", "\0\0", "\0\0", "\0\0",
};

enum
{
    NUM_Zchars = sizeof(z_alphabet) / sizeof(const char *)
};

#if 0
        // codes 0x67 and up
        "[NextPic]", "[Choose]", "[Item]", "[Name]", "[Window ",
        "[Number ", "[Position ", "[ScrollSpd ", "[SelChng]",
    
        // codes 0x70 and up
        "[Command 70]", "[Choose2]", "[Choose3]", "[Scroll]", 
        "[Line1]", "[Line2]", "[Line3]", "[Color ",
        "[Wait ", "[Sound ", "[Speed ",
        "[Command 7B]", "[Command 7C]", "[Command 7D]", "[WaitKey]", "[End]",

        // codes 0x80 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0x90 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0xA0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0xB0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0xC0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0xD0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",

        // codes 0xE0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
 
        // codes 0xF0 and up
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0",
        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0"

#endif

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
    NUM_CmdStrings = sizeof(tcmd_str) / sizeof(tcmd_str[0]),
};

// =============================================================================

char const * text_error = NULL;

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
    text_offsets_ty const * const to = &offsets.text;
    
    text_codes_ty const * const tc = &to->codes;
    
    int data_pos = to->region1;
    
    int msg_count     = 0;
    int max_msg_count = 0x200;
    
    unsigned char *rom = doc->rom;
    
    // -----------------------------
    
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
            
            if(a == tc->region_switch)
            {
                data_pos = to->region2;
            }
            else if(a == tc->abs_terminator)
            {
                doc->t_number = msg_count;
                
                // indicate that text is loaded and return
                doc->t_loaded = 1;
                
                return;
            }
            else if(a < tc->zchar_bound)
            {
                // if it's a character, just copy verbatim to the destination
                // buffer.
                data_pos++;
                
                ZTextMessage_AppendChar(&msg, a);
            }
            else if(a >= tc->dict_base)
            {
                uint8_t const adj_code = (a - tc->dict_base);
                
                // use the dictionary to load the characters up
                uint16_t const l = ldle16b_i(rom + to->dictionary,
                                             adj_code);
                
                uint16_t const k = ldle16b_i(rom + to->dictionary,
                                             adj_code + 1);
                
                uint32_t const src_addr = rom_addr_split(to->bank, l);
                
                size_t len = (k - l);
                size_t i   = 0;
                
                for( ; i < len; i += 1)
                {
                    ZTextMessage_AppendChar(&msg, rom[src_addr + i]);
                }
                
                data_pos += 1;
            }
            else if(a == tc->msg_terminator)
            {
                data_pos += 1;
                
                break;
            }
            else
            {
                int l = rom[to->param_counts + a];
                
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
    text_offsets_ty const * const to = &offsets.text;
    
    text_codes_ty const * const tc = &to->codes;
    
    int bd;
    short m,n,o,p,q,v,u;
    
    // Index of which message we're dealing with.
    size_t m_i = 0;
    
    // \task Can b2 overflow or is this just... fantasy?
    unsigned char b2[2048];
    
    unsigned char * rom = doc->rom;
    
    size_t write_pos = to->region1;
    
    unsigned const dict_loc = to->dictionary;
    
    // Number of dictionary entries
    unsigned const num_dict = ( ldle16b(rom + dict_loc) - 0xc705 ) >> 1;
    
    // -----------------------------
    
    if( ! doc->t_modf )
    {
        return;
    }
    
    doc->t_modf = 0;
    
    for(m_i = 0; m_i < doc->t_number; m_i += 1)
    {
        ZTextMessage const * const msg = &doc->text_bufz[m_i];
        
        size_t r = num_dict;
        
        // Index into the zchar array in the current message.
        size_t z_i = 0;
        
        uint16_t const msg_len = msg->m_len;
        
        // -----------------------------
        
        m = bd = 0;
        
        for(z_i = 0; z_i < msg_len; )
        {
            q = b2[bd] = msg->m_text[z_i];
            
            bd += 1;
            z_i += 1;
            
            if(q >= tc->command_base)
            {
                uint8_t l = rom[to->param_counts + q] - 1;
                
                // -----------------------------
                
                for
                (
                    ;
                    l--;
                    bd += 1, z_i += 1
                )
                {
                    b2[bd] = msg->m_text[z_i];
                }
                
                m = bd;
            }
            
            if(bd > m + 1)
            {
                size_t l = 0;
                size_t t = num_dict;
                
                // -----------------------------
                
                o = ldle16b(rom + dict_loc);
                
                v = 255;
                
                for(l = 0; l < num_dict; l += 1)
                {
                    n = o;
                    o = ldle16b_i(rom + dict_loc, l + 1);
                    p = o - n - bd + m;
                    
                    if(p >= 0)
                    {
                        if( ! memcmp(rom + 0x78000 + n, b2 + m, bd - m) )
                        {
                            if(p < v)
                            {
                                t = l;
                                v = p;
                            }
                            
                            if(!p)
                            {
                                r = t;
                                u = z_i;
                                
                                break;
                            }
                        }
                    }
                }
                
                if
                (
                    (t == num_dict)
                 || (msg->m_text[z_i] >= tc->command_base)
                )
                {
                    if(r != num_dict)
                    {
                        b2[m] = r + tc->dict_base;
                        
                        m   += 1;
                        z_i  = u;
                        bd   = m;
                        
                        r = num_dict;
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
        // terminator code and the potential need for a master
        // terminator code for all of the text data.
        if
        (
            (write_pos < to->region1)
         && (write_pos > (to->region2_bound - 2) )
        )
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
        // account for a message terminator code as well as a possible
        // additional bank switch code.
        if( write_pos > (to->region1_bound - 2) )
        {
            rom[write_pos - bd] = tc->region_switch;
            
            write_pos = to->region2 + bd;
        }
        
        memcpy(rom + write_pos - bd, b2, bd);
        
        rom[write_pos] = tc->msg_terminator;
        
        write_pos += 1;
    }
    
    rom[write_pos] = tc->abs_terminator;
    
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
    text_codes_ty const * const tc = &offsets.text.codes;
    
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
            
            for(l = 0; l < NUM_Zchars; l += 1)
            {
                // \task It's wasteful iterating through all of the zchar
                // strings when we only need to look at the ones bounded
                // by brackets.
                
                // Make sure there's an actual matching right bracket within
                // the rest of the string.
                if( abuf[i + m] == '\0' )
                {
                    asprintf(&text_buf,
                             "Unmatched left bracket at position %u",
                             i);
                    
                    goto error;
                }
                
                if
                (
                    bi_strnistr
                    (
                        abuf + i - 1,
                        z_alphabet[l],
                        m + 2,
                        strlen(z_alphabet[l])
                    )
                )
                {
                    // Located a string that matches a known zchar
                    // representation
                    break;
                }
            }
            
            if(l != NUM_Zchars)
            {
                // Move past the end of the bracketed expression, write
                // the zchar into the buffer, and move onto the next bit of
                // ascii text.
                i += (m + 1);
                
                ZTextMessage_AppendChar(p_zmsg, l);
                
                continue;
            }

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
                        bi_strnistr
                        (
                            abuf + i,
                            tcmd_str[l],
                            m,
                            strlen(tcmd_str[l])
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
                    
                    // if the string doesn't match the pattern [XX] fogedda boud
                    // it                 
                    if(k > 2 || k < 1)
                    {
                        asprintf(&text_buf,
                                 "Invalid command \"%s\"",
                                 abuf + i);
                        
error:
                        
                        MessageBox(framewnd, text_buf,
                                    "Bad error happened", MB_OK);

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
                    ZTextMessage_AppendChar(p_zmsg,
                                            l + tc->command_base);
                    
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
            
            for(l = 0; l < NUM_Zchars; l += 1)
            {
                if
                (
                    (z_alphabet[l][0] == j)
                 && (z_alphabet[l][1] == 0)
                )
                {
                    // Matches an ascii string that is a single character.
                    break;
                }
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
        // Fake a one character string to pass on to another "member" function.
        char str[2] = { p_char, '\0' };
        
        // -----------------------------
        
        return AsciiTextMessage_AppendString(p_msg, str);
    }

// =============================================================================

    extern int
    AsciiTextMessage_AppendString
    (
        AsciiTextMessage       * const p_msg,
        char             const * const p_str
    )
    {
        size_t i = 0;
        
        size_t const k = p_msg->m_len;
        size_t const n = strlen(p_str);
        
        // -----------------------------
        
        // Check if the buffer needs expanding.
        if( (k + n) > p_msg->m_capacity )
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
        
        for(i = 0; i < n; i += 1)
        {
            p_msg->m_text[i + k]     = p_str[i];
            p_msg->m_text[i + k + 1] = '\0';
        }
        
        p_msg->m_len += n;
        
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
    ZTextMessage_Empty(ZTextMessage * const p_msg)
    {
        if(p_msg)
        {
            p_msg->m_len = 0;
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
    
    text_offsets_ty const * const to = &offsets.text;
    
    text_codes_ty const * const tc = &to->codes;
    
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
            if( ! z_alphabet[k][0] )
            {
                // \task We don't really know what happens for these
                // characters if they're used in game. These are the ones
                // on the upper edge of the zchar range.
                exit(-1);
            }
            else
            {
                AsciiTextMessage_AppendString(p_msg,
                                              z_alphabet[k]);
            }
        }
        else if
        (
            (k != tc->msg_terminator)
         && (k >= tc->command_base)
         && (k <  tc->command_bound)
        )
        {
            size_t n = 0;
            
            m = wsprintf(text_buf,"[%s",tcmd_str[k - tc->command_base]);
            n = doc->rom[to->param_counts + k] - 1;
            
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

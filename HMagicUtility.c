
#include "prototypes.h"

// =============================================================================

// this calculates rom addresses from (lo-rom) cpu addresses
int
romaddr(int const addr)
{
    int ret = 0;
    
    if( !(addr & 0x8000) )
    {
        // e.g. 0x44444 -> 0x00000 -> return 0;
        ret = 0;
    }
    else
    {
        // e.g. 0x289AB -> 0x109AB
        ret = ( (addr & 0x3f0000) >> 1) | (addr & 0x7fff );
    }
    
    return ret;
}

// =============================================================================

/// Converts a cpu address to a rom address by combining a bank byte and
/// a 16-bit address within that bank.
uint32_t
rom_addr_split(uint8_t  const p_bank,
               uint16_t const p_addr)
{
    BOOL const even_bank = ( (p_bank % 2) == 0 );
    
    uint32_t const upper = (p_bank >> 1) << 16;
    
    uint32_t const lower = (even_bank) ? p_addr - 0x8000
                                       : p_addr;
    
    // -----------------------------
    
    if(p_addr < 0x8000)
    {
        return 0;
    }
    
    return (upper | lower); 
}

// =============================================================================

// this calculates (lo-rom) cpu addresses from rom addreses.
int
cpuaddr(int addr)
{
    return ((addr&0x1f8000)<<1) | (addr&0x7fff) | 0x8000;
}

// =============================================================================

void *
recalloc(void   const * const p_old_buf,
         size_t         const p_new_count,
         size_t         const p_old_count,
         size_t         const p_element_size)
{
    void * const new_buf = calloc(p_new_count, p_element_size);
    
    if(new_buf)
    {
        // Copy using the smaller of the two count values.
        size_t const limit_count = p_old_count > p_new_count ? p_new_count
                                                             : p_old_count;
        
        memcpy(new_buf, p_old_buf, (limit_count * p_element_size) );
    }
    
    return new_buf;
}

// =============================================================================

// "load little endian value at the given byte offset and shift to get its
// value relative to the base offset (powers of 256, essentially)"
unsigned
ldle(uint8_t const * const p_arr,
     unsigned        const p_index)
{
    uint32_t v = p_arr[p_index];
    
    v <<= (8 * p_index);
    
    return v;
}

// Helper function to get the first byte in a little endian number
uint32_t
ldle0(uint8_t const * const p_arr)
{
    return ldle(p_arr, 0);
}

// Helper function to get the second byte in a little endian number
uint32_t
ldle1(uint8_t const * const p_arr)
{
    return ldle(p_arr, 1);
}

// Helper function to get the third byte in a little endian number
uint32_t
ldle2(uint8_t const * const p_arr)
{
    return ldle(p_arr, 2);
}

// Helper function to get the third byte in a little endian number
uint32_t
ldle3(uint8_t const * const p_arr)
{
    return ldle(p_arr, 3);
}

// =============================================================================

void
stle(uint8_t  * const p_arr,
     size_t     const p_index,
     unsigned   const p_val)
{
    uint8_t v = (p_val >> (8 * p_index) ) & 0xff;
    
    p_arr[p_index] = v;
}

void
stle0(uint8_t * const p_arr,
      unsigned  const p_val)
{
    stle(p_arr, 0, p_val);
}

void
stle1(uint8_t * const p_arr,
      unsigned  const p_val)
{
    stle(p_arr, 1, p_val);
}

void
stle2(uint8_t * const p_arr,
      unsigned  const p_val)
{
    stle(p_arr, 2, p_val);
}

void
stle3(uint8_t * const p_arr,
      unsigned  const p_val)
{
    stle(p_arr, 3, p_val);
}

// =============================================================================

// Load little endian halfword (16-bit) dereferenced from 
uint16_t
ldle16b(uint8_t const * const p_arr)
{
    uint16_t v = 0;
    
    v |= ( ldle0(p_arr) | ldle1(p_arr) );
    
    return v;
}

// =============================================================================

// Load little endian halfword (16-bit) dereferenced from an arrays of bytes.
// This version provides an index that will be multiplied by 2 and added to the
// base address.
uint16_t
ldle16b_i(uint8_t const * const p_arr,
          size_t          const p_index)
{
    return ldle16b(p_arr + (2 * p_index) );
}

// =============================================================================

uint16_t
ldle16h(uint16_t const * const p_arr)
{
    return ldle16b( (uint8_t *) p_arr);
}

// =============================================================================

// Load little endian halfword (16-bit) dereferenced from a pointer to a
// halfword.
uint16_t
ldle16h_i(uint16_t const * const p_arr,
          size_t           const p_index)
{
    return ldle16b_i((uint8_t *) p_arr, p_index);
}

// =============================================================================

// "load little endian 24-bit value using a byte pointer"
uint32_t
ldle24b(uint8_t const * const p_arr)
{
    uint32_t v = ldle0(p_arr) | ldle1(p_arr) | ldle2(p_arr);
    
    return v;
}

// =============================================================================

uint32_t
ldle24b_i(uint8_t const * const p_arr,
          unsigned        const p_index)
{
    uint32_t v = ldle24b( p_arr + (p_index * 3) );
    
    return v;
}

// =============================================================================

uint32_t
ldle32b(uint8_t const * const p_arr)
{
    uint32_t v = 0;
    
    v = ldle0(p_arr) | ldle1(p_arr) | ldle2(p_arr) | ldle3(p_arr);
    
    return v;
}        

// =============================================================================

void
stle16b(uint8_t * const p_arr,
        uint16_t  const p_val)
{
    stle0(p_arr, p_val);
    stle1(p_arr, p_val);
}

// =============================================================================

void
stle16b_i(uint8_t * const p_arr,
          size_t    const p_index,
          uint16_t  const p_val)
{
    stle16b(p_arr + (p_index * 2), p_val);
}

// =============================================================================

// "store little endian 24-bit value using a byte pointer"
void
stle24b(uint8_t  * const p_arr,
        uint32_t   const p_val)
{
    stle0(p_arr, p_val);
    stle1(p_arr, p_val);
    stle2(p_arr, p_val);
}

// =============================================================================

void
stle32b(uint8_t  * const p_arr,
        uint32_t   const p_val)
{
    stle0(p_arr, p_val);
    stle1(p_arr, p_val);
    stle2(p_arr, p_val);
    stle3(p_arr, p_val);
}

// =============================================================================

void
stle32b_i(uint8_t  * const p_arr,
          size_t     const p_index,
          uint32_t   const p_val)
{
    stle32b(p_arr + (p_index * 4), p_val);
}

// =============================================================================

void
addle16b(uint8_t * const p_arr,
         uint16_t  const p_addend)
{
    uint16_t v = ldle16b(p_arr);
    
    v += p_addend;
    
    stle16b(p_arr, v);
}

// =============================================================================

void
addle16b_i(uint8_t * const p_arr,
           size_t    const p_index,
           uint16_t  const p_addend)
{
    uint16_t v = ldle16b_i(p_arr, p_index);
    
    v += p_addend;
    
    stle16b_i(p_arr, p_index, v);
}

// =============================================================================

// Read a half word at the given octec-pointer and if its bit pattern is
// 0xffff, return one. Else, zero. Note that endianness is irrelevant for
// this check.
int
is16b_neg1(uint8_t const * const p_arr)
{
    uint16_t v = ((uint16_t*) p_arr)[0];
    
    return (v == 0xffff);
}

// =============================================================================

int
is16b_neg1_i(uint8_t const * const p_arr,
             size_t          const p_index)
{
    return is16b_neg1(p_arr + (2 * p_index) );
}

// =============================================================================

// Read a half word at the given halfword-pointer and if its bit patter is
// 0xffff, return one. Else, zero.
int
is16h_neg1(uint16_t const * const p_arr)
{
    return (p_arr[0] == 0xffff);
}

int
is16h_neg1_i(uint16_t const * const p_arr,
             size_t           const p_index)
{
    return is16h_neg1(p_arr + p_index);
}

// =============================================================================

uint16_t
u16_neg(int const p_value)
{
    return (uint16_t) ( - p_value );
}

// =============================================================================

int
truth(int value)
{
    return (value != 0);
}

// =============================================================================

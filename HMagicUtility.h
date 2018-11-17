
#if ! defined HMAGIC_UTILITY_HEADER_GUARD

    #define HMAGIC_UTILITY_HEADER_GUARD

// =============================================================================

    int
    romaddr(int const addr);

    uint32_t
    rom_addr_split(uint8_t  const p_bank,
                   uint16_t const p_addr);

    int
    cpuaddr(int addr);

    extern void *
    recalloc(void   * const p_old_buf,
             size_t   const p_new_count,
             size_t   const p_old_count,
             size_t   const p_element_size);

    uint16_t
    ldle16b(uint8_t const * const p_arr);

    uint16_t
    ldle16b_i(uint8_t const * const p_arr,
              size_t          const p_index);

    uint16_t
    ldle16h_i(uint16_t const * const p_arr,
              size_t           const p_index);

    uint32_t
    ldle24b(uint8_t const * const p_arr);

    /// "indexed load little endian 24-bit value using a byte pointer"
    uint32_t
    ldle24b_i(uint8_t const * const p_arr,
              unsigned        const p_index);

    uint32_t
    ldle32b(uint8_t const * const p_arr);

    void
    stle16b(uint8_t * const p_arr,
            uint16_t  const p_val);

    void
    stle16b_i(uint8_t * const p_arr,
              size_t    const p_index,
              uint16_t  const p_val);

    void
    stle16h_i(uint16_t * const p_arr,
              size_t     const p_index,
              uint16_t   const p_val);
    
    void
    stle24b(uint8_t * const p_arr,
            uint32_t  const p_value);

    void
    stle32b(uint8_t  * const p_arr,
            uint32_t   const p_val);

    void
    stle32b_i(uint8_t  * const p_arr,
              size_t     const p_index,
              uint32_t   const p_val);

    void
    addle16b(uint8_t * const p_arr,
             uint16_t  const p_addend);

    void
    addle16b_i(uint8_t * const p_arr,
               size_t    const p_index,
               uint16_t  const p_addend);

    int
    is16b_neg1(uint8_t const * const p_arr);

    int
    is16b_neg1_i(uint8_t const * const p_arr,
                 size_t          const p_index);

    int
    is16h_neg1(uint16_t const * const p_arr);

    int
    is16h_neg1_i(uint16_t const * const p_arr,
                 size_t           const p_index);

    uint16_t
    u16_neg(int const p_value);

    int
    truth(int value);

// =============================================================================

#endif
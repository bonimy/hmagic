
    #include "structs.h"

    #include "Wrappers.h"

    // For Modifywaves()
    #include "AudioLogic.h"

// =============================================================================

#pragma warning(push)

/**
    We actually don't care in this case that the string members of this
    structure are not terminated, so ignore this warning.
*/
#pragma warning(disable: 4295)

    const static HM_WaveFileData wavehdr_base =
    {
        "RIFF",
        
        0,

        "WAVE",
        
        // subchunk 1 ID
        "fmt ",

        // subchunk 1 Size
        16,
        
        // Audio format. This indicates PCM.
        1,

        // Number of channels (BRR is mono, btw)
        1,
        
        // Sample rate in hZ
        11025,
        
        // Byte rate
        22050,
        
        // Block alignment
        2,

        // Bits per sample (16-bit PCM here)
        16,
        
        // Subchunk2 ID
        "data",
    };

#pragma warning(pop)

// =============================================================================

    /**
        \task[low] Move this to a file called SampleEditLogic.c once it is
        created.
        It is business logic and has nothing to do with any window class or
        object.
    */
    extern void
    SampleEdit_CopyToClipboard
    (
        CP2C(SAMPEDIT) p_ed
    )
    {
        CP2C(ZWAVE) zw = p_ed->zw;
        
        // (As measured in bytes)
        size_t data_len = (p_ed->selr - p_ed->sell) << 1;
        
        size_t start = p_ed->sell;
        
        HGLOBAL hgl;
        
        HM_WaveFileData * wav_data = NULL;
        
        size_t const chunk_size = sizeof(HM_WaveFileData)
                                - sizeof(wav_data->m_chunk_id)
                                - sizeof(wav_data->m_chunk_size)
                                - sizeof(wav_data->m_data);
        
        // -----------------------------
        
        if( Is(zw->end, 0) )
        {
            HM_OK_MsgBox
            (
                framewnd,
                (
                    "Cannot copy a zero length sample.\n"
                    "Nothing has been copied to the clipboard."
                ),
                "Error"
            );
            
            return;
        }
        if(data_len == 0)
        {
            // If there's nothing explicitly selected, copy everything
            data_len = zw->end << 1;
            start    = 0;
        }
        
        hgl = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT,
                          sizeof(HM_WaveFileData) + data_len);
        
        wav_data = (HM_WaveFileData*) GlobalLock(hgl);
        
        wav_data[0] = wavehdr_base;
        
        wav_data->m_chunk_size      = (chunk_size + data_len);
        wav_data->m_subchunk_2_size = data_len;
        
        memcpy(wav_data->m_data,
               zw->buf + start,
               data_len);
        
        GlobalUnlock(hgl);
        OpenClipboard(0);
        EmptyClipboard();
        
        /*
            \author MathOnNapkins (of this comment block, anyway)
            
            \note One may ask why sephiroth3 wanted to copy wave file data
            in this fashion, when he could have just copied the raw PCM data.
            In other words, this seems a bit overwrought to achieve the desired
            goal, however we must consider that they may have had a pet
            application they liked to use that allowed copy and paste in this
            fashion. Audacity, for example, may accept this sort of data from
            the clipboard, but I don't konw that off hand.
            
            In the current year (2018) I have not encountered any applications
            that seem to accept this type of data from the clipboard, but that
            is to some degree from a lack of seeking any out. Office
            applications and WordPad don't seem to accept this data though.
        */
        
        SetClipboardData(CF_WAVE, hgl);
        CloseClipboard();
    }

// =============================================================================

    typedef
    struct
    {
        /**
            Just indicates whether the clipboard is open and whether we need
            to close it upon exit.
        */
        BOOL m_cb_open;
        
        /// Handle to global data block that the clipboard owns and manages.
        HGLOBAL m_hgl;
        
        /// Length of the data on the clipboard
        size_t m_cb_data_len;
        
        /**
            Pointer to raw data on the clipboard. If this is not NULL, it
            should be assumed that the data has been "locked" from m_hgl,
            and therefore will need to be unlocked when we're done with it.
        */
        char const * m_data;
        
        /*
            Aliased verison of m_data with a structure type.
        */
        HM_WaveFileData const * m_wav_data;

        
    } SampleEdit_PasteContext;

// =============================================================================

    BOOL
    SampleEdit_ValidatePasteData
    (
        SampleEdit_PasteContext * const p_c
    )
    {
        HM_WaveFileData const * w = NULL;
        
        size_t internal_size_2 = 0;
        
        // -----------------------------
        
        p_c->m_cb_open     = FALSE;
        p_c->m_hgl         = INVALID_HANDLE_VALUE;
        p_c->m_cb_data_len = 0;
        p_c->m_data        = NULL;
        p_c->m_wav_data    = NULL;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        p_c->m_cb_open = OpenClipboard(0);
        
        if(p_c->m_cb_open == FALSE )
        {
            MessageBox
            (
                framewnd,
                "Could not open clipboard for paste operation",
                "Error",
                MB_OK
            );
            
            return FALSE;
        }
        
        p_c->m_hgl = GetClipboardData(CF_WAVE);
        
        if( ! p_c->m_hgl )
        {
            MessageBox
            (
                framewnd,
                "Nothing is on the clipboard.",
                "Bad error happened",
                MB_OK
            );
            
            return FALSE;
        }
        
        p_c->m_cb_data_len = GlobalSize(p_c->m_hgl);
        
        if( p_c->m_cb_data_len < sizeof(HM_WaveFileData) )
        {
            MessageBox
            (
                framewnd,
                ("The data stream in the clipboard is too small to be WAV "
                "formatted audio data"),
                "Error",
                MB_OK
            );
            
            return FALSE;
        }
        
        p_c->m_data = (char const *) GlobalLock(p_c->m_hgl);
        
        if(p_c->m_data == NULL)
        {
            MessageBox
            (
                framewnd,
                "Failed to acquire a pointer to clipboard data",
                "Error",
                MB_OK
            );
            
            return FALSE;
        }
        
        w = p_c->m_wav_data = (HM_WaveFileData const *) p_c->m_data;
        
        internal_size_2 = (w->m_subchunk_2_size + sizeof(HM_WaveFileData) );

        if
        (
            // The eight bytes account for the two 32-bit values at the
            // start of the WAV file.
            (w->m_chunk_size + 8) > p_c->m_cb_data_len
         || (internal_size_2 > p_c->m_cb_data_len)
        )
        {
            MessageBox
            (
                framewnd,
                "The internally reported size of the WAV data exceeds the "
                "size of the buffer provided by the clipboard",
                "Error",
                MB_OK
            );

            return FALSE;
        }
        
        if
        (
            ! HM_CheckEmbeddedStr(w->m_chunk_id, "RIFF")
         || ! HM_CheckEmbeddedStr(w->m_format,   "WAVE")
         || ! HM_CheckEmbeddedStr(w->m_subchunk_1_id, "fmt ")
         || ! HM_CheckEmbeddedStr(w->m_subchunk_2_id, "data")
        )
        {
            MessageBox
            (
                framewnd,
                "This is not a wave.",
                "Bad error happened",
                MB_OK
            );
            
            return FALSE;
        }
        
        if(w->m_audio_format != 1)
        {
            MessageBox
            (
                framewnd,
                "The wave is not PCM",
                "Bad error happened",
                MB_OK
            );
            
            return FALSE;
        }
        
        if(w->m_num_channels != 1)
        {
            MessageBox
            (
                framewnd,
                "Only mono is allowed.",
                "Bad error happened",
                MB_OK
            );
            
            return FALSE;
        }
        
        return TRUE;
    }

// =============================================================================

    static void
    SampleEdit_CleanupPasteContext
    (
        SampleEdit_PasteContext * const p_c
    )
    {
        if(p_c->m_data)
        {
            GlobalUnlock(p_c->m_hgl);
        }
        
        if(p_c->m_data)
        {
            p_c->m_data = NULL;
        }
        
        if(p_c->m_wav_data)
        {
            p_c->m_wav_data = NULL;
        }
        
        if(p_c->m_cb_open)
        {
            CloseClipboard();
            
            p_c->m_cb_open = FALSE;
        }
        
        p_c->m_cb_data_len = 0;
    }

// =============================================================================

    extern BOOL
    SampleEdit_PasteFromClipboard(SAMPEDIT * const p_ed)
    {
        int i = 0;
        int j = 0;
        int k = 0;
        
        char const * dat = NULL;
        
        ZWAVE * const zw = p_ed->zw;
        
        SampleEdit_PasteContext c;
        
        BOOL is_valid = SampleEdit_ValidatePasteData(&c);
        
        // -----------------------------
        
        if(is_valid == FALSE)
        {
            SampleEdit_CleanupPasteContext(&c);

            return FALSE;
        }
        
        j = (c.m_wav_data->m_chunk_size + 4);
        
        k = c.m_wav_data->m_subchunk_2_size;
        
        if(c.m_wav_data->m_bits_per_sample == 16)
        {
            k >>= 1;
        }
        
        if( (p_ed->sell > zw->end) || (p_ed->selr > zw->end) )
        {
            p_ed->sell = zw->end;
            p_ed->selr = zw->end;
        }
        
        zw->end += k - (p_ed->selr - p_ed->sell);
        
        if(k > (p_ed->selr - p_ed->sell) )
        {
            // Resize the sample buffer if the size of the data being
            // pasted in exceeds the size of the currently selected
            // region.
            zw->buf = (short*) realloc(zw->buf, (zw->end + 1) << 1);
        }
        
        // Move the part of the sample that is to the right of the selection
        // region in such a way that there is just enough room to copy
        // the new data in.
        memmove(zw->buf + p_ed->sell + k,
                zw->buf + p_ed->selr,
                (zw->end - k - p_ed->sell) << 1);
        
        if(k < (p_ed->selr - p_ed->sell) )
        {
            // Shrink the sample buffer if the pasted in data is smaller
            // than the selection region.
            zw->buf = (short*) realloc(zw->buf, ( (zw->end + 1) << 1 ) );
        }
        
        if(zw->lopst >= p_ed->selr)
        {
            zw->lopst += p_ed->sell + k - p_ed->selr;
        }
        
        if(zw->lopst >= zw->end)
        {
            zw->lflag = 0, zw->lopst = 0;
        }
        
        if(zw->lflag)
        {
            zw->buf[zw->end] = zw->buf[zw->lopst];
        }
        
        p_ed->selr = p_ed->sell + k;
        
        dat = (const char *) c.m_wav_data->m_data;
        
        if(c.m_wav_data->m_bits_per_sample == 16)
        {
            memcpy(zw->buf + p_ed->sell,
                   dat,
                   k << 1);
        }
        else
        {
            j = p_ed->sell;
            
            for(i = 0; i < k; i += 1)
            {
                zw->buf[j++] = ( (dat[i] - 128) << 8 );
            }
        }
        
        p_ed->ew.doc->m_modf = 1;
        p_ed->ew.doc->w_modf = 1;
        
        SampleEdit_CleanupPasteContext(&c);
        
        Modifywaves(p_ed->ew.doc,
                    p_ed->editsamp);
        
        p_ed->init = 1;
        
        return TRUE;
    }

// =============================================================================

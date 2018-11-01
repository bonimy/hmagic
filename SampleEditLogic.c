
    #include "structs.h"

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

    // \task Move this to a file called SampleEditLogic.c once it is created.
    // It is business logic and has nothing to do with any window class or
    // object.
    extern void
    SampleEdit_CopyToClipboard(SAMPEDIT const * const p_ed)
    {
        ZWAVE const * const zw = p_ed->zw;
        
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
        
        SetClipboardData(CF_WAVE, hgl);
        CloseClipboard();
    }

// =============================================================================

    extern BOOL
    SampleEdit_PasteFromClipboard(SAMPEDIT * const p_ed)
    {
        int i = 0;
        int j = 0;
        int k = 0;
        
        char const * b   = NULL;
        char const * dat = NULL;
        
        HGLOBAL hgl = INVALID_HANDLE_VALUE;
        
        WAVEFORMATEX * wfx = NULL;
        
        ZWAVE * const zw = p_ed->zw;
        
        // -----------------------------
        
        OpenClipboard(0);
        
        hgl = GetClipboardData(CF_WAVE);
        
        if(!hgl)
        {
            MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
            CloseClipboard();
            
            return FALSE;
        }
        
        b = (char*) GlobalLock(hgl);
        
        if( ( *(int*)b!=0x46464952) || *(int*)(b+8)!=0x45564157)
        {
error:
            MessageBox(framewnd,"This is not a wave.","Bad error happened",MB_OK);
noclip:
            GlobalUnlock(hgl);
            CloseClipboard();
            
            return FALSE;
        }
        
        j = *(int*) (b + 4);
        b += 8;
        dat = 0;
        wfx = 0;
        
        for(i = 4; i < j;)
        {
            switch(*(int*)(b+i))
            {
            
            case 0x20746d66:
                wfx = (WAVEFORMATEX*) (b + i + 8);
                
                break;
            
            case 0x61746164:
                dat = b + i + 8;
                
                k = *(int*)(b + i + 4);
                
                if(wfx)
                    goto foundall;
                
                break;
            }
            
            i += 8 + *(int*) ( b + i + 4);
        }
        
        if( ( ! wfx) || ! dat )
        {
            goto error;
        }
        
foundall:
        
        if(wfx->wFormatTag != 1)
        {
            MessageBox(framewnd,"The wave is not PCM","Bad error happened",MB_OK);
            goto noclip;
        }
        
        if(wfx->nChannels != 1)
        {
            MessageBox(framewnd,"Only mono is allowed.","Bad error happened",MB_OK);
            goto noclip;
        }
        
        if(wfx->wBitsPerSample == 16)
        {
            k >>= 1;
        }
        
        if(p_ed->sell > zw->end)
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
            zw->lopst += p_ed->sell + k - p_ed->selr;
        
        if(zw->lopst >= zw->end)
            zw->lflag = 0, zw->lopst = 0;
        
        if(zw->lflag)
            zw->buf[zw->end] = zw->buf[zw->lopst];
        
        p_ed->selr = p_ed->sell + k;
        
        if(wfx->wBitsPerSample == 16)
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
        
        GlobalUnlock(hgl);
        CloseClipboard();
        
        Modifywaves(p_ed->ew.doc,
                    p_ed->editsamp);
        
        p_ed->init = 1;
        
        return TRUE;
    }

// =============================================================================

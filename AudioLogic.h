
#if ! defined HMAGIC_AUDIO_LOGIC_HEADER_GUARD

    #define HMAGIC_AUDIO_LOGIC_HEADER_GUARD

// =============================================================================

    extern char midinst[16],
                midivol[16],
                midipan[16],
                midibank[16],
                midipbr[16];

    extern uint8_t noteflag;

    extern unsigned short globvol,globvolt,globvold;
    extern short globtrans;

    extern short midipb[16];

    extern short * wdata;

    extern int midi_timer;

    extern int miditimeres;
    
    extern int mix_freq,
               mix_flags;

    extern int ms_tim1,
               ms_tim2,
               ms_tim3;

    extern int playedpatt;
    
    extern int sounddev;

    extern int soundthr;

    extern int ws_freq;

    extern int ws_bufs;

    extern int ws_len;

    extern int ws_flags;

    extern int wcurbuf;

    extern int wbuflen;

    extern int wnumbufs;

    extern int * mixbuf;

    extern ZMIXWAVE zwaves[8];

    extern SONG * playedsong;

    extern HANDLE wave_end;

    extern WAVEHDR * wbufs;
    
    extern HWAVEOUT hwo;

    extern char const * mus_str[];

// =============================================================================

    void
    Initsound(void);

    void
    Exitsound(void);

    void
    Mixbuffer(void);

    int CALLBACK
    soundproc(HWAVEOUT bah, UINT msg, DWORD inst, DWORD p1, DWORD p2);

    void
    Loadsongs(FDOC * const doc);

    void
    Unloadsongs(FDOC * const param);

    void
    Updatesong(void);

    void
    Playsong(FDOC * const doc,
             int    const num);

    void
    Playpatt(void);

    void
    Modifywaves(FDOC * const doc,
                int    const es);


    extern void
    Edittrack(FDOC  * doc,
              short   i);

    extern void
    NewSR(FDOC*doc,int bank);

    void
    midinoteoff(ZCHANNEL * const zch);

// =============================================================================


    ;

#endif
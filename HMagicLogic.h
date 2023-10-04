
#if ! defined HMAGIC_LOGIC_HEADER_GUARD

    #define HMAGIC_LOGIC_HEADER_GUARD

// =============================================================================

    #include "structs.h"

// =============================================================================

    enum
    {
        MIDI_ARR_WORDS = 0x19,
        MIDI_ARR_BYTES = MIDI_ARR_WORDS * 2,
    };

// =============================================================================

    enum
    {
        NUM_MaxMRU = 4
    };

// =============================================================================

    enum tag_HM_AppMsgs
    {
        HM_WM_1,

    } HM_AppMsgs;

// =============================================================================

typedef
struct
{
    int sprsetedit;
    int namechg;
    char savespr[0x11c][16];
    int flag;
    int soundvol;
    unsigned short inst[25];
    unsigned short trans[25];
    char saveasmp[MAX_PATH];
} CONFIG;

// =============================================================================

    extern uint8_t sndinterp;

    extern uint8_t volflag;

    extern uint8_t activeflag;

    extern char asmpath[MAX_PATH];

    extern char currdir[MAX_PATH];

    extern char const
    vererror_str[];

    extern char const * mrulist[NUM_MaxMRU];

    extern uint16_t soundvol;

    extern uint16_t midi_inst[MIDI_ARR_WORDS];
    extern uint16_t midi_trans[MIDI_ARR_WORDS];

    extern int cfg_flag;

    extern int cfg_modf;
    
    extern int gbtnt;

    extern ZCHANNEL zchans[8];

    extern HMENU filemenu;
    
// =============================================================================

    extern void
    AddMRU(char * f);

    extern void
    UpdMRU(void);

    extern void
    FreeMRU(void);

    extern void
    ProcessMessage(MSG * msg);

// =============================================================================

#endif
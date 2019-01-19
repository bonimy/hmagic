
    #include "structs.h"

    #include "HMagicEnum.h"
    #include "HMagicLogic.h"

// =============================================================================

uint8_t sndinterp = 0;

uint8_t volflag = 0;

uint8_t activeflag = 255;

char const
vererror_str[] =
"This file was edited with a newer version of Gigasoft Hyrule Magic. "
"You need version %d.%00d or higher to open it.";

char asmpath[MAX_PATH] = { 0 };

char currdir[MAX_PATH] = { 0 };

char const * mrulist[NUM_MaxMRU] =
{
    0, 0, 0, 0
};

// Sound volume level.
uint16_t soundvol = 180;

uint16_t midi_inst[MIDI_ARR_WORDS] =
{
    0x0050,0x0077,0x002f,0x0050,0x0051,0x001b,0x0051,0x0051,
    0x004d,0x0030,0x002c,0x0039,0x00b1,0x004f,0x000b,0x0004,
    0x10a5,0x0038,0x003c,0x00a8,0x00a8,0x0036,0x0048,0x0035,
    0x001a
};

uint16_t midi_trans[MIDI_ARR_WORDS] =
{
    0x00fb,0x0005,0x000c,0x0c00,0x0000,0x0002,0xfb02,0xf602,
    0x0027,0x0000,0x0000,0xf400,0x0024,0x0000,0x0018,0x000c,
    0x0020,0x0000,0x0000,0x0028,0x0028,0x000c,0x000c,0x0001,
    0x0000
};

int cfg_flag = CFG_NOFLAGS;
int cfg_modf = 0;

int gbtnt = 0;

int mruload = 0;

ZCHANNEL zchans[8];

HMENU filemenu = 0;

// =============================================================================

extern void
AddMRU(char * f)
{
    int i = 0;
    
    // -----------------------------
    
    for(i = 0; i < NUM_MaxMRU; i++)
    {
        if( mrulist[i] && ! _stricmp(mrulist[i], f) )
        {
            char const * f2 = mrulist[i];
            
            // -----------------------------
            
            for( ; i; i--)
            {
                mrulist[i] = mrulist[i - 1];
            }
            
            mrulist[0] = f2;
            
            goto foundmru;
        }
    }
    
    free( (char*) mrulist[NUM_MaxMRU - 1] );
    
    for(i = (NUM_MaxMRU - 1); i; i--)
    {
        mrulist[i] = mrulist[i - 1];
    }
    
    mrulist[0] = _strdup(f);
    
foundmru:
    
    cfg_flag |= CFG_MRU_LOADED;
    
    cfg_modf = 1;
}

// =============================================================================

extern void
UpdMRU(void)
{
    int i = 0;
    
    // -----------------------------
    
    for(i = 0; i < NUM_MaxMRU; i += 1)
    {
        DeleteMenu(filemenu, ID_MRU1 + i, 0);
        
        if(mrulist[i])
        {
            text_buf_ty text_buf = { 0 };
            
            if(!mruload)
                AppendMenu(filemenu,MF_SEPARATOR,0,0);
            
            mruload=1;
            
            wsprintf(text_buf,
                     "%s\tCtrl+Alt+%d",
                     mrulist[i], i + 1);
            
            AppendMenu(filemenu, MF_STRING, ID_MRU1 + i, text_buf);
        }
    }
}

// =============================================================================

extern void
FreeMRU(void)
{
    int i = 0;
    
    // -----------------------------
    
    for(i = 0; i < NUM_MaxMRU; i += 1)
    {
        if( mrulist[i] )
        {
            free( (void*) mrulist[i] );
            
            mrulist[i] = NULL;
        }
    }
}

// =============================================================================

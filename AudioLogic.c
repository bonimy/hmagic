
    #include "structs.h"
    #include "prototypes.h"

    #include "Callbacks.h"

    #include "HMagicLogic.h"

    #include "AudioLogic.h"

// =============================================================================

    char midinst[16],
         midivol[16],
         midipan[16],
         midibank[16],
         midipbr[16];

    uint8_t pbwayflag;

    uint8_t pbstatflag,pbflag,fqflag;
    
    uint8_t noteflag = 0;

    uint8_t const nvol[16] =
    {
        0x19,0x32,0x4c,0x65,0x72,0x7f,0x8c,0x98,0xa5,0xb2,0xbf,0xcb,0xd8,0xe5,0xf2,0xfc
    };

    uint8_t const nlength[8] =
    {
        0x32,0x65,0x7f,0x98,0xb2,0xcb,0xe5,0xfc
    };

    unsigned short globvol,globvolt,globvold;

    short globtrans;

    short midipb[16];

    short * wdata = 0;

    int midi_timer = 0;
    
    int miditimeres = 5;

    int mix_freq  = 0,
        mix_flags = 0;

    int playedpatt = 0;

    int songtim = 0;
    int songspd = 100;
    
    int songspdt = 0;
    int songspdd = 0;

    // Sound device?
    int sounddev = 0x10001;

    int soundthr = 0;

    int ws_freq = 22050;

    int ws_bufs = 24;

    int ws_len = 256;

    int ws_flags = 3;

    int ms_tim1 = 5,
        ms_tim2 = 20,
        ms_tim3 = 5;

    int wcurbuf = 0;

    int wbuflen = 0;

    int wnumbufs = 0;

    int const freqvals[13] =
    {
        0x157,0x16c,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x221,
        0x241,0x263,0x288,0x2ae
    };

    int envdat[] =
    {
        0,0x3ff00,0x54900,0x64d31,0x83216,0xaec33,0xc9a63,0x10624d,0x154725,
        0x199999,0x202020,0x2B1DA4,0x333333,0x3F03F0,0x563B48,
        0x667000,0x7E07E0,0xAAAAAA,0xCCCCCC,0x1000000,0x1555555,0x1999999,
        0x2000000,0x2AAAAAA,0x3333333,0x4000000,0x5555555,0x6670000,
        0x8000000,0xAAAAAAA,0x10000000,0x20000000
    };

    int * mixbuf = 0;

    ZMIXWAVE zwaves[8];

    SONG * playedsong = 0;
    
    HANDLE wave_end = 0;

    WAVEHDR * wbufs = 0;

    HWAVEOUT hwo = 0;

// =============================================================================

void
Initsound(void)
{
    text_buf_ty text_buf = { 0 };
    
    int n,sh;
    
    WAVEFORMATEX wfx;
    WAVEHDR *wh;
    
    char *err;
    const unsigned char blkal[4]={1,2,2,4};
    
    if(sndinit)
        Exitsound();
    
    if((sounddev >> 16) == 2)
    {
        HMIDIOUT hmo;
        
        n = midiOutOpen(&hmo, sounddev - 0x20001, 0, 0, 0);
        
        if(n)
            goto openerror;
        
        hwo = (HWAVEOUT) hmo;
        
        for(n = 0; n < 16; n++)
        {
            midinst[n] = midivol[n]=midipan[n]=midibank[n]=255;
            midipbr[n] = 2;
            midipb[n] = 65535;
            midiOutShortMsg(hmo, 0x64b0 + n);
            midiOutShortMsg(hmo, 0x65b0 + n);
            midiOutShortMsg(hmo, 0x206b0 + n);
        }
        
        noteflag = 0;
        
        for(n = 0; n < 8; n++)
            zchans[n].midnote = 0xff;
        
        if(wver)
        {
//          miditimeres=ms_tim1;
//          timeBeginPeriod(miditimeres);
//          midi_timer=timeSetEvent(ms_tim2,ms_tim3,testfunc,0,TIME_PERIODIC);
            
            midi_timer=SetTimer(framewnd,1,ms_tim2,(TIMERPROC)midifunc2);
        }
        else
        {
            miditimeres = ms_tim1;
            timeBeginPeriod(miditimeres);
            midi_timer=timeSetEvent(ms_tim2,ms_tim3,midifunc,0,TIME_PERIODIC);
            soundthr = 1;
        }
        
        if(!midi_timer)
        {
            wsprintf(text_buf,
                     "Can't initialize timer: %08X",
                     GetLastError());
            
            MessageBox(framewnd,
                       text_buf,
                       "Bad error happened",
                       MB_OK);
            
            if(!wver)
                timeEndPeriod(miditimeres);
            
            midiOutClose(hmo);
            
            return;
        }
        
//      InitializeCriticalSection(&cs_song);
        
        sndinit = 1;
        
        goto endinit;
    }

    soundthr=0;
//  ht=CreateThread(0,0,soundfunc,0,0,&sndthread);
//  if(ht) CloseHandle(ht);
    wbuflen=ws_len<<(ws_flags&1);
    sh=(ws_flags>>1);
    wfx.wFormatTag=1;
    wfx.nChannels=(ws_flags&1)+1;
    wfx.nSamplesPerSec=ws_freq;
    wfx.nAvgBytesPerSec = ws_freq << ( sh + ( (ws_flags & 2) >> 1 ) );
    wfx.nBlockAlign=blkal[ws_flags];
    wfx.wBitsPerSample=(ws_flags&2)?16:8;
    wfx.cbSize=0;
    wnumbufs=ws_bufs;
//  if(ht) n=waveOutOpen(&hwo,sounddev - 0x10002,&wfx,sndthread,0,CALLBACK_THREAD),soundthr=1;
/*  else*/
    
    n = waveOutOpen(&hwo,
                    sounddev - 0x10002,
                    &wfx,
                    (DWORD_PTR) soundproc,
                    0,
                    CALLBACK_FUNCTION);
    
    if(n)
    {
openerror:
        switch(n) {
        case MIDIERR_NODEVICE:
            err="No MIDI port was found.";
            break;
        case MMSYSERR_ALLOCATED:
            err="Already in use";
            break;
        case MMSYSERR_BADDEVICEID:
            err="The device was removed";
            break;
        case MMSYSERR_NODRIVER:
            err="No driver found";
            break;
        case MMSYSERR_NOMEM:
            err="Not enough memory";
            break;
        case WAVERR_BADFORMAT:
            err="Unsupported playback quality";
            break;
        case WAVERR_SYNC:
            err="It is synchronous";
            break;
        default:
            
            wsprintf(text_buf,
                     "Unknown error %08X",
                     n);
            
            err = text_buf + 256;
            
            break;
        }
        
        wsprintf(text_buf,
                 "Cannot initialize sound (%s)",
                 err);
        
        MessageBox(framewnd,
                   text_buf,
                   "Bad error happened",
                   MB_OK);
        return;
    }
//  InitializeCriticalSection(&cs_song);
    wh=wbufs=malloc(sizeof(WAVEHDR)*ws_bufs);
    wdata=malloc(wbuflen*ws_bufs<<sh);
    for(n=0;n<8;n++) zwaves[n].pflag=0;
    for(n=0;n<wnumbufs;n++) {
        wh->lpData=(LPSTR)(wdata)+(n*wbuflen<<sh);
        wh->dwBufferLength=wbuflen<<sh;
        wh->dwFlags=0;
        wh->dwUser=n;
        waveOutPrepareHeader(hwo,wh,sizeof(WAVEHDR));
        wh++;
    }
    sndinit=1;
    mixbuf=malloc(wbuflen*ws_bufs<<2);
    mix_freq=ws_freq/6;
    mix_flags=ws_flags;
    for(wcurbuf=0;wcurbuf<wnumbufs;wcurbuf++)
        Mixbuffer();
    wcurbuf=0;
    for(n=0;n<wnumbufs;n++)
        waveOutWrite(hwo,wbufs+n,sizeof(WAVEHDR));
endinit:
    wave_end=CreateEvent(0,0,0,0);
}

// =============================================================================

void
Mixbuffer(void)
{
    static int i,j,k,l,m,n,o;
    ZMIXWAVE*zmw=zwaves;
    ZWAVE*zw;
    short chf=1;
    int*b;
    short*c;
    unsigned char*d;
    static char v1,v2;
    
    // \task Nitpicky, but why are all these static. Just makes things even
    // less re-entrant for no apparent reason.
    static unsigned short f;
    static unsigned envx,
                    envclk,
                    envclklo,
                    envclkadd,
                    envmul;
    
    // if(soundthr) EnterCriticalSection(&cs_song);
    
    Updatesong();
    ZeroMemory(mixbuf,wbuflen<<2);
    envmul = ( wbuflen << ( 16 - (mix_flags & 1) ) ) / mix_freq << 16;
    for(i=0;i<8;i++,zmw++,chf<<=1) {
        if(!(activeflag&chf)) continue;
        if(!zmw->pflag) continue;
        n=zmw->wnum;
        if(n<0 || n>=sounddoc->numwave) continue;
        zw=sounddoc->waves+n;
        b=mixbuf;
        j=zmw->pos;
        c=zw->buf;
        envx=zmw->envx;
        envclk=zmw->envclk;
        envclklo=zmw->envclklo;
        switch(zmw->envs)
        {
        
        case 0:
            envclkadd=envdat[zmw->atk];
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm adc edx,edx
            __asm add envx,edx
            
            if(envx > 0xfe0000)
            {
                envx = 0xfe0000;
                envclklo = 0;
                
                if(zmw->sus == 7)
                    zmw->envs = 2;
                else
                    zmw->envs = 1;
            }
            
            break;
        
        case 1:
            
            envclkadd=envdat[zmw->dec];
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm adc envclk,edx
            
            while(envclk > 0x20000)
            {
                envclk -= 0x20000;
                envx -= envx >> 8;
            }
            
            if(envx < (unsigned int) (zmw->sus << 21))
            {
                envx = zmw->sus << 21;
                envclklo = 0;
                zmw->envs = 2;
            }
            
            break;
        
        case 2:
            envclkadd=envdat[zmw->rel];
            
#if 1
            __asm mov eax, envclkadd
            __asm mov edx, envmul
            __asm imul edx
            __asm add envclklo, eax
            __asm adc envclk, edx
#else
            // \task Not exactly equivalent. does carry really matter here?
            // Do regression test.
            {
                uint64_t w = envclkadd;
                uint64_t x = envmul;
                uint64_t y = envclklo;
                uint64_t z = ( (uint64_t) envclk ) << 32;
                
                uint64_t a = (w * x);
                uint64_t b = (y | z) + a;
                
                envclklo = ( 0xffffffff &        b  );
                envclk   = ( 0xffffffff & (b >> 32) );
            }
#endif
            
            while(envclk > 0x20000)
            {
                envclk -= 0x20000,
                envx -= envx >> 8;
            }
            
            break;
        
        case 3:
            envclkadd=envdat[0x1f]>>1;
            
#if 1
            __asm mov eax,envclkadd
            __asm mov edx,envmul
            __asm imul edx
            __asm add envclklo,eax
            __asm sbb envx,edx
            
#else
            // \task not exactly equivalent? (sbb vs. sub) Do regression test.
            {
                signed borrow = 0;
                
                int32_t a = (envclkadd * envmul);
                
                envclklo += a;
                
                envx -= (envmul - borrow);
            }
#endif
            
            if(envx>=0x80000000) {
                zmw->pflag=0;
                envx=0;
            }
            break;
        }
        zmw->envx=envx;
        zmw->envclk=envclk;
        zmw->envclklo=envclklo;
        v1=((zmw->vol1*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
        v2=((zmw->vol2*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
        f=(zmw->freq<<12)/mix_freq;
        k=wbuflen;
        if(zw->lopst<zw->end) l=zw->lopst<<12; else l=0;
        m=zw->end<<12;
        if(!m) continue;
        if(mix_flags&1) {
            k>>=1;
            if(sndinterp) {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    *(b++)+=o*v2;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    *(b++)+=o*v2;
                    j+=f;
                }
            } else {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    *(b++)+=c[j>>12]*v1;
                    *(b++)+=c[j>>12]*v2;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    *(b++)+=c[j>>12]*v1;
                    *(b++)+=c[j>>12]*v2;
                    j+=f;
                }
            }
        } else {
            v1 = (v1 + v2) >> 1;
            if(sndinterp) {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    o=j>>12;
                    o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
                    *(b++)+=o*v1;
                    j+=f;
                }
            } else {
                if(zw->lflag) while(k--) {
                    if(j>=m) j+=l-m;
                    *(b++)+=c[j>>12]*v1;
                    j+=f;
                } else while(k--) {
                    if(j>=m) {zmw->pflag=0; break;}
                    *(b++)+=c[j>>12]*v1;
                    j+=f;
                }
            }
        }
        zmw->pos=j;
    }
    
    // if(soundthr) LeaveCriticalSection(&cs_song);
    
    k = wbuflen;
    b = mixbuf;
    
    if(mix_flags & 2)
    {
        c = wdata + (wcurbuf * wbuflen);
        
        while(k--)
        {
            *(c++) = ( *(b++) ) >> 7;
        }
    }
    else
    {
        d = (uint8_t*) ( (wdata) + (wcurbuf * wbuflen) );
        
        while(k--)
        {
            *(d++) = ( (*(b++) ) >> 15 ) ^ 0x80;
        }
    }
}

// =============================================================================

void
Unloadsongs(FDOC * const param)
{
    int i,j,k;
    SONG*s;
    SONGPART*sp;
    if(param->m_loaded) {
        if(sndinit) Stopsong();
        k=param->numsong[0]+param->numsong[1]+param->numsong[2];
        for(i=0;i<k;i++) {
            s=param->songs[i];
            if(!s) continue;
            if(!--s->inst) {
                for(j=0;j<s->numparts;j++) {
                    sp=s->tbl[j];
                    if(!--sp->inst) free(sp);
                }
                free(s->tbl);
                free(s);
            }
        }
        free(param->scmd);
        param->m_loaded=0;
        free(param->waves);
        free(param->insts);
        free(param->sndinsts);
        free(param->snddat1);
        free(param->snddat2);
    }
}

// =============================================================================

void
Updatesong(void)
{
    ZCHANNEL*zch;
    ZMIXWAVE*zw;
    SCMD*sc;
    ZINST*zi;
    
    HMIDIOUT const hmo = (HMIDIOUT) hwo;
    
    int i,j,k,l,m;
    
    unsigned char chf;
    
    if(!(sounddoc&&playedsong)) return;
    
    if(sounddev < 0x20000)
    {
        songtim += ( (songspd * wbuflen) << ( 3 - (mix_flags & 1) ) )
                 / mix_freq;
    }
    else songtim+=songspd*ms_tim2/20;
    for(;songtim>0;) {
        k=0;
        zch=zchans;
        zw=zwaves;
        chf=1;
        for(i=0;i<8;i++,zch++,zw++,chf<<=1) {
            if(!zch->playing) continue;
            zch->tim--;
            k=1;
            if(zch->ntim) {
                zch->ntim--;
                if(!zch->ntim) {
                    l=zch->pos;
                    j=zch->loop;
nexttime:
                    if(l==-1) {
                        j--;
                        if(j<=0) goto endnote;
                        l=zch->lopst;
                        goto nexttime;
                    }
                    sc=sounddoc->scmd+l;
                    if(sc->cmd!=0xc8) {
                        if(sc->cmd==0xef) {
                            l=*(short*)&(sc->p1);
                            goto nexttime;
                        }
                        if(sc->cmd>=0xe0) {
                            l=sc->next;
                            goto nexttime;
                        }
endnote:
                        if(sounddev<0x20000) zw->envs=3;
                        else midinoteoff(zch);
                    }
                }
            }
            if(zch->volt) {
                zch->volt--;
                zch->vol+=zch->vold;
                volflag|=chf;
            }
            if(zch->pant) {
                zch->pant--;
                zch->pan+=zch->pand;
                volflag|=chf;
            }
            
            if(pbflag&chf)
            {
                if(!zch->pbt) {
                    if(!zch->pbtim) zch->pbtim++;
                    if(pbstatflag&chf) pbflag&=~chf; else {
                        zch->pbt=zch->pbtim+1;
                        
                        // What is the significance of this constant?
                        // Midi vs. non-midi?
                        if(sounddev < 0x20000)
                        {
                            zch->note = j = zch->pbdst;
                            
                            l = j % 12;
                            
                            j =
                            (
                                freqvals[l]
                              + ( (freqvals[l + 1] - freqvals[l]) * zch->ft )
                            ) << (j / 12 + 4);
                            
                            zch->fdlt = (j - zch->freq) / zch->pbtim;
                        }
                        else
                        {
                            j = zch->pbdst - zch->note;
                            zch->midpbdlt = ((j << 13) / midipbr[zch->midch] + 0x2000 - zch->midpb) / zch->pbtim;
                        }
                        pbstatflag|=chf;
                    }
                } else if(pbstatflag&chf) {
                    if(sounddev<0x20000)
                        zch->freq+=zch->fdlt;
                    else zch->midpb+=zch->midpbdlt;
                    fqflag|=chf;
                }
                zch->pbt--;
            }
            if(zch->vibt) {
                zch->vibt--;
                zch->vibdep+=zch->vibdlt;
            }
            zch->vibcnt+=zch->vibspd;
            if(zch->vibdlt) fqflag|=chf;
            if(!zch->tim) {
again:
                if(zch->pos==-1) {zch->playing=0;continue;}
                sc=sounddoc->scmd+zch->pos;
                j=sc->next;
                if(j==-1 && zch->loop) {
                    zch->loop--;
                    if(!zch->loop) zch->pos=zch->callpos;
                    else zch->pos=zch->lopst;
                } else {
                    if(j>=sounddoc->m_size) zch->playing=0,zch->pos=-1;
                    else zch->pos=j;
                }
                if(sc->flag&1) zch->t1=sc->b1;
                if(!zch->t1) zch->t1=255;
                if(sc->flag&2) {
                    j=sc->b2;
                    zch->nvol=nvol[j&15];
                    zch->nlength=nlength[(j>>4)&7];
                }
                if(sc->cmd<0xc8) {
                    zch->ftcopy=zch->ft;
                    j=sc->cmd - 0x80 + zch->trans+globtrans;
                    
                    if(pbwayflag & chf)
                        j -= zch->pbdlt;
                    
                    if(sounddev < 0x20000)
                    {
                        zi = sounddoc->insts+zch->wnum;
                        l = j % 12;
                        
                        zch->freq =
                        (
                            freqvals[l]
                          + ( (freqvals[l + 1] - freqvals[l]) * zch->ft >> 8 )
                        ) << (j / 12 + 4);
                        
                        zw->pos=0;
                        zw->wnum=zi->samp;
                        zw->envclk=0;
                        if(zi->ad&128) {
                            zw->atk=((zi->ad<<1)&31)+1;
                            zw->dec=((zi->ad>>3)&14)+16;
                            zw->sus=zi->sr>>5;
                            zw->rel=zi->sr&31;
                            zw->envs=0;
                            zw->envx=0;
                        } else {
                            zw->sus=7;
                            zw->rel=0;
                            zw->atk=31;
                            zw->dec=0;
                            zw->envs=0;
                        }
                        zw->envclklo=0;
                        zw->pflag=1;
                    } else {
                        midinoteoff(zch);
                        if(activeflag&chf) {
                            if(zch->wnum>=25) goto nonote;
                            zch->midch=(midi_inst[zch->wnum]&0x80)?9:i;
                            
                            zch->midnum = (unsigned char) zch->wnum;
                            
                            if(zch->midch<8)
                            {
                                if(midibank[zch->midch]!=midi_inst[zch->wnum]>>8) {
                                    midibank[zch->midch]=midi_inst[zch->wnum]>>8;
                                    midiOutShortMsg(hmo, 0xb0 + zch->midch|(midibank[zch->midch]<<16));
                                }
                                if(midinst[zch->midch]!=midi_inst[zch->wnum]) {
                                    midinst[zch->midch] = (char) midi_inst[zch->wnum];
                                    midiOutShortMsg(hmo, 0xc0 + zch->midch|(midinst[zch->midch]<<8));
                                }
                                l=j+(char)(midi_trans[zch->wnum])+24;
                                if(l>=0 && l<0x80) zch->midnote=l;
                                if(midi_trans[zch->wnum]&0xff00) {
                                    l=j+(char)(midi_trans[zch->wnum]>>8)+24;
                                    if(l>=0 && l<0x80) zch->midnote|=l<<8;
                                }
                            } else {
                                l=midi_inst[zch->wnum]&0x7f;
                                if(midinst[zch->midch]!=midi_inst[zch->wnum]>>8) {
                                    midinst[zch->midch]=midi_inst[zch->wnum]>>8;
                                    midiOutShortMsg(hmo, 0xc0 + zch->midch|(midinst[zch->midch]<<8));
                                }
                                zch->midnote=l;
                                zch->pbdst=(j<<1)-midi_trans[zch->wnum];
                                zch->pbtim=0;
                                pbflag|=chf;
                            }
                            zch->midpb = 0x2000 + (zch->ftcopy<<5)/midipbr[zch->midch];
                            noteflag|=chf;
                        }
                    }
                    fqflag|=chf;
                    zch->pbt=zch->pbdly;
                    zch->note=j;
                    zch->vibt=zch->vibtim;
                    zch->vibcnt=0;
                    zch->vibdep=0;
                    pbstatflag&=~chf;
                    if(zch->pbdlt) pbflag|=chf,zch->pbdst=zch->pbdlt+zch->note;
                    volflag|=chf;
nonote:;
                }
                if(sc->cmd == 0xc9)
                {
                    if(sounddev < 0x20000)
                        zw->envs = 3;
                    else
                        midinoteoff(zch);
                    
                    pbflag&=~chf;
                }
                if(sc->cmd < 0xe0)
                {
                    zch->tim = zch->t1;
                    zch->ntim = (zch->nlength*zch->tim)>>8;
                    j = zch->pos;
                    
                    if(j != -1)
                    {
                        sc = sounddoc->scmd + j;
                        
                        if(sc->cmd == 249)
                        {
                            zch->pbt = sc->p1;
                            zch->pbtim = sc->p2;
                            zch->pbdst = sc->p3-128+zch->trans+globtrans;
                            pbstatflag &= ~chf;
                            pbflag |= chf;
                        }
                        
                        j = sc->next;
                    }
                    
                    if(sounddev >= 0x20000)
                    {
                        if(zch->midpb == 0x2000 && (pbflag & chf))
                        {
                            l = zch->pbdst-zch->note;
                            
                            if(l < 0)
                                l = -l;
                            
                            while(j != -1)
                            {
                                sc = sounddoc->scmd + j;
                                
                                if(sc->cmd < 0xe0 && sc->cmd == 0xc8)
                                    break;
                                
                                if(sc->cmd == 0xe9)
                                {
                                    m = sc->p3 - 128 + zch->trans + globtrans - zch->note;
                                    
                                    if(m < 0)
                                        m = -m;
                                    
                                    if(m > l)
                                        l = m;
                                }
                                
                                j = sc->next;
                            }
                            
                            l += 2;
                            
                            if(midipbr[zch->midch] != l)
                            {
                                midiOutShortMsg(hmo, 0x6b0 + zch->midch + (l << 16));
                                midipbr[zch->midch] = l;
                            }
                        }
                        
                        if((pbflag & chf) && (!zch->pbtim))
                        {
                            pbflag &= ~chf;
                            j = zch->pbdst - zch->note;
                            zch->midpb = ((j << 13) + (zch->ftcopy << 5)) / midipbr[zch->midch] + 0x2000;
                        }
                    }
                }
                else
                {
                    switch(sc->cmd)
                    {
                    case 224:
                        
                        zch->wnum=sc->p1;
                        
                        break;
                    
                    case 225:
                        
                        zch->pan=sc->p1<<8;
                        volflag|=chf;
                        
                        break;
                    
                    case 226:
                        
                        j=sc->p1+1;
                        zch->pant=j;
                        zch->pand=(((sc->p2<<8)-zch->pan)/j);
                        
                        break;
                    case 227:
                        zch->vibtim=sc->p1+1;
                        zch->vibspd=sc->p2;
                        zch->vibdlt=(sc->p3<<8)/zch->vibtim;
                        break;
                    case 229:
                        globvol=sc->p1<<8;
                        break;
                    case 230:
                        j=sc->p1+1;
                        globvolt=j;
                        globvold=((sc->p2<<8)-globvol)/j;
                        volflag|=chf;
                        break;
                    case 231:
                        songspd=sc->p1<<8;
                        break;
                    case 232:
                        j=sc->p1+1;
                        songspdt=j;
                        songspdd=((sc->p2<<8)-songspd)/j;
                        break;
                    case 233:
                        globtrans=(char)sc->p1;
                        break;
                    case 234:
                        zch->trans=sc->p1;
                        break;
                    case 237:
                        zch->vol=sc->p1<<8;
                        volflag|=chf;
                        break;
                    case 238:
                        j=sc->p1+1;
                        zch->volt=j;
                        zch->vold=((sc->p2<<8)-zch->vol)/j;
                        volflag|=chf;
                        break;
                    case 239:
                        if(*(unsigned short*)&(sc->p1)>=sounddoc->m_size) break;
                        zch->callpos=zch->pos;
                        zch->lopst=zch->pos=*(unsigned short*)&(sc->p1);
                        if(zch->pos>=sounddoc->m_size) zch->playing=0,zch->pos=zch->lopst=0;
                        zch->loop=sc->p3;
                        break;
                    case 241:
                        zch->pbdly=sc->p1;
                        zch->pbtim=sc->p2;
                        zch->pbdlt=sc->p3;
                        pbwayflag&=~chf;
                        break;
                    case 242:
                        zch->pbdly=sc->p1;
                        zch->pbtim=sc->p2;
                        zch->pbdlt=sc->p3;
                        pbwayflag|=chf;
                        break;
                    case 244:
                        zch->ft=sc->p1;
                        break;
                    }
                    goto again;
                }
            }
            if(volflag&chf)
            {
                volflag&=~chf;
                if(sounddev<0x20000) {
                    zw->vol1 = 0x19999 * (zch->nvol*zch->vol*((zch->pan>>8))>>22)>>16;
                    zw->vol2=(0x19999 * ((zch->nvol*zch->vol*(20-((zch->pan>>8)))>>22))>>16);
                } else {
                    l=((zch->vol>>9)*globvol>>16)*soundvol>>8;
                    if(l!=midivol[zch->midch]) {
                        midivol[zch->midch]=l;
                        midiOutShortMsg(hmo,(0x7b0 + zch->midch)|(l<<16));
                    }
                    l=((0x1400 - zch->pan) * 0x666)>>16;
                    if(l!=midipan[zch->midch]) {
                        midipan[zch->midch]=l;
                        midiOutShortMsg(hmo,(0xab0 + zch->midch)|(l<<16));
                    }
                }
            }
            if(fqflag&chf) {
                fqflag&=~chf;
                if(sounddev<0x20000) {
                    zi=sounddoc->insts+zch->wnum;
                    l=zch->freq*((zi->multhi<<8)+zi->multlo)>>14;
                    if(zch->vibdep) if(zch->vibcnt<128)
                        l=l*(65536+(zch->vibdep*(zch->vibcnt-64)>>11))>>16;
                    else l=l*(65536+(zch->vibdep*(191-zch->vibcnt)>>11))>>16;
                    zw->freq=l;
                } else {
                    l=zch->midpb;
                    if(zch->vibdep) if(zch->vibcnt<128)
                    l+=(zch->vibdep*(zch->vibcnt-64)>>10)/midipbr[zch->midch];
                    else l+=(zch->vibdep*(191-zch->vibcnt)>>10)/midipbr[zch->midch];
                    if(l!=midipb[zch->midch]) {
                        midipb[zch->midch]=l;
                        midiOutShortMsg(hmo, 0xe0 + zch->midch+((l&0x7f)<<8)+((l&0x3f80)<<9));
                    }
                }
            }
            if(noteflag&chf) {
                noteflag&=~chf;
                if(zch->midnote!=0xff) {
                    midiOutShortMsg(hmo, 0x90 + zch->midch|(zch->midnote<<8)|((zch->nvol<<15)&0x7f0000));
                    if(zch->midnote&0xff00)
                    midiOutShortMsg(hmo, 0x90 + zch->midch|(zch->midnote&0xff00)|((zch->nvol<<15)&0x7f0000));
                }
            }
        }
        
        if(songspdt)
        {
            songspdt--;
            songspd += songspdd;
        }
        
        if(globvolt)
        {
            globvolt--;
            globvol += globvold;
        }
        
        if(!k)
        {
            playedpatt++;
            if(playedpatt>=playedsong->numparts) if(playedsong->flag&2) playedpatt=playedsong->lopst; else {Stopsong();break;}
            if(playedpatt>=playedsong->numparts) {Stopsong();break;}
            Playpatt();
        } else songtim-=6100;
    }
}

// =============================================================================

void
Playsong(FDOC * const doc,
         int    const num)
{
    ZCHANNEL*zch=zchans;
    int i;
//  EnterCriticalSection(&cs_song);
    Stopsong();
    playedsong=doc->songs[num];
    for(i=0;i<8;i++) {
        zch->pand=zch->pant=zch->volt=zch->vold=zch->vibtim=0;
        zch->vibdep=zch->vibdlt=zch->pbdlt=0;
        zch->vol=65535;
        zch->pan=2048;
        zch->midpb=0x2000;
        zch->midch=i;
        zch->trans=0;
        zch->ft=0;
        zch->t1=255;
        zch++;
    }
    songspd=5500;
    songtim=0;
    songspdt=0;
    globvol=65535;
    globvolt=0;
    globtrans=0;
    playedpatt=0;
    volflag=pbflag=0;
    sounddoc=doc;
    Playpatt();
//  LeaveCriticalSection(&cs_song);
}

// =============================================================================

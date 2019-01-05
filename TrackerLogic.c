
    #include "structs.h"

    #include "Wrappers.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

    #include "HMagicUtility.h"

    #include "AudioLogic.h"

    #include "SampleEnum.h"

// =============================================================================

    int mark_sr;

    int mark_start;
    int mark_end;
    int mark_first;
    int mark_last;

    char op_len[32]=
        {1,1,2,3,0,1,2,1,2,1,1,3,0,1,2,3,1,3,3,0,1,3,0,3,3,3,1,2,0,0,0,0};

// =============================================================================

    short spcbank;

// =============================================================================
    
    short lastsr;
    short ss_lasttime;

    static int sbank_ofs[] = { 0xc8000, 0, 0xd8000, 0};

    unsigned short spclen;

    char fil1[4]={0,15,61,115};
    char fil2[4]={0,4,5,6};
    char fil3[4]={0,0,15,13};

    int ss_num,ss_size;
    unsigned short ss_next=0;

// =============================================================================

    int sndinit=0;

    FDOC * sounddoc=0;

// =============================================================================

static unsigned char *
Getspcaddr(FDOC *doc, unsigned short addr, short bank)
{
    unsigned char *rom;
    unsigned short a,b;
    spcbank = bank + 1;

again:
    rom = doc->rom + sbank_ofs[spcbank];
    
    for(;;)
    {
        a = *(unsigned short*) rom;
        
        if(!a)
        {
            if(spcbank)
            {
                spcbank = 0;
                
                goto again;
            }
            else
                return 0;
        }
        
        b = *(unsigned short*) (rom + 2);
        rom += 4;
        
        if(addr >= b && addr - b < a)
        {
            spclen = a;
            
            return rom + addr - b;
        }
        
        rom += a;
    }
}

// =============================================================================

extern short
AllocScmd(FDOC*doc)
{
    int i=doc->m_free;
    int j,k;
    SCMD*sc;
    if(i==-1) {
        j=doc->m_size;
        doc->m_size+=1024;
        sc=doc->scmd=realloc(doc->scmd,doc->m_size*sizeof(SCMD));
        k=1023;
        while(k--) sc[j].next=j+1,j++;
        sc[j].next=-1;
        k=1023;
        while(k--) sc[j].prev=j-1,j--;
        sc[j].prev=-1;
        i=j;
    } else sc=doc->scmd;
    doc->m_free=sc[doc->m_free].next;
    if(doc->m_free!=-1) sc[doc->m_free].prev=-1;
    return i;
}

// =============================================================================

extern short
Getblocktime(FDOC *doc, short num, short prevtime)
{
    SCMD *sc = doc->scmd, *sc2;
    
    int i = -1, j = 0, k = 0, l, m = 0, n = prevtime;
    l = num;
    
    if(l == -1)
        return 0;
    
    for(;;)
    {
        if(sc[l].flag&4)
        {
            j = sc[l].tim;
            m = sc[l].tim2;
            k=1;
        }
        
        if(!k)
            i = l;
        
        if(sc[l].flag & 1)
            n = sc[l].b1;
        
        l = sc[l].next;
        
        if(l == -1)
        {
            if(!k)
            {
                m = 0;
                j = 0;
            }
            
            break;
        }
    }
    
    if(i!=-1) for(;;) {
        if(i==-1) {
            MessageBox(framewnd,"Really bad error","Bad error happened",MB_OK);
            doc->m_modf=1;
            return 0;
        }
        sc2=sc+i;
        if(sc2->cmd==0xef)
        {
            k=*(short*)&(sc2->p1);
            if(k>=doc->m_size) {MessageBox(framewnd,"Invalid music address","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
            if(sc2->flag&1) {
                j+=Getblocktime(doc,k,0)*sc2->p3;
                if(ss_lasttime) {
                    j+=ss_lasttime*m;
                    j+=sc[k].tim2*sc2->b1;
                    j+=(sc2->p3-1)*sc[k].tim2*ss_lasttime;
                } else {
                    j+=sc2->b1*(m+sc[k].tim2*sc2->p3);
                }
                m=0;
            }
            else
            {
                j+=Getblocktime(doc,k,0)*sc2->p3;
                j+=ss_lasttime*m;
                if(ss_lasttime) j+=(sc2->p3-1)*ss_lasttime*sc[k].tim2,m=sc[k].tim2;
                else m+=sc[k].tim2*sc2->p3;
            }
        }
        else
        {
            if(sc2->cmd<0xe0) m++;
            if(sc2->flag&1) {j+=m*sc[i].b1; m=0; }
        }
        sc2->tim=j;
        sc2->tim2=m;
        sc2->flag|=4;
        
        if(i==num)
            break;
        
        i=sc2->prev;
    }
    
    ss_lasttime=n;
    return sc[num].tim+prevtime*sc[num].tim2;
}

// =============================================================================

short Loadscmd(FDOC*doc,unsigned short addr,short bank,int t)
{
    int b = 0,
        c = 0,
        d = 0,
        e = 0,
        f = 0,
        g = 0,
        h = 0,
        i = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0;
    
    unsigned char j = 0,
                  k = 0;
    
    unsigned char *a = 0;
    SRANGE*sr;
    SCMD*sc=doc->scmd,*sc2;
    if(!addr) return -1;
    
    a = Getspcaddr(doc,addr,bank);
    d = spcbank;
    if(!a) {
        MessageBox(framewnd,"Address not found when loading track","Bad error happened",MB_OK);
        return -1;
    }
    sr=doc->sr;
    e=doc->srnum;
    f=0x10000;
    for(c=0;c<e;c++) {
        if(sr[c].bank==d) if(sr[c].start>addr) {if(sr[c].start<f) f=sr[c].start; n=c;}
        else if(sr[c].end>addr) {
            for(f=sr[c].first;f!=-1;f=sc[f].next) {
                if(sc[f].flag&4) m=sc[f].tim,o=sc[f].tim2;
                if(sc[f].addr==addr) {
                    sc[f].tim=m;
                    sc[f].tim2=o;
                    lastsr=c;
                    return f;
                }
                if(sc[f].flag&1) k=sc[f].b1;
                if(sc[f].cmd<0xca) if(k) m-=k; else o--;
            }
            MessageBox(framewnd,"Misaligned music pointer","Bad error happened",MB_OK);
            return -1;
        }
    }
    c=n;
    i=h=doc->m_free;
    a-=addr;
    m=0;
    k=0;
    o=0;
    for(g=addr;g<f;) {
        sc2=sc+i;
        if(sc2->next==-1) {
            l=doc->m_size;
            sc=doc->scmd=realloc(sc,sizeof(SCMD)*(doc->m_size+=1024));
            sc2=sc+i;
            n=l+1023;
            while(l<n) sc[l].next=l+1,l++;
            sc[l].next=-1;
            n-=1023;
            while(l>n) sc[l].prev=l-1,l--;
            sc[l].prev=i;
            sc2->next=l;
        }
        sc2->addr=g;
        b=a[g];
        if(!b) break;
        if(m>=t && b!=0xf9) break;
        g++;
        j=0;
        if(b<128) {
            j=1;
            k=sc2->b1=b;
            b=a[g++];
            if(b<128) j=3,sc2->b2=b,b=a[g++];
        }
        if(b<0xe0) if(k) m+=k; else o++;
        sc2->cmd=b;
        sc2->flag=j;
        if(b>=0xe0) {
            b-=0xe0;
            if(op_len[b]) sc2->p1=a[g++];
            if(op_len[b]>1) sc2->p2=a[g++];
            if(op_len[b]>2) sc2->p3=a[g++];
            if(b==15) {
                doc->m_free=sc2->next;
                sc[sc2->next].prev=-1;
                l = Loadscmd(doc,*(short*)(&(sc2->p1)),bank,t-m);
                sc=doc->scmd;
                sc2=sc+i;
                *(short*)(&(sc2->p1))=l;
                sc2->next=doc->m_free;
                sc[sc2->next].prev=i;
                Getblocktime(doc,l,0);
                if(sc[l].flag&4) m+=(sc[l].tim+sc[l].tim2*k)*sc2->p3; else {i=sc2->next;break;}
                if(doc->sr[lastsr].endtime) k=doc->sr[lastsr].endtime;
            }
        }
        i=sc2->next;
    }
    sc[h].tim=m;sc[h].tim2=o;sc[h].flag|=4;
    if(f==g && m<t) {
        l=sc[i].prev;
        lastsr=c;
        sc[sr[lastsr].first].prev=l;
        l=sc[l].next=sr[lastsr].first;
        if(sc[l].flag&4) sc[h].tim=sc[l].tim+m+sc[l].tim2*k,sc[h].flag|=4;
        sr[lastsr].first=h;
        sr[lastsr].start=addr;
        sr[lastsr].inst++;
    }
    else
    {
        if(doc->srsize == doc->srnum)
            doc->sr = realloc(doc->sr,
                              (doc->srsize += 16) * sizeof(SRANGE) );
        
        lastsr=doc->srnum;
        sr=doc->sr+(doc->srnum++);
        sr->start=addr;
        sr->end=g;
        sr->first=h;
        sr->endtime=k;
        sr->inst=1;
        sr->editor=0;
        sr->bank=d;
        sc[sc[i].prev].next=-1;
    }
    
    sc[i].prev=-1;
    doc->m_free=i;
    
    return h;
}

// =============================================================================

extern void
Loadsongs(FDOC * const doc)
{
    unsigned char*b,*c,*d;
    short*e;
    SONG*s,*s2;
    SONGPART*sp;
    SCMD*sc;
    ZWAVE*zw;
    int i,j,k,l=0,m,n,o,p,q,r,t,u;
    int range,filter;
    sc=doc->scmd=malloc(1024*sizeof(SCMD));
    doc->m_free=0;
    doc->m_size=1024;
    doc->srnum=0;
    doc->srsize=0;
    doc->sr=0;
    doc->sp_mark=0;
    b=doc->rom;
    sbank_ofs[1]=(b[0x91c]<<15)+((b[0x918]&127)<<8)+b[0x914];
    sbank_ofs[3]=(b[0x93a]<<15)+((b[0x936]&127)<<8)+b[0x932];
    for(i=0;i<1024;i++) sc[i].next=i+1,sc[i].prev=i-1;
    sc[1023].next=-1;
    for(i=0;i<3;i++) {
        
        b = Getspcaddr(doc,0xd000,i);
        
        for(j = 0; ; j++)
        {
            if((r=((unsigned short*)b)[j])>=0xd000)
            {
                r = (r - 0xd000) >> 1;
                break;
            }
        }
        
        doc->numsong[i]=r;
        for(j=0;j<r;j++) {
            k=((unsigned short*)b)[j];
            if(!k) doc->songs[l]=0;
            else
            {
                c = Getspcaddr(doc,k,i);
                
                if(!spcbank)
                    m = 0;
                else
                    m = l - j;
                
                for( ; m < l; m++)
                    if(doc->songs[m] && doc->songs[m]->addr == k)
                    {
                        (doc->songs[l]=doc->songs[m])->inst++;
                        
                        break;
                    }
                
                if(m == l)
                {
                    doc->songs[l] = s = malloc(sizeof(SONG));
                    s->inst=1;
                    s->addr=k;
                    s->flag=!spcbank;
                    
                    for(m=0;;m++)
                        if((n=((unsigned short*)c)[m])<256) break;
                    
                    if(n > 0)
                        s->flag |= 2,
                        s->lopst = ( ((unsigned short*)c)[m + 1] - k ) >> 1;
                    
                    s->numparts=m;
                    s->tbl=malloc(4*m);
                    for(m=0;m<s->numparts;m++) {
                        k=((unsigned short*)c)[m];
                        d=Getspcaddr(doc,k,i);
                        if(!spcbank) n=0; else n=l-j;
                        for(;n<l;n++) {
                            s2=doc->songs[n];
                            if(s2) for(o=0;o<s2->numparts;o++)
                                if(s2->tbl[o]->addr==k) {
                                    (s->tbl[m]=s2->tbl[o])->inst++;
                                    goto foundpart;
                                }
                        }
                        for(o=0;o<m;o++) if(s->tbl[o]->addr==k) {
                            (s->tbl[m]=s->tbl[o])->inst++;
                            goto foundpart;
                        }
                        sp=s->tbl[m]=malloc(sizeof(SONGPART));
                        sp->flag=!spcbank;
                        sp->inst=1;
                        sp->addr=k;
                        p=50000;
                        for(o=0;o<8;o++) {
                            q=sp->tbl[o]=Loadscmd(doc,((unsigned short*)d)[o],i,p);
                            sc=doc->scmd+q;
                            if((sc->flag&4) && sc->tim<p) p=sc->tim;
                        }
foundpart:;
                    }
                }
            }
            l++;
        }
    }
    b=Getspcaddr(doc,0x800,0);
    doc->snddat1=malloc(spclen);
    doc->sndlen1=spclen;
    memcpy(doc->snddat1,b,spclen);
    b=Getspcaddr(doc,0x17c0,0);
    doc->snddat2=malloc(spclen);
    doc->sndlen2=spclen;
    memcpy(doc->snddat2,b,spclen);
    b=Getspcaddr(doc,0x3d00,0);
    doc->insts=malloc(spclen);
    memcpy(doc->insts,b,spclen);
    doc->numinst=spclen/6;
    b=Getspcaddr(doc,0x3e00,0);
    doc->m_ofs=b-doc->rom+spclen;
    doc->sndinsts=malloc(spclen);
    memcpy(doc->sndinsts,b,spclen);
    doc->numsndinst=spclen/9;
    b=Getspcaddr(doc,0x3c00,0);
    zw=doc->waves=malloc(sizeof(ZWAVE)*(spclen>>2));
    p=spclen>>1;
    
    for(i=0;i<p;i+=2)
    {
        j=((unsigned short*)b)[i];
        
        if(j==65535) break;
        
        for(k=0;k<i;k+=2)
        {
            if(((unsigned short*)b)[k]==j)
            {
                zw->copy = (short) (k >> 1);
                goto foundwave;
            }
        }
        
        zw->copy=-1;
        
foundwave:
        
        d = Getspcaddr(doc,j,0);
        e = malloc(2048);
        
        k = 0;
        l = 1024;
        u = t = 0;
        
        for(;;)
        {
            m = *(d++);
            
            range  = (m >> 4) + 8;
            filter = (m & 12) >> 2;
            
            for(n = 0; n < 8; n++)
            {
                o = (*d) >> 4;
                
                if(o > 7)
                    o -= 16;
                
                o <<= range;
                
                if(filter)
                    o += ( t * fil1[filter] >> fil2[filter] )
                       - ( (u & -256) * fil3[filter] >> 4 );
                
                if(o > 0x7fffff)
                    o = 0x7fffff;
                
                if(o < -0x800000)
                    o = -0x800000;
                
                u = o;
                
// \code             if(t>0x7fffff) t=0x7fffff;
// \code              if(t < -0x800000) t=-0x800000;
                
                e[k++] = o >> 8;
                
                o = *(d++) &15;
                
                if(o > 7)
                    o -= 16;
                
                o <<= range;
                
                if(filter)
                    o+=(u*fil1[filter]>>fil2[filter])-((t&-256)*fil3[filter]>>4);
                
                if(o>0x7fffff) o=0x7fffff;
                
                if(o < -0x800000) o = -0x800000;
                
                t = o;
// \code             if(u>0x7fffff) u=0x7fffff;
// \code             if(u < -0x800000) u= -0x800000;
                e[k++]=o>>8;
            }
            
            if(m & 1)
            {
                zw->lflag=(m&2)>>1; break;}
            if(k==l) {l+=1024;e=realloc(e,l<<1);}
        }
        
        e = zw->buf = realloc(e, (k + 1) << 1);
        
        zw->lopst = (((unsigned short *) b)[i + 1] - j) * 16 / 9;
        
        if(zw->lflag)
            e[k] = e[zw->lopst];
        else
            e[k] = 0;
        
        zw->end = k;
        
        zw++;
    }
    
    doc->numwave=i>>1;
    doc->m_loaded=1;
    doc->w_modf=0;
}

// =============================================================================

SSBLOCK * Allocspcblock(int len,int bank)
{
    SSBLOCK*sbl;
    if(!len) {
        MessageBox(framewnd,"Warning zero length block allocated","Bad error happened",MB_OK);
    }
    if(ss_num==ss_size) {
        ss_size+=512;
        ssblt=realloc(ssblt,ss_size<<2);
    }
    ssblt[ss_num]=sbl=malloc(sizeof(SSBLOCK));
    ss_num++;
    sbl->start=ss_next;
    sbl->len=len;
    sbl->buf=malloc(len);
    sbl->relocs=malloc(32);
    sbl->relsz=16;
    sbl->relnum=0;
    sbl->bank=bank&7;
    sbl->flag=bank>>3;
    ss_next+=len;
    return sbl;
}
void Addspcreloc(SSBLOCK*sbl,short addr)
{
    sbl->relocs[sbl->relnum++]=addr;
    if(sbl->relnum==sbl->relsz) {
        sbl->relsz+=16;
        sbl->relocs=realloc(sbl->relocs,sbl->relsz<<1);
    }
}
short Savescmd(FDOC*doc,short num,short songtime,short endtr)
{
    SCMD*sc=doc->scmd,*sc2;
    SRANGE*sr=doc->sr;
    SSBLOCK*sbl;
    
    text_buf_ty buf;
    
    unsigned char*b;
    int i = num,
        j = 0,
        k = 0,
        l = 0,
        m = 0,
        n = 0,
        o = 0,
        p = 0;
    
    if(i==-1) return 0;
    if(i>=doc->m_size) {MessageBox(framewnd,"Error.","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
    if(sc[i].flag&8) return sc[i].addr;
    for(;;) {
        j=sc[i].prev;
        if(j==-1) break;
        i=j;
    }
    for(j=0;j<doc->srnum;j++) {
        if(sr[j].first==i) {
            l=Getblocktime(doc,i,0);
            m=i;
            for(;;) {
                if(m==-1) break;
                k++;
                sc2=sc+m;
                if(sc2->flag&1) k++,n=sc2->b1;
                if(sc2->flag&2) k++;
                if(sc2->cmd>=0xe0) k+=op_len[sc2->cmd - 0xe0];
                m=sc2->next;
            }
            songtime-=l;
            if(songtime>0) {
                l=(songtime+126)/127;
                if(songtime%l) l+=2;
                l++;
                if(n && !songtime%n) {
                    p=songtime/n;
                    if(p<l) l=p;
                } else p=-1;
                k+=l;
            }
            k++;
            sbl=Allocspcblock(k,sr[j].bank|((!endtr)<<3)|16);
            b=sbl->buf;
            for(;;) {
                if(i==-1) break;
                sc2=sc+i;
                sc2->addr=b-sbl->buf+sbl->start;
                sc2->flag|=8;
                if(sc2->flag&1) *(b++)=sc2->b1;
                if(sc2->flag&2) *(b++)=sc2->b2;
                *(b++)=sc2->cmd;
                if(sc2->cmd>=0xe0) {
                    o=op_len[sc2->cmd - 0xe0];
                    if(sc2->cmd==0xef) {
                        *(short*)b=Savescmd(doc,*(short*)&(sc2->p1),0,1);
                        if(b) Addspcreloc(sbl,b-sbl->buf);
                        b[2]=sc2->p3;
                        b+=3;
                    } else {
                        if(o) *(b++)=sc2->p1;
                        if(o>1) *(b++)=sc2->p2;
                        if(o>2) *(b++)=sc2->p3;
                    }
                }
                i=sc2->next;
            }
            if(songtime>0) {
                if(l!=p) {
                    l=(songtime+126)/127;
                    if(songtime%l) n=127; else n=songtime/l;
                    *(b++)=n;
                }
                
                for( ; songtime >= n; songtime -= n)
                    *(b++) = 0xc9;
                
                if(songtime)
                {
                    *(b++) = (uint8_t) songtime;
                    *(b++) = 0xc9;
                }
            }
            *(b++)=0;
            return sc[num].addr;
        }
    }
    
    wsprintf(buf, "Address %04X not found", num);
    
    MessageBox(framewnd, buf, "Bad error happened", MB_OK);
    
    doc->m_modf = 1;
    
    return 0;
}

// =============================================================================

int
Writespcdata(FDOC*doc,void*buf,int len,int addr,int spc,int limit)
{
    unsigned char * rom = doc->rom;
    
    if(!len) return addr;
    
    if(((addr+len+4)&0x7fff)>0x7ffb)
    {
        if(addr+5>limit) goto error;
        *(int*)(rom+addr)=0x00010140;
        rom[addr+4]=0xff;
        addr+=5;
    }
    
    if(addr+len+4>limit)
    {
error:
        MessageBox(framewnd,"Not enough space for sound data","Bad error happened",MB_OK);
        doc->m_modf=1;
        return 0xc8000;
    }
    
    *(short*)(rom+addr)=len;
    *(short*)(rom+addr+2)=spc;
    
    memcpy(rom+addr+4,buf,len);
    
    return addr+len+4;
}

// =============================================================================

void Savesongs(FDOC *doc)
{
    int i,j,k,l=0,m,n,o,p,q,r,t,u,v,w,a,e,f,g;
    unsigned short bank_next[4],bank_lwr[4];
    short *c, *d;
    unsigned char *rom, *b;
    
    SONG *s;
    SCMD *sc;
    SONGPART *sp;
    SSBLOCK *stbl, *sptbl, *trtbl, *pstbl;
    ZWAVE *zw, *zw2;
    ZINST *zi;
    SAMPEDIT *sed;
    
    short wtbl[128],x[16],y[18];
    unsigned char z[64];
    
    ss_num = 0;
    ss_size = 512;
    ss_next = 0;
    ssblt = malloc(512 * sizeof(SSBLOCK));
    
    // if the music has not been modified, return. 
    if(! (doc->m_modf) )
        return;
    
    // set it so the music has not been modified. (reset the status)
    doc->m_modf = 0;
    rom = doc->rom;
    
    SetCursor(wait_cursor);
    
    for(i = 0; i < 3; i++)
    {
        k = doc->numsong[i];
        
        for(j = 0; j < k; j++)
        {
            s = doc->songs[l++];
            
            if(!s)
                continue;
            
            s->flag &= -5;
            
            for(m = 0; m < s->numparts; m++)
            {
                sp = s->tbl[m];
                sp->flag &= -3;
            }
        }
    }
    
    j = doc->m_size;
    sc = doc->scmd;
    
    for(i = 0; i < j; i++)
    {
        sc->flag &= -13;
        sc++;
    }
    
    l = 0;
    
    for(i = 0; i < 3; i++)
    {
        k = doc->numsong[i];
        stbl = Allocspcblock(k<<1,i+1);
        
        for(j = 0; j < k; j++)
        {
            s = doc->songs[l++];
            
            if(!s)
            {
                ( (short*) (stbl->buf) )[j] = 0;
                
                continue;
            }
            
            if(s->flag & 4)
                goto alreadysaved;
            
            sptbl = Allocspcblock(((s->numparts+1)<<1)+(s->flag&2),(s->flag&1)?0:(i+1));
            
            for(m = 0; m < s->numparts; m++)
            {
                sp = s->tbl[m];
                
                if(sp->flag&2)
                    goto spsaved;
                
                trtbl = Allocspcblock(16,(sp->flag&1)?0:(i+1));
                
                p = 0;
                
                for(n = 0; n < 8; n++)
                {
                    o = Getblocktime(doc,sp->tbl[n],0);
                    
                    if(o > p)
                        p = o;
                }
                
                q = 1;
                
                for(n = 0; n < 8; n++)
                {
                    stle16b_i(trtbl->buf,
                              n,
                              Savescmd(doc, sp->tbl[n], p, q) );
                    
                    if( ldle16b_i(trtbl->buf, n) )
                        Addspcreloc(trtbl,n<<1),q=0;
                }
                
                sp->addr = trtbl->start;
                sp->flag |= 2;
spsaved:
                ( (short*) (sptbl->buf) )[m] = sp->addr;
                
                Addspcreloc(sptbl,m<<1);
            }
            
            if(s->flag&2)
            {
                ( (short*) (sptbl->buf) )[m++] = 255;
                ( (short*) (sptbl->buf) )[m] = sptbl->start + (s->lopst << 1);
                
                Addspcreloc(sptbl,m<<1);
            }
            else
                ( (short*) (sptbl->buf) )[m++] = 0;
            
            s->addr = sptbl->start;
            s->flag |= 4;
alreadysaved:
            ( (short*) (stbl->buf) )[j] = s->addr;
            
            Addspcreloc(stbl, j << 1);
        }
    }
    
    if(doc->w_modf)
    {
        b = (uint8_t*) malloc(0xc000);
        j = 0;
        
        zw = doc->waves;
        
        if(doc->mbanks[3])
            sed = (SAMPEDIT*)GetWindowLongPtr(doc->mbanks[3],GWLP_USERDATA);
        else
            sed = 0;
        
        for(i = 0; i < doc->numwave; i++, zw++)
        {
            if(zw->copy != -1)
                continue;
            
            wtbl[i << 1] = j + 0x4000;
            
            if(zw->lflag)
            {
                l = zw->end-zw->lopst;
                
                if(l&15)
                {
                    k = (l << 15) / ((l + 15) & -16);
                    p = (zw->end << 15)/k;
                    c = malloc(p << 1);
                    n = 0;
                    d = zw->buf;
                    
                    for(m = 0;;)
                    {
                        c[n++] = (d[m >> 15] * ( (m & 32767)^32767) + d[(m>>15)+1]*(m&32767))/32767;
                        
                        m += k;
                        
                        if(n >= p)
                            break;
                    }
                    
                    zw->lopst = (zw->lopst << 15) / k;
                    zw->end = p;
                    zw->buf = (short*) realloc(zw->buf, (zw->end + 1) << 1);
                    memcpy(zw->buf,c,zw->end<<1);
                    free(c);
                    zw->buf[zw->end]=zw->buf[zw->lopst];
                    zw2=doc->waves;
                    
                    for(m=0;m<doc->numwave;m++,zw2++)
                        if(zw2->copy==i)
                            zw2->lopst=zw2->lopst<<15/k;
                    
                    zi=doc->insts;
                    
                    for(m=0;m<doc->numinst;m++)
                    {
                        n=zi->samp;
                        
                        if(n>=doc->numwave)
                            continue;
                        
                        if(n==i || doc->waves[n].copy==i)
                        {
                            o=(zi->multhi<<8)+zi->multlo;
                            o=(o<<15)/k;
                            zi->multlo=o;
                            zi->multhi=o>>8;
                            
                            if(sed && sed->editinst==m)
                            {
                                sed->init=1;
                                SetDlgItemInt(sed->dlg,3014,o,0);
                                sed->init=0;
                            }
                        }
                        
                        zi++;
                    }
                    
                    Modifywaves(doc,i);
                }
        }
        k=(-zw->end)&15;
        d=zw->buf;
        n=0;
        wtbl[ (i << 1) + 1] = ( (zw->lopst + k) >> 4) * 9 + wtbl[i << 1];
        y[0]=y[1]=0;
        u=4;
        for(;;) {
            for(o=0;o<16;o++) {
                if(k) k--,x[o]=0;
                else x[o]=d[n++];
            }
            p=0x7fffffff;
            a=0;
            for(t=0;t<4;t++) {
                r=0;
                for(o=0;o<16;o++) {
                    l=x[o];
                    y[o+2]=l;
                    l+=(y[o]*fil3[t]>>4)-(y[o+1]*fil1[t]>>fil2[t]);
                    if(l>r) r=l;
                    else if(-l>r) r=-l;
                }
                r<<=1;
                if(t) m=14; else m=15;
                for(q=0;q<12;q++,m+=m) if(m>=r) break;
tryagain:
                if(q && (q<12 || m == r))
                    v = (1 << (q - 1) ) - 1;
                else
                    v = 0;
                m=0;
                for(o=0;o<16;o++) {
                    l=(y[o+1]*fil1[t]>>fil2[t])-(y[o]*fil3[t]>>4);
                    w=x[o];
                    r = (w - l + v) >> q;
                    if((r+8)&0xfff0) {q++; a-=o; goto tryagain;}
                    z[a++]=r;
                    l=(r<<q)+l;
                    y[o+2]=l;
                    l-=w;
                    m+=l*l;
                }
                if(u==4) { u=0,e=q,f=y[16],g=y[17]; break; }
                if(m<p) p=m,u=t,e=q,f=y[16],g=y[17];
            }
            m=(e<<4)|(u<<2);
            if(n==zw->end) m|=1;
            if(zw->lflag) m|=2;
            b[j++]=m;
            m=0;
            a=u<<4;
            for(o=0;o<16;o++) {
                m|=z[a++]&15;
                if(o&1) b[j++]=m,m=0; else m<<=4;
            }
            if(n==zw->end) break;
            y[0]=f;
            y[1]=g;
        }
    }
    
    if(sed)
    {
        SetDlgItemInt(sed->dlg, ID_Samp_SampleLengthEdit, sed->zw->end,0);
        SetDlgItemInt(sed->dlg, ID_Samp_LoopPointEdit, sed->zw->lopst,0);
        
        InvalidateRect( GetDlgItem(sed->dlg, ID_Samp_Display), 0, 1);
    }
    
    zw = doc->waves;
    
    for(i=0;i<doc->numwave;i++,zw++) if(zw->copy!=-1) {
            wtbl[i<<1]=wtbl[zw->copy<<1];
            wtbl[(i<<1)+1]=(zw->lopst>>4)*9+wtbl[i<<1];
        }
    m=Writespcdata(doc,wtbl,doc->numwave<<2,0xc8000,0x3c00,0xd74fc);
    m=Writespcdata(doc,b,j,m,0x4000,0xd74fc);
    free(b);
    m=Writespcdata(doc,doc->insts,doc->numinst*6,m,0x3d00,0xd74fc);
    m=Writespcdata(doc,doc->snddat1,doc->sndlen1,m,0x800,0xd74fc);
    m=Writespcdata(doc,doc->snddat2,doc->sndlen2,m,0x17c0,0xd74fc);
    m=Writespcdata(doc,doc->sndinsts,doc->numsndinst*9,m,0x3e00,0xd74fc);
    doc->m_ofs=m;
    } else m=doc->m_ofs;
    bank_next[0]=0x2880;
    bank_next[1]=0xd000;
    bank_next[2]=0xd000;
    bank_next[3]=0xd000;
    bank_lwr[0]=0x2880;
    for(k=0;k<4;k++) {
        pstbl=0;
        for(i=0;i<ss_num;i++) {
            stbl=ssblt[i];
            if(stbl->bank!=k) continue;
            j=bank_next[k];
            if(j+stbl->len>0xffc0) { if(k==3) j=0x2880; else j=bank_next[0]; bank_lwr[k]=j; pstbl=0; }
            if(j+stbl->len>0x3c00 && j<0xd000)
            {
                text_buf_ty buf;
                
                SetCursor(normal_cursor);
                
                wsprintf(buf, "Not enough space for music bank %d", k);
                MessageBox(framewnd, buf, "Bad error happened", MB_OK);
                
                doc->m_modf=1;
                
                return;
            }
            if(pstbl && (pstbl->flag&1) && (stbl->flag&2)) j--,pstbl->len--;
            stbl->addr=j;
            pstbl=stbl;
            bank_next[k]=j+stbl->len;
        }
    }
    for(i=0;i<ss_num;i++) {
        stbl=ssblt[i];
        for(j=stbl->relnum-1;j>=0;j--) {
            k=*(unsigned short*)(stbl->buf+stbl->relocs[j]);
            for(l=0;l<ss_num;l++) {
                sptbl=ssblt[l];
                if(sptbl->start<=k && sptbl->len>k-sptbl->start) goto noerror;
            }
            MessageBox(framewnd,"Internal error","Bad error happened",MB_OK);
            doc->m_modf=1;
            return;
noerror:
            
            if(((!sptbl->bank) && stbl->bank<3)||(sptbl->bank==stbl->bank))
            {
                *(unsigned short*)(stbl->buf+stbl->relocs[j])=sptbl->addr+k-sptbl->start;
            }
            else
            {
                text_buf_ty buf;
                
                wsprintf(buf,
                         "An address outside the bank was referenced",
                         sptbl->bank,
                         stbl->bank);
                
                MessageBox(framewnd,
                           buf,
                           "Bad error happened",
                           MB_OK);
                
                doc->m_modf = 1;
                
                return;
            }
        }
    }
    l=m;
    for(k=0;k<4;k++) {
        switch(k) {
        case 1:
            rom[0x914]=l;
            rom[0x918]=(l>>8)|128;
            rom[0x91c]=l>>15;
            break;
        case 2:
            l=0xd8000;
            break;
        case 3:
            l=m;
            rom[0x932]=l;
            rom[0x936]=(l>>8)|128;
            rom[0x93a]=l>>15;
            break;
        }
        for(o=0;o<2;o++) {
            n=l+4;
            for(i=0;i<ss_num;i++) {
                stbl=ssblt[i];
                if(!stbl) continue;
                if((stbl->addr<0xd000)^o) continue;
                if(stbl->bank!=k) continue;
                if(n+stbl->len>((k==2)?0xdb7fc:0xd74fc)) {
                    MessageBox(framewnd,"Not enough space for music","Bad error happened",MB_OK);
                    doc->m_modf=1;
                    return;
                }
                memcpy(rom+n,stbl->buf,stbl->len);
                n+=stbl->len;
                free(stbl->relocs);
                free(stbl->buf);
                free(stbl);
                ssblt[i]=0;
            }
            if(n>l+4) {
                *(short*)(rom+l)=n-l-4;
                *(short*)(rom+l+2)=o?bank_lwr[k]:0xd000;
                l=n;
            }
        }
        *(short*)(rom+l)=0;
        *(short*)(rom+l+2)=0x800;
        if(k==1) m=l+4;
    }
    free(ssblt);
    SetCursor(normal_cursor);
}

// =============================================================================

void
Edittrack(FDOC*doc,short i)
{
    int j, k, l;
    SRANGE * sr = doc->sr;
    SCMD * sc;
    
    text_buf_ty buf;
    
    // -----------------------------
    
    k = doc->srnum;
    
    sc = doc->scmd;
    
    if(i == -1) return;
    
    if(i >= doc->m_size)
    {
        wsprintf(buf,
                 "Invalid address: %04X",i);
        goto error;
    }
    
    for(;;)
    {
        if((j=sc[i].prev)!=-1)
            i = j;
        else
            break;
    }
    
    for(l=0;l<k;l++)
        if(sr->first==i)
            break;
        else
            sr++;
    
    if(l == k)
    {
        wsprintf(buf, "Not found: %04X",i);
error:
        MessageBox(framewnd, buf,"Bad error happened",MB_OK);
        return;
    }
    
    if(sr->editor)
        HM_MDI_ActivateChild(clientwnd, sr->editor);
    else
        Editwin(doc,"TRACKEDIT","Song editor",l+(i<<16),sizeof(TRACKEDIT));
}

//CRITICAL_SECTION cs_song;
// =============================================================================

extern void
NewSR(FDOC*doc,int bank)
{
    SCMD * sc;
    SRANGE * sr;
    
    if(doc->srnum==doc->srsize)
    {
        doc->srsize+=16;
        doc->sr=realloc(doc->sr,doc->srsize*sizeof(SRANGE));
    }
    
    sr=doc->sr+doc->srnum;
    doc->srnum++;
    sr->first=AllocScmd(doc);
    sr->bank=bank;
    sr->editor=0;
    sc=doc->scmd+sr->first;
    sc->prev=-1;
    sc->next=-1;
    sc->cmd=128;
    sc->flag=0;
    Edittrack(doc,sr->first);
}

// =============================================================================





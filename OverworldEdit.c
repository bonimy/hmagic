
    #include "structs.h"

    #include "prototypes.h"

    #include "HMagicUtility.h"

    #include "OverworldEdit.h"

// =============================================================================

unsigned short ovlblk[4];

int const blkx[16]={0,8,0,8,16,24,16,24,0,8,0,8,16,24,16,24};
int const blky[16]={0,0,8,8,0,0,8,8,16,16,24,24,16,16,24,24};

short const tool_ovt[]={0,0,0,1,0,0,0,2,3,5,4};

char const * sprset_str[] =
{
    "Beginning",
    "First part",
    "Second part"
};

unsigned short copy_w = 0;
unsigned short copy_h = 0;

unsigned short * copybuf = 0;

int const sprset_loc[3] = {0x4c881, 0x4c901, 0x4ca21};

int const map_ind[4] = {0, 1, 8, 9};

int const map_ofs[5] = {0, 16, 512, 528};

// =============================================================================

void
Getblock32(uint8_t const * const rom,
           int             const m,
           short         * const l)
{
    int n;
    if(m==-1) { l[0]=l[1]=l[2]=l[3]=0xdc4; return; }
    n=(m>>2)*6;
    switch(m&3) {
    case 0:
        l[0] = rom[0x18000 + n] | (rom[0x18004 + n] >> 4) << 8;
        l[1] = rom[0x1b400 + n] | (rom[0x1b404 + n] >> 4) << 8;
        l[2] = rom[0x20000 + n] | (rom[0x20004 + n] >> 4) << 8;
        l[3] = rom[0x23400 + n] | (rom[0x23404 + n] >> 4) << 8;
        break;
    case 1:
        l[0] = rom[0x18001 + n] | (rom[0x18004 + n] & 15) << 8;
        l[1] = rom[0x1b401 + n] | (rom[0x1b404 + n] & 15) << 8;
        l[2] = rom[0x20001 + n] | (rom[0x20004 + n] & 15) << 8;
        l[3] = rom[0x23401 + n] | (rom[0x23404 + n] & 15) << 8;
        break;
    case 2:
        l[0] = rom[0x18002 + n] | (rom[0x18005 + n] >> 4) << 8;
        l[1] = rom[0x1b402 + n] | (rom[0x1b405 + n] >> 4) << 8;
        l[2] = rom[0x20002 + n] | (rom[0x20005 + n] >> 4) << 8;
        l[3] = rom[0x23402 + n] | (rom[0x23405 + n] >> 4) << 8;
        break;
    case 3:
        l[0] = rom[0x18003 + n] | (rom[0x18005 + n] & 15) << 8;
        l[1] = rom[0x1b403 + n] | (rom[0x1b405 + n] & 15) << 8;
        l[2] = rom[0x20003 + n] | (rom[0x20005 + n] & 15) << 8;
        l[3] = rom[0x23403 + n] | (rom[0x23405 + n] & 15) << 8;
        break;
    }
}

// =============================================================================

void
Drawblock32(OVEREDIT * const ed,
            int        const m,
            int        const t)
{
    int k,
        n,
        p,
        q;
    
    unsigned short *o;
    unsigned char *rom = ed->ew.doc->rom;
    
    short l[4];
    
    Getblock32(rom, m, l);
    
    p = 0;
    
    for(k = 0; k < 4; k++)
    {
        o = (unsigned short*)(rom + 0x78000 + ((((ed->disp & 8) && ovlblk[k] != 65535) ? ovlblk[k] : l[k]) << 3));
        
        for(q = 0; q < 4; q++)
        {
            n = *(o++);
            Drawblock(ed, blkx[p], blky[p], n, t);
            p++;
        }
    }
}

// =============================================================================

void
Getselectionrect(OVEREDIT const * const ed,
                 RECT           * const rc)
{
    int i,j;
    i=(ed->rectleft-ed->mapscrollh)<<5;
    j=((ed->rectright-ed->mapscrollh)<<5);
    if(i<j) rc->left=i,rc->right=j; else rc->left=j,rc->right=i;
    i=(ed->recttop-ed->mapscrollv)<<5;
    j=((ed->rectbot-ed->mapscrollv)<<5);
    if(i<j) rc->top=i,rc->bottom=j; else rc->top=j,rc->bottom=i;
}

// =============================================================================

void
Overselchg(OVEREDIT const * const ed,
           HWND             const win)
{
    text_buf_ty text_buf;
    
    RECT rc;
    if(ed->selobj==-1) return;
    rc.left=ed->objx-(ed->mapscrollh<<5);
    rc.top=ed->objy-(ed->mapscrollv<<5);
    
    GetOverString(ed, ed->tool, ed->selobj, text_buf);

    Getstringobjsize(text_buf, &rc);
    
    InvalidateRect(win, &rc, 0);
}

// =============================================================================

void
Overtoolchg(OVEREDIT * const ed,
            int        const i,
            HWND       const win)
{
    HWND hc=GetDlgItem(win,3001);
    
    if(tool_ovt[i]!=tool_ovt[ed->tool])
        InvalidateRect(hc,0,0);
    else
        Overselchg(ed,hc);
    
    ed->tool=i;
    ed->selobj=-1;
    
    Overselchg(ed,hc);
}

// =============================================================================

void
GetOverString(OVEREDIT const * const ed,
              int              const t,
              int              const n,
              text_buf_ty            p_text_buf)
{
    int a;
    
    switch(t)
    {
    
    case 3:
        wsprintf(p_text_buf,
                 "%02X",
                 ed->ew.doc->rom[0xdbb73 + n]);
        break;
    
    case 5:
        
        a=ed->ebuf[ed->sprset][n+2];
        
        wsprintf(p_text_buf,
                 "%02X-%s",
                 a,
                 sprname[a]);
        
        break;
    
    case 7:
        wsprintf(p_text_buf,
                 "%04X",
                 ((unsigned short*)(ed->ew.doc->rom + 0x15d8a))[n]);
        break;
    
    case 8:
        
        wsprintf(p_text_buf,
                 "%02X",
                 ed->ew.doc->rom[0xdb84c + n]);
        
        break;
    
    case 9:
        
        if(n > 8)
        {
            wsprintf(p_text_buf,
                     "Whirl-%02X",
                     ((unsigned short*)(ed->ew.doc->rom + 0x16cf8))[n-9]);
        }
        else
        {
            wsprintf(p_text_buf,
                     "Fly-%d",
                     n + 1);
        }
        
        break;
    
    case 10:
        
        wsprintf(p_text_buf,
                 "%02X-",
                 ed->sbuf[n+2]);
        
        strcat(p_text_buf,
               Getsecretstring(ed->ew.doc->rom,ed->sbuf[n+2]));
        
        break;
    
    default:
        
        wsprintf(p_text_buf,"Invalid object!");
    }
}

// =============================================================================

void
Overselectwrite(OVEREDIT * const ed)
{
    int i,j,k,l,m,n,o,p,q;
    m=ed->mapsize?32:16;
    if(ed->stselx!=ed->rectleft || ed->stsely!=ed->recttop) {
        ed->undomodf=ed->ov->modf;
        memcpy(ed->undobuf,ed->ov->buf,0x800);
        if(ed->rectleft<0) i=-ed->rectleft; else i=0;
        if(ed->recttop<0) j=-ed->recttop; else j=0;
        if(ed->rectright>m) k=m; else k=ed->rectright;
        k-=ed->rectleft;
        if(ed->rectbot>m) l=m; else l=ed->rectbot;
        l-=ed->recttop;
        q=ed->rectright-ed->rectleft;
        
        p=j*q;
        
        o = ( (ed->recttop + j) << 5) + ed->rectleft;
        
        for(;j<l;j++)
        {
            for(n=i;n<k;n++)
            {
                ed->ov->buf[n+o]=ed->selbuf[n+p];
            }
            
            o += 32;
            p += q;
        }
        
        ed->ov->modf=1;
    }
    
    ed->selflag=0;
    
    free(ed->selbuf);
}

// =============================================================================

void
Savemap(OVEREDIT * const ed)
{
    unsigned char*rom,*b2,*b3,*b5;
    int i,j,k,l,m,n,p;
    short o[4];
    ZOVER*ov;
    FDOC*doc;
    
    unsigned short * b6;
    
    uint16_t * b4 = 0;
    
    static int sprofs[]={0,0x40,0xd0};
    ov=ed->ov;
    if(!ov->modf) return;
    m=ed->ew.param;
    doc=ed->ew.doc;
    rom=doc->rom;
    if(m<0x90) {
        if(ed->ecopy[ed->sprset]!=-1) {
            n=ed->ecopy[ed->sprset];
            ed->esize[n]=ed->esize[ed->sprset];
            ed->e_modf[n]=ed->e_modf[ed->sprset];
            ed->ebuf[n]=ed->ebuf[ed->sprset];
        }
        for(k=0;k<3;k++) o[k]=*(short*)(rom+sprset_loc[k]+m*2);
        for(k=1;k<3;k++)
            if(ed->e_modf[k]) {
                for(l=m>>7;l<k;l++) if(o[l]==o[k]) {
                    if(ed->ecopy[k]==-1) *(unsigned short*)(rom+sprset_loc[k]+m*2)=0xcb41;
                    else *(unsigned short*)(rom+sprset_loc[k]+m*2)=o[ed->ecopy[k]],ed->e_modf[k]=0;
                    break;
                }
            }
        for(k=1;k<3;k++) {
            if(ed->ecopy[k]!=-1) {
                Savesprites(doc,sprofs[k]+m + 0x10000,0,0);
                *(unsigned short*)(rom+sprset_loc[k]+m*2)=*(unsigned short*)(rom+sprset_loc[ed->ecopy[k]]+m*2);
            }
        }
        for(k=0;k<3;k++)
            if(ed->e_modf[k] && ed->ecopy[k]==-1) {
                Savesprites(doc,sprofs[k]+m,ed->ebuf[k],ed->esize[k]);
            }
        if(m<0x80) Savesecrets(doc,m,ed->sbuf,ed->ssize);
    }
    if(m<0x40) {
        rom[0x7a41 + ed->ew.param]=ed->sprgfx[0];
        rom[0x7a81 + ed->ew.param]=ed->sprgfx[1];
        rom[0x7b41 + ed->ew.param]=ed->sprpal[0];
        rom[0x7b81 + ed->ew.param]=ed->sprpal[1];
    }
    rom[0x7ac1 + ed->ew.param]=ed->sprgfx[2];
    rom[0x7bc1 + ed->ew.param]=ed->sprpal[2];
    b2=malloc(0x200);
    b3=b2 + 0x100;
    if(ed->mapsize) n=4; else n=1;
    
    for(k=0;k<n;k++)
    {
        if(m + map_ind[k] > 0x9f)
            continue;
        
        b4 = ov->buf + map_ofs[k];
        
        for(j=0;j<16;j++) {
            for(i=0;i<16;i++) {
                l=*(b4++);
                *(b3++)=l;
                *(b2++)=l>>8;
            }
            b4+=16;
        }
        b2-=0x100;
        b3-=0x100;
        b5=Compress(b2,0x100,&i,1);
        issplit=0;
        memcpy(rom+Changesize(doc,m+map_ind[k] + 0x30000,i),b5,i);
        free(b5);
        b5=Compress(b3,0x100,&i,1);
        memcpy(rom+Changesize(doc,m+map_ind[k]+(issplit?0x100a0:160),i),b5,i);
        free(b5);
    }
    free(b2);
    if(ed->ovlmodf) {
        l=0;
        n=256;
        b6=malloc(512);
        b4=doc->ovlbuf;
        j=0;
        for(;;) {
            k=0xffff;
            for(i=0;i<0x1400;i++) {
                p=ed->ovlmap[i];
                if(p>=j && p<k) k=p;
            }
            j=k+1;
            if(k==0xffff) break;
            if(l) b6[l++]=0xffff;
            b6[l++]=k;
            for(i=0;i<0x1400;i++)
                if(ed->ovlmap[i]==k) b6[l++]=i;
            if(l>=192) n+=256,b6=realloc(b6,n<<1);
        }
        b6[l++]=0xfffe;
        if(!(doc->o_enb[m>>3]&(1<<(m&7)))) {
            k=b4[m];
            doc->o_enb[m>>3]|=(1<<(m&7));
            goto foundnextovl;
        }
        for(i=0;i<128;i++) {
            if(i!=m && b4[i]==b4[m])
            {
                text_buf_ty text_buf;
                
                wsprintf(text_buf,
                         "The overlay in area %02X is used several times. Modify only in this area?",
                         m);
                
                if(MessageBox(0,
                              text_buf,
                              "Gigasoft Hyrule Magic",
                              MB_YESNO) == IDYES)
                {
                    k = b4[m];
                    
                    goto foundnextovl;
                }
                else
                {
                    goto modallovl;
                }
            }
        }
modallovl:
        k=b4[128];
        for(i=0;i<128;i++) {
            if(i==m) continue;
            j=b4[i];
            if(j<k && j>b4[m]) k=j;
        }
foundnextovl:
        j=b4[m];
        
        if(l+j==k) goto nochgovls;
        
        for(i=0;i<129;i++)
            if(b4[i]>=k) b4[i]+=l+j-k;
        
        b4[m]=j;
        
        if(l+j<k)
            memmove(b4 + l + j,
                    b4 + k,
                    (b4[128] - l - j) << 1);
        
        doc->ovlbuf = (uint16_t*) b4 = (uint16_t*) realloc(b4, b4[128] << 1);
        
        if(l + j > k)
            memmove(b4 + l + j,
                    b4 + k,
                    (b4[128] - l - j) << 1);
        
    nochgovls:
        
        memcpy(b4+j,b6,l<<1);
        free(b6);
        doc->o_modf=1;
    }
    ed->ovlmodf=0;
    ed->ov->modf=0;
    ed->e_modf[0]=ed->e_modf[1]=ed->e_modf[2];
    doc->modf=1;
}

// =============================================================================

extern int
getbgmap(OVEREDIT * const ed,
         int        const a,
         int        const b)
{
    int c,d;
    
    switch(a)
    {
    
    case 0:
        d=0;
        if(b==2) c=158; else c=151;
        break;
    
    case 64:
        d=0;
        c=151;
        break;
    
    case 128:
        d=2;
        if(b==2) c=160; else c=151;
        break;
    
    case 3: case 5: case 7:
        d=1;
        c=149;
        break;
    
    case 67: case 69: case 71:
        d=1;
        c=156;
        break;
    
    case 136: case 147:
        d=2;
        c=159;
        break;
    
    case 91:
        if(b)
        {
            d=1;
            c=150;
            break;
        }
    
    default:
        if(b)
            d=2;
        else
    
    case 112:
        d=0;
        c=159;
    }
    
    if(b || a >= 128)
    {
        ed->layering = d;
    }
    else
    {
        ed->layering = 0;
    }
    
    return c;
}

// =============================================================================

extern int
Savesecrets(FDOC          * const doc,
            int             const num,
            uint8_t const * const buf,
            int             const size)
{
    int i,j,k;
    int adr[128];
    unsigned char*rom=doc->rom;
    for(i=0;i<128;i++)
        adr[i]=0xe0000 + ((short*)(rom + 0xdc2f9))[i];
    k=doc->sctend;
    if(rom[0x125ec + (num&63)]!=(num&63)) {
        j=(num&64)?doc->sctend:adr[64];
        for(i=num&63;i<64;i++) {
            if(rom[0x125ec + i]!=i) continue;
            j=adr[i+(num&64)];
            if(*(short*)(rom+j)!=-1) j-=2;
            break;
        }
        ((short*)(rom + 0xdc2f9))[num]=adr[num]=j;
    } else j=adr[num];
    for(i=num+1;i<128;i++) {
        if(rom[0x125ec + (i&63)]!=(i&63)) continue;
        k=adr[i];
        break;
    }
    
    if( is16b_neg1(rom + j) )
    {
        if(!size) return 0;
        if(k>j) k-=2;
        j+=size+2;
        adr[num]+=2;
    } else {
        if(!size) {
            if(j>0xdc3f9) {
                j-=2;
                adr[num]-=2;
            }
        } else j+=size;
        if(*(short*)(rom+k)!=-1) k-=2;
    }
    if(doc->sctend-k+j>0xdc894) {
        MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
        return 1;
    }
    memmove(rom+j,rom+k,doc->sctend-k);
    if(size) memcpy(rom+adr[num],buf,size);
    if(j==k) return 0;
    ((short*)(rom + 0xdc2f9))[num]=adr[num];
    doc->sctend+=j-k;
    for(i=num+1;i<128;i++) {
        ((short*)(rom + 0xdc2f9))[i]=adr[i]+j-k;
    }
    return 0;
}

// =============================================================================

extern void
SaveOverlays(FDOC * const doc)
{
    int i, // counter variable
        j = 0,
        k, // another counter variable
        l,
        m,
        n,
        o,
        p,
        q = 0,
        r = 0;
    
    unsigned short *b = doc->ovlbuf;
    unsigned char *rom = doc->rom;
    
    char buf[4096];
    
    // if the overlays have not been loaded, exit the routine. 
    if(doc->o_loaded != 1)
        return;
    
    m = 0;
    o = 0;
    
    for(i = 0; i < 128; i++)
    {
        for(k = 0; k < i; k++)
        {
            // if b[k] is the same as the last one in the list (b[i]), copy it in the rom
            if(b[k] == b[i])
            {
                ( (short*) ( rom + 0x77664 ) )[i] = ((short*) ( rom + 0x77664 ))[k];
                
                // skip to the next overlay. i.e. increment i.
                goto nextovl;
            }
        }
        
        // if none of them match, take i and make k into the value of b[i].
        k = b[i];
        
        ( (short*) ( rom + 0x77664 ) )[i] = j + 0xf764;
        
        if( !( doc->o_enb[i >> 3] & (1 << (i & 7) ) ))
            // beats me. seems like loop around if certain overlays are not enabled.
            continue;
        
        // convert m to the next lowest even integer. 
        if(b[k] == 0xfffe && m)
        {
            ((short*) ( rom + 0x77664 ))[i] = m + 0xf864;
            
            continue;
        }
        
        p = 0;
        
        k = b[i];
        
        n = 0xfffe;
        
        for(;;)
        {
            l = b[k++];
            
            if(l >= 0xfffe)
                break;
            
            if(l == 0x918)
                q = j;
            
            if(l - n == 1)
            {
                if(n == p + 0x918)
                    p++;
                else
                    p = 0;
                
                buf[j++] = 0x1a;
            }
            else
            {
                p = 0;
                
                if(l - n == 2)
                {
                    *(short*) (buf + j) = 0x1a1a;
                    j += 2;
                }
                else
                {
                    buf[j] = 0xa9;
                    
                    *(short*) (buf + j + 1) = l;
                    
                    j += 3;
                }
            }
            
            n = l;
            
            for(;;)
            {
                if(j >= 0x89c)
                {
error:
                    MessageBox(framewnd,"Not enough room for overlays","Bad error happened",MB_OK);
                    return;
                }
                
                l = b[k++];
                
                if(l >= 0xfffe)
                    break;
                
                if(!p)
                    r = l;
                else if( map16ofs[p] != l - r)
                    p = 0;
                
                buf[j] = 0x8d;
                
                *(short*) (buf + j + 1) = (l << 1) + 0x2000;
                j += 3;
            }
            
            if(l == 0xfffe)
                break;
        }
        
        if(p == 3)
        {
            j = q;
            buf[j] = 0xa2;
            
            *(short*) (buf + j + 1) = r << 1;
            
            if(o)
            {
                buf[j + 3] = 0x4c;
                
                *(short*) (buf + j + 4) = o + 0xf764;
                j += 6;
            }
            else
            {
                j += 3;
                o = j;
                
                *(int*) (buf + j) = 0x9d0918a9;
                *(int*) (buf + j + 4) = 0x9d1a2000;
                *(int*) (buf + j + 8) = 0x9d1a2002;
                *(int*) (buf + j + 12) = 0x9d1a2080;
                *(int*) (buf + j + 16) = 0x602082;
                
                j += 19;
            }
        }
        else
            buf[j++] = 0x60;
        
        if(!m)
            m = j;
        
        if(j >= 0x89c)
            goto error;
        
nextovl:;
    }
    
    memcpy(rom + 0x77764, buf, j);
    
    doc->oolend = j + 0x77764;
    doc->o_modf = 0;
}

// =============================================================================

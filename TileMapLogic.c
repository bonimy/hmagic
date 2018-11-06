
    #include "structs.h"

    #include "prototypes.h"

// =============================================================================

    int tmap_ofs[] = 
    {
        0x64ed0,
        0x64e5f,
        0x654c0,
        0x65148,
        0x65292
    };

// =============================================================================

extern void
Updtmap(TMAPEDIT * const ed)
{
    int i,j,k,l;
    unsigned char nb[0x6000];
    unsigned char*b=ed->buf;
    for(i=0;i<0x2000;i++) ((short*)nb)[i]=ed->back1;
    for(;i<0x3000;i++) ((short*)nb)[i]=ed->back2;
    for(;;) {
        j=(*b<<8)+b[1];
        if(j>=32768) break;
        if(j>=0x6000) j-=0x4000;
        j+=j;
        k=((b[2]&63)<<8)+b[3]+1;
        l=b[2];
        b+=4;
        if(l&64) k++;
        k>>=1;
        if(l&128) {
            if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=64; b+=2; }
            else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=64;
        } else {
            if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=2; b+=2; }
            else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=2;
        }
    }
    for(i=0;i<0x6000;i+=64) {
        b=(unsigned char*)(ed->nbuf+((i&0x7000)>>1)+(i&0x7c0)+((i&0x800)>>6));
        for(j=0;j<64;j+=2) *(short*)(b+j)=*(short*)(nb+i+j);
    }
}

// =============================================================================

extern void
Savetmap(TMAPEDIT * const ed)
{
    int i,j,k = 0,l,m;
    unsigned char*rom;
    j=ed->datnum;
    l=ed->len;
    if(j<7) {
        i=Changesize(ed->ew.doc,ed->datnum+669,ed->len);
        if(!i) return;
    } else {
        rom=ed->ew.doc->rom;
        i=*(short*)(rom+tmap_ofs[j-7]) + 0x68000;
        if(j==7 || j==9) i++;
        switch(j) {
        case 7:
            k=*(unsigned short*)(rom + 0x64ecd);
            break;
        case 8:
            k=*(unsigned short*)(rom + 0x64e5c)+2;
            l--;
            break;
        case 9:
            k=*(unsigned short*)(rom + 0x654bd);
            break;
        case 10:
            k=*(unsigned short*)(rom + 0x65142)+1;
            break;
        case 11:
            k=*(unsigned short*)(rom + 0x6528d)+1;
            break;
        }
        if(l>k) {
            if(!Changesize(ed->ew.doc,4096,ed->ew.doc->tmend3+l-k)) return;
            rom=ed->ew.doc->rom;
            memmove(rom+i+l,rom+i+k,ed->ew.doc->tmend3-i-l);
        } else {
            rom=ed->ew.doc->rom;
            memmove(rom+i+l,rom+i+k,ed->ew.doc->tmend3-i-k);
            Changesize(ed->ew.doc,4096,ed->ew.doc->tmend3+l-k);
        }
        for(m=0;m<5;m++)
            if(*(short*)(rom+tmap_ofs[m]) + 0x68000>i) *(short*)(rom+tmap_ofs[m])+=l-k;
        switch(j) {
        case 7:
            *(unsigned short*)(rom + 0x64ecd)=l;
            break;
        case 8:
            *(unsigned short*)(rom + 0x64e5c)=l-2;
            break;
        case 9:
            *(unsigned short*)(rom + 0x654bd)=l;
            break;
        case 10:
            *(unsigned short*)(rom + 0x65142)=l-1;
            break;
        case 11:
            *(unsigned short*)(rom + 0x6528d)=l-1;
            break;
        }
    }
    memcpy(ed->ew.doc->rom+i,ed->buf,l);
    ed->ew.doc->modf=1;
}

// =============================================================================

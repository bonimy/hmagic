
    #include "structs.h"

    #include "prototypes.h"

    #include "GdiObjects.h"

// =============================================================================

extern BOOL CALLBACK
editvarious(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    static unsigned char *rom;
    
    // \task While it is true that this is a modal dialog, this still has
    // a bit of code smell to it. Consider refactoring this into something
    // managed by the caller and e.g. passed in via lparam.
    static EDITCOMMON ec;
    
    static int gfxnum,sprgfx,palnum;
    static int init = 0;
    static BLOCKSEL8 bs;
    
    unsigned char*buf2;
    
    static int *const set_ofs[4]={&ec.gfxtmp,&gfxnum,&sprgfx,&palnum};
    
    const static unsigned set_max[4]={37,82,144,72};
    const static int ctl_ids[4]={3004,3012,3016,3020};
    const static int stuff_ofs[4]={0x54b7,0x51d3,0x4f8f,0x74894};
    unsigned i;
    int j,k,l;
    static HDC hdc;
    static HWND hc;
    switch(msg) {
    case WM_INITDIALOG:
        
        memset(&ec,0,sizeof(ec));
        
        rom=(ec.ew.doc=(FDOC*)lparam)->rom;
        
        ec.bmih=zbmih;
        ec.gfxtmp=0;
        ec.anim=3;
        gfxnum=0;
        
        init=1;
        
        SetDlgItemInt(win,3000,0,0);
        SetDlgItemInt(win,3001,0,0);
        SetDlgItemInt(win,3002,0,0);
        SetDlgItemInt(win,3003,0,0);
        
        init=0;
        
        Addgraphwin((void*)&ec,1);
        
        bs.ed = (OVEREDIT*) &ec;
        
        hdc = GetDC(win);
        
        InitBlksel8(hc = GetDlgItem(win, IDC_CUSTOM3),
                    &bs,ec.hpal,
                    hdc);
        
        ReleaseDC(win,hdc);
        
        Setdispwin( (DUNGEDIT*) &ec);
        
loadnewgfx:
        
        for(i = 0; i < 8; i++)
        {
            ec.blocksets[i] = rom[0x6073 + i + (ec.gfxtmp << 3)];
        }
        
        for(i = 0; i < 4; i++)
        {
            j = rom[0x5d97 + i + (gfxnum << 2)];
            
            if(j != 0)
                ec.blocksets[i + 3] = j;
        }
        
        for(i = 0; i < 8; i++)
            Getblocks(ec.ew.doc, ec.blocksets[i]);
updscrn:
        
        if(init < 2)
        {
            for(i = 0; i < 256; i++)
                Updateblk8sel(&bs, i);
            
            InvalidateRect(hc, 0, 0);
        }
        
        break;
    
    case WM_DESTROY:
        
        DeleteDC(bs.bufdc);
        DeleteObject(bs.bufbmp);
        Delgraphwin( (DUNGEDIT*) &ec);

        for(i = 0; i < 8; i++)
        {
            Releaseblks(ec.ew.doc,
                        ec.blocksets[i] );
        }
        
        break;
    
    case 4000:
        
        i = wparam & 0x3ff;
        
        Changeblk8sel(hc,&bs);
        
        bs.sel = i;
        
        Changeblk8sel(hc,&bs);
        
        break;
    
    case WM_COMMAND:
        
        if(wparam == IDCANCEL)
        {
            EndDialog(win, 0);
            
            break;
        }
        
        if( (wparam >> 16) != EN_CHANGE )
            break;
        
        wparam &= 65535;
        
        i = GetDlgItemInt(win, wparam, 0, 0);
        
        if(wparam == IDC_EDIT1)
        {
            bs.flags=(i&7)<<10;
            goto updscrn;
        }
        if(wparam<3004) {
            if(i>=set_max[wparam-3000]) {SetDlgItemInt(win,wparam,set_max[wparam-3000]-1,0);break;}
            *set_ofs[wparam-3000]=i;
            j=3;
            if(wparam==3000) j=7,i<<=1;
            k=ctl_ids[wparam-3000]+j;
            l=k+(i<<2);
            init++;
            for(;j>=0;j--) {
                SetDlgItemInt(win,k,rom[stuff_ofs[wparam-3000]+l],0);
                k--;
                l--;
            }
            init--;
            break;
        }
        if(wparam<3020) if(i>219) {SetDlgItemInt(win,wparam,219,0);break;}
        if(wparam<3012) {
            if(init>1) break;
            if(!init) rom[0x54b7 + (ec.gfxtmp<<3)+wparam]=i,ec.ew.doc->modf=1;
            
        freegfx:
            
            for(i = 0; i < 8; i++)
            {
                Releaseblks(ec.ew.doc,
                            ec.blocksets[i]);
            }
            
            goto loadnewgfx;
        }
        
        if(wparam < 3016)
        {
            if(init>1) break;
            if(!init) rom[0x51d3 + (gfxnum<<2)+wparam]=i,ec.ew.doc->modf=1;
            goto freegfx;
        }
        
        if(wparam < 3020)
        {
            if( ! init )
                rom[0x4f8f + (sprgfx<<2)+wparam]=i,ec.ew.doc->modf=1;
            
            if(init < 2)
                Releaseblks(ec.ew.doc,ec.blocksets[wparam-3005]);
            
            i += 115;
            
            ec.blocksets[wparam - 3005] = i;
            Getblocks(ec.ew.doc, i);
            
            if(init > 1 && wparam != 3016)
                break;
            
            goto updscrn;
        }
        
        if(wparam < 3024)
        {
            if(init > 1 && wparam != 3020)
                break;
            
            if( ! init )
            {
                rom[0x74894 + (palnum << 2) + wparam] = i;

                ec.ew.doc->modf = 1;
            }
            
            buf2 = rom + (palnum << 2) + 0x75460;
            
            if(palnum < 41)
            {
                i = 0x1bd734 + buf2[0] * 90;
                
                Loadpal(&ec, rom, i, 0x21, 15, 6);
                Loadpal(&ec, rom, i, 0x89, 7, 1);
                Loadpal(&ec, rom, 0x1bd39e + buf2[1] * 14, 0x81, 7, 1);
                Loadpal(&ec, rom, 0x1bd4e0 + buf2[2] * 14, 0xd1, 7, 1);
                Loadpal(&ec, rom, 0x1bd4e0 + buf2[3] * 14, 0xe1, 7, 1);
            }
            else
            {
                if(buf2[0] < 128)
                    Loadpal(&ec,rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[buf2[0]]),0x29,7,3);
                
                if(buf2[1] < 128)
                    Loadpal(&ec,rom, 0x1be86c + (((unsigned short*)(rom + 0xdec13))[buf2[1]]),0x59,7,3);
                
                if(buf2[2] < 128)
                    Loadpal(&ec, rom, 0x1be604 + (((unsigned char*)(rom + 0xdebc6))[buf2[2]]),0x71,7,1);
            }
            
            if( ! ec.hpal )
                goto updscrn;
        }
    }
    return 0;
}

// =============================================================================
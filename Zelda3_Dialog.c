
    #include "structs.h"
    #include "prototypes.h"

    #include "Callbacks.h"
    #include "GdiObjects.h"

    #include "OverworldEdit.h"

    #include "DungeonEnum.h"
    #include "DungeonLogic.h"

    // For the names of the various editable misc screens.
    #include "ScreenEditorLogic.h"

    #include "LevelMapLogic.h"


// =============================================================================

SD_ENTRY z3_sd[]={
    {WC_TREEVIEW,"",0,0,0,0,3000,WS_VISIBLE|WS_TABSTOP|WS_BORDER|WS_CHILD|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_HASLINES|TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP,WS_EX_CLIENTEDGE,10},
};

SUPERDLG z3_dlg={
    "",z3dlgproc,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,60,60,1,z3_sd
};

// =============================================================================

BOOL CALLBACK
z3dlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    FDOC*doc;
    HWND hc;
    int i,j,k,l;
    TVINSERTSTRUCT tvi;
    TVHITTESTINFO hti;
    TVITEM*itemstr;
    HTREEITEM hitem;
    RECT rc;
    ZOVER*ov;
    OVEREDIT*oed;
    unsigned char*rom;
    int lp;
    static char *pal_text[]={
        "Sword",
        "Shield",
        "Clothes",
        "World colors 1",
        "World colors 2",
        "Area colors 1",
        "Area colors 2",
        "Enemies 1",
        "Dungeons",
        "Miscellanous colors",
        "World map",
        "Enemies 2",
        "Other sprites",
        "Dungeon map",
        "Triforce",
        "Crystal"
    };
    static char*locs_text[]={
        "Pendant 1",
        "Pendant 2",
        "Pendant 3",
        "Agahnim 1",
        "Crystal 2",
        "Crystal 1",
        "Crystal 3",
        "Crystal 6",
        "Crystal 5",
        "Crystal 7",
        "Crystal 4",
        "Agahnim 2"
    };
    static char pal_num[]={
        4,3,3,6,2,20,16,24,20,2,2,16,18,1,1,1
    };
    char buf[0x200];
    switch(msg) {
    case WM_INITDIALOG:
        SetWindowLong(win,DWL_USER,lparam);
        doc=(FDOC*)lparam;
        hc=GetDlgItem(win,3000);
        tvi.hParent=0;
        tvi.hInsertAfter=TVI_LAST;
        tvi.item.mask=TVIF_CHILDREN|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
        tvi.item.stateMask=TVIS_BOLD;
        tvi.item.state=0;
        tvi.item.lParam=0;
        tvi.item.pszText="Overworld";
        tvi.item.cChildren=1;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<160;i++)
        {
            tvi.item.lParam=i + 0x20000;
            wsprintf(buf,"Area %02X - %s", i, area_names.m_lines[i]);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Dungeons";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<133;i++)
        {
            tvi.item.lParam=i + 0x30000;
            wsprintf(buf,
                     "Entrance %02X - %s",
                     i,
                     entrance_names.m_lines[i]);
            
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<7;i++)
        {
            tvi.item.lParam=i + 0x30085;
            wsprintf(buf,"Starting location %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<19;i++)
        {
            tvi.item.lParam=i + 0x3008c;
            wsprintf(buf,"Overlay %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        for(i=0;i<8;i++)
        {
            tvi.item.lParam=i + 0x3009f;
            wsprintf(buf,"Layout %02X",i);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.lParam=0x300a7;
        tvi.item.pszText="Watergate overlay";
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Music";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText=buf;
        tvi.item.cChildren=0;
        for(i=0;i<3;i++)
        {
            tvi.item.lParam=i + 0x40000;
            wsprintf(buf,"Bank %d",i+1);
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.lParam=0x40003;
        tvi.item.pszText="Waves";
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="World maps";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Normal world";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x60000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Dark world";
        tvi.item.lParam=0x60001;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Monologue";
        tvi.item.cChildren=0;
        tvi.item.lParam=0x50000;
        tvi.hParent=0;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Palettes";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        hitem=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        k=0;
        for(i=0;i<16;i++) {
            l=pal_num[i];
            tvi.item.pszText=pal_text[i];
            tvi.item.cChildren=1;
            tvi.item.lParam=0;
            tvi.hParent=hitem;
            tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
            tvi.item.pszText=buf;
            tvi.item.cChildren=0;
            for(j=0;j<l;j++) {
                tvi.item.lParam=k + 0x70000;
                wsprintf(buf,"%s pal %d",pal_text[i],j);
                SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
                k++;
            }
        }
        tvi.item.pszText="Dungeon maps";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<14;i++)
        {
            tvi.item.lParam=i + 0x80000;
            tvi.item.pszText = (char*) level_str[i+1];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Dungeon properties";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<12;i++)
        {
            tvi.item.lParam=i + 0x90000;
            tvi.item.pszText=locs_text[i];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="Menu screens";
        tvi.item.cChildren=1;
        tvi.item.lParam=0;
        tvi.hParent=0;
        tvi.hParent=(HTREEITEM)SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.cChildren=0;
        for(i=0;i<11;i++)
        {
            tvi.item.lParam=i + 0xa0000;
            tvi.item.pszText = (char*) screen_text[i];
            SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        }
        tvi.item.pszText="3D Objects";
        tvi.item.cChildren=0;
        tvi.item.lParam=0xb0000;
        tvi.hParent=0;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Link's graphics";
        tvi.item.lParam=0xc0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="ASM hacks";
        tvi.item.lParam=0xd0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        tvi.item.pszText="Graphic schemes";
        tvi.item.lParam=0xe0000;
        SendMessage(hc,TVM_INSERTITEM,0,(long)&tvi);
        break;
    case WM_NOTIFY:
        switch(wparam)
        {
        
        case 3000:
            switch(((NMHDR*)lparam)->code)
            {
            
            case NM_DBLCLK:
                GetWindowRect(((NMHDR*)lparam)->hwndFrom,&rc);
                
                hti.pt.x = mouse_x-rc.left;
                hti.pt.y = mouse_y-rc.top;
                hitem = (HTREEITEM) SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_HITTEST,0,(long)&hti);
                
                if(!hitem)
                    break;
                
                if(!(hti.flags & TVHT_ONITEM))
                    break;
                
                itemstr = &(tvi.item);
                itemstr->hItem = hitem;
                itemstr->mask = TVIF_PARAM;
                
                SendMessage(((NMHDR*)lparam)->hwndFrom,TVM_GETITEM,0,(long)itemstr);
                
                lp = itemstr->lParam;
open_edt:
                
                doc = (FDOC*)GetWindowLong(win,DWL_USER);
                
                j = lp & 0xffff;
                
                switch(lp >> 16)
                {

                    // double clicked on an overworld area
                case 2:
                    ov = doc->overworld;
                    
                    if(j < 128)
                        j = doc->rom[0x125ec + (j & 0x3f)] | (j & 0x40);
                    
                    for(i=0;i<4;i++)
                    {
                        k = map_ind[i];
                        
                        if(j >= k && ov[j - k].win)
                        {
                            hc = ov[j - k].win;
                            oed = (OVEREDIT*)GetWindowLong(hc,GWL_USERDATA);
                            
                            if(i && !(oed->mapsize))
                                continue;
                            
                            SendMessage(clientwnd,WM_MDIACTIVATE,(int)hc,0);
                            
                            hc = GetDlgItem(oed->dlg,3001);
                            SendMessage(hc,WM_HSCROLL,SB_THUMBPOSITION|((i&1)<<20),0);
                            SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((i&2)<<19),0);
                            
                            return FALSE;
                        }
                    }
                    
                    wsprintf(buf,
                             "Area %02X - %s",
                             j,
                             area_names.m_lines[j]);
                    
                    ov[j].win = Editwin(doc,"ZEOVER",buf,j,sizeof(OVEREDIT));
                    
                    break;
                
                case 3:
                    
                    // double clicked on a dungeon item
                    if(doc->ents[j])
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (int) (doc->ents[j]),
                                    0);
                        
                        break;
                    }
                    
                    if(j < 0x8c)
                    {
                        k = ((short*) (doc->rom + (j >= 0x85 ? 0x15a64 : 0x14813)))[j];
                        
                        if(doc->dungs[k])
                        {
                            MessageBox(framewnd,
                                       "The room is already open in another editor",
                                       "Bad error happened",
                                       MB_OK);
                            
                            break;
                        }
                        
                        if(j >= 0x85)
                            wsprintf(buf,"Start location %02X",j - 0x85);
                        else
                        {
                            wsprintf(buf,
                                     "Entrance %02X - %s",
                                     j,
                                     entrance_names.m_lines[j]);
                        }
                    }
                    else if(j < 0x9f)
                        wsprintf(buf,"Overlay %d",j - 0x8c);
                    else if(j < 0xa7)
                        wsprintf(buf,"Layout %d",j - 0x9f);
                    else
                        wsprintf(buf,"Watergate overlay");
                    
                    hc = Editwin(doc,"ZEDUNGEON",buf,j, sizeof(DUNGEDIT));
                    
                    if(hc)
                    {
                        DUNGEDIT * ed = (DUNGEDIT*) GetWindowLong(hc, GWL_USERDATA);
                        HWND map_win = GetDlgItem(ed->dlg, ID_DungEditWindow);
                    
                        Dungselectchg(ed, map_win, 1);
                    }
                    
                    break;
                case 4:
                    if(doc->mbanks[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->mbanks[j]),0);break;}
                    if(j==3) doc->mbanks[3]=Editwin(doc,"MUSBANK","Wave editor",3,sizeof(SAMPEDIT)); else {
                        wsprintf(buf,"Song bank %d",j+1);
                        doc->mbanks[j]=Editwin(doc,"MUSBANK",buf,j,sizeof(MUSEDIT));
                    }
                    break;
                case 6:
                    if(doc->wmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->wmaps[j]),0);break;}
                    wsprintf(buf,"World map %d",j+1);
                    doc->wmaps[j]=Editwin(doc,"WORLDMAP",buf,j,sizeof(WMAPEDIT));
                    break;
                case 5:
                    
                    if(doc->t_wnd)
                    {
                        SendMessage(clientwnd, WM_MDIACTIVATE, (int)(doc->t_wnd), 0);
                        
                        break;
                    }
                    
                    doc->t_wnd = Editwin(doc,"ZTXTEDIT","Text editor",0,sizeof(TEXTEDIT));
                    
                    break;
                
                case 7:
                    if(doc->pals[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->pals[j]),0);break;}
                    
                    k = 0;
                    
                    for(i=0;i<16;i++)
                    {
                        if(k + pal_num[i] > j)
                            break;
                        
                        k += pal_num[i];
                    }
                    
                    wsprintf(buf,"%s palette %d",pal_text[i],j-k);
                    
                    doc->pals[j] = Editwin(doc,
                                           "PALEDIT",
                                           buf,
                                           j | (i << 10) | ( (j - k) << 16),
                                           sizeof(PALEDIT));
                    
                    break;
                
                case 8:
                    
                    if(doc->dmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->dmaps[j]),0);break;}
                    
                    doc->dmaps[j]=Editwin(doc,"LEVELMAP",level_str[j+1],j,sizeof(LMAPEDIT));
                    
                    break;
                
                case 9:
                    
                    activedoc=doc;
                    
                    ShowDialog(hinstance,MAKEINTRESOURCE(IDD_DIALOG17),framewnd,editbosslocs,j);
                    
                    break;
                
                case 10:
                    
                    if(doc->tmaps[j]) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->tmaps[j]),0);break;}
                    doc->tmaps[j]=Editwin(doc,"TILEMAP",screen_text[j],j,sizeof(TMAPEDIT));
                    
                    break;
                
                case 11:
                    if(doc->perspwnd) {SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->perspwnd),0);break;}
                    doc->perspwnd=Editwin(doc,"PERSPEDIT","3D object editor",0,sizeof(PERSPEDIT));
                    break;
                case 12:
                    
                    oed = (OVEREDIT*) malloc(sizeof(OVEREDIT));
                    
                    oed->bmih=zbmih;
                    oed->hpal=0;
                    oed->ew.doc=doc;
                    oed->gfxnum=0;
                    oed->paltype=3;
                    
                    if(palmode)
                        Setpalmode((DUNGEDIT*)oed);
                    
                    rom=doc->rom;
                    Getblocks(doc,225);
                    Loadpal(oed,rom,0x1bd308,0xf1,15,1);
                    Loadpal(oed,rom,0x1bd648,0xdc,4,1);
                    Loadpal(oed,rom,0x1bd630,0xd9,3,1);
                    Editblocks(oed,0xf0104,framewnd);
                    Releaseblks(doc,225);
                    
                    if(oed->hpal)
                        DeleteObject(oed->hpal);
                    
                    free(oed);
                    
                    break;
                
                case 13:
                    
                    if(doc->hackwnd)
                    {
                        SendMessage(clientwnd,WM_MDIACTIVATE,(int)(doc->hackwnd),0);
                        
                        break;
                    }
                    
                    doc->hackwnd = Editwin(doc, "PATCHLOAD", "Patch modules", 0, sizeof(PATCHLOAD));
                    
                    break;
                
                case 14:
                    
                    // Graphic Themes
                    ShowDialog(hinstance,
                               MAKEINTRESOURCE(IDD_GRAPHIC_THEMES),
                               framewnd,
                               editvarious,
                               (int) doc);
                    
                    break;
                }
            }
        }
        
        break;
    
    case 4000:
        
        lp = wparam;
        
        goto open_edt;
    }
    return FALSE;
}
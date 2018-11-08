
    #include "structs.h"
    #include "prototypes.h"

    #include "Wrappers.h"
    #include "Callbacks.h"

    #include "HMagicEnum.h"
    #include "HMagicLogic.h"
    #include "HMagicUtility.h"

    #include "OverworldEnum.h"
    #include "OverworldEdit.h"

    #include "DungeonEnum.h"
    #include "DungeonLogic.h"

    #include "PaletteEdit.h"

    #include "TextLogic.h"

    #include "AudioLogic.h"

    #include "WorldMapLogic.h"

    #include "TileMapLogic.h"

    #include "PatchLogic.h"

// =============================================================================

extern HWND debug_box;

extern void * firstgraph;

// =============================================================================

void
Updatepals(void)
{
    DUNGEDIT*ed = (DUNGEDIT*) firstgraph;
    
    while(ed)
    {
        SendMessage(ed->dlg,4002,0,0);
        ed = (DUNGEDIT*)(ed->nextgraph);
    }
}

// =============================================================================

// window procedure for the main frame window
LRESULT CALLBACK
frameproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    text_buf_ty text_buf = { 0 };
    
    CLIENTCREATESTRUCT ccs;
    
    OPENFILENAME ofn;
    
    HWND hc;
    
    FDOC *doc, *doc2;
    
    DUNGEDIT *ed;
    
    unsigned char *rom;
    char *m;
    
    HANDLE h,h2;
    MDICREATESTRUCT mdic;
    
    SAMPEDIT *sed;
    ASMHACK *mod;
    FILETIME tim;
    
    int i = 0,
        j = 0,
        k = 0,
        l = 0;
    
    DWORD read_size  = 0;
    DWORD write_size = 0;
    
    unsigned char *buf;
    
    switch(msg)
    {
    
#if 0
    case WM_NCCALCSIZE:
        
        if(wparam)
        {
            NCCALCSIZE_PARAMS * const params = (NCCALCSIZE_PARAMS*) lparam;
            
            LRESULT d_r = DefWindowProc(win, msg, wparam, lparam);
            
            params->rgrc[0].right  -= 32;
            params->rgrc[0].bottom -= 32;
            
            return WVR_VALIDRECTS;
        }
        else
        {
            RECT * const r = (RECT*) lparam;
            
            r->right  -= 32;
            r->bottom -= 32;
            
            return 0;
        }
        
        break;
    
    case WM_NCPAINT:
         
        if(always)
        {
            return 1;
        }
        if(wparam == 1)
        {
            RECT const whole_rect = HM_GetWindowRect(win);
            
            HRGN const region = CreateRectRgnIndirect(&whole_rect);
            
            LRESULT result = DefWindowProc(win, msg, (HRGN) region, lparam);
            
            DeleteObject(region);
            
            return result;
        }
        
        if( always)
        {
            RECT const whole_rect = HM_GetWindowRect(win);
            
            HRGN const region = (wparam == 1)
                              ? CreateRectRgnIndirect(&whole_rect)
                              : (HRGN) wparam;
            
            HDC const dc = GetDCEx(win,
                                   region,
                                   DCX_WINDOW  | DCX_INTERSECTRGN);
            
            DWORD region_size = GetRegionData(region, 1, NULL);
            
            RGNDATA * region_data = region_size
                                  ? (RGNDATA*) calloc(1, region_size)
                                  : NULL;
            
            if(region_data)
            {
                int i = 0;
                
                DWORD dummy = GetRegionData(region, region_size, region_data);

                RECT * const rect_arr = (RECT*) region_data->Buffer;
                
                DWORD const rect_count = region_data->rdh.nCount;
                
                HDC const win_dc = GetWindowDC(win);
                
                // -----------------------------
                
                (void) dummy;
                
#if 1
                DefWindowProc(win, msg, (HRGN) region, lparam);
#else
                for(i = 0; i < rect_count; i += 1)
                {
                    FillRect(win_dc, &rect_arr[i],
                             blue_brush);
                }
#endif
                
                free(region_data);
                
                ReleaseDC(win, win_dc);
                
                region_data = 0;
            }
            
            // Windows will deallocate it it was a specific set of rectangles
            // rather than the whole window as a region.
            if(wparam == 1)
            {
                DeleteObject(region);
            }
            
            ReleaseDC(win, dc);
        }
        
        return 0;
#endif

#if 1

    case WM_VSCROLL:

        if(always)
        {
            return DefFrameProc(win, clientwnd, msg, wparam, lparam);
        }
        
        break;

#endif

#if 1
    case WM_MOUSEWHEEL:

        // A hacky load of shit, but it works... seemingly.
        if(always)
        {
            HM_MouseWheelData CONST d = HM_GetMouseWheelData(wparam, lparam);
            
            unsigned const is_horiz = (d.m_control_key);
            
            int const  scroll_amount = d.m_distance > 0
                                     ? SB_LINEUP : SB_LINEDOWN;
            
            SCROLLINFO const si = (is_horiz)
                                ? HM_GetHorizScrollInfo(clientwnd)
                                : HM_GetVertScrollInfo(clientwnd);
            
            unsigned scroll_msg = (is_horiz) ? WM_HSCROLL : WM_VSCROLL;
            
            WPARAM fake_wp = MAKEWPARAM(scroll_amount, 0);
            
            WPARAM const active_child =
            (WPARAM) SendMessage(clientwnd,
                                 WM_MDIGETACTIVE,
                                 (WPARAM) NULL,
                                 (LPARAM) NULL);
            
            // -----------------------------
            
            if( (si.nMax - si.nMin) == 0)
            {
                // Clearly no scrollbar would be visible in such cases.
                // Could probably be handled more gracefully, but works.
                return 0;
            }
            
            if(active_child)
            {
                SendMessage(clientwnd, scroll_msg, fake_wp, HM_NullLP() );
            
                SendMessage(clientwnd, WM_MDIACTIVATE, active_child,
                            HM_NullLP() );
            
                SendMessage((HWND) active_child,
                            WM_NCACTIVATE,
                            TRUE,
                            0);
            }
            
            return 0;
        }
        
        break;
#endif


#if 1
    case WM_SIZE:
        
        if(always)
        {
            LRESULT result = DefFrameProc(win, clientwnd, msg, wparam, lparam);
            
            if(debug_box)
            {
                RECT const r = HM_GetWindowRect(win);
                
                SetWindowPos(debug_box,
                             NULL,
                             r.right - 220, r.bottom - 60,
                             0, 0,
                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            
            return result;
        }
        
        break;
        
#endif
    
#if 1
    case WM_MOVE:

        DefFrameProc(win, clientwnd, msg, wparam, lparam);
        
        if(debug_box)
        {
            RECT const r = HM_GetWindowRect(win);
            
            SetWindowPos(debug_box,
                         NULL,
                         r.right - 220, r.bottom - 60,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            
        }
        
        break;
#endif
    
    case WM_DISPLAYCHANGE:
        
        if(always)
        {
            HDC const dc = GetDC(framewnd);
            
            i = palmode;
            
            palmode = 0;
            
            if( GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE )
                palmode = 1;
            
            ReleaseDC(framewnd, dc);
        }
        
        if(palmode > i)
        {
            ed = firstgraph;
            
            while(ed)
            {
                Setpalmode(ed);
                ed = ed->nextgraph;
            }
        }
        else if(palmode < i)
        {
            ed = firstgraph;
            
            while(ed)
            {
                Setfullmode(ed);
                ed = ed->nextgraph;
            }
        }
        
        break;
    
    case WM_QUERYNEWPALETTE:
        if(!dispwnd)
            break;
        
        Setpalette(win,dispwnd->hpal);
        
        return 1;
    
    case WM_PALETTECHANGED:
        Updatepals();
        
        break;
    
/*  case WM_ACTIVATE:
        if(disppal && (wparam&65535)) {
            InvalidateRect(dispwnd,0,0);
//          hc=GetDC(dispwnd);
//          SelectPalette(hc,disppal,1);
//          RealizePalette(hc);
//          ReleaseDC(dispwnd,hc);
        }
        break;*/
    
    case WM_CREATE:
        ccs.hWindowMenu=GetSubMenu(GetMenu(win),2);
        ccs.idFirstChild=3600;
        clientwnd=CreateWindowEx(0,"MDICLIENT",0,MDIS_ALLCHILDSTYLES|WS_VSCROLL|WS_HSCROLL|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,0,0,0,0,win,(HMENU)3599,hinstance,(LPSTR)&ccs);
        
        break;
    
    case WM_CLOSE:
        goto fileexit;
    
    case WM_COMMAND:
        switch(wparam&65535)
        {
        
        case ID_MRU1:
        case ID_MRU2:
        case ID_MRU3:
        case ID_MRU4:
            
            wparam = (wparam & 0xffff) - ID_MRU1;
            
            if(!mrulist[wparam])
                break;
            
            doc = (FDOC*) malloc(sizeof(FDOC));
            strcpy(doc->filename,mrulist[wparam]);
            
            goto openrom;
        
        case ID_Z3_ABOUT:
            ShowDialog(hinstance,MAKEINTRESOURCE(IDD_DIALOG1),framewnd,aboutfunc,0);
            
            break;
        
        case ID_Z3_HELP:
            SetCurrentDirectory(currdir);
            WinHelp(framewnd,"Z3ED.HLP",HELP_CONTENTS,0);
            
            break;
        
fileexit:
        case ID_Z3_EXIT:
            doc=firstdoc;
            
            while(doc)
            {
                hc=doc->mdiwin;
                doc2=doc->next;
                
                if(SendMessage(hc,WM_CLOSE,0,0))
                    return 0;
                
                doc=doc2;
            }
            
            PostQuitMessage(0);
            
            break;
        
        case ID_Z3_SAVEAS:
            if(!activedoc)
                break;
            
saveas:
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner=win;
            ofn.hInstance=hinstance;
            ofn.lpstrFilter="SNES roms\0*.SMC;*.SFC\0All files\0*.*\0";
            ofn.lpstrCustomFilter=0;
            ofn.nFilterIndex=1;
            ofn.lpstrFile=activedoc->filename;
            ofn.nMaxFile=MAX_PATH;
            ofn.lpstrFileTitle=0;
            ofn.lpstrInitialDir=0;
            ofn.lpstrTitle="Save game";
            ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt="smc";
            ofn.lpfnHook=0;
            
            if(!GetSaveFileName(&ofn))
                return 1;
        
        case ID_Z3_SAVE:
            
            if(!activedoc)
                break;
            
            if(!*activedoc->filename)
                goto saveas;
            
            for(i=0;i<226;i++)
            {
                j = activedoc->blks[i].count;
                
                if(j != 0)
                {
                    activedoc->blks[i].count=1;
                    Releaseblks(activedoc,i);
                    Getblocks(activedoc,i);
                    activedoc->blks[i].count=j;
                }
            }
            
            for(i=0;i<160;i++)
            {
                if(activedoc->overworld[i].win)
                    Savemap((OVEREDIT*)GetWindowLong(activedoc->overworld[i].win,GWL_USERDATA));
            }
            
            for(i=0;i<168;i++)
            {
                if(activedoc->ents[i])
                    Saveroom((DUNGEDIT*)GetWindowLong(activedoc->ents[i],GWL_USERDATA));
            }
            
            for(i=0;i<128;i++)
            {
                if(activedoc->pals[i])
                    Savepal((PALEDIT*)GetWindowLong(activedoc->pals[i],GWL_USERDATA));
            }
            
            for(i=0;i<2;i++)
            {
                if(activedoc->wmaps[i])
                    Saveworldmap((WMAPEDIT*)GetWindowLong(activedoc->wmaps[i],GWL_USERDATA));
            }
            
            for(i=0;i<14;i++)
            {
                if(activedoc->dmaps[i])
                    Savedungmap((LMAPEDIT*)GetWindowLong(activedoc->dmaps[i],GWL_USERDATA));
            }
            
            for(i=0;i<11;i++)
            {
                if(activedoc->tmaps[i])
                    Savetmap((TMAPEDIT*)GetWindowLong(activedoc->tmaps[i],GWL_USERDATA));
            }
            
            if(activedoc->perspwnd)
                Savepersp((PERSPEDIT*) GetWindowLong(activedoc->perspwnd,GWL_USERDATA));
            
            Savesongs(activedoc);
            Savetext(activedoc);
            SaveOverlays(activedoc);
            
            if(activedoc->m_modf || activedoc->t_modf || activedoc->o_modf)
                return 1;
            
            rom = activedoc->rom;
            
            *(int*)(rom + 0x64118) = activedoc->mapend2;
            *(int*)(rom + 0x6411c) = activedoc->mapend;
            *(int*)(rom + 0x17f80) = activedoc->roomend;
            *(int*)(rom + 0x17f84) = activedoc->roomend2;
            *(int*)(rom + 0x17f88) = activedoc->roomend3;
            
            // \note This is tagging the rom "Z3ED" in what the editor assumes
            // is unused space. In a vanilla rom that is the case.
            *(int*)(rom + 0x17f8c) = 0x4445335A;
            
            // \note Tagging it with the version number of Hyrule Magic,
            // presumably.
            *(int*)(rom + 0x17f90) = 962;
            *(int*)(rom + 0x17f94) = 3;
            *(int*)(rom + 0x17f98) = activedoc->gfxend;
            *(short*)(rom + 0x4c298) = *(int*)(rom + 0x17f9c)=activedoc->dungspr;
            *(int*)(rom + 0x17fa0) = activedoc->sprend;
            
            if(activedoc->nummod)
            {
                if(activedoc->p_modf)
                {
updatemods:
                    if(Buildpatches(activedoc))
                        return 1;
                }
                else
                {
                    mod = activedoc->modules;
                    j = activedoc->nummod;
                    
                    for(i=0;i<j;i++,mod++)
                    {
                        h = CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
                        GetFileTime(h,0,0,&tim);
                        CloseHandle(h);
                        
                        __asm
                        {
                            mov eax,tim.dwLowDateTime
                            mov edx,tim.dwHighDateTime
                            mov ebx,activedoc
                            sub eax,[ebx+FDOC.lastbuild]
                            sbb edx,[ebx+FDOC.lastbuild+4]
                            jnc updatemods
                        }
                        
                        // if(tim.dwLowDateTime>activedoc->lastbuild.dwLowDateTime || tim.dwHighDateTime>activedoc->lastbuild.dwHighDateTime) goto updatemods;
                    }
                }
            }
            else
            {
                Removepatches(activedoc);
            }
            
            stle32b(rom + 0x17fa8, activedoc->sctend);
            
            stle16b(rom + 0x17fac,
                    (short) (activedoc->dmend - 0x8000) );
            
            stle32b(rom + 0x17fae, activedoc->ovlend);
            stle32b(rom + 0x17fb2, activedoc->tmend1);
            stle32b(rom + 0x17fb6, activedoc->tmend2);
            stle32b(rom + 0x17fba, activedoc->tmend3);
            stle32b(rom + 0x17fbe, activedoc->oolend);
            
            if(activedoc->mapexp)
                stle32b(rom + 0x100000, activedoc->mapexp);
            
            h = CreateFile(activedoc->filename,
                           GENERIC_WRITE,
                           0,
                           0,
                           OPEN_ALWAYS,
                           FILE_FLAG_SEQUENTIAL_SCAN,
                           0);
            
            if(h == INVALID_HANDLE_VALUE)
            {
saveerror:
                MessageBox(framewnd,"Unable to save","Bad error happened",MB_OK);
                
                return 1;
            }
            
            m = strrchr(activedoc->filename,'.');
            
            if
            (
                ( ! m )
             || strrchr(activedoc->filename, '\\') > m
            )
            {
                m = activedoc->filename + strlen(activedoc->filename);
            }
            
            l = *(int*)m;
            
            // Appears to attempt to append ".HMD" to the file path.
            strcpy(m, ".HMD");
            
            if(activedoc->nummod)
            {
                h2=CreateFile("HMTEMP.DAT",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,0);
                
                if(h2==(HANDLE)-1)
                {
                    *(int*)m=l;
                    CloseHandle(h);
                    
                    goto saveerror;
                }
                
                i='HMD0';
                WriteFile(h2,&i,4,&write_size,0);
                WriteFile(h2,&(activedoc->nummod),14,&write_size,0);
                
                for(i = 0; i < activedoc->numpatch; i++)
                {
                    WriteFile(h2,activedoc->patches+i,8,&write_size,0);
                    
                    WriteFile(h2,
                              activedoc->patches[i].pv,
                              activedoc->patches[i].len,
                              &write_size,
                              0);
                }
                
                WriteFile(h2,activedoc->segs,activedoc->numseg<<2,&write_size,0);
                mod = activedoc->modules;
                
                for(i=0;i<activedoc->nummod;i++,mod++)
                {
                    enum { PATCH_HEADER_SIZE = 8 };
                    
                    uint8_t patch_header[PATCH_HEADER_SIZE] = { 0 };
                    
                    j = strlen(mod->filename);
                    
                    stle32b_i(patch_header, 0, j);
                    stle32b_i(patch_header, 1, activedoc->modules[i].flag);
                    
                    WriteFile(h2,
                              patch_header,
                              PATCH_HEADER_SIZE,
                              &write_size,
                              0);
                    
                    WriteFile(h2,mod->filename,j,&write_size,0);
                }
                
                CloseHandle(h2);
                
                if(!(rom[0x17fa4]&1))
                {
                    if(!MoveFile("HMTEMP.DAT",activedoc->filename))
                    {
                        if(GetLastError() == ERROR_ALREADY_EXISTS)
                        {
                            wsprintf(text_buf,
                                     "%s exists already. Do you want to overwrite it?",
                                     activedoc->filename);
                            
                            if
                            (
                                MessageBox
                                (
                                    framewnd,
                                    text_buf,
                                    "Gigasoft Hyrule Magic",
                                    MB_YESNO
                                ) == IDYES
                            )
                            {
                                goto okay;
                            }
                            else
                            {
othersaveerror:
                                *(int*)m=l;
                                DeleteFile("HMTEMP.DAT");
                                CloseHandle(h);
                                
                                return 1;
                            }
                        }
                        else
                            goto othersaveerror;
                    }
                    
                    goto okay2;
                }
                else
                {
okay:
                    DeleteFile(activedoc->filename);
                    
                    if(!MoveFile("HMTEMP.DAT",activedoc->filename))
                    {
                        MessageBox(framewnd,"Unable to save hack database","Bad error happened",MB_OK);
                        
                        goto othersaveerror;
                    }
                }
okay2:
                rom[0x17fa4]|=1;
            }
            else
            {
                if(rom[0x17fa4]&1)
                    DeleteFile(activedoc->filename);
                
                rom[0x17fa4]&=-2;
            }
            
            *(int*)m=l;
            
            if(activedoc->withhead)
            {
                char header_buf[0x200] = { 0 };
                
                header_buf[0] = -128;
                header_buf[8] = -86;
                header_buf[9] = -69;
                header_buf[10] = 4;
                
                WriteFile(h,
                          header_buf,
                          0x200,
                          &write_size,
                          0);
            }
            
            WriteFile(h, activedoc->rom, activedoc->fsize, &write_size, 0);
            
            SetEndOfFile(h);
            CloseHandle(h);
            activedoc->modf=0;
            SetWindowText(activedoc->mdiwin,activedoc->filename);
            AddMRU(activedoc->filename);
            UpdMRU();
            DrawMenuBar(framewnd);
            
            return 0;
        
        case ID_Z3_OPEN:
            
            // check if the shift key is down for some reason.
            if(GetKeyState(VK_SHIFT) & 128)
            {
                // if there is no active document, stop.
                if(!activedoc)
                    break;
                
                // If the music data is loaded...
                if(activedoc->m_loaded)
                {
                    // save the songs in the current document.
                    Savesongs(activedoc);
                    
                    // if the music has been modified, get out...
                    if(activedoc->m_modf)
                        break;
                    
                    Unloadsongs(activedoc);
                    Loadsongs(activedoc);
                    
                    // if the HWND handle for the third music bank is valid.
                    if(activedoc->mbanks[3])
                    {
                        sed = ((SAMPEDIT*)GetWindowLong(activedoc->mbanks[3],GWL_USERDATA));
                        sed->zw = activedoc->waves + sed->editsamp;
                    }
                }
                
                // if overlays are loaded
                if(activedoc->o_loaded)
                {
                    // save the overlays of the active rom.
                    SaveOverlays(activedoc);
                    
                    // if the overlays have been modified, exit.
                    if(activedoc->o_modf)
                    {
                        // what this generally means is that the overlays didn't save
                        // properly
                        break;
                    }
                    
                    Unloadovl(activedoc);
                    LoadOverlays(activedoc);
                }
                
                break;
            }
            
            // allocate enough memory for the object.
            doc = (FDOC*) malloc(sizeof(FDOC));
            
            if(!doc) // not enough memory.
            {
                MessageBox(framewnd,"No memory at all","Bad error happened",MB_OK);
                
                break;
            }
            
            // ofn = open file name
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = win;
            ofn.hInstance = hinstance;
            ofn.lpstrFilter = "SNES roms\0*.SMC;*.SFC\0All files\0*.*\0";
            ofn.lpstrCustomFilter = 0;
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = doc->filename;
            doc->filename[0] = 0;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFileTitle = 0;
            ofn.lpstrInitialDir = 0;
            ofn.lpstrTitle = "Open game";
            ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
            ofn.lpstrDefExt = "smc";
            ofn.lpfnHook = 0;
            
            if(!GetOpenFileName(&ofn))
            {
//              i=CommDlgExtendedError();
//              if(i) {
//                  wsprintf(text_buf,"GUI error. Error: %08X",i);
//                  MessageBox(framewnd,text_buf,"Bad error happened",MB_OK);
//              }
                free(doc); break;
            }
openrom:
            
            // handles duplicate files already being open, by checking pointer values.
            for(doc2 = firstdoc; doc2; doc2 = doc2->next)
            {
                if(!_stricmp(doc2->filename,doc->filename))
                {
                    free(doc);
                    SendMessage(clientwnd,WM_MDIACTIVATE,(long)(doc2->mdiwin),0);
                    
                    return 0;
                }
            }
            
            // load the rom file
            h = CreateFile(doc->filename,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
            
            if(h == INVALID_HANDLE_VALUE)
            {
                MessageBox(framewnd,"Unable to open file","Bad error happened",MB_OK);
                free(doc);
                
                break;
            }
            
            // j is the size of the file, passing the handle of the file, 
            j = GetFileSize(h,0);
            
            //read the first four bytes of the file.
            ReadFile(h,&i,4,&read_size,0);
            
            //these first four bytes have to match, apparently. 
            //kind of primitive error checking...eh?
            //for the curious, that is SEI, STZ $4200 in hex, backwards.
            if(i != 0x42009c78)
            {
                //move the file's base offset up by 512 bytes if we detect a header.
                SetFilePointer(h,512,0,0);
                
                // the filesize is now regarded as (size - header).
                j -= 512;
                
                // as above, we note there is a header.
                doc->withhead = 1;
            }
            else
            {
                // else leave it as is.
                SetFilePointer(h,0,0,0);
                
                // and the file has no header.
                doc->withhead = 0;
            }
            
            // allocate a rom (buffer) the same size as the file.
            rom = doc->rom = malloc(j);
            
            if(!rom)
            {
                MessageBox(framewnd,"Not enough memory.","Bad error happened",MB_OK);
                CloseHandle(h);
                free(doc);
                
                break;
            }
            
            // set the internal file size variable (in an FDOC)
            doc->fsize = j;
            
            // dump the file into the variable "rom".
            ReadFile(h, rom, j, &read_size, 0);
            CloseHandle(h);
            
            //this is LDA $2801, BEQ... another primitive file check (as though romhackers 
            //would never change such things... >_> <_<
            if( ldle32b(rom + 0x200) != 0xf00128ad)
            {
                MessageBox(framewnd,
                           "Hey! This is not Zelda 3.",
                           "Bad error happened",
                           MB_OK);
error:
                free(rom);
                free(doc);
                
                break;
            }
            
            // these tell us where certain data structures end.
            doc->mapend = 0x6410c;
            doc->mapend2 = 0x5fe5e;
            doc->roomend = 0xfff4e;
            doc->roomend2 = 0x5372a;
            doc->roomend3 = 0x1fff5;
            doc->dungspr = 0x4d62e;
            doc->sprend = 0x4ec9f;
            doc->sctend = 0xdc894;
            doc->ovlend = 0x271de;
            doc->dmend = 0x57ce0;
            doc->tmend1 = 0x66d7a;
            doc->tmend2 = 0x75d31;
            doc->oolend = 0x77b64;
            
            if( HM_CheckEmbeddedStr(rom + 0x17f8c, "Z3ED") )
            {
                i = *(int*) (rom + 0x17f94);
                k = *(int*) (rom + 0x17f90);
            }
            else
            {
                i = 0;
                k = 0;
            }
            
            if(k == 92)
            {
                rom[0x9bad] = 0x7e;
                *(short*) (rom + 0x9bb2) = 0x229f;
            }
            
            if(i > 3)
            {
                vererror_str[90] = ((k & 0xe000) >> 13) + 48;
                wsprintf(text_buf, vererror_str, k >> 16, k & 8191);
                
                    MessageBox(framewnd,
                           text_buf,
                           "Bad error happened",
                           MB_OK);
                
                goto error;
            }
            
            if( i >= 3 ) if( rom[0x17fa4] & 1 )
            {
                // truncate to the last '.' (period) in the filename.
                m = strrchr(doc->filename, '.');
                
                // comment later.
                if( (!m) || strrchr(doc->filename, '\\') > m)
                    //go to the end of the filenamme.
                    m = doc->filename + strlen(doc->filename);
                
                l = *(int*)m; // what is the int value there?
                // change the path extensions .HMD.
                strcpy(m, ".HMD");
                
                h = CreateFile(doc->filename,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               0,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               0);
                
                if(h == (HANDLE) -1)
                {
                    wsprintf(text_buf,
                             "A required file, %s, is missing",
                             doc->filename);

                errormsg:
                    
                    MessageBox(framewnd,
                               text_buf,
                               "Bad error happened",
                               MB_OK);
                    
                    goto error;
                }
                
                ReadFile(h,&i,4,&read_size,0);
                
                if(i != 'HMD0')
                {
                    wsprintf(text_buf,
                             "%s has an invalid format.",
                             doc->filename);
                    
                    goto errormsg;
                }
                
                ReadFile(h,&(doc->nummod),14,&read_size,0);
                
                doc->patches = malloc(doc->numpatch*sizeof(PATCH));
                
                for(i = 0; i < doc->numpatch; i++)
                {
                    // read in the patches
                    ReadFile(h,doc->patches+i,8,&read_size,0);
                    doc->patches[i].pv=malloc(doc->patches[i].len);
                    ReadFile(h,doc->patches[i].pv,doc->patches[i].len,&read_size,0);
                }
                
                ReadFile(h,doc->segs,doc->numseg<<2,&read_size,0);
                
                mod = doc->modules = malloc(doc->nummod * sizeof(ASMHACK));
                
                for(i = 0; i < doc->nummod; i++, mod++)
                {
                    ReadFile(h,doc->modules+i,sizeof(ASMHACK),&read_size,0);
                    
                    k = (int) mod->filename;
                    mod->filename = malloc(k + 1);
                    
                    ReadFile(h,mod->filename,k,&read_size,0);
                    mod->filename[k]=0;
                }
                
                CloseHandle(h);
                
                *(int*)m = l;
            }
            else
            {
nomod:
                doc->lastbuild.dwLowDateTime=0;
                doc->lastbuild.dwHighDateTime=0;
                doc->numseg=doc->numpatch=doc->nummod=0;
                doc->patches = 0;
                doc->modules = 0;
            }
            else
            {
                *(int*) (rom + 0x17fa4) = 0;
                
                goto nomod;
            }
            
            // if the file size is larger or equal to...
            if(j >= 0x100004)
                // the map expansion value is...
                doc->mapexp = ldle32b(rom + 0x100000);
            else
                // otherwise, there is no expansion.
                doc->mapexp = 0;
            
            i = *(int*) (rom + 0x6411c);
            
            // if this has been modified...
            if(i > 0)
                // assign the map ending bound.
                doc->mapend = i;
            
            i = *(int*) (rom + 0x64118);
            
            if(i > 0)
                doc->mapend2 = i; // similarly...
            
            i = *(int*) (rom + 0x17f80);
            
            if(i > 0)
                doc->roomend = i;
            
            i = *(int*) (rom + 0x17f84);
            
            if(i > 0)
                doc->roomend2 = i;
            
            i = *(int*) (rom + 0x17f88);
            
            if(i > 0)
                doc->roomend3 = i;
            
            i = *(int*) (rom + 0x17f9c);
            
            if(i > 0)
            {
                doc->dungspr = *(int*) (rom + 0x17f9c);
                doc->sprend = *(int*) (rom + 0x17fa0);
            }
            
            // bank + HByte + LByte for something...
            i = romaddr((rom[0x505e] << 16) + (rom[0x513d] << 8) + rom[0x521c]);
            
            for(;;)
            {
                j = rom[i++];
                
                if(j == 0xff)
                    break;
                
                if(j >= 0xe0)
                {
                    k = ((j & 3) << 8) + rom[i++];
                    l = (j & 28) >> 2;
                }
                else
                {
                    k = j & 31;
                    l = j >> 5;
                }
                
                switch(l)
                {
                case 0:
                    i += k + 1;
                    
                    break;
                
                case 1: case 3:
                    i++;
                    
                    break;
                
                default:
                    i += 2;
                }
            }
            
            doc->gfxend = i;
            
            i = *(int*)(rom + 0x17fa8);
            
            if(i > 0)
                doc->sctend = i;
            
            i = *(short*)(rom + 0x17fac);
            
            if(i != -1)
                doc->dmend = i + 0x58000;
            
            i = *(int*) (rom + 0x17fae);
            
            if(i > 0)
                doc->ovlend = i;
            
            i = *(int*) (rom + 0x17fb2);
            
            if(i > 0)
            {
                doc->tmend1 = i;
                doc->tmend2 = *(int*) (rom + 0x17fb6);
                doc->tmend3 = *(short*) (rom + 0x17fba) + 0x60000;
            }
            else
            {
                buf = malloc(0x100d);
                
                memcpy(buf, rom + 0x66359,0xfd);
                memcpy(buf + 0xfd, rom + 0x661c8,0xe0);
                memcpy(buf + 0x1dd, rom + 0x6653f,0xfd);
                memcpy(buf + 0x2da, rom + 0x6668d,0x132);
                memcpy(buf + 0x40c, rom + 0x65d6d,0x45b);
                memcpy(buf + 0x867, rom + 0x662a8,0xb1);
                memcpy(buf + 0x918, rom + 0x66456,0xe9);
                memcpy(buf + 0xa01, rom + 0x6663c,0x51);
                memcpy(buf + 0xa52, rom + 0x667bf,0x5bb);
                memcpy(rom + 0x65d6d, buf, 0x40c);
                memcpy(rom + 0x66179, buf + 0x40c, 0xc01);
                
                *(USHORT*)(rom + 0x64ed0) = 0xdd6c;
                *(USHORT*)(rom + 0x64e5f) = 0xde6a;
                *(USHORT*)(rom + 0x654c0) = 0xdf49;
                *(USHORT*)(rom + 0x65148) = 0xe047;
                *(USHORT*)(rom + 0x65292) = 0xe0f4;
                
                // \task look into these 32-bit constants.
                *(int*)(rom + 0x137d) = 0xd4bf1b79;
                *(USHORT*)(rom + 0x1381) = 0x856e;
                *(int*)(rom + 0x1386) = 0xe5e702e1;
                *(USHORT*)(rom + 0x138a)=0xe6e7;
                
                free(buf);
                
                doc->tmend3 = 0x66179;
            }
            
            i = *(int*)(rom + 0x17fbe);
            
            if(i > 0)
                doc->oolend = i;
            
            if(doc->mapexp == 0x100004)
                if(!Changesize(doc,4097,0))
                    goto error;
            
            if(doc->mapexp && doc->mapexp != 0x100080)
            {
                MessageBox(framewnd,"This ROM is corrupt.","Bad error happened",MB_OK);
                goto error;
            }
            
            for(j = 0; j < 5; j++)
            {
                for(i = 0; i < 64; i++)
                {
                    if(rom[0x125ec + i] != i)
                    {
                        Savesprites(doc, sprs[j] + i + 0x10000, 0 , 0);
                    }
                }
            }
            
            mdic.szClass = "ZEDOC";
            mdic.szTitle = doc->filename;
            mdic.hOwner = hinstance;
            mdic.x = mdic.y = mdic.cx=mdic.cy=CW_USEDEFAULT;
            mdic.style = WS_OVERLAPPEDWINDOW|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
            mdic.lParam = (long)doc;
            hc = (HWND)SendMessage(clientwnd,WM_MDICREATE,0,(long)&mdic);
            
            if(!firstdoc)
                firstdoc = doc;
            
            if(lastdoc)
                lastdoc->next = doc;
            
            doc->prev = lastdoc;
            doc->next = 0;
            
            lastdoc = doc;
            
            doc->modf = 0;
            
            SendMessage(clientwnd,WM_MDIACTIVATE,(long)hc,0);
            SendMessage(clientwnd,WM_MDIREFRESHMENU,0,0);
            
            doc->mdiwin=hc;
            
            for(i=0;i<226;i++)
                doc->blks[i].count = 0,doc->blks[i].buf = 0;
            
            for(i=0;i<160;i++)
                doc->overworld[i].win = 0, doc->overworld[i].modf = 0;
            
            for(i=0;i<168;i++)
                doc->ents[i] = 0;
            
            for(i=0;i<0x128;i++)
                doc->dungs[i] = 0;
            
            for(i=0;i<PALNUM;i++)
                doc->pals[i] = 0;
            
            for(i=0;i<14;i++)
                doc->dmaps[i] = 0;
            
            for(i=0;i<11;i++)
                doc->tmaps[i] = 0;
            
            for(i=0;i<4;i++)
                doc->mbanks[i] = 0;
            
            doc->m_loaded = 0;
            doc->t_loaded = 0;
            doc->o_loaded = 0;
            doc->t_wnd = 0;
            doc->m_modf = 0;
            doc->t_modf = 0;
            doc->o_modf = 0;
            doc->wmaps[0] = doc->wmaps[1] = 0;
            doc->perspwnd = 0;
            doc->hackwnd = 0;
            doc->p_modf = 0;
            
            if(always)
            {
                HMENU const menu = GetMenu(framewnd);
                
                HMENU const submenu =
                GetSubMenu(menu,
                           GetMenuItemCount(menu) == 5 ? 0 : 1);
                
                EnableMenuItem(submenu, ID_Z3_SAVE, MF_ENABLED);
                EnableMenuItem(submenu, ID_Z3_SAVEAS, MF_ENABLED);
                EnableMenuItem(menu, GetMenuItemCount(menu) == 5 ? 1 : 2,
                               MF_ENABLED | MF_BYPOSITION);
            }
            
            AddMRU(doc->filename);
            UpdMRU();
            
            DrawMenuBar(framewnd);
            
            break;
        
        case ID_FILE_SPRNAME:
            
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG5,framewnd,editsprname,0);
            
            break;
        
        case ID_WINDOW_MDITILE:
            SendMessage(clientwnd,WM_MDITILE,0,0);
            break;
        
        case ID_WINDOW_MDICASCADE:
            SendMessage(clientwnd,WM_MDICASCADE,0,0);
            
            break;
        
        case ID_WINDOW_MDIARRANGEICONS:
            SendMessage(clientwnd,WM_MDIICONARRANGE,0,0);
            
            break;
        
        case ID_OPTION_DEVICE:
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG2,framewnd,seldevproc,0);
            
            break;
        
        case ID_OPTION_CLOSESOUND:
            Exitsound();
            
            break;
        
        case ID_OPTION_CHANNEL1:
        case ID_OPTION_CHANNEL2:
        case ID_OPTION_CHANNEL3:
        case ID_OPTION_CHANNEL4:
        case ID_OPTION_CHANNEL5:
        case ID_OPTION_CHANNEL6:
        case ID_OPTION_CHANNEL7:
        case ID_OPTION_CHANNEL8:
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                i = 1 << (wparam - ID_OPTION_CHANNEL1);
                
                activeflag ^= i;
                
                CheckMenuItem
                (
                    GetSubMenu
                    (
                        GetSubMenu
                        (
                            menu,
                            GetMenuItemCount(menu) == 5 ? 3 : 4
                        ),
                        2
                    ),
                    wparam,
                    (activeflag & i) ? MF_CHECKED : MF_UNCHECKED
                );
                
                if((!(activeflag & i)) && sounddev >= 0x20000)
                    midinoteoff(zchans + ((wparam & 65535) - ID_OPTION_CHANNEL1));
            }
            
            break;
        
        case ID_OPTION_SNDINTERP:
            
            sndinterp = !sndinterp;
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                CheckMenuItem(GetSubMenu(menu,
                                         GetMenuItemCount(menu) == 5 ? 3 : 4),
                                         ID_OPTION_SNDINTERP,
                                         sndinterp ? MF_CHECKED : MF_UNCHECKED);
            }
            
            break;
        
        // \note Unused ID not currently referenced in menus or elsewhere.
        case ID_OPTION_GBTNT:
            
            gbtnt = !gbtnt;
            
            if(always)
            {
                HMENU menu = GetMenu(framewnd);
                
                CheckMenuItem(GetSubMenu(menu,
                                         GetMenuItemCount(menu) == 5 ? 3 : 4),
                                         ID_OPTION_GBTNT,
                                         gbtnt ? MF_CHECKED : MF_UNCHECKED);
            }
            
            break;
            
        case ID_Z3_CUT:
            SendMessage(GetFocus(),WM_CUT,0,0);
            
            break;
            
        case ID_Z3_COPY:
            SendMessage(GetFocus(),WM_COPY,0,0);
            
            break;
            
        case ID_Z3_PASTE:
            SendMessage(GetFocus(),WM_PASTE,0,0);
            
            break;
            
        case ID_EDITING_DELOVERITEM:
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 128; i++)
                ((unsigned short*) (rom + 0xdc2f9))[i] = 0xc3f9;
            
            *(short*) (rom + 0xdc3f9) = -1;
            activedoc->sctend=0xdc3fb;
            
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELDUNGITEM:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x140;i++)
                ((unsigned short*) (rom + 0xdb69))[i] = 0xdde9;
            
            *(short*)(rom + 0xdde9)=-1;
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_DELOVERSPR:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x160;i++)
                ((unsigned short*) (rom + 0x4c881))[i] = 0xcb41;
            
            rom[0x4cb41] = 0xff;
            
            memcpy(rom + 0x4cb42,
                   rom + activedoc->dungspr,
                   activedoc->sprend - activedoc->dungspr);
            
            for(i = 0; i < 0x128; i++)
            {
                addle16b_i(rom + 0x4cb42,
                           i,
                           (short) (0x4cb42 - activedoc->dungspr) );
            }
            
            activedoc->sprend += 0x4cb42 - activedoc->dungspr;
            activedoc->dungspr = 0x4cb42;
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELDUNGSPR:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            j = activedoc->dungspr;
            
            for(i = 0; i < 0x128; i++)
            {
                stle16b_i(rom + j,
                          i,
                          (short) (j + 0x300));
            }
            
            stle16b(rom + j + 0x300, 0xff00);
            
            activedoc->sprend = j + 0x302;
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_DELBLOCKS:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<0x18c;i+=4)
                *((short*) (rom + 0x271de + i)) = -1;
            
            activedoc->modf=1;
            
            for(i = 0; i < 0x128; i++)
            {
                if(activedoc->dungs[i])
                {
                    hc = GetDlgItem(activedoc->dungs[i], ID_DungEditWindow);
                    Updatemap((DUNGEDIT*)GetWindowLong(hc,GWL_USERDATA));
                    InvalidateRect(hc,0,0);
                }
            }
            
            break;
        
        case ID_EDITING_DELTORCH:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            if(always)
            {
                rom_ty const torches = rom + offsets.dungeon.torches;
                
                rom_ty const torch_count = (rom + offsets.dungeon.torch_count);
                
                stle16b(torches,     u16_neg1);
                stle16b(torches + 2, u16_neg1);
                
                stle16b(torch_count, 4);
            }
            
            
            activedoc->modf = 1;
            
            break;
        
        case ID_EDITING_DELENT:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 129; i++)
            {
                stle16b_i(rom + 0xdb96f, i, u16_neg1);
            }
            
        upddisp:
            
            activedoc->modf=1;
            
            for(i = 0; i < 160; i++)
            {
                if(activedoc->overworld[i].win)
                {
                    InvalidateRect
                    (
                        GetDlgItem
                        (
                            GetDlgItem
                            (
                                activedoc->overworld[i].win,
                                ID_SuperDlg
                            ),
                            SD_Over_Display
                        ),
                        0,
                        0
                    );
                }
            }
            
            break;
        
        case ID_EDITING_DELEXIT:
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=0;i<79;i++)
            {
                rom[0x15e28 + i] = 255;
                
                stle16b_i(rom + 0x15d8a, i, u16_neg1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELFLY:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i = 0; i < 9; i++)
            {
                stle16b_i(rom + 0x16ae5, i, u16_neg1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELWHIRL:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            for(i=9;i<17;i++)
            {
                stle16b_i(rom + 0x16ae5, i, u16_neg1);
            }
            
            goto upddisp;
        
        case ID_EDITING_DELHOLES:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            memset(rom + 0xdb826,255,19);
            
            goto upddisp;
        
        case ID_EDITING_DELPITS:
            
            if(!activedoc)
                break;
            
            rom = activedoc->rom;
            
            memset(rom + 0x190c,255,114);
            
            break;
        
        case ID_EDIT_ROMPROPERTIES:
            ShowDialog(hinstance,(LPSTR)IDD_DIALOG18,framewnd,rompropdlg,(long)activedoc);
            
            break;
        
        case ID_EDITING_CLEARDUNGS:
            
            for(i=0;i<320;i++)
            {
                door_ofs = 6;
                buf = activedoc->rom+Changesize(activedoc,i+320,8);
                
                if(buf)
                {
                    *(int*)buf=-65535;
                    *(int*)(buf+4)=-1;
                }
            }
            
            activedoc->modf=1;
            
            break;
        
        case ID_EDITING_REPLACE:
            
            // we pass it this program, a pointer to the duplicator dialog, pass framewnd as the 
            // owner window. duproom is the dialogue handling procedure, and active doc is
            // a pointer to the currently active rom file.
            ShowDialog(hinstance, (LPSTR) IDD_DIALOG20, framewnd, duproom, (long) activedoc);
            
            break;
        }
        
    default:
        
        return DefFrameProc(win,clientwnd,msg,wparam,lparam);
    }
    
    return 0;
}
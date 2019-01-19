
    #include "structs.h"
    #include "prototypes.h"

    #include "AudioLogic.h"

    #include "HMagicEnum.h"

    #include "TextLogic.h"

    #include "PatchLogic.h"

// =============================================================================

    void
    DocumentFrame_OnCreate(HWND   const p_win,
                           LPARAM const p_lp)
    {
        CP2C(CREATESTRUCT) cs = (CREATESTRUCT*) p_lp;
        
        CP2C(MDICREATESTRUCT) mdi_cs = (MDICREATESTRUCT*) cs->lpCreateParams;
        
        CP2(FDOC) doc = (FDOC*) mdi_cs->lParam;
        
        // -----------------------------
        
        SetWindowLongPtr(p_win, GWLP_USERDATA, (LONG_PTR) doc);
        
        ShowWindow(p_win, SW_SHOW);
        
        doc->editwin = CreateSuperDialog
        (
            &z3_dlg,
            p_win,
            0, 0, 0, 0,
            (LPARAM) doc
        );
    }

// =============================================================================

void
DocumentFrame_OnClose(FDOC * const param,
                      HWND   const win)
{
    char text_buf[0x200];
    
    HMENU h  = 0;
    HMENU h2 = 0;
    
    int i = 0;
    
    if(param->modf || param->m_modf || param->t_modf || param->p_modf)
        goto save;
    
    for(i = 0; i < 160; i++)
    {
        if(param->overworld[i].modf)
            goto save;
    }
    
    for(i = 0; i < 2; i++)
    {
        if(param->wmaps[i] && ((WMAPEDIT*) (GetWindowLongPtr(param->wmaps[i], GWLP_USERDATA)))->modf)
            goto save;
    }
    
    for(i=0;i<168;i++) {
        if(param->ents[i] && ((DUNGEDIT*)(GetWindowLongPtr(param->ents[i],GWLP_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<PALNUM;i++) {
        if(param->pals[i] && ((PALEDIT*)(GetWindowLongPtr(param->pals[i], GWLP_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<14;i++) {
        if(param->dmaps[i] && ((LMAPEDIT*)(GetWindowLongPtr(param->dmaps[i], GWLP_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<11;i++) {
        if(param->tmaps[i] && ((TMAPEDIT*)(GetWindowLongPtr(param->tmaps[i], GWLP_USERDATA)))->modf) goto save;
    }
    
    if(param->perspwnd && ((PERSPEDIT*)(GetWindowLongPtr(param->perspwnd, GWLP_USERDATA)))->modf) goto save;
    goto dontsave;
    
save:
    
    wsprintf(text_buf,
             "Do you want to save the changes to %s?",
             param->filename);
    
    i = MessageBox(framewnd,
                   text_buf,
                   "Gigasoft Hyrule Magic",
                   MB_YESNOCANCEL);
    
    if(i == IDYES)
        if( SendMessage(framewnd, WM_COMMAND, ID_Z3_SAVE, 0) )
            return;
    
    if(i == IDCANCEL)
        return;
    
dontsave:
    
    DestroyWindow( GetDlgItem(win, ID_SuperDlg) );
    
    HM_MDI_DestroyChild(clientwnd, win);
    
    activedoc=0;
    param->modf=2;
    
    for(i=0;i<160;i++)
    {
        if(param->overworld[i].win)
            SendMessage(param->overworld[i].win,WM_CLOSE,0,0);
    }
    
    for(i=0;i<168;i++)
    {
        if(param->ents[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->ents[i]);
        }
    }
    
    for(i=0;i<4;i++)
    {
        if(param->mbanks[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->mbanks[i]);
        }
    }
    
    for(i=0;i<PALNUM;i++)
    {
        if(param->pals[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->pals[i]);
        }
    }
    
    for(i=0;i<2;i++)
    {
        if(param->wmaps[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->wmaps[i]);
        }
    }
    
    for(i = 0; i < 14; i++)
    {
        if(param->dmaps[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->dmaps[i]);
        }
    }
    
    for(i = 0; i < 11; i++)
    {
        if(param->tmaps[i])
        {
            HM_MDI_DestroyChild(clientwnd, param->tmaps[i]);
        }
    }
    
    if(param->perspwnd) HM_MDI_DestroyChild(clientwnd, param->perspwnd);
    
    if(param->hackwnd) HM_MDI_DestroyChild(clientwnd, param->hackwnd);
    
    if(param->m_loaded)
    {
        for(i = 0; i < param->srnum; i++)
            if(param->sr[i].editor)
                HM_MDI_DestroyChild(clientwnd, param->sr[i].editor);
    }
    
    if(param->t_wnd) HM_MDI_DestroyChild(clientwnd, param->t_wnd);
    
    if(param->perspwnd)
        HM_MDI_DestroyChild(clientwnd, param->perspwnd);
    
    if(param->prev) param->prev->next=param->next;
    else firstdoc=param->next;
    
    if(param->next) param->next->prev=param->prev;
    else lastdoc=param->prev;
    
    h2=GetMenu(framewnd);
    h=GetSubMenu(h2,GetMenuItemCount(h2)==5?0:1);
    i=firstdoc?MF_ENABLED:MF_GRAYED;
    EnableMenuItem(h,ID_Z3_SAVE,i);
    EnableMenuItem(h,ID_Z3_SAVEAS,i);
    EnableMenuItem(h2,GetMenuItemCount(h2)==5?1:2,i|MF_BYPOSITION);
    DrawMenuBar(framewnd);
    if(sounddoc==param) {
        if(sndinit) {
            // EnterCriticalSection(&cs_song);
            Stopsong();
            // LeaveCriticalSection(&cs_song);
        } else sounddoc=0;
    }
    free(param->rom);
    Unloadsongs(param);
    Unloadovl(param);
    
    if(param->t_loaded)
    {
        size_t m_i = 0;
        
        for(m_i = 0; m_i < param->t_number; m_i += 1)
        {
            ZTextMessage_Free( &param->text_bufz[m_i] );
        }
        
        free(param->text_bufz);
    }
    
    Doc_FreePatchInputs(param);
    
    free(param);
}

// =============================================================================

    // Window procedure for the document window with the large tree control.
    LRESULT CALLBACK
    docproc(HWND win,UINT msg, WPARAM wparam,LPARAM p_lp)
    {
        FDOC * param;
        
        switch(msg)
        {
    
        case WM_MDIACTIVATE:
            
            activedoc = (FDOC*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            goto default_case;
        
        case WM_CLOSE:
            
            param = (FDOC*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            if(param)
            {
                DocumentFrame_OnClose(param, win);
            }
            
            break;
        
        case WM_GETMINMAXINFO:
            
            param = (FDOC*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            DefMDIChildProc(win,msg,wparam, p_lp);
            
            if( IsNull(param) )
            {
                break;
            }
            
            return SendMessage
            (
                param->editwin,
                WM_GETMINMAXINFO,
                wparam,
                p_lp
            );
            
        case WM_SIZE:
            
            param = (FDOC*) GetWindowLongPtr(win, GWLP_USERDATA);
            
            SetWindowPos
            (
                param->editwin,
                0,
                0, 0,
                LOWORD(p_lp), HIWORD(p_lp),
                (SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE)
            );
            
            goto default_case;
        
        case WM_CREATE:
            
            DocumentFrame_OnCreate(win, p_lp);
            
        case WM_CHILDACTIVATE:
        default:
        default_case:
            
            return DefMDIChildProc(win, msg, wparam, p_lp);
        }
        
        return 0;
    }

// =============================================================================


    #include "structs.h"
    #include "prototypes.h"

    #include "AudioLogic.h"

    #include "HMagicEnum.h"

    #include "TextLogic.h"

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
        if(param->wmaps[i] && ((WMAPEDIT*) (GetWindowLong(param->wmaps[i], GWL_USERDATA)))->modf)
            goto save;
    }
    
    for(i=0;i<168;i++) {
        if(param->ents[i] && ((DUNGEDIT*)(GetWindowLong(param->ents[i],GWL_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<PALNUM;i++) {
        if(param->pals[i] && ((PALEDIT*)(GetWindowLong(param->pals[i],GWL_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<14;i++) {
        if(param->dmaps[i] && ((LMAPEDIT*)(GetWindowLong(param->dmaps[i],GWL_USERDATA)))->modf) goto save;
    }
    
    for(i=0;i<11;i++) {
        if(param->tmaps[i] && ((TMAPEDIT*)(GetWindowLong(param->tmaps[i],GWL_USERDATA)))->modf) goto save;
    }
    
    if(param->perspwnd && ((PERSPEDIT*)(GetWindowLong(param->perspwnd,GWL_USERDATA)))->modf) goto save;
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
    
    SendMessage(clientwnd, WM_MDIDESTROY, (WPARAM) win, 0);
    
    activedoc=0;
    param->modf=2;
    for(i=0;i<160;i++)
    {
        if(param->overworld[i].win) SendMessage(param->overworld[i].win,WM_CLOSE,0,0);
    }
    
    for(i=0;i<168;i++)
    {
        if(param->ents[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->ents[i],0);
    }
    
    for(i=0;i<4;i++)
    {
        if(param->mbanks[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->mbanks[i],0);
    }
    
    for(i=0;i<PALNUM;i++)
    {
        if(param->pals[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->pals[i],0);
    }
    
    for(i=0;i<2;i++)
    {
        if(param->wmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->wmaps[i],0);
    }
    
    for(i=0;i<14;i++)
    {
        if(param->dmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->dmaps[i],0);
    }
    
    for(i=0;i<11;i++)
    {
        if(param->tmaps[i]) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->tmaps[i],0);
    }
    
    if(param->perspwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->perspwnd,0);
    if(param->hackwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->hackwnd,0);
    if(param->m_loaded) {
        for(i=0;i<param->srnum;i++)
            if(param->sr[i].editor) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->sr[i].editor,0);
    }
    if(param->t_wnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->t_wnd,0);
    if(param->perspwnd) SendMessage(clientwnd,WM_MDIDESTROY,(long)param->perspwnd,0);
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
    
    free(param);
}

// =============================================================================

// Window procedure for the document window with the large tree control.
LRESULT CALLBACK
docproc(HWND win,UINT msg, WPARAM wparam,LPARAM lparam)
{
    FDOC *param;
    
    switch(msg)
    {
    
    case WM_MDIACTIVATE:
        activedoc=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        
        goto deflt;
    
    case WM_CLOSE:
        
        param = (FDOC*) GetWindowLong(win, GWL_USERDATA);
        
        if(param)
        {
            DocumentFrame_OnClose(param, win);
        }
        
        break;

    
    case WM_GETMINMAXINFO:
        param=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        DefMDIChildProc(win,msg,wparam,lparam);
        if(!param) break;
        return SendMessage(param->editwin,WM_GETMINMAXINFO,wparam,lparam);
    case WM_SIZE:
        param=(FDOC*)GetWindowLong(win,GWL_USERDATA);
        SetWindowPos(param->editwin,0,0,0,LOWORD(lparam),HIWORD(lparam),SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
        goto deflt;
    case WM_CREATE:
        
        param=(FDOC*)(((MDICREATESTRUCT*)(((CREATESTRUCT*)lparam)->lpCreateParams))->lParam);
        
        SetWindowLongPtr(win, GWLP_USERDATA, (LONG_PTR) param);
        
        ShowWindow(win,SW_SHOW);
        param->editwin=CreateSuperDialog(&z3_dlg,win,0,0,0,0,(long)param);
    default:
deflt:
        
        return DefMDIChildProc(win,msg,wparam,lparam);
    }
    
    return 0;
}

// =============================================================================

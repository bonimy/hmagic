
#include "structs.h"
#include "prototypes.h"

#include "HMagicUtility.h"

#include "TextLogic.h"

// =============================================================================

BOOL CALLBACK
textdlgproc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    text_buf_ty text_buf = { 0 };
    
    TEXTEDIT * ed;
    
    FDOC *doc;
    
    int i, j, k, l, m;
    
    char *b = 0;
    
    uint8_t * c = 0;
    
    unsigned char *rom;
    
    HWND hc;
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win, DWL_USER, lparam);
        
        ed = (TEXTEDIT*) lparam;
        
        CheckDlgButton(win, 3003, BST_CHECKED);
        
        ed->dlg = win;
        ed->num = 0;
        
        doc = ed->ew.doc;
        
        if( ! doc->t_loaded)
        {
            LoadText(doc);
        }
        
    updstrings:
        
        hc = GetDlgItem(win,3000);
        
        b = (char*) malloc(256);
        
        if(ed->num)
        {
            rom = doc->rom;
            
            j = ( ldle16b(rom + 0x74703) - 0xc705 ) >> 1;
            
            l = ldle16b(rom + 0x74703);
            
            for(i = 0; i < j; i++)
            {
                k = ldle16b_i(rom + 0x74705, i);
                
                memcpy(b+130,rom+l + 0x78000,k-l);
                
                *(short*)(b + 128) = k-l+2;
                
                Makeasciistring(doc, b, b + 128, 128);
                
                wsprintf(text_buf, "%03d: %s", i, b);
                
                SendMessage(hc, LB_ADDSTRING, 0, (LPARAM) text_buf);
                
                l = k;
            }
        }
        else
        {
            for(i = 0; i < doc->t_number; i++)
            {
                Makeasciistring(doc, b, doc->tbuf[i], 256);
                
                wsprintf(text_buf, "%03d: %s", i, b);
                
                SendMessage(hc, LB_ADDSTRING, 0, (LPARAM) text_buf);
            }
        }
        
        free(b);
        
        break;
    
    case WM_CLOSE:
        
        return 1;
    
    case WM_COMMAND:
        
        ed = (TEXTEDIT*) GetWindowLong(win, DWL_USER);
        
        if(!ed)
            break;
        
        doc = ed->ew.doc;
        
        rom = doc->rom;
        
        switch(wparam)
        {
        
        case 3000 | (LBN_DBLCLK << 16):
            
            i = SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
            
            if(i != -1)
            {
                b = malloc(2048);
                
                if(ed->num)
                {
                    k=((short*)(rom + 0x74705))[i];
                    l=((short*)(rom + 0x74703))[i];
                    memcpy(b+1026,rom+l + 0x78000,k-l);
                    *(short*)(b+1024)=k-l+2;
                    Makeasciistring(doc,b,b+1024,1024);
                }
                else
                {
                    Makeasciistring(doc, b, doc->tbuf[i], 2048);
                }
                
                SetDlgItemText(win, 3001, b);
                
                free(b);
            }
            else
            {
                SetDlgItemText(win, 3001, 0);
            }
            
            break;
        
        case 3002:
            
            i = SendDlgItemMessage(win,
                                   3000,
                                   LB_GETCURSEL,
                                   0,
                                   0);
            
            if(i != -1)
            {
                b = malloc(2048);
                
                GetDlgItemText(win, 3001, b, 2048);
                
                c = Makezeldastring(doc, b);
                
                hc = GetDlgItem(win, 3000);
                
                if(c)
                {
                    if(ed->num)
                    {
                        m = (*(short*)c) - 2;
                        j = *(unsigned short*) (rom + 0x77ffe + *(short*)(rom + 0x74703));
                        k = ((unsigned short*) (rom + 0x74705))[i];
                        l = ((unsigned short*) (rom + 0x74703))[i];
                        
                        if(j + m + l - k > 0xc8d9)
                        {
                            MessageBeep(0);
                            free(c);
                            free(b);
                            
                            break;
                        }
                        
                        memcpy(rom + 0x68000 + l + m, rom + 0x68000 + k, j - k);
                        memcpy(rom + 0x68000 + l, c + 2, m);
                        k -= l;
                        
                        l = ( ldle16b(rom + 0x74703) - 0xc703 ) >> 1;
                        
                        for(j = i + 1; j < l; j++)
                            ((short*) (rom + 0x74703))[j] += m - k;
                    }
                    else
                    {
                        free(doc->tbuf[i]);
                        doc->tbuf[i] = c;
                    }
                    
                    Makeasciistring(doc,b,c,256);
                    
                    wsprintf(text_buf,
                             "%03d: %s",
                             i,
                             b);
                    
                    if(ed->num)
                        free(c);
                    
                    SendMessage(hc, LB_DELETESTRING, i, 0);
                    SendMessage(hc, LB_INSERTSTRING, i, (LPARAM) text_buf);
                    SendMessage(hc, LB_SETCURSEL, i, 0);
                }
                else
                {
                    i = text_error - b;
                    
                    hc = GetDlgItem(win, 3001);
                    
                    SendMessage(hc, EM_SETSEL, i, i);
                    
                    SetFocus(hc);
                    
                    free(b);
                    
                    break;
                }
                
                free(b);
                
                doc->t_modf = 1;
            }
            
            break;
        
        case 3003:
            
            ed->num = 0;
            
            SendDlgItemMessage(win, 3000, LB_RESETCONTENT, 0, 0);
            
            goto updstrings;
        
        case 3004:
            
            ed->num = 1;
            
            SendDlgItemMessage(win, 3000, LB_RESETCONTENT, 0, 0);
            
            goto updstrings;
        }
        
        break;
    }
    
    return FALSE;
}

// =============================================================================

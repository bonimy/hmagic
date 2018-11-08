
    #include "structs.h"

    #include "Callbacks.h"

    #include "TrackerLogic.h"

    #include "prototypes.h"

// =============================================================================

BOOL CALLBACK
trackdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
    TRACKEDIT * ed;
    FDOC * doc;
    SCMD * sc;
    
    short*l;
    int i,j,k,m,n,o;
    
    HWND hc;
    
    (void) wparam;
    
    // -----------------------------
    
    switch(msg)
    {
    
    case WM_INITDIALOG:
        
        SetWindowLong(win,DWL_USER,lparam);
        ed=(TRACKEDIT*)lparam;
        ed->dlg=win;
        doc=ed->ew.doc;
        sc=doc->scmd;
        i=0;
        k=0;
        l=0;
        m=ed->ew.param&65535;
        o=ed->ew.param>>16;
        ed->ew.param=m;
        n=-1;
        
        for(j=doc->sr[m].first;j!=-1;j=sc[j].next)
        {
            if(i==k) k+=64,l=realloc(l,k*sizeof(SCMD));
            
            if(j==o) n=i;
            
            if(doc==mark_doc && m==mark_sr)
            {
                if(j==mark_first) mark_start=i;
                if(j==mark_last) mark_end=i;
            }
            
            l[i++]=j;
        }
        ed->debugflag=0;
        ed->tbl=l;
        ed->len=i;
        ed->sel=n;
        ed->csel=-1;
        ed->withcapt=0;
        SetDlgItemInt(win,3001,doc->sr[m].bank,0);
        hc=GetDlgItem(win,3000);
        SetWindowLong(hc,GWL_USERDATA,lparam);
        Updatesize(hc);
        
        break;
    
    case 3001 | (EN_CHANGE<<16):
        
        ed = (TRACKEDIT*) GetWindowLong(win,DWL_USER);
        
        ed->ew.doc->sr[ed->ew.param].bank = GetDlgItemInt(win,3001,0,0);
        
        break;
    }
    return FALSE;
}

// =============================================================================
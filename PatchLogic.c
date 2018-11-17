
    #include "structs.h"

    #include "prototypes.h"

    #include "Callbacks.h"

    #include "HMagicLogic.h"
    #include "HMagicUtility.h"

    #include "PatchLogic.h"

// =============================================================================

static BOOL
FileTimeEarlier(FILETIME const p_left,
                FILETIME const p_right)
{
    if(p_left.dwHighDateTime < p_right.dwHighDateTime)
    {
        return TRUE;
    }
    else if(p_left.dwHighDateTime == p_right.dwHighDateTime)
    {
        if(p_left.dwLowDateTime < p_right.dwLowDateTime)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

// =============================================================================

extern int
Buildpatches(FDOC * const doc)
{
    DWORD q = 0;
    
    int i, j, k, l, r, s;
    
    ASMHACK * mod;
    
    MSG msg;
    
    PATCH*p;
    
    HANDLE h, h2, h3[2];
    
    FILETIME tim,tim2;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO sti;
    
    PROCESS_INFORMATION pinfo;
    
    char *m, *t;
    int *n, *o = 0;
    
    text_buf_ty buf;
    
    // -----------------------------
    
    j = doc->nummod;
    
    mod = doc->modules;
    
    Removepatches(doc);
    
    if( ! j )
    {
        
    nomod:
        
        MessageBox(framewnd,
                   "No modules are loaded",
                   "Bad error happened",
                   MB_OK);
        
        return 0;
    }
    
    k = wsprintf(buf, "\"%s\"", asmpath);
    
    l = 0;
    
    for(i = 0; i < j; i++, mod++)
    {
        if(mod->flag & 1)
            continue;
        
        l++;
        
        t = buf + k;
        
        k += wsprintf(buf + k,
                      " \"%s\"",
                      mod->filename);
        
        m = strrchr(t,'.');
        
        if( ! m )
            continue;
        
        if( strrchr(t, '\\') > m )
            continue;
        
        if( _stricmp(m, ".asm") )
            continue;
        
        h = CreateFile(mod->filename,
                       GENERIC_READ,
                       0,
                       0,
                       OPEN_EXISTING,
                       0,
                       0);
        
        // Now check if it's an object file (*.obj) and openable.
        if(h == INVALID_HANDLE_VALUE)
        {
            wsprintf(buf, "Unable to open %s", mod->filename);
            
            MessageBox(framewnd, buf, "Bad error happened", MB_OK);
            
            return 1;
        }
        
        GetFileTime(h,0,0,&tim);
        
        CloseHandle(h);
        
        // \task Unsafe if the extension is shorter than 3 characters.
        *(int*)m = 'jbo.';
        
        h = CreateFile(mod->filename,
                       GENERIC_READ,
                       0,
                       0,
                       OPEN_EXISTING,
                       0,
                       0);
        
        if(h == (HANDLE) -1)
            goto buildnew;
        
        GetFileTime(h, 0, 0, &tim2);
        CloseHandle(h);
        
#if 0
        __asm
        {
            mov eax,tim.dwLowDateTime
            mov edx,tim.dwHighDateTime
            sub eax,tim2.dwLowDateTime
            sbb edx,tim2.dwHighDateTime
            jb nonew
        }
#else
        
        // \task Check that this actually works.
        if( FileTimeEarlier(tim, tim2) )
        {
            goto nonew;
        }
        
#endif
        
buildnew:
        // \task Poor form, fix this.
        *(int*)m = 'msa.';
nonew:;
    }
    
    if( ! l )
        goto nomod;
    
    sa.nLength = 12;
    sa.lpSecurityDescriptor = 0;
    sa.bInheritHandle = 1;
    
    h = CreateFileMapping((HANDLE) -1,
                          &sa,
                          PAGE_READWRITE,
                          0,
                          4096,
                          0);
    
    if( ! h )
    {
        
    nomem:
        
        MessageBox(framewnd,"Not enough memory.","Bad error happened",MB_OK);
        
        return 1;
    }
    
    n = MapViewOfFile(h,FILE_MAP_WRITE,0,0,0);
    
    if( ! n )
    {
        CloseHandle(h);
        
        goto nomem;
    }
    
    wsprintf(buf + k,
             " -l -h $%X -o HMTEMP.DAT",
             h);
    
    h2 = CreateFile("HMAGIC.ERR",
                    GENERIC_WRITE,
                    0,
                    &sa,
                    CREATE_ALWAYS,
                    0,
                    0);
    
    sti.cb = sizeof(sti);
    
    sti.lpReserved = 0;
    sti.lpDesktop = 0;
    sti.lpTitle = 0;
    sti.dwFlags = STARTF_USESTDHANDLES;
    sti.cbReserved2 = 0;
    sti.lpReserved2 = 0;
    sti.hStdInput = (HANDLE) -1;
    sti.hStdOutput = h2;
    sti.hStdError = h2;
    
    n[0] = (int) CreateEvent(&sa, 0, 0, 0);
    n[1] = (int) CreateEvent(&sa, 0, 0, 0);
    
    if
    (
        ! CreateProcess
        (
            0,
            buf,
            0,
            0,
            1,
            DETACHED_PROCESS,
            0,
            0,
            &sti,
            &pinfo
        )
    )
    {
        CloseHandle(h2);
        UnmapViewOfFile(n);
        
        CloseHandle(h);
        
        wsprintf(buf, "Unable to start %s", asmpath);
        
        MessageBox(framewnd, buf, "Bad error happened", MB_OK);
        
        return 1;
    }
    
    CloseHandle(h2);
    
    j = 0;
    k = 0;
    l = 0;
    
    h3[0] = (void*)(n[0]);
    h3[1] = pinfo.hProcess;
    
    for( ; ; )
    {
        switch( MsgWaitForMultipleObjects(2, h3, 0, INFINITE, QS_ALLEVENTS) )
        {
        
        case WAIT_OBJECT_0:
            
            switch( n[2] )
            {
            
            case 0:
                
                if(n[3] >= 3)
                    goto error;
                
                if(doc->numseg == 92)
                {
                    MessageBox(framewnd,"Too many segments","Bad error happened",MB_OK);
                    
                    break;
                }
                
                doc->numseg++;
                doc->segs[k] = 0;
                
                i = Changesize(doc, 0x802a4 + k, n[4]);
                
                n[3] = cpuaddr(i);
                
                k++;
                
                if( ! i )
                    goto error;
                
                goto regseg;
            
            case 1:
                
                if(doc->patches)
                    goto error;
                
                i = n[6];
                
                if(n[3] < 3)
                {
                    i = romaddr(i);
                    
                    if(i >= 0x100000)
                    {
                        
                    error:
                        
                        n[2] = 1;
                        
                        break;
                    }
                    
                    l++;
                    
                regseg:
                    
                    if( ! (j & 255) )
                        o = realloc(o, (j + 512) << 2);
                    
                    o[j]     = i;
                    o[j + 1] = n[4];
                    o[j + 2] = n[5];
                    o[j + 3] = n[2];
                    
                    j += 4;
                }
                
                n[2] = 0;
                
                break;
            
            case 2:
                
                doc->patches = malloc( l * sizeof(PATCH) );
                
                for(i = 0; i < j; i++)
                {
                    doc->patches[i].len  = 0;
                    doc->patches[i].addr = 0;
                    doc->patches[i].pv   = 0;
                }
                
                break;
            
            default:
                
                n[2] = 1;
            }
            
            SetEvent( (HANDLE) n[1] );
            
            break;
        
        case WAIT_OBJECT_0 + 1:
            
            goto done;
        
        case WAIT_OBJECT_0 + 2:
            
            while( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
            {
                if(msg.message == WM_QUIT)
                    break;
                
                ProcessMessage(&msg);
            }
        }
    }
    
done:
    
    GetExitCodeProcess(pinfo.hProcess,&q);
    
    if(q)
    {
        DialogBoxParam(hinstance,
                       (LPSTR) IDD_DIALOG23,
                       framewnd,
                       errorsproc,
                       0);
        
    errors:
        
        Removepatches(doc);
    }
    else
    {
        DWORD read_bytes = 0;
        
        DeleteFile("HMAGIC.ERR");
        
        h2 = CreateFile("HMTEMP.DAT",
                        GENERIC_READ,
                        0,
                        0,
                        OPEN_EXISTING,
                        FILE_FLAG_DELETE_ON_CLOSE,
                        0);
        
        if( h2 == (HANDLE) -1 )
        {
            MessageBox(framewnd,
                       "Unable to apply patch",
                       "Bad error happened",
                       MB_OK);
            
            q = 1;
            
            goto errors;
        }
        
        p = doc->patches = malloc(l * sizeof(PATCH) );
        
        doc->numpatch = l;
        
        s = j;
        j >>= 2;
        
        for(r = 0; r < j; r++)
        {
            for(i = 0; i < s; i += 4)
                if(o[i + 2] == r)
                    break;
            
            if( o[i + 3] )
            {
                p->addr = o[i];
                p->len  = o[i+1];
                
                p->pv = malloc(p->len);
                
                memcpy(p->pv,doc->rom+p->addr,p->len);
                
                p++;
            }
            
            ReadFile(h2,
                     doc->rom + o[i],
                     o[i+1],
                     &read_bytes,
                     0);
        }
        
        CloseHandle(h2);
    }
    
    free(o);
    
    CloseHandle(pinfo.hThread);
    CloseHandle(pinfo.hProcess);
    
    UnmapViewOfFile(n);
    CloseHandle(h);
    
    if( ! q )
    {
        GetSystemTimeAsFileTime( &(doc->lastbuild) );
        
        doc->modf   = 1;
        doc->p_modf = 0;
    }
    
    return q;
}

// =============================================================================

    extern void
    Removepatches(FDOC * const doc)
    {
        size_t i;
        
        size_t const num_patches = doc->numpatch;
        
        // -----------------------------
        
        for(i = 0; i < num_patches; i += 1)
        {
            PATCH const * const p = &doc->patches[i];
            
            // -----------------------------
            
            memcpy(doc->rom + p->addr,
                   p->pv,
                   p->len);
            
            free(p->pv);
        }
        
        for(i = 0; i < doc->numseg; i += 1)
        {
            Changesize(doc, 676 + i, 0);
        }
        
        doc->numpatch = 0;
        
        free(doc->patches);
        
        doc->patches = 0;
        doc->numseg  = 0;
    }

// =============================================================================

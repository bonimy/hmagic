
    #include "structs.h"

    #include "prototypes.h"

    #include "Wrappers.h"

    #include "Callbacks.h"

    #include "HMagicLogic.h"
    #include "HMagicUtility.h"

    #include "TextLogic.h"

    #include "PatchEnum.h"
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

    static char *
    Path_GetExtension
    (
        CP2C(AString) p_str
    )
    {
        char * const last_dot = strrchr(p_str->m_text, '.');
        char * const last_bs  = strrchr(p_str->m_text, '\\');
        
        // -----------------------------
        
        if(last_dot)
        {
            if( last_bs && (last_bs < last_dot) )
            {
                return (last_dot + 1);
            }
        }

        return NULL;
    }

// =============================================================================

    static int
    Path_ChangeExtension
    (
        CP2(AString) p_str,
        CP2C(char)   p_new_ext
    )
    {
        char * ext = Path_GetExtension(p_str);
        
        // -----------------------------
        
        if(ext)
        {
            p_str->m_len -= ( strlen(ext) );
            
            ext[0] = '\0';
            
            return AString_AppendString(p_str, p_new_ext);
        }
        
        return -1;
    }

// =============================================================================

    static int
    PatchLogic_Form_ASM_Command
    (
        CP2C(FDOC)   p_doc,
        CP2(AString) p_buf
    )
    {
        int i = 0;
        int l = 0;
        
        AString arg_path = { 0 };
        AString err_msg  = { 0 };
        
        HANDLE h = INVALID_HANDLE_VALUE;
        
        // -----------------------------
        
        AString_AppendFormatted(p_buf, "\"%s\"", asmpath);
        
        for(i = 0; i < p_doc->nummod; i += 1)
        {
            CP2C(ASMHACK) mod = &p_doc->modules[i];
            
            char const * ext_pos = NULL;
            
            
            FILETIME asm_time;
            FILETIME obj_time;
            
            // -----------------------------
            
            // Presumably some sort of flag to disable the patch.
            if(mod->flag & FLG_Patch_Disabled)
            {
                continue;
            }
            
            // Duplicate each filename specified in the dialog.
            AString_InitFromNativeString(&arg_path, mod->filename);
            
            ext_pos = Path_GetExtension(&arg_path);
            
            // FSNASM will complain about and not accept file with extensions
            // other than .asm or .obj
            if( Is(ext_pos, NULL) )
            {
                continue;
            }
            else if
            (
                _stricmp(ext_pos, "asm")
             && _stricmp(ext_pos, "obj")
            )
            {
                continue;
            }
            
            l += 1;
            
            // Now check if it's an object file (*.obj) and openable.
            if( ! HM_FileExists(arg_path.m_text, &h) )
            {
                AString_InitFormatted
                (
                    &err_msg,
                    "Unable to open %s",
                    arg_path.m_text
                );
                
                MessageBox(framewnd,
                           err_msg.m_text,
                           "Bad error happened",
                           MB_OK);
                
                continue;
            }
            
            GetFileTime(h, 0, 0, &asm_time);
            
            CloseHandle(h);
            
            Path_ChangeExtension(&arg_path, "obj");
            
            if( ! HM_FileExists(arg_path.m_text, &h) )
            {
                goto buildnew;
            }
            
            // Get file time of ... object file?
            GetFileTime(h, 0, 0, &obj_time);
            
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
            
            // Checking if the asm file's modified time is earlier than the
            // object file's modified time. If so, we know that the asm file
            // has already been assembled into object code.
            if( FileTimeEarlier(asm_time, obj_time) )
            {
                goto obj_up_to_date;
            }
            
    #endif
            
        buildnew:
            
            Path_ChangeExtension(&arg_path, "asm");
            
        obj_up_to_date:
            
            // Add the patch to the command line string, and surround it in
            // quotes.
            AString_AppendFormatted
            (
                p_buf,
                " \"%s\"",
                arg_path.m_text
            );
            
        }
        
        AString_Free(&arg_path);
        AString_Free(&err_msg);
        
        return l;
    }

// =============================================================================

    extern int
    Patch_Build(FDOC * const doc)
    {
        int * n, * o = 0;
        
        int i;
        
        int j2 = 0;
        
        int k, l, r, s;
        
        DWORD q = 0;
        
        MSG msg;
        
        PATCH * p;
        
        HANDLE h, h2;
        
        HANDLE h3[2];
        
        SECURITY_ATTRIBUTES sa;
        
        STARTUPINFO sti;
        
        PROCESS_INFORMATION pinfo;
        
        AString buf = { 0 };
        
        AString err_msg = { 0 };
        
        // -----------------------------
        
        // Clear out any patches that have been loaded. We're going to add them
        // back. Note that "loaded" means they've been assigned a segment
        // of some sort in the rom. The ones to be added are the ones
        // supplied in the patch dialog.
        Removepatches(doc);
        
        if( ! doc->nummod )
        {
            
        nomod:
            
            MessageBox(framewnd,
                       "No modules are loaded",
                       "Bad error happened",
                       MB_OK);
            
            goto cleanup;
        }
        
        l = PatchLogic_Form_ASM_Command(doc, &buf);
        
        if( ! l )
        {
            goto nomod;
        }
        
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
            
            MessageBox(framewnd,
                       "Not enough memory.",
                       "Bad error happened",
                       MB_OK);
            
            q = 1;
            
            goto cleanup;
        }
        
        n = MapViewOfFile(h, FILE_MAP_WRITE, 0, 0, 0);
        
        if( ! n )
        {
            CloseHandle(h);
            
            goto nomem;
        }
        
        AString_AppendFormatted
        (
            &buf,
            " -l -h $%X -o HMTEMP.DAT",
            h
        );
        
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
        
        // Launch the assembler and wait for it to complete.
        if
        (
            ! CreateProcess
            (
                0,
                buf.m_text,
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
            
            AString_InitFormatted
            (
                &err_msg,
                "Unable to start %s",
                asmpath
            );
            
            MessageBox(framewnd,
                       err_msg.m_text,
                       "Bad error happened",
                       MB_OK);
            
            q = 1;
            
            goto cleanup;
        }
        
        CloseHandle(h2);
        
        j2 = 0;
        k  = 0;
        l  = 0;
        
        h3[0] = (void*) (n[0]);
        h3[1] = pinfo.hProcess;
        
        for( ; ; )
        {
            switch( MsgWaitForMultipleObjects(2, h3, 0, INFINITE, QS_ALLEVENTS) )
            {
            
            case WAIT_OBJECT_0:
                
                switch( n[2] )
                {
                
                // Add a segment?
                case 0:
                    
                    // Type of segment?
                    if( n[3] >= 3 )
                    {
                        goto error;
                    }
                    
                    if(doc->numseg == 92)
                    {
                        MessageBox(framewnd,
                                   "Too many segments",
                                   "Bad error happened",
                                   MB_OK);
                        
                        break;
                    }
                    
                    doc->numseg++;
                    doc->segs[k] = 0;
                    
                    i = Changesize(doc, 0x802a4 + k, n[4]);
                    
                    n[3] = cpuaddr(i);
                    
                    k += 1;
                    
                    if( ! i )
                        goto error;
                    
                    goto regseg;
                
                case 1:
                    
                    if(doc->patches)
                    {
                        goto error;
                    }
                    
                    i = n[6];
                    
                    if( n[3] < 3 )
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
                        
                        if( ! (j2 & 0xff) )
                            o = realloc(o, (j2 + 512) << 2);
                        
                        o[j2 ]     = i;
                        o[j2 + 1] = n[4];
                        o[j2 + 2] = n[5];
                        o[j2 + 3] = n[2];
                        
                        j2 += 4;
                    }
                    
                    // Is FSNASM waiting for this as their signal to continue?
                    n[2] = 0;
                    
                    break;
                
                case 2:
                    
                    doc->patches = malloc( l * sizeof(PATCH) );
                    
                    for(i = 0; i < j2; i++)
                    {
                        doc->patches[i].len  = 0;
                        doc->patches[i].addr = 0;
                        doc->patches[i].pv   = 0;
                    }
                    
                    break;
                
                default:
                    
                    // Does FSNASM interpret this as
                    // "received, but not handled"?
                    n[2] = 1;
                }
                
                // Is FSNASM waiting for this to be signaled before it
                // continues?
                SetEvent( (HANDLE) n[1] );
                
                break;
            
            case WAIT_OBJECT_0 + 1:
                
                goto done;
            
            case WAIT_OBJECT_0 + 2:
                
                while( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
                {
                    if(msg.message == WM_QUIT)
                    {
                        break;
                    }
                    
                    ProcessMessage(&msg);
                }
            }
        }
        
    done:
        
        GetExitCodeProcess(pinfo.hProcess, &q);
        
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
            
            // -----------------------------
            
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
            
            s = j2;
            
            j2 >>= 2;
            
            for(r = 0; r < j2; r++)
            {
                for(i = 0; i < s; i += 4)
                {
                    if(o[i + 2] == r)
                    {
                        break;
                    }
                }
                
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
        
    cleanup:
        
        AString_Free(&buf);

        if(o)
        {
            free(o);
        }
        
        return q;
    }

// =============================================================================

    extern void
    Removepatches(FDOC * const doc)
    {
        size_t i = 0;
        
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
        
        if(doc->patches)
        {
            free(doc->patches);
            
            doc->patches = 0;
        }
        
        doc->numseg  = 0;
    }

// =============================================================================

    extern int
    Doc_FreePatchInputs(CP2(FDOC) p_doc)
    {
        int i = 0;
        
        // -----------------------------
        
        for(i = 0; i < p_doc->nummod; i += 1)
        {
            CP2(ASMHACK) asmh = &p_doc->modules[i];
            
            // -----------------------------
            
            if(asmh->filename)
            {
                free(asmh->filename);
                
                asmh->filename = NULL;
            }
        }
        
        if(p_doc->modules)
        {
            free(p_doc->modules);
            
            p_doc->modules = NULL;
        }
        
        return i;
    }


// =============================================================================

/**
    From NetSplit on acmlmboards. Dumping it here because information on fsnasm
    is limited and should be expanded.

Given that FSNASM seems to have fallen off the face of the planet, I've uploaded my copy: http://acmlm.kafuka.org/uploader/get.php?id=3213

I might be remembering wrong, but I think this one is slightly newer than the Hyrule Magic one, allowing you to use tabs without exploding. I don't have a readme for it; I'm not sure if I ever did. The author doesn't seem to have a readme anymore, either. So, I'll go ahead and note down everything I know about it and what I could get from the author last time I talked to him.

Running it on the command line yields:

FSNASM (C) 1999,2001-2002 Sephiroth of Gigasoft
Usage: FSNASM [option] srcfile
Options:
-l: Produce LO-ROM
-o : Select output filename
    <MoN> (Update) There is also a parameter -h which expects an 8-bit hex value
    referring to a memory mapped file. Example: -h $22334411 If the handle to
    the memory mapped file has value 0x22334411. This appears to put FSNASM
    into an interprocess operation mode where it sends data to a client
    application and waits before proceeding. Hyrule Magic creates a memory
    mapped file that is sized at 4096 bytes, though it's not yet clear if
    FSNASM needs all of that, or whether it was just a convenient sounding
    size to use (that's 4 Kbytes).

You can use .b, .w, and .l to be explicit about what version of a particular instruction you want (so, for example, LDA.w). If you don't use those, the assembler will make its best guess.

Commands are: charset, defchar, endb, base, data, block, align, end, code, global, zram, incbin and org

"block" corresponds to resb in nasm
data, code, ram, zram and org define sections
I believe 'end' ends these segments.

defchar "EXAMPLECHARSET" "AZ"=1,"az"=27,"09"=118," "=0,s".'!?:,-;&/"=63,"\n"=129
charset "EXAMPLECHARSET"
;You can use \ to escape, for characters like "

ZRAM corresponds to addresses 0-$1fff i think, or maybe it's 0-$ff
    <MoN> (Update) Zram is the range $000000-$001fff. seph3 probably called it
    that because it's often accessed from bank zero and mirrors
    $7e0000-$7e1fff. I have not encountered a direct page ram segment
    declaration mechanism yet.

the base directive tells it where the code really is
endb ends the base directive

If you're coding for the NES, you can use org $c00000 to put the code at the beginning of the ROM. With -l (the LoROM option), that would be org $808000

You can use incbin to include data. For example:
incbin "NESFONT.dat"
to include some NESFONT.dat file, which would presumably be your font graphics.

Use dc.x to include data, where x is b, w, or l. Data can be a mix of numbers (hex ($) or dec) and labels (or operations on labels, such as yourlabel-1)


I've used this for NES development before and it works well. I had to do org $c00000 at the start, define the header (example: dc.b "NES",26,2,1,1,0,0,0,0,0,0,0,0,0), base $8000, and then you're ready to code (just be sure to endb, end, and set up your interrupt vectors at the end of the file). You can do variable = $blah to set up your variables. I think labels require a colon after the declaration unless they're naming a table, in which case it's just yourlabel dc.x yourdata.

Hopefully someone finds this useful. 
*/
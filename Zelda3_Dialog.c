
    #include "structs.h"
    #include "prototypes.h"

    #include "Callbacks.h"
    #include "GdiObjects.h"

    #include "Wrappers.h"

    #include "Zelda3_Enum.h"

    #include "OverworldEdit.h"

    #include "DungeonEnum.h"
    #include "DungeonLogic.h"

    // For the names of the various editable misc screens.
    #include "ScreenEditorLogic.h"

    #include "LevelMapLogic.h"

// =============================================================================

    SD_ENTRY z3_sd[] =
    {
        {
            "HMagic-TreeView",
            "",
            0, 0, 0, 0,
            ID_Z3Dlg_TreeView,
            (
                WS_VISIBLE
              | WS_TABSTOP
              | WS_BORDER
              | WS_CHILD
              | TVS_HASBUTTONS
              | TVS_LINESATROOT
              | TVS_HASLINES
              | TVS_SHOWSELALWAYS
              | TVS_DISABLEDRAGDROP
            ),
            WS_EX_CLIENTEDGE,
            FLG_SDCH_FOWH
        },
    };

    enum
    {
        NUM_Z3Dlg_Controls = MACRO_ArrayLength(z3_sd)
    };

// =============================================================================

    SUPERDLG z3_dlg =
    {
        "",
        z3dlgproc,
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        60, 60,
        NUM_Z3Dlg_Controls,
        z3_sd
    };

// =============================================================================

    static char const * pal_text[] =
    {
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
    
    static char const * locs_text[] =
    {
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
    
    static char const pal_num[] =
    {
        4,3,3,6,2,20,16,24,20,2,2,16,18,1,1,1
    };

// =============================================================================

    typedef
    struct tag_TreeViewNode
    {
        HTREEITEM node;
        
        size_t m_child_count;
        
        struct tag_TreeViewNode * m_children;
        
        struct tag_TreeViewNode * m_next_sibling;
        
    } TreeViewNode;

// =============================================================================

    static TreeViewNode default_tvn = { 0 };

// =============================================================================

    typedef
    struct
    {
        TreeViewNode ow_root;
        TreeViewNode dung_root;
        TreeViewNode music_root;
        TreeViewNode world_map_root;
        TreeViewNode pal_root;
        TreeViewNode palace_map_root;
        
    } HM_Z3Dlg_TreeView;

// =============================================================================

    extern HTREEITEM
    HM_TreeView_InsertItem
    (
        HWND                   const p_treeview,
        HTREEITEM              const p_parent,
        TVINSERTSTRUCT const * const p_tvis
    )
    {
        BOOL has_children = FALSE;
        
        HTREEITEM hitem = NULL;
        
        TVITEM parent_tv_item = { 0 };
        
        // Copy so we can adjust the parent field.
        TVINSERTSTRUCT adj_tvis = p_tvis[0];
        
        // -----------------------------
        
        // So that the GUI setting of having children will be considered valid
        // when passed for insertion.
        adj_tvis.item.mask |= (TVIF_CHILDREN);
        
        // A newly added node can't possibly have any children.
        adj_tvis.item.cChildren = 0;
        
        // Explicitly define the relationship between the node we wish to
        // insert and its parent.
        adj_tvis.hParent = p_parent;
        
        hitem = (HTREEITEM) SendMessage
        (
            p_treeview,
            TVM_INSERTITEM,
            0,
            (LPARAM) &adj_tvis
        );
        
        if( (p_parent == NULL) || (p_parent == TVI_ROOT) )
        {
            // Can't retrieve the root's item handle anyway, so we're done.
            return hitem;
        }
        
        HM_TreeView_GetItem(p_treeview, p_parent, &parent_tv_item);
        
        // Simultaneously checks for success of insertion of the item and
        // also helps us decide how to set the apperance of the parent node.
        has_children = HM_TreeView_HasChildren(p_treeview, p_parent);
        
        parent_tv_item.cChildren = has_children ? 1 : 0;
        
        HM_TreeView_SetItem(p_treeview, p_parent, &parent_tv_item);
        
        return hitem;
    }

// =============================================================================

    // \task[med] Move these functions to Wrappers.c, or begin subdividing the
    // wrapper functions into logical units if that translation unit gets too big.
    extern HTREEITEM
    HM_TreeView_InsertSubroot
    (
        HWND                   const p_treeview,
        HTREEITEM              const p_parent,
        char           const * const p_label
    )
    {
        BOOL has_children = FALSE;
        
        HTREEITEM hitem = NULL;
        
        TVITEM parent_tv_item = { 0 };
        
        // Copy so we can adjust the parent field.
        TVINSERTSTRUCT tvis = { 0 };
        
        TVITEM * const item = &tvis.item;
        
        // -----------------------------
        
        tvis.hParent      = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        
        // So that the configurations that follow this instruction are taken
        // into consideration when inserting the item.
        item->mask =
        (
            TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT | TVIF_STATE
        );
        
        item->cChildren = 0;
        item->lParam    = 0;
        
        item->pszText    = (LPTSTR) p_label;
        
        item->state      = 0;
        item->stateMask  = TVIS_BOLD;
        
        hitem = HM_TreeView_InsertItem(p_treeview, p_parent, &tvis);
        
        if( (p_parent != NULL) && (p_parent != TVI_ROOT) )
        {
            HM_TreeView_GetItem(p_treeview,
                                p_parent,
                                &parent_tv_item);
            
            // Simultaneously checks for success of insertion of the item and
            // also helps us decide how to set the apperance of the parent node.
            has_children = HM_TreeView_HasChildren(p_treeview,
                                                   p_parent);
            
            parent_tv_item.cChildren = has_children ? 1 : 0;
            
            HM_TreeView_SetItem(p_treeview,
                                p_parent,
                                &parent_tv_item);
        }
        
        return hitem;
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_InsertRoot
    (
        HWND         const p_treeview,
        char const * const p_label
    )
    {
        return HM_TreeView_InsertSubroot(p_treeview, TVI_ROOT, p_label);
    }

// =============================================================================

    extern BOOL
    HM_TreeView_DeleteItem
    (
        HWND      const p_treeview,
        HTREEITEM const p_item
    )
    {
        BOOL r = (BOOL) SendMessage
        (
            p_treeview,
            TVM_INSERTITEM,
            0,
            (LPARAM) p_item
        );
        
        // -----------------------------
        
        return r;
    }

// =============================================================================

    /**
        \task[high] The name of this is misleading b/c it currently only deals in
        whether the node is graphically considered to have child nodes.
    */
    extern BOOL
    HM_TreeView_GetItem
    (
        HWND        const p_treeview,
        HTREEITEM   const p_item_handle,
        TVITEM    * const p_item
    )
    {
        LRESULT r = 0;
        
        // -----------------------------
        
        p_item->mask = TVIF_CHILDREN;
        
        p_item->hItem = p_item_handle;
        
        r = SendMessage
        (
            p_treeview,
            TVM_GETITEM,
            0,
            (LPARAM) p_item
        );
        
        // (The message above returns what is intended to be interpreted as
        // a boolean)
        return (r == 1);
    }

// =============================================================================

    extern LPARAM
    HM_TreeView_GetItemParam
    (
        HWND      const p_treeview,
        HTREEITEM const p_item_handle
    )
    {
        LRESULT r = 0;
        
        TVITEM item = { 0 };
        
        // -----------------------------
        
        item.mask = TVIF_PARAM;
        
        item.hItem = p_item_handle;
        
        r = SendMessage
        (
            p_treeview,
            TVM_GETITEM,
            0,
            (LPARAM) &item
        );
        
        // (The message above returns what is intended to be interpreted as
        // a boolean)
        return item.lParam;
    }

// =============================================================================

    extern BOOL
    HM_TreeView_SetItem
    (
        HWND        const p_treeview,
        HTREEITEM   const p_item_handle,
        TVITEM    * const p_item
    )
    {
        LRESULT r = 0;
        
        // -----------------------------
        
        p_item->mask = TVIF_CHILDREN;
        
        p_item->hItem = p_item_handle;
        
        r = SendMessage
        (
            p_treeview,
            TVM_SETITEM,
            0,
            (LPARAM) p_item
        );
        
        // (The message above returns what is intended to be interpreted as
        // a boolean)
        return (r == 1);
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_InsertChild
    (
        HWND      const p_treeview,
        HTREEITEM const p_parent
    )
    {
        TVITEM item = { 0 };
        
        HM_TreeView_GetItem(p_treeview,
                            p_parent,
                            &item);
        
        if(item.cChildren != 1)
        {
            item.cChildren = 1;
        }
        
        // \task[high] Finish this implementation. this is just placeholder stuff.
        return TVI_ROOT;
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_GetFirstChild
    (
        HWND      const p_treeview,
        HTREEITEM const p_node
    )
    {
        HTREEITEM r;
        
        // -----------------------------
        
        r = (HTREEITEM) SendMessage
        (
            p_treeview,
            TVM_GETNEXTITEM,
            TVGN_CHILD,
            (LPARAM) p_node
        );
        
        return r;
    }

// =============================================================================

    /**
        Convenience function that merely returns a boolean based on whether
        the tree view node has no children (FALSE) or at least one child
        (TRUE)
    */
    extern BOOL
    HM_TreeView_HasChildren
    (
        HWND      const p_treeview,
        HTREEITEM const p_node
    )
    {
        HTREEITEM r = HM_TreeView_GetFirstChild(p_treeview, p_node);
        
        // -----------------------------
        
        return (r != NULL);
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_GetParent
    (
        HWND      const p_treeview,
        HTREEITEM const p_node
    )
    {
        HTREEITEM r;
        
        // -----------------------------
        
        r = (HTREEITEM) SendMessage
        (
            p_treeview,
            TVM_GETNEXTITEM,
            TVGN_PARENT,
            (LPARAM) p_node
        );
        
        return r;
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_GetNextSibling
    (
        HWND      const p_treeview,
        HTREEITEM const p_child
    )
    {
        HTREEITEM r;
        
        // -----------------------------
        
        r = (HTREEITEM) SendMessage
        (
            p_treeview,
            TVM_GETNEXTITEM,
            TVGN_NEXT,
            (LPARAM) p_child
        );
        
        return r;
    }

// =============================================================================

    extern size_t
    HM_TreeView_CountChildren
    (
        HWND      const p_treeview,
        HTREEITEM const p_parent
    )
    {
        size_t count = 0;
         
        HTREEITEM r = 0;
        
        // -----------------------------
        
        // Check if there are any child nodes at all.
        r = HM_TreeView_GetFirstChild(p_treeview, p_parent);
        
        while(r != NULL)
        {
            // Please note this is counting the most recent success in locating
            // the first child node or a subsequent sibling node.
            count += 1;
            
            r = HM_TreeView_GetNextSibling(p_treeview, r);
        }

        return count;
    }

// =============================================================================

    extern BOOL
    HM_TreeView_SetParam
    (
        HWND      const p_treeview,
        HTREEITEM const p_node,
        LPARAM    const p_lp
    )
    {
        TVITEM item = { 0 };
        
        LRESULT r_get = 0;
        LRESULT r_set = 0;
        
        // -----------------------------
        
        item.hItem = p_node;
        
        item.mask = (TVIF_HANDLE | TVIF_PARAM);
        
        r_get = SendMessage(p_treeview,
                            TVM_GETITEM,
                            0,
                           (LPARAM) &item);
        
        item.lParam = p_lp;
        
        r_set = SendMessage(p_treeview,
                            TVM_SETITEM,
                            0,
                            (LPARAM) &item);
        
        // (The message above returns what is intended to be interpreted as
        // a boolean)
        return (r_get & r_set);

        

    }

// =============================================================================

    extern BOOL
    HM_TreeView_SelectItem
    (
        HWND      const p_treeview,
        HTREEITEM const p_item
    )
    {
        BOOL r = FALSE;
        
        // -----------------------------
        
        r = SendMessage
        (
            p_treeview,
            TVM_SELECTITEM,
            TVGN_CARET,
            (LPARAM) p_item
        );
        
        return r;
    }

// =============================================================================

    extern HTREEITEM
    HM_TreeView_GetNextSelected
    (
        HWND const p_treeview
    )
    {
        HTREEITEM item = (HTREEITEM) SendMessage
        (
            p_treeview,
            TVM_GETNEXTITEM,
            TVGN_CARET,
            HM_NullLP()
        );
        
        // -----------------------------
        
        return item;
    }

// =============================================================================

    BOOL
    Z3Dlg_OnInit
    (
        HWND   const p_win,
        LPARAM const p_lp
    )
    {
        char buf[0x200];
        
        int i = 0;
        int j = 0;
        int k = 0;
        int l = 0;
        
        HWND const hc = GetDlgItem(p_win, ID_Z3Dlg_TreeView);
        
        TVINSERTSTRUCT tvi = { 0 };
        
        HTREEITEM ow_root;
        HTREEITEM dung_root;
        HTREEITEM music_root;
        HTREEITEM world_map_root;
        HTREEITEM text_root;
        HTREEITEM pal_root;
        HTREEITEM palace_map_root;
        HTREEITEM dung_props_root;
        HTREEITEM menu_root;
        
        HTREEITEM persp_root;
        HTREEITEM player_gfx_root;
        HTREEITEM patch_root;
        HTREEITEM gfx_schemes_root;
        
        // -----------------------------
        
        // \note (This is an FDOC pointer under the hood)
        SetWindowLongPtr(p_win, DWLP_USER, p_lp);
        
        tvi.hInsertAfter = TVI_LAST;
        
        tvi.item.mask =
        (
            TVIF_CHILDREN
          | TVIF_PARAM
          | TVIF_TEXT
          | TVIF_STATE
        );
        
        tvi.item.state     = 0;
        tvi.item.stateMask = TVIS_BOLD;
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        ow_root = HM_TreeView_InsertRoot(hc, "Overworld");
        
        tvi.item.pszText = buf;
        
        for(i = 0; i < 160; i += 1)
        {
            tvi.item.lParam = (i + 0x20000);
            
            wsprintf(buf, "Area %02X - %s", i, area_names.m_lines[i]);
            
            HM_TreeView_InsertItem(hc, ow_root, &tvi);
        }
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        dung_root = HM_TreeView_InsertRoot(hc, "Dungeons");
        
        tvi.item.pszText = buf;
        
        for(i = 0; i < 133; i += 1)
        {
            tvi.item.lParam = (i + 0x30000);
            
            wsprintf(buf,
                     "Entrance %02X - %s",
                     i,
                     entrance_names.m_lines[i]);
            
            HM_TreeView_InsertItem(hc, dung_root, &tvi);
        }
        
        for(i = 0; i < 7; i += 1)
        {
            tvi.item.lParam = (i + 0x30085);
            
            wsprintf(buf, "Starting location %02X", i);
            
            HM_TreeView_InsertItem(hc, dung_root, &tvi);
        }
        
        for(i = 0; i < 19; i += 1)
        {
            tvi.item.lParam = (i + 0x3008c);
            
            wsprintf(buf, "Overlay %02X", i);
            
            HM_TreeView_InsertItem(hc, dung_root, &tvi);
        }
        
        for(i = 0; i < 8; i += 1)
        {
            tvi.item.lParam = (i + 0x3009f);
            
            wsprintf(buf, "Layout %02X", i);
            
            HM_TreeView_InsertItem(hc, dung_root, &tvi);
        }
        
        tvi.item.lParam = 0x300a7;
        tvi.item.pszText = "Watergate overlay";
        
        HM_TreeView_InsertItem(hc, dung_root, &tvi);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        music_root = HM_TreeView_InsertRoot(hc, "Music");
        
        tvi.item.pszText = buf;
        
        for(i = 0; i < 3; i += 1)
        {
            tvi.item.lParam = (i + 0x40000);
            
            wsprintf(buf, "Bank %d", i + 1);
            
            HM_TreeView_InsertItem(hc, music_root, &tvi);
        }
        
        tvi.item.lParam  = 0x40003;
        tvi.item.pszText = "Waves";
        
        HM_TreeView_InsertItem(hc, music_root, &tvi);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        world_map_root = HM_TreeView_InsertRoot(hc, "World maps");
        
        tvi.item.pszText = "Normal world";
        tvi.item.lParam  = 0x60000;
        
        HM_TreeView_InsertItem(hc, world_map_root, &tvi);
        
        tvi.item.pszText = "Dark world";
        tvi.item.lParam  = 0x60001;
        
        HM_TreeView_InsertItem(hc, world_map_root, &tvi);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        text_root = HM_TreeView_InsertRoot(hc, "Monologue");
        
        HM_TreeView_SetParam(hc, text_root, 0x50000);
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        pal_root = HM_TreeView_InsertRoot(hc, "Palettes");
        
        k = 0;
        
        for(i = 0; i < 16; i += 1)
        {
            HTREEITEM sub_root = NULL;
            
            // -----------------------------
            
            l = pal_num[i];
            
            sub_root = HM_TreeView_InsertSubroot(hc,
                                                 pal_root,
                                                 pal_text[i]);
            
            tvi.item.pszText = buf;
            
            for(j = 0; j < l; j++)
            {
                tvi.item.lParam = (k + 0x70000);
                
                wsprintf(buf,
                         "%s pal %d",
                         pal_text[i],
                         j);
                
                HM_TreeView_InsertItem(hc, sub_root, &tvi);
                
                k += 1;
            }
        }
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        palace_map_root = HM_TreeView_InsertRoot(hc, "Dungeon Maps");
        
        for(i = 0; i < 14; i += 1)
        {
            tvi.item.lParam = (i + 0x80000);
            
            // Cast is to suppress a warning that is not very useful here.
            tvi.item.pszText = (LPSTR) level_str[i + 1];
            
            HM_TreeView_InsertItem(hc, palace_map_root, &tvi);
        }
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        dung_props_root = HM_TreeView_InsertRoot(hc, "Dungeon properties");
        
        for(i = 0; i < 12; i += 1)
        {
            tvi.item.lParam = (i + 0x90000);
            
            // Cast is to suppress a warning that is not very useful here.
            tvi.item.pszText = (LPSTR) locs_text[i];
            
             HM_TreeView_InsertItem(hc, dung_props_root, &tvi);
        }
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        menu_root = HM_TreeView_InsertRoot(hc, "Menu screens");
        
        for(i = 0; i < 11; i += 1)
        {
            tvi.item.lParam = (i + 0xa0000);
            
            // Cast is to suppress a warning that is not very useful here.
            tvi.item.pszText = (LPSTR) screen_text[i];
            
            HM_TreeView_InsertItem(hc, menu_root, &tvi);
        }
        
        // ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
        
        persp_root       = HM_TreeView_InsertRoot(hc, "3D Objects");
        player_gfx_root  = HM_TreeView_InsertRoot(hc, "Link's Graphics");
        patch_root       = HM_TreeView_InsertRoot(hc, "ASM hacks");
        gfx_schemes_root = HM_TreeView_InsertRoot(hc, "Graphic schemes");
        
        HM_TreeView_SetParam(hc, persp_root, 0xb0000);
        HM_TreeView_SetParam(hc, player_gfx_root, 0xc0000);
        HM_TreeView_SetParam(hc, patch_root, 0xd0000);
        HM_TreeView_SetParam(hc, gfx_schemes_root, 0xe0000);
        
        return FALSE;
    }

// =============================================================================

BOOL CALLBACK
z3dlgproc
(
    HWND   p_win,
    UINT   p_msg,
    WPARAM p_wp,
    LPARAM p_lp
)
{
    FDOC*doc;
    
    HWND hc;
    
    int i,k;
    
    BOOL r = FALSE;
    
    TVINSERTSTRUCT tvi;
    TVHITTESTINFO hti;
    TVITEM*itemstr;
    
    HTREEITEM hitem;
    
    RECT rc;
    
    ZOVER*ov;
    
    OVEREDIT*oed;
    
    // "Notification message header" ?
    P2C(NMHDR) notific;
    
    unsigned char * rom;
    
    uint16_t item_id       = 0;
    uint16_t item_category = u16_neg1;
    
    uint32_t item_param = u32_neg1;
    
    HWND const treeview = GetDlgItem(p_win, ID_Z3Dlg_TreeView);
    
    char buf[0x200];
    
    // -----------------------------
    
#if defined _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );      
#endif
    
    switch(p_msg)
    {
       
    case WM_INITDIALOG:
        
        return Z3Dlg_OnInit(p_win, p_lp);
    
    case WM_KEYDOWN:
        
        switch(p_wp)
        {
        
        case VK_RETURN:
            
            SendMessage
            (
                treeview,
                p_msg,
                p_wp,
                p_lp
            );
            
            break;
        }
        
        break;
    
    case WM_SETFOCUS:
        
        SetFocus(treeview);
        SetDlgItemText(debug_window, IDC_STATIC1, "2");
        
        break;
    
    case WM_NOTIFY:
        
        notific = (NMHDR*) p_lp;
        
        switch(p_wp)
        {
        
        case ID_Z3Dlg_TreeView:
            
            switch(notific->code)
            {
                
            case TVN_KEYDOWN:
                
                /*
                    \task[med] The treeview control causes a system beep
                    if it receives a spacebar key input, as well as for other
                    keys. This has to do with incremental search using characters,
                    I believe. This indicates that spacebar would be treated
                    as part of a search, but since none of our nodes
                    have a title that starts with a space character, it beeps.
                    
                    Ideally I'd like to use the Return / Enter keys to perform
                    this, but I'm not sure if it's even possible without
                    directly hooking this common control. Trying to catch
                    the Enter key has not met with any success yet.
                */
                {
                    P2C(NMTVKEYDOWN) tv_key = (NMTVKEYDOWN*) p_lp;
                    
                    if
                    (
                        Is(tv_key->wVKey, VK_SPACE)
                     |  Is(tv_key->wVKey, VK_RETURN)
                    )
                    {
                        HTREEITEM item = HM_TreeView_GetNextSelected(treeview);
                        
                        item_param = HM_TreeView_GetItemParam(treeview, item);
                        
                        if( Is(item_param, 0) )
                        {
                            SendMessage
                            (
                                treeview,
                                TVM_EXPAND,
                                TVE_TOGGLE,
                                (LPARAM) item
                            );
                            
                            return TRUE;
                        }
                        
                        r = TRUE;
                        
                        goto open_edt;
                    }
                }
                
                break;
            
            case NM_RCLICK:
                
                // Is joek. Put in a collapse all or expand all at some point
                // maybe.
                // MessageBox(p_win, "Hey don't do that!", "Really!", MB_OK);
            
            case NM_DBLCLK:
                
                GetWindowRect(notific->hwndFrom, &rc);
                
                hti.pt.x = mouse_x-rc.left;
                hti.pt.y = mouse_y-rc.top;
                
                hitem = (HTREEITEM) SendMessage
                (
                    notific->hwndFrom,
                    TVM_HITTEST,
                    0,
                    (LPARAM) &hti
                );
                
                if( ! hitem )
                    break;
                
                if( ! (hti.flags & TVHT_ONITEM) )
                    break;
                
                itemstr = &(tvi.item);
                itemstr->hItem = hitem;
                itemstr->mask = TVIF_PARAM;
                
                SendMessage
                (
                    notific->hwndFrom,
                    TVM_GETITEM,
                    0,
                    (LPARAM) itemstr
                );
                
                item_param = itemstr->lParam;
                
                
open_edt:
                
                doc = (FDOC*) GetWindowLongPtr(p_win, DWLP_USER);
                
                item_category = HIWORD(item_param);
                item_id       = LOWORD(item_param);
                
                switch(item_category)
                {

                    // double clicked on an overworld area
                case 2:
                    ov = doc->overworld;
                    
                    if(item_id < 128)
                    {
                        item_id = doc->rom[0x125ec + (item_id & 0x3f)]
                                | (item_id & 0x40);
                    }
                    
                    for(i = 0; i < 4; i += 1)
                    {
                        k = map_ind[i];
                        
                        if(item_id >= k && ov[item_id - k].win)
                        {
                            hc = ov[item_id - k].win;
                            
                            oed = (OVEREDIT*)GetWindowLongPtr(hc, GWLP_USERDATA);
                            
                            if(i && ! (oed->mapsize) )
                                continue;
                            
                            SendMessage(clientwnd,
                                        WM_MDIACTIVATE,
                                        (WPARAM) hc,
                                        0);
                            
                            hc = GetDlgItem(oed->dlg,3001);
                            
                            SendMessage(hc, WM_HSCROLL, SB_THUMBPOSITION | ( (i & 1) << 20),0);
                            SendMessage(hc, WM_VSCROLL, SB_THUMBPOSITION | ( (i & 2) << 19),0);
                            
                            return FALSE;
                        }
                    }
                    
                    wsprintf(buf,
                             "Area %02X - %s",
                             item_id,
                             area_names.m_lines[item_id]);
                    
                    ov[item_id].win = Editwin
                    (
                        doc,
                        "ZEOVER",
                        buf,
                        item_id,
                        sizeof(OVEREDIT)
                    );
                    
                    break;
                
                case 3:
                    
                    // double clicked on a dungeon item
                    if(doc->ents[item_id])
                    {
                        HM_MDI_ActivateChild(clientwnd, doc->ents[item_id]);
                        
                        break;
                    }
                    
                    if(item_id < 0x8c)
                    {
                        k = ((short*)
                        (
                            doc->rom
                          + (item_id >= 0x85 ? 0x15a64 : 0x14813) )
                        )[item_id];
                        
                        if(doc->dungs[k])
                        {
                            MessageBox(framewnd,
                                       "The room is already open in another editor",
                                       "Bad error happened",
                                       MB_OK);
                            
                            break;
                        }
                        
                        if(item_id >= 0x85)
                            wsprintf(buf,"Start location %02X", item_id - 0x85);
                        else
                        {
                            wsprintf(buf,
                                     "Entrance %02X - %s",
                                     item_id,
                                     entrance_names.m_lines[item_id]);
                        }
                    }
                    else if(item_id < 0x9f)
                        wsprintf(buf,"Overlay %d", item_id - 0x8c);
                    else if(item_id < 0xa7)
                        wsprintf(buf,"Layout %d", item_id - 0x9f);
                    else
                        wsprintf(buf,"Watergate overlay");
                    
                    hc = Editwin
                    (
                        doc,
                        "ZEDUNGEON",
                        buf,
                        item_id,
                        sizeof(DUNGEDIT)
                    );
                    
                    if(hc)
                    {
                        DUNGEDIT * ed = (DUNGEDIT*) GetWindowLongPtr(hc, GWLP_USERDATA);
                        HWND map_win = GetDlgItem(ed->dlg, ID_DungEditWindow);
                    
                        Dungselectchg(ed, map_win, 1);
                    }
                    
                    break;
                case 4:
                    
                    if(doc->mbanks[item_id])
                    {
                         SendMessage
                         (
                            clientwnd,
                            WM_MDIACTIVATE,
                            (WPARAM) doc->mbanks[item_id],
                            0
                        );
                        
                        break;
                    }
                    
                    if(item_id == 3)
                    {
                        doc->mbanks[3] = Editwin
                        (
                            doc,
                            "MUSBANK",
                            "Wave editor",
                            3,
                            sizeof(SAMPEDIT)
                        );
                    }
                    else
                    {
                        wsprintf(buf,
                                 "Song bank %d",
                                 item_id + 1);
                        
                        doc->mbanks[item_id] = Editwin
                        (
                            doc,
                            "MUSBANK",
                            buf,
                            item_id,
                            sizeof(MUSEDIT)
                        );
                    }
                    
                    break;
                
                case 6:
                    
                    if( doc->wmaps[item_id] )
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->wmaps[item_id],
                                    0);
                        
                        break;
                    }
                    
                    wsprintf(buf,
                             "World map %d",
                             item_id + 1);
                    
                    doc->wmaps[item_id] = Editwin
                    (
                        doc,
                        "WORLDMAP",
                        buf,
                        item_id,
                        sizeof(WMAPEDIT)
                    );
                    
                    break;
                
                case 5:
                    
                    // \task[high] This works, so restructure the rest of
                    // this window procedure to handle other categories this
                    // way. This prevents the treeview from stealing keyboard
                    // focus back from the newly opened MDI child windows.
                    // (treeviews are bastards!)
                    if( Is(p_msg, WM_NOTIFY) )
                    {
                        PostMessage
                        (
                            p_win, 
                            4000,
                            item_param,
                            0
                        );

                        break;
                    }
                    
                    if(doc->t_wnd)
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->t_wnd,
                                    0);
                        
                        break;
                    }
                    
                    doc->t_wnd = Editwin
                    (
                        doc,
                        "ZTXTEDIT",
                        "Text editor",
                        0,
                        sizeof(TEXTEDIT)
                    );
                    
                    return FALSE;
                
                case 7:
                    
                    if(doc->pals[item_id])
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->pals[item_id],
                                    0);
                        
                        break;
                    }
                    
                    k = 0;
                    
                    for(i = 0; i < 16; i += 1)
                    {
                        if(k + pal_num[i] > item_id)
                            break;
                        
                        k += pal_num[i];
                    }
                    
                    wsprintf(buf,
                             "%s palette %d",
                             pal_text[i],
                             item_id - k);
                    
                    doc->pals[item_id] = Editwin
                    (
                        doc,
                        "PALEDIT",
                        buf,
                        item_id | (i << 10) | ( (item_id - k) << 16),
                        sizeof(PALEDIT)
                    );
                    
                    break;
                
                case 8:
                    
                    if( doc->dmaps[item_id] )
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->dmaps[item_id],
                                    0);
                        
                        break;
                    }
                    
                    doc->dmaps[item_id] = Editwin
                    (
                        doc,
                        "LEVELMAP",
                        level_str[item_id + 1],
                        item_id,
                        sizeof(LMAPEDIT)
                    );
                    
                    break;
                
                case 9:
                    
                    activedoc = doc;
                    
                    ShowDialog(hinstance,
                               MAKEINTRESOURCE(IDD_DIALOG17),
                               framewnd,
                               editbosslocs,
                               item_id);
                    
                    break;
                
                case 10:
                    
                    if( doc->tmaps[item_id] )
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->tmaps[item_id],
                                    0);
                        
                        break;
                    }
                    
                    doc->tmaps[item_id] = Editwin
                    (
                        doc,
                        "TILEMAP",
                        screen_text[item_id],
                        item_id,
                        sizeof(TMAPEDIT)
                    );
                    
                    break;
                
                case 11:
                    
                    if(doc->perspwnd)
                    {
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->perspwnd,
                                    0);
                        
                        break;
                    }
                    
                    doc->perspwnd = Editwin(doc,
                                            "PERSPEDIT",
                                            "3D object editor",
                                            0,
                                            sizeof(PERSPEDIT) );
                    
                    break;
                
                case 12:
                    
                    oed = (OVEREDIT*) malloc( sizeof(OVEREDIT) );
                    
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
                        SendMessage(clientwnd,
                                    WM_MDIACTIVATE,
                                    (WPARAM) doc->hackwnd,
                                    0);
                        
                        break;
                    }
                    
                    doc->hackwnd = Editwin(doc,
                                           "PATCHLOAD",
                                           "Patch modules",
                                           0,
                                           sizeof(PATCHLOAD) );
                    
                    break;
                
                case 14:
                    
                    // Graphic Themes
                    ShowDialog(hinstance,
                               MAKEINTRESOURCE(IDD_GRAPHIC_THEMES),
                               framewnd,
                               editvarious,
                               (LPARAM) doc);
                    
                    break;
                }
            }
        }
        
        break;
    
    case 4000:
        
        item_param = p_wp;
        
        goto open_edt;
    }
    
    return r;
}
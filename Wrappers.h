
#if ! defined HMAGIC_WRAPPERS_HEADER_GUARD

    #define HMAGIC_WRAPPERS_HEADER_GUARD

    #include "structs.h"

// =============================================================================

    /**
        Convenience macro to avoid accidental assignment when testing for
        equality of two entities.
    */
    #define Is(x, y) ( (x) == (y) )

    /**
        Convenience macro to avoid accidental assignment when testing for
        inequality of two entities.
    */
    #define IsNot(x, y) ( (x) != (y) )

    /**
        Convenience macro for testing for a true condition.
    */
    #define IsTrue(x) Is( (x), TRUE)

    /**
        Convenience macro for testing for a false condition.
    */
    #define IsFalse(x) Is( (x), FALSE)

    /**
        Convenience macro for testing whether a variable is equal to zero.
    */
    #define IsZero(x) Is( (x), 0 )

    /**
        Convenience macro for testing whether a variable is not equal to zero.
    */
    #define IsNonZero(x) ( ! IsZero(x) )

    /**
        Convenience macro for testing whether a pointer is a NULL pointer.
    */
    #define IsNull(x) ( Is(x, NULL) )

    /**
        Convenience macro for testing whether a pointer is non-NULL.
    */
    #define IsNonNull(x) ( ! IsNull(x) )

// =============================================================================

    /*
        Convenience macro for testing if an integer is even.
    */
    #define IsEven(x) ( IsZero( (x) % 2 ) )

    /*
        Convenience macro for testing if an integer is odd.
    */
    #define IsOdd(x) ( IsNonZero( (x) % 2 ) )

// =============================================================================

    /**
        Convenience macro for declarations. Also horizontal code golf.
        Read "constant pointer to constant x"
    */
    #define CP2C(x) x const * const

    /**
        Convenience macro for declarations. Also horizontal code golf.
        Read "constant pointer to x"
    */
    #define CP2(x) x * const

    /**
        Convenience macro for declarations. Also horizontal code golf.
        Read "pointer to a constant x"
    */
    #define P2C(x) x const *

// =============================================================================

    /**
        Convenicne macro for calculating the number of elements in an array
        of indeterminate size, but whose size is known at compile time.
        
        E.g. int foo[] = { 1, 2, 4, 6, 9, 10 }; has 6 elements and this macro
        will resolve to 6 in that case.
    */
    #define MACRO_ArrayLength(arr) ( sizeof(arr) / sizeof(arr[0]) )

// =============================================================================

// Not supported in the C compiler until VS 2013
#if defined _MSC_VER

#if ! defined __cplusplus

#if _MSC_VER < 1800
    #define va_copy(d, s) ((d) = (s))
#endif

#endif

#endif

// =============================================================================

typedef
struct
{
    BOOL m_control_down;

    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseMoveData;

// =============================================================================

/// For use with mouse button down / up messages, click, hover messages, and 
typedef
struct
{
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;
    
    /// Is the ALT key down?
    BOOL m_alt_key;
    
    POINT m_rel_pos;
    POINT m_screen_pos;
    
} HM_MouseData;

// =============================================================================

typedef
struct
{
    signed int m_distance;
    
    /// Full copy of all the flags just for reference.
    unsigned m_flags;
    
    /// Is the shift key down?
    BOOL m_shift_key;
    
    /// Is the control key down?
    BOOL m_control_key;

    /// Is the ALT key down?
    BOOL m_alt_key;

    POINT m_screen_pos;

} HM_MouseWheelData;

// =============================================================================

typedef
struct
{
    HWND m_deactivating;
    HWND m_activating;
}
HM_MdiActivateData;

// =============================================================================

typedef
struct
{
    BOOL m_valid_file;
    
    HANDLE m_h;
    
    DWORD m_file_size;
    
    char * m_contents;
    
} HM_FileStat;

// =============================================================================

typedef
struct
{
    unsigned m_num_lines;
    
    char ** m_lines;
    
} HM_TextResource;

// =============================================================================

    RECT
    HM_GetWindowRect(HWND const p_win);

    RECT
    HM_GetClientRect(HWND const p_win);

    RECT
    HM_GetDlgItemRect(HWND     const p_dlg,
                      unsigned const p_item_id);
    
    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win);

    HM_MouseMoveData
    HM_GetMouseMoveData(HWND   const p_win,
                        WPARAM const wparam,
                        LPARAM const lparam);

    HM_MouseData
    HM_GetMouseData(MSG const p_packed_msg);

    HM_MouseWheelData
    HM_GetMouseWheelData(WPARAM const p_wp, LPARAM const p_lp);


    BOOL
    HM_DrawRectangle(HDC const p_device_context,
                     RECT const p_rect);

    SCROLLINFO
    HM_GetVertScrollInfo(HWND const p_win);

    SCROLLINFO
    HM_GetHorizScrollInfo(HWND const p_win);

    BOOL
    HM_IsEmptyRect(RECT const p_rect);

    MSG
    HM_PackMessage(HWND const p_win,
                   UINT       p_msg_id,
                   WPARAM     p_wp,
                   LPARAM     p_lp);

    RGBQUAD
    HM_MakeRgb(uint8_t const p_red,
               uint8_t const p_green,
               uint8_t const p_blue);

    /// From "5 bits per channel"
    RGBQUAD
    HM_RgbFrom5bpc(uint16_t const p_color);

    /// COLORREF to "5 bits per channel" snes color.
    uint16_t
    HM_ColRefTo5bpc(COLORREF const p_cr);

    /// RGBQUAD structure to "5 bits per channel" snes color.
    uint16_t
    HM_ColQuadTo5bpc(RGBQUAD const p_cr);

    /// Converts RGBQUAD to COLORREF.
    COLORREF
    HM_ColQuadToRef(RGBQUAD const p_quad);

    BOOL
    HM_BinaryCheckDlgButton(HWND     const p_win,
                            unsigned const p_dlg_control,
                            BOOL     const p_is_checked);

    HM_FileStat
    HM_LoadFileContents(char const * const p_file_name);

    void
    HM_FreeFileStat(HM_FileStat * const p_s);

    WPARAM
    HM_NullWP(void);

    LPARAM
    HM_NullLP(void);

// =============================================================================

    extern int
    HM_ComboBox_AddString
    (
        HWND       const p_combobox,
        CP2C(char)       p_string
    );

    extern int
    HM_ComboBox_SelectItem
    (
        HWND const p_combobox,
        int  const p_item_index
    );

    extern int
    HM_ComboBox_GetSelectedItem
    (
        HWND const p_combobox
    );

    // _________________________________________________________________________
    #define HM_CBN_SELCHANGE(id) (id | (CBN_SELCHANGE << 16) )

// =============================================================================

    extern unsigned long
    HM_NumPadKeyDownFilter
    (
        MSG const p_packed_msg
    );

    extern void
    HM_OK_MsgBox
    (
        HWND       const p_win,
        CP2C(char)       p_prompt,
        CP2C(char)       p_title_bar
    );

    extern BOOL
    HM_YesNo_MsgBox
    (
        HWND       const p_win,
        CP2C(char)       p_prompt,
        CP2C(char)       p_title_bar
    );

// =============================================================================

    /**
        This section has convenience functions modeled after the standard
        C library. Note that this is why they're styled as all lowercase,
        to evoke the same naming scheme, but with a hm_ (hyrule magic) prefix.
    */

    extern void *
    hm_memdup
    (
        CP2C(void)    p_array,
        size_t  const p_length
    );

    /**
        Provided because while strdup() shows up in some compiler vendor
        libraries, it's not really considered standard, afaik.
    */
    extern char *
    hm_strdup
    (
        CP2C(char) p_string
    );

    extern char *
    hm_strndup
    (
        CP2C(char)       p_string,
        size_t     const p_length
    );

    BOOL
    HM_CheckEmbeddedStr(void const * const p_buffer,
                        char const * const p_string);

// =============================================================================

    BOOL
    HM_FileExists
    (
        char const * const p_filename,
        HANDLE     * const p_handle
    );

// =============================================================================

    int __stdcall
    askinteger(int max, char *caption, char *text);

// =============================================================================

    /**
        Variadic version of vsprintf that allocates the required buffer
    */
    extern int
    vasprintf
    (
        char       ** const p_buf_out,
        const char  * const p_fmt,
        va_list       const p_var_args
    );

    extern int
    asprintf
    (
        char       ** const p_buf_out,
        const char *  const p_fmt,
        ...
    );

// =============================================================================

    extern int
    vascatf
    (
        char       ** const p_buf_out,
        CP2C(char)          p_fmt,
        va_list       const p_var_args
    );

    extern int
    ascatf
    (
        char       ** const p_buf_out,
        CP2C(char)          p_fmt,
        ...
    );

// =============================================================================

    extern HM_MdiActivateData
    HM_MDI_GetActivateData
    (
        WPARAM const p_wp,
        LPARAM const p_lp
    );

    extern HWND
    HM_MDI_GetActiveChild
    (
        HWND const p_mdi_client_wnd
    );

    extern LRESULT
    HM_MDI_ActivateChild
    (
        HWND const p_mdi_client_wnd,
        HWND const p_mdi_child_wnd
    );

    extern void
    HM_MDI_DestroyChild
    (
        HWND const p_mdi_client_wnd,
        HWND const p_mdi_child_wnd
    );

// =============================================================================

    extern HTREEITEM
    HM_TreeView_InsertItem
    (
        HWND                   const p_treeview,
        HTREEITEM              const p_parent,
        TVINSERTSTRUCT const * const p_tvis
    );

    extern BOOL
    HM_TreeView_DeleteItem
    (
        HWND      const p_treeview,
        HTREEITEM const p_item
    );

    extern BOOL
    HM_TreeView_GetItem
    (
        HWND        const p_treeview,
        HTREEITEM   const p_item_handle,
        TVITEM    * const p_item
    );

    extern BOOL
    HM_TreeView_SetItem
    (
        HWND        const p_treeview,
        HTREEITEM   const p_item_handle,
        TVITEM    * const p_item
    );

    extern HTREEITEM
    HM_TreeView_InsertRoot
    (
        HWND         const p_treeview,
        char const * const p_label
    );

    extern HTREEITEM
    HM_TreeView_InsertChild
    (
        HWND      const p_treeview,
        HTREEITEM const p_parent
    );

    extern HTREEITEM
    HM_TreeView_GetFirstChild
    (
        HWND      const p_treeview,
        HTREEITEM const p_node
    );

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
    );

    extern HTREEITEM
    HM_TreeView_GetParent
    (
        HWND      const p_treeview,
        HTREEITEM const p_node
    );

    extern HTREEITEM
    HM_TreeView_GetNextSibling
    (
        HWND      const p_treeview,
        HTREEITEM const p_child
    );

    extern size_t
    HM_TreeView_CountChildren
    (
        HWND      const p_treeview,
        HTREEITEM const p_parent
    );

// =============================================================================

    extern int
    HM_ListBox_GetCount
    (
        HWND const p_listbox
    );

    extern int
    HM_ListBox_AddString
    (
        HWND    const p_listbox,
        LPCSTR  const p_string
    );

    extern int
    HM_ListBox_SetItemData
    (
        HWND   const p_listbox,
        int    const p_item_index,
        LPARAM const p_data
    );

    extern int
    HM_ListBox_GetSelectedItem
    (
        HWND const p_listbox
    );

    extern int
    HM_ListBox_SelectItem
    (
        HWND const p_listbox,
        int  const p_item_index
    );

    extern int
    HM_ListBox_InsertString
    (
        HWND       const p_listbox,
        int        const p_insertion_index,
        CP2C(char)       p_string
    );

    extern int
    HM_ListBox_DeleteString
    (
        HWND const p_listbox,
        int  const p_deletion_index
    );

    extern int
    HM_ListBox_GetText
    (
        HWND      const p_listbox,
        int       const p_item_index,
        CP2(char)       p_buffer
    );

    extern int
    HM_ListBox_GetTextLength
    (
        HWND      const p_listbox,
        int       const p_item_index
    );

    extern void
    HM_ListBox_ResetContent
    (
        HWND const p_listbox
    );

    extern int
    HM_ListBox_GetItemRect
    (
        HWND      const p_listbox,
        int       const p_item_index,
        CP2(RECT)       p_item_rect
    );

    extern void
    HM_ListBox_SetHorizontalExtent
    (
        HWND const p_listbox,
        int  const p_extent
    );

    extern int
    HM_ListBox_CalcMaxTextExtent
    (
        HWND const p_listbox
    );

    #define IsListBoxError(x) Is(x, LB_ERR)

    #define IsNotListBoxError(x) IsFalse( IsListBoxError(x) )

// =============================================================================

#define HM_EN_KILLFOCUS(id) ( (EN_KILLFOCUS << 16) | (id & 0xffff) )

// =============================================================================

#define HM_EN_CHANGE(id) ( (EN_CHANGE << 16) | (id & 0xffff) )

// =============================================================================

#if ! defined GWLP_STYLE
    enum { GWLP_STYLE = GWL_STYLE};
#endif

#if ! defined GWLP_EXSTYLE
    enum { GWLP_EXSTYLE = GWL_EXSTYLE };
#endif

#endif
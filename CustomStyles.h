
// symbol used is 
#if ! defined HMAGIC_CUSTOM_STYLES_HGUARD

#define HMAGIC_CUSTOM_STYLES_HGUARD

#include "windows.h"

enum
{
    WS_DEBUG_DLG_STYLE = (WS_EX_TOPMOST | WS_EX_TOOLWINDOW) & ~WS_CLOSEBOX
} CustomStyles;

#endif

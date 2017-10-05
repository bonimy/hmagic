
#if !defined HMAGIC_DUNGEON_ENUM_HEADER_GUARD

    #define HMAGIC_DUNGEON_ENUM_HEADER_GUARD

// =============================================================================

/// IDs for controls used in the dungeon superdialog.
enum
{
    ID_DungDlgFirst   = 3000,
    
    ID_DungRoomNumber = ID_DungDlgFirst,
    ID_DungStatic1,
    ID_DungEntrRoomNumber,
    ID_DungStatic2,
    ID_DungEntrYCoord,
    ID_DungStatic3,
    ID_DungEntrXCoord,
    ID_DungStatic4,
    ID_DungEntrYScroll,
    ID_DungStatic5,
    ID_DungEntrXScroll,
    ID_DungEditWindow,
    ID_DungStartLocGroupBox,
    
    /// Provides details on the currently selected entity of whatever sort.
    ID_DungDetailText,
    
    ID_DungLeftArrow,
    ID_DungRightArrow,
    ID_DungUpArrow,
    ID_DungDownArrow,
    ID_DungStatic6,
    ID_DungFloor1,
    ID_DungStatic7,
    ID_DungFloor2,
    ID_DungStatic8,
    ID_DungEntrTileSet,
    ID_DungStatic9,
    ID_DungEntrSong,
    ID_DungStatic10,
    ID_DungEntrPalaceID,
    ID_DungStatic11,
    ID_DungLayout,
    ID_DungShowBG1,
    ID_DungShowBG2,
    ID_DungDispGroupBox,
    ID_DungAnimateButton,
    ID_DungObjLayer1,
    ID_DungObjLayer2,
    ID_DungObjLayer3,
    ID_DungEditGroupBox,
    ID_DungStatic12,
    ID_DungBG2Settings,
    ID_DungStatic13,
    ID_DungTileSet,
    ID_DungStatic14,
    ID_DungPalette,
    ID_DungStatic15,
    ID_DungSprTileSet,
    
    /// Collision settings
    ID_DungCollSettings,
    
    ID_DungSprLayer,
    
    /// Button that brings up dialog box asking if you'd like to switch to a
    /// different room.
    ID_DungChangeRoom,
    
    ID_DungShowSprites,
    ID_DungStatic16,
    ID_DungStatic17,
    ID_DungEntrCameraX,
    ID_DungStatic18,
    ID_DungEntrCameraY,
    ID_DungEntrProps,
    ID_DungEditHeader,
    ID_DungItemLayer,
    ID_DungBlockLayer,
    ID_DungTorchLayer,
    ID_DungSortSprites,
    ID_DungExit,
    
    // Should be considered an invalid entry.
    ID_DungDlgAfterLast,
    
    ID_DungNumControls = (ID_DungDlgAfterLast - ID_DungDlgFirst)
};

// =============================================================================

    extern unsigned const overlay_hide[];

// =============================================================================

    enum
    {
        SD_DungShowBG1 = 1 << 0,
        
        SD_DungShowBG2 = 1 << 1,
        
        /// Set if the map should show sprite and item markers.
        SD_DungShowMarkers = 1 << 2,
        
        SD_DungShowBothBGs = (SD_DungShowBG1 | SD_DungShowBG2),

        SD_DungHideBG1     = ~SD_DungShowBG1,
        SD_DungHideBG2     = ~SD_DungShowBG2,
        SD_DungHideBothBGs = ~SD_DungShowBothBGs,
        SD_DungHideMarkers = ~SD_DungShowMarkers,
    };

// =============================================================================

    enum
    {
        SD_DungSprLayerSelected = 6,
        SD_DungItemLayerSelected,
        SD_DungBlockLayerSelected,
        SD_DungTorchLayerSelected,
    };


#endif
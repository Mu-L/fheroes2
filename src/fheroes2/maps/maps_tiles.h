/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2024                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef H2TILES_H
#define H2TILES_H

#include <array>
#include <cstdint>
#include <list>
#include <string>
#include <vector>

#include "color.h"
#include "direction.h"
#include "ground.h"
#include "heroes.h"
#include "math_base.h"
#include "mp2.h"
#include "world_regions.h"

class IStreamBase;
class OStreamBase;

namespace Maps
{
    enum ObjectLayerType : uint8_t
    {
        OBJECT_LAYER = 0, // main and action objects like mines, forest, mountains, castles and etc.
        BACKGROUND_LAYER = 1, // background objects like lakes or bushes.
        SHADOW_LAYER = 2, // shadows and some special objects like castle's entrance road.
        TERRAIN_LAYER = 3 // roads, water flaws and cracks. Essentially everything what is a part of terrain.
    };

    struct TilesAddon
    {
        TilesAddon() = default;

        TilesAddon( const uint8_t layerType, const uint32_t uid, const MP2::ObjectIcnType objectIcnType, const uint8_t imageIndex )
            : _uid( uid )
            , _layerType( layerType )
            , _objectIcnType( objectIcnType )
            , _imageIndex( imageIndex )
        {
            // Do nothing.
        }

        TilesAddon( const TilesAddon & ) = default;

        ~TilesAddon() = default;

        // Returns true if it can be passed be hero/boat: addon's layer type is SHADOW or TERRAIN.
        bool isPassabilityTransparent() const
        {
            return _layerType == SHADOW_LAYER || _layerType == TERRAIN_LAYER;
        }

        bool operator==( const TilesAddon & addon ) const
        {
            return ( _uid == addon._uid ) && ( _layerType == addon._layerType ) && ( _objectIcnType == addon._objectIcnType ) && ( _imageIndex == addon._imageIndex );
        }

        // Unique identifier of an object. UID can be shared among multiple object parts if an object is bigger than 1 tile.
        uint32_t _uid{ 0 };

        // Layer type shows how the object is rendered on Adventure Map. See ObjectLayerType enumeration.
        uint8_t _layerType{ OBJECT_LAYER };

        // The type of object which correlates to ICN id. See MP2::getIcnIdFromObjectIcnType() function for more details.
        MP2::ObjectIcnType _objectIcnType{ MP2::OBJ_ICN_TYPE_UNKNOWN };

        // Image index to define which part of the object is. This index corresponds to an index in ICN objects storing multiple sprites (images).
        uint8_t _imageIndex{ 255 };
    };

    class Tiles
    {
    public:
        Tiles() = default;

        bool operator==( const Tiles & tile ) const
        {
            return ( _addonBottomLayer == tile._addonBottomLayer ) && ( _addonTopLayer == tile._addonTopLayer ) && ( _index == tile._index )
                   && ( _terrainImageIndex == tile._terrainImageIndex ) && ( _terrainFlags == tile._terrainFlags ) && ( _mainAddon == tile._mainAddon )
                   && ( _mainObjectType == tile._mainObjectType ) && ( _metadata == tile._metadata ) && ( _tilePassabilityDirections == tile._tilePassabilityDirections )
                   && ( _isTileMarkedAsRoad == tile._isTileMarkedAsRoad ) && ( _occupantHeroId == tile._occupantHeroId );
        }

        bool operator!=( const Tiles & tile ) const
        {
            return !operator==( tile );
        }

        void Init( int32_t index, const MP2::MP2TileInfo & mp2 );

        void setIndex( const int32_t index )
        {
            _index = index;
        }

        int32_t GetIndex() const
        {
            return _index;
        }

        fheroes2::Point GetCenter() const;

        MP2::MapObjectType GetObject( bool ignoreObjectUnderHero = true ) const;

        const TilesAddon & getMainObjectPart() const
        {
            return _mainAddon;
        }

        TilesAddon & getMainObjectPart()
        {
            return _mainAddon;
        }

        uint16_t GetPassable() const
        {
            return _tilePassabilityDirections;
        }

        int GetGround() const
        {
            return Ground::getGroundByImageIndex( _terrainImageIndex );
        }

        bool isWater() const
        {
            return Ground::getGroundByImageIndex( _terrainImageIndex ) == Ground::WATER;
        }

        bool isSameMainObject( const MP2::MapObjectType objectType ) const
        {
            return objectType == _mainObjectType;
        }

        // Returns true if tile's main and bottom layer addons do not contain any objects: layer type is SHADOW or TERRAIN.
        bool isPassabilityTransparent() const;

        // Checks whether it is possible to move into this tile from the specified direction
        bool isPassableFrom( const int direction ) const
        {
            return ( direction & _tilePassabilityDirections ) != 0;
        }

        // Checks whether it is possible to move into this tile from the specified direction under the specified conditions
        bool isPassableFrom( const int direction, const bool fromWater, const bool ignoreFog, const int heroColor ) const;

        // Checks whether it is possible to exit this tile in the specified direction
        bool isPassableTo( const int direction ) const
        {
            return ( direction & _tilePassabilityDirections ) != 0;
        }

        bool isRoad() const
        {
            return _isTileMarkedAsRoad || _mainObjectType == MP2::OBJ_CASTLE;
        }

        bool isStream() const;
        bool GoodForUltimateArtifact() const;

        TilesAddon * getBottomLayerAddon( const uint32_t uid );
        TilesAddon * getTopLayerAddon( const uint32_t uid );

        void SetObject( const MP2::MapObjectType objectType );

        void resetBoatOwnerColor()
        {
            _boatOwnerColor = Color::NONE;
        }

        int getBoatOwnerColor() const
        {
            return _boatOwnerColor;
        }

        void setBoat( const int direction, const int color );
        int getBoatDirection() const;

        void resetMainObjectPart()
        {
            _mainAddon = {};
        }

        uint32_t GetRegion() const
        {
            return _region;
        }

        void UpdateRegion( uint32_t newRegionID );

        // Set initial passability based on information read from mp2 and addon structures.
        void setInitialPassability();

        // Update passability based on neighbours around.
        void updatePassability();

        void setOwnershipFlag( const MP2::MapObjectType objectType, int color );

        // Return fog direction of tile. A tile without fog returns "Direction::UNKNOWN".
        uint16_t getFogDirection() const
        {
            return _fogDirection;
        }

        void pushBottomLayerAddon( const MP2::MP2AddonInfo & ma );

        void pushBottomLayerAddon( TilesAddon ta );

        void pushTopLayerAddon( const MP2::MP2AddonInfo & ma );

        void pushTopLayerAddon( TilesAddon ta )
        {
            _addonTopLayer.emplace_back( ta );
        }

        const std::list<TilesAddon> & getBottomLayerAddons() const
        {
            return _addonBottomLayer;
        }

        std::list<TilesAddon> & getBottomLayerAddons()
        {
            return _addonBottomLayer;
        }

        const std::list<TilesAddon> & getTopLayerAddons() const
        {
            return _addonTopLayer;
        }

        void moveMainAddonToBottomLayer()
        {
            if ( _mainAddon._objectIcnType != MP2::OBJ_ICN_TYPE_UNKNOWN ) {
                _addonBottomLayer.emplace_back( _mainAddon );
                _mainAddon = {};
            }
        }

        void AddonsSort();

        // Returns true if any object part was removed.
        bool removeObjectPartsByUID( const uint32_t objectUID );

        // Use to remove object by ICN type only from this tile. Should be used only for 1 tile size objects and roads or streams.
        void removeObjects( const MP2::ObjectIcnType objectIcnType );

        void updateObjectImageIndex( const uint32_t objectUid, const MP2::ObjectIcnType objectIcnType, const int imageIndexOffset );
        void replaceObject( const uint32_t objectUid, const MP2::ObjectIcnType originalObjectIcnType, const MP2::ObjectIcnType newObjectIcnType,
                            const uint8_t originalImageIndex, const uint8_t newImageIndex );

        std::string String() const;

        bool isFog( const int colors ) const
        {
            // colors may be the union friends
            return ( _fogColors & colors ) == colors;
        }

        void ClearFog( const int colors );

        void SetObjectPassable( bool pass );

        const std::array<uint32_t, 3> & metadata() const
        {
            return _metadata;
        }

        std::array<uint32_t, 3> & metadata()
        {
            return _metadata;
        }

        uint8_t getTerrainFlags() const
        {
            return _terrainFlags;
        }

        uint16_t getTerrainImageIndex() const
        {
            return _terrainImageIndex;
        }

        void setTerrain( const uint16_t terrainImageIndex, const bool horizontalFlip, const bool verticalFlip );

        Heroes * getHero() const;
        void setHero( Heroes * hero );

        // Set tile's object type according to the object's sprite if there is any, otherwise
        // it is set to coast (MP2::OBJ_COAST) if it's near water or to empty (MP2::OBJ_NONE).
        // This method works perfectly only on Resurrection (.fh2m) maps.
        // It might not work properly on the original maps due to small differences in object types.
        void updateObjectType();

        uint32_t getObjectIdByObjectIcnType( const MP2::ObjectIcnType objectIcnType ) const;

        bool containsAnyObjectIcnType( const std::vector<MP2::ObjectIcnType> & objectIcnTypes ) const;

        bool containsSprite( const MP2::ObjectIcnType objectIcnType, const uint32_t imageIdx ) const;

        // Do NOT call this method directly!!!
        void setFogDirection( const uint16_t fogDirection )
        {
            _fogDirection = fogDirection;
        }

        // Some tiles have incorrect object type. This is due to original Editor issues.
        static void fixMP2MapTileObjectType( Tiles & tile );

        static int32_t getIndexOfMainTile( const Tiles & tile );

        // Update tile or bottom layer object image index.
        static void updateTileObjectIcnIndex( Tiles & tile, const uint32_t uid, const uint8_t newIndex );

    private:
        bool isShadow() const;

        TilesAddon * getAddonWithFlag( const uint32_t uid );

        // Set or remove a flag which belongs to UID of the object.
        void updateFlag( const int color, const uint8_t objectSpriteIndex, const uint32_t uid, const bool setOnUpperLayer );

        void _updateRoadFlag();

        bool isTallObject() const;

        bool isDetachedObject() const;

        int getOriginalPassability() const;

        bool doesObjectExist( const uint32_t uid ) const;

        std::vector<MP2::ObjectIcnType> getValidObjectIcnTypes() const;

        friend OStreamBase & operator<<( OStreamBase & stream, const Tiles & tile );
        friend IStreamBase & operator>>( IStreamBase & stream, Tiles & tile );

        // The following members are used in the Editor and in the game.

        TilesAddon _mainAddon;

        std::list<TilesAddon> _addonBottomLayer;

        std::list<TilesAddon> _addonTopLayer;

        int32_t _index{ 0 };

        uint16_t _terrainImageIndex{ 0 };

        MP2::MapObjectType _mainObjectType{ MP2::OBJ_NONE };

        std::array<uint32_t, 3> _metadata{ 0 };

        uint16_t _tilePassabilityDirections{ DIRECTION_ALL };

        uint8_t _terrainFlags{ 0 };

        bool _isTileMarkedAsRoad{ false };

        uint8_t _occupantHeroId{ Heroes::UNKNOWN };

        // The following members are only used in the game.

        uint8_t _fogColors{ Color::ALL };

        // Heroes can only summon neutral empty boats or empty boats belonging to their kingdom.
        uint8_t _boatOwnerColor{ Color::NONE };

        // Fog direction to render fog in Game Area.
        uint16_t _fogDirection{ DIRECTION_ALL };

        // This field does not persist in savegame.
        uint32_t _region{ REGION_NODE_BLOCKED };
    };

    OStreamBase & operator<<( OStreamBase & stream, const TilesAddon & ta );
    OStreamBase & operator<<( OStreamBase & stream, const Tiles & tile );
    IStreamBase & operator>>( IStreamBase & stream, TilesAddon & ta );
    IStreamBase & operator>>( IStreamBase & stream, Tiles & tile );
}

#endif

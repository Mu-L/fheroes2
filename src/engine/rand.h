/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2025                                             *
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

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
// IWYU issue workaround. When <exception> is included IWYU will remove it and require <string>.
// When <string> is included it'll remove it and require <exception>
// IWYU pragma: no_include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace Rand
{
    uint32_t uniformIntDistribution( const uint32_t from, const uint32_t to, std::mt19937 & gen );

    // Fisher-Yates shuffle AKA Knuth shuffle, probably the same as std::shuffle.
    // NOTE: we can't use std::shuffle here because it uses std::uniform_int_distribution which behaves differently on different platforms.
    template <class Iter>
    void shuffle( Iter first, Iter last, std::mt19937 & gen )
    {
        if ( first == last ) {
            return;
        }

        assert( first < last );

        // Change last from once-past-the-end to last element
        --last;
        const typename std::iterator_traits<Iter>::difference_type interval = last - first;

        // Our implementation doesn't work for intervals bigger than 2**32 - 1
        assert( interval <= static_cast<typename std::iterator_traits<Iter>::difference_type>( std::numeric_limits<uint32_t>::max() ) );

        uint32_t remainingSwaps = static_cast<uint32_t>( interval );
        while ( remainingSwaps > 0 ) {
            // Allow argument-dependant lookup (ADL) for swap: first try in the namespace of the type, then in the std namespace.
            using std::swap;
            const uint32_t index = uniformIntDistribution( 0, remainingSwaps, gen );
            swap( *last, *( first + index ) );
            --last;
            --remainingSwaps;
        }
    }

    std::mt19937 & CurrentThreadRandomDevice();

    uint32_t Get( uint32_t from, uint32_t to = 0 );

    template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
    T Get( const T from, const T to )
    {
        return static_cast<T>( Get( static_cast<uint32_t>( from ), static_cast<uint32_t>( to ) ) );
    }

    uint32_t GetWithSeed( uint32_t from, uint32_t to, uint32_t seed );

    template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
    T GetWithSeed( const T from, const T to, const uint32_t seed )
    {
        return static_cast<T>( GetWithSeed( static_cast<uint32_t>( from ), static_cast<uint32_t>( to ), seed ) );
    }

    uint32_t GetWithGen( uint32_t from, uint32_t to, std::mt19937 & gen );

    template <typename T>
    void Shuffle( std::vector<T> & vec )
    {
        Rand::shuffle( vec.begin(), vec.end(), CurrentThreadRandomDevice() );
    }

    template <typename T>
    void ShuffleWithGen( std::vector<T> & vec, std::mt19937 & gen )
    {
        Rand::shuffle( vec.begin(), vec.end(), gen );
    }

    template <typename T>
    const T & Get( const std::vector<T> & vec )
    {
        assert( !vec.empty() );

        const uint32_t id = Get( static_cast<uint32_t>( vec.size() - 1 ) );
        return vec[id];
    }

    template <typename T>
    const T & GetWithGen( const std::vector<T> & vec, std::mt19937 & gen )
    {
        assert( !vec.empty() );

        const uint32_t id = GetWithGen( 0, static_cast<uint32_t>( vec.size() - 1 ), gen );
        return vec[id];
    }

    using ValueWeight = std::pair<int32_t, uint32_t>;

    class Queue : private std::vector<ValueWeight>
    {
    public:
        explicit Queue( uint32_t size = 0 )
        {
            reserve( size );
        }

        void Push( const int32_t value, const uint32_t weight )
        {
            if ( weight == 0 ) {
                return;
            }

            emplace_back( value, weight );
        }

        size_t Size() const
        {
            return size();
        }

        int32_t Get() const
        {
            return Get( []( const uint32_t max ) { return Rand::Get( 0, max ); } );
        }

        int32_t GetWithSeed( const uint32_t seed ) const
        {
            return Get( [seed]( const uint32_t max ) { return Rand::GetWithSeed( 0, max, seed ); } );
        }

    private:
        int32_t Get( const std::function<uint32_t( uint32_t )> & randomFunc ) const;
    };

    // Specific random generator that keeps and update its state
    class DeterministicRandomGenerator
    {
    public:
        explicit DeterministicRandomGenerator( const uint32_t initialSeed );
        DeterministicRandomGenerator( const DeterministicRandomGenerator & ) = delete;

        DeterministicRandomGenerator & operator=( const DeterministicRandomGenerator & ) = delete;

        uint32_t GetSeed() const;
        void UpdateSeed( const uint32_t seed );

        uint32_t Get( const uint32_t from, const uint32_t to = 0 );

        template <typename T>
        const T & Get( const std::vector<T> & vec )
        {
            ++_currentSeed;
            std::mt19937 seededGen( _currentSeed );
            return Rand::GetWithGen( vec, seededGen );
        }

    private:
        uint32_t _currentSeed;
    };
}

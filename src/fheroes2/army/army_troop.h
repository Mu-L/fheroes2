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

#ifndef H2ARMYTROOP_H
#define H2ARMYTROOP_H

#include <cstdint>
#include <string>

#include "monster.h"
#include "resource.h"

class IStreamBase;
class OStreamBase;

class Army;

class Troop : public Monster
{
public:
    Troop();
    Troop( const Monster &, uint32_t );

    bool operator==( const Monster & ) const;

    void Set( const Troop & );
    void Set( const Monster &, uint32_t );
    void SetMonster( const Monster & );
    void SetCount( uint32_t );
    void Reset();

    bool isMonster( int ) const;
    const char * GetName() const;
    uint32_t GetCount() const;
    uint32_t GetHitPoints() const;
    Monster GetMonster() const;

    uint32_t GetDamageMin() const;
    uint32_t GetDamageMax() const;

    Funds GetTotalCost() const;

    // Returns the cost of an upgrade if a monster has an upgrade. Otherwise returns no resources.
    // IMPORTANT!!! Make sure that you call this method after checking by isAllowUpgrade() method.
    Funds GetTotalUpgradeCost() const;

    bool isEmpty() const;

    virtual bool isValid() const;
    virtual bool isBattle() const;
    virtual bool isModes( uint32_t ) const;
    virtual std::string GetAttackString() const;
    virtual std::string GetDefenseString() const;
    virtual std::string GetShotString() const;
    virtual std::string GetSpeedString() const;
    virtual uint32_t GetHitPointsLeft() const;
    virtual uint32_t GetSpeed() const;
    virtual uint32_t GetAffectedDuration( uint32_t ) const;

    double GetStrength() const;
    double GetStrengthWithBonus( int bonusAttack, int bonusDefense ) const;

protected:
    friend OStreamBase & operator<<( OStreamBase & stream, const Troop & troop );
    friend IStreamBase & operator>>( IStreamBase & stream, Troop & troop );

    static std::string GetSpeedString( uint32_t speed );

    uint32_t count;
};

class ArmyTroop : public Troop
{
public:
    explicit ArmyTroop( const Army * );
    ArmyTroop( const Army *, const Troop & );

    ArmyTroop & operator=( const Troop & ) = delete;

    uint32_t GetAttack() const override;
    uint32_t GetDefense() const override;
    int GetMorale() const override;
    int GetLuck() const override;

    int GetColor() const;

    void SetArmy( const Army & );
    const Army * GetArmy() const;

    std::string GetAttackString() const override;
    std::string GetDefenseString() const override;

protected:
    const Army * army;
};

#endif

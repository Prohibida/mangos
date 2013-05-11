/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"
#include "DBCEnums.h"
#include "TransportSystem.h"

#include <map>
#include <set>
#include <string>

class TransportKit;
class TransportPathMovementGenerator;

class MANGOS_DLL_SPEC Transport : public GameObject
{
    public:
        explicit Transport();
        virtual ~Transport();

        static uint32 GetPossibleMapByEntry(uint32 entry, bool start = true);
        static bool   IsSpawnedAtDifficulty(uint32 entry, Difficulty difficulty);

        void SetPeriod(uint32 time) { SetUInt32Value(GAMEOBJECT_LEVEL, time);}
        uint32 GetPeriod() const { return GetUInt32Value(GAMEOBJECT_LEVEL);}

        bool Create(uint32 guidlow, Map* map, uint32 period, uint8 animprogress, uint16 dynamicHighValue);
        void Update(uint32 update_diff, uint32 p_time) override;

        bool SetPosition(WorldLocation const& loc, bool teleport);

        bool AddPassenger(WorldObject* passenger, Position const& transportPos);
        bool RemovePassenger(WorldObject* passenger);

        void Start();
        void Stop();

        TransportKit* GetTransportKit() { return m_transportKit; };

        void BuildStartMovePacket(Map const *targetMap);
        void BuildStopMovePacket(Map const *targetMap);

        uint32 GetTransportMapId() const { return GetGOInfo() ? GetGOInfo()->moTransport.mapID : 0; };

        virtual bool IsTransport() const override { return bool(m_transportKit); };
        TransportBase* GetTransportBase() { return (TransportBase*)m_transportKit; };

        TransportPathMovementGenerator const*  CurrentMovementGenerator() const { return m_moveGen; };

    private:
        void MoveToNextWayPoint();                          // move m_next/m_cur to next points

        TransportKit* m_transportKit;
        TransportPathMovementGenerator* m_moveGen;

};

class  MANGOS_DLL_SPEC TransportKit : public TransportBase
{
    public:
        explicit TransportKit(Transport& base);
        virtual ~TransportKit();

        void Initialize();
        bool IsInitialized() const { return m_isInitialized; }

        void Reset();

        bool AddPassenger(WorldObject* passenger, Position const& transportPos);
        void RemovePassenger(WorldObject* passenger);
        void RemoveAllPassengers();

        Transport* GetBase() const { return (Transport*)GetOwner(); }
        PassengerMap const& GetPassengers() const { return m_passengers; };

    private:
        // Internal use to calculate the boarding position
        virtual Position CalculateBoardingPositionOf(Position const& pos) const override;

        bool m_isInitialized;
};

#endif

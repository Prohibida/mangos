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

/**
 * @addtogroup TransportSystem
 * @{
 *
 * @file TransportSystem.cpp
 * This file contains the code needed for MaNGOS to provide abstract support for transported entities
 * Currently implemented
 * - Calculating between local and global coords
 * - Abstract storage of passengers (added by BoardPassenger, UnboardPassenger)
 */

#include "TransportSystem.h"
#include "Unit.h"
#include "Vehicle.h"
#include "MapManager.h"

/* **************************************** TransportBase ****************************************/

TransportBase::TransportBase(WorldObject* owner) :
    m_owner(owner),
    m_lastPosition(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation()),
    m_sinO(sin(m_lastPosition.o)),
    m_cosO(cos(m_lastPosition.o)),
    m_updatePositionsTimer(500)
{
    MANGOS_ASSERT(m_owner);
}

// Update every now and then (after some change of transporter's position)
// This is used to calculate global positions (which don't have to be exact, they are only required for some server-side calculations
void TransportBase::Update(uint32 diff)
{
    if (m_updatePositionsTimer < diff)
    {
        if (fabs(m_owner->GetPositionX() - m_lastPosition.x) +
                fabs(m_owner->GetPositionY() - m_lastPosition.y) +
                fabs(m_owner->GetPositionZ() - m_lastPosition.z) > 1.0f ||
                MapManager::NormalizeOrientation(m_owner->GetOrientation() - m_lastPosition.o) > 0.01f)
            UpdateGlobalPositions();

        m_updatePositionsTimer = 500;
    }
    else
        m_updatePositionsTimer -= diff;
}

// Update the global positions of all passengers
void TransportBase::UpdateGlobalPositions()
{
    WorldLocation pos = m_owner->GetPosition();

    // Calculate new direction multipliers
    if (MapManager::NormalizeOrientation(pos.o - m_lastPosition.o) > 0.01f)
    {
        m_sinO = sin(pos.o);
        m_cosO = cos(pos.o);
    }

    if (!m_passengers.empty())
    {
        MAPLOCK_READ(GetOwner(), MAP_LOCK_TYPE_MOVEMENT);
        // Update global positions
        for (PassengerMap::const_iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
            UpdateGlobalPositionOf(itr->first, itr->second.GetLocalPosition());
    }

    m_lastPosition = pos;
}

// Update the global position of a passenger
void TransportBase::UpdateGlobalPositionOf(ObjectGuid const& passengerGuid, Position const& pos) const
{
    WorldObject* passenger = GetOwner()->GetMap()->GetWorldObject(passengerGuid);

    if (!passenger)
        return;

    Position g = CalculateGlobalPositionOf(pos);

    switch(passenger->GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
        case TYPEID_DYNAMICOBJECT:
            m_owner->GetMap()->Relocation((GameObject*)passenger, g.x, g.y, g.z, g.o);
            break;
        case TYPEID_UNIT:
            m_owner->GetMap()->Relocation((Creature*)passenger, g.x, g.y, g.z, g.o);
            // If passenger is vehicle
            if (((Unit*)passenger)->IsVehicle())
                ((Unit*)passenger)->GetVehicleKit()->UpdateGlobalPositions();
            break;
        case TYPEID_PLAYER:
            m_owner->GetMap()->Relocation((Player*)passenger, g.x, g.y, g.z, g.o);
            // If passenger is vehicle
            if (((Unit*)passenger)->IsVehicle())
                ((Unit*)passenger)->GetVehicleKit()->UpdateGlobalPositions();
            break;
        case TYPEID_CORPSE:
        // TODO - add corpse relocation
        default:
            break;
    }
}

// This rotates the vector (lx, ly) by transporter->orientation
void TransportBase::RotateLocalPosition(float lx, float ly, float& rx, float& ry) const
{
    rx = lx * m_cosO - ly * m_sinO;
    ry = lx * m_sinO + ly * m_cosO;
}

// This rotates the vector (rx, ry) by -transporter->orientation
void TransportBase::NormalizeRotatedPosition(float rx, float ry, float& lx, float& ly) const
{
    lx = rx * -m_cosO - ry * -m_sinO;
    ly = rx * -m_sinO + ry * -m_cosO;
}

// Calculate a global position of local positions based on this transporter
Position const& TransportBase::CalculateGlobalPositionOf(Position const& pos) const
{
    Position g(pos);
    RotateLocalPosition(pos.x, pos.y, g.x, g.y);
    g.x += m_owner->GetPositionX();
    g.y += m_owner->GetPositionY();

    g.z = pos.z + m_owner->GetPositionZ();
    g.o = MapManager::NormalizeOrientation(pos.o + m_owner->GetOrientation());
    return g;
}

void TransportBase::BoardPassenger(WorldObject* passenger, Position const& pos, int8 seat)
{
    if (!passenger)
        return;

    MAPLOCK_WRITE(GetOwner(), MAP_LOCK_TYPE_MOVEMENT);

    // Insert our new passenger
    m_passengers.insert(PassengerMap::value_type(passenger->GetObjectGuid(),TransportInfo(passenger, this, pos, seat)));

    PassengerMap::iterator itr = m_passengers.find(passenger->GetObjectGuid());
    MANGOS_ASSERT(itr != m_passengers.end());

    // The passenger needs fast access to transportInfo
    passenger->SetTransportInfo(&itr->second);
}

void TransportBase::UnBoardPassenger(WorldObject* passenger)
{
    if (!passenger)
        return;

    MAPLOCK_WRITE(GetOwner(), MAP_LOCK_TYPE_MOVEMENT);

    PassengerMap::iterator itr = m_passengers.find(passenger->GetObjectGuid());

    if (itr == m_passengers.end())
        return;

    // Set passengers transportInfo to NULL
    passenger->SetTransportInfo(NULL);

    // Delete transportInfo
    // Unboard finally
    m_passengers.erase(itr);
}

/* **************************************** TransportInfo ****************************************/

TransportInfo::TransportInfo(WorldObject* owner, TransportBase* transport, Position const& pos, int8 seat) :
    m_owner(owner),
    m_transport(transport),
    m_localPosition(pos),
    m_seat(seat)
{
    MANGOS_ASSERT(owner && m_transport);
}

TransportInfo::TransportInfo(TransportInfo const& info) :
    m_owner(info.m_owner),
    m_transport(info.m_transport),
    m_localPosition(info.m_localPosition),
    m_seat(info.m_seat)
{
    MANGOS_ASSERT(m_owner && m_transport);
}

void TransportInfo::SetLocalPosition(Position const& pos)
{
    m_localPosition = pos;

    // Update global position
    m_transport->UpdateGlobalPositionOf(m_owner->GetObjectGuid(), pos);
}

TransportInfo::~TransportInfo()
{
    if (m_owner)
        m_owner->SetTransportInfo(NULL);
}

WorldObject* TransportInfo::GetTransport() const 
{
    return m_transport->GetOwner();
}

ObjectGuid TransportInfo::GetTransportGuid() const
{
    return m_transport->GetOwner()->GetObjectGuid();
}

bool TransportInfo::IsOnVehicle() const
{
    return m_transport->GetOwner()->GetTypeId() == TYPEID_PLAYER || m_transport->GetOwner()->GetTypeId() == TYPEID_UNIT;
}

/*! @} */

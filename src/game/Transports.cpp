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

#include "Common.h"

#include "Transports.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Path.h"

#include "WorldPacket.h"
#include "DBCStores.h"
#include "ScriptMgr.h"
#include "TransportSystem.h"
#include "WaypointMovementGenerator.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

Transport::Transport() : GameObject(), m_transportKit(NULL), m_moveGen(NULL)
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
}

Transport::~Transport()
{
    if (m_moveGen)
        delete m_moveGen;

    if (m_transportKit)
        delete m_transportKit;
}

bool Transport::Create(uint32 guidlow, Map* map, uint32 period, uint8 animprogress, uint16 dynamicLowValue)
{
    if (!map)
        return false;

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(guidlow);

    if (!goinfo)
    {
        sLog.outErrorDb("Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u ",guidlow, map->GetId());
        return false;
    }
    m_goInfo = goinfo;
    Object::_Create(ObjectGuid(HIGHGUID_MO_TRANSPORT, guidlow));

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    //SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetUInt32Value(GAMEOBJECT_FLAGS, (GO_FLAG_TRANSPORT | GO_FLAG_NODESPAWN));
    SetEntry(goinfo->id);
    SetDisplayId(goinfo->displayId);
    SetGoState(GO_STATE_READY);
    SetGoType(GameobjectTypes(goinfo->type));
    SetGoArtKit(0);
    SetGoAnimProgress(animprogress);
    SetPeriod(period);
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 0, dynamicLowValue);
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 1, 0);

    SetName(goinfo->name);

    SetMap(map);

    m_transportKit = new TransportKit(*this);

    if (goinfo->moTransport.taxiPathId < sTaxiPathNodesByPath.size())
    {
        m_moveGen = new TransportPathMovementGenerator(sTaxiPathNodesByPath[goinfo->moTransport.taxiPathId], 0);
        MANGOS_ASSERT(m_moveGen);
        m_moveGen->Initialize(*this);
        // Relocate(WorldLocation(mapid, x, y, z, ang));
        // FIXME - instance id and phaseMask isn't set to values different from std.
    }
    // FIXME - instance id and phaseMask isn't set to values different from std.

    if(!IsPositionValid())
    {
        sLog.outError("Transport %s not created. Suggested coordinates isn't valid (%f, %f, %f, map %u)",
            GetObjectGuid().GetString().c_str(), GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId());
        return false;
    }
    map->Add((GameObject*)this);

    if (GetGoState() == GO_STATE_READY)
        Start();

    return true;
}

bool Transport::AddPassenger(WorldObject* passenger, Position const& transportPos)
{
    GetTransportKit()->AddPassenger(passenger, transportPos);
    if (passenger->isType(TYPEMASK_UNIT))
    {
        GroupPetList m_groupPets = ((Unit*)passenger)->GetPets();
        if (!m_groupPets.empty())
        {
            for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                if (Pet* _pet = GetMap()->GetPet(*itr))
                    if (_pet && _pet->IsInWorld())
                        GetTransportKit()->AddPassenger(_pet, transportPos);
        }
    }
    return true;
}

bool Transport::RemovePassenger(WorldObject* passenger)
{
    GetTransportKit()->RemovePassenger(passenger);
    if (passenger->isType(TYPEMASK_UNIT))
    {
        GroupPetList m_groupPets = ((Unit*)passenger)->GetPets();
        if (!m_groupPets.empty())
        {
            for (GroupPetList::const_iterator itr = m_groupPets.begin(); itr != m_groupPets.end(); ++itr)
                if (Pet* _pet = GetMap()->GetPet(*itr))
                    if (_pet && _pet->IsInWorld())
                        GetTransportKit()->RemovePassenger(_pet);
        }
    }
    return true;
}

void Transport::Update(uint32 update_diff, uint32 p_time)
{
    UpdateSplineMovement(p_time);

    if (!m_moveGen)
        return;

    m_moveGen->Update(*this, update_diff);
}

void Transport::BuildStartMovePacket(Map const* targetMap)
{
    SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_ACTIVE);
}

void Transport::BuildStopMovePacket(Map const* targetMap)
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_READY);
}

uint32 Transport::GetPossibleMapByEntry(uint32 entry, bool start)
{
    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);
    if (!goinfo || goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        return UINT32_MAX;

    if (goinfo->moTransport.taxiPathId >= sTaxiPathNodesByPath.size())
        return UINT32_MAX;

    TaxiPathNodeList const& path = sTaxiPathNodesByPath[goinfo->moTransport.taxiPathId];

    if (path.empty())
        return UINT32_MAX;

    if (!start)
    {
        for (size_t i = 0; i < path.size(); ++i)
            if (path[i].mapid != path[0].mapid)
                return path[i].mapid;
    }

    return path[0].mapid;
}

bool Transport::IsSpawnedAtDifficulty(uint32 entry, Difficulty difficulty)
{
    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);
    if (!goinfo || goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        return false;
    if (!goinfo->moTransport.difficultyMask)
        return true;
    return goinfo->moTransport.difficultyMask & uint32( 1 << difficulty);
}

void Transport::Start()
{
    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::StartMovement %s (%s) start moves, period %u %s",
        GetObjectGuid().GetString().c_str(),
        GetName(),
        GetPeriod(),
        m_moveGen ? "with generator" : ""
        );
    SetActiveObjectState(true);
    if (m_moveGen)
        m_moveGen->Reset(*this);
    BuildStartMovePacket(GetMap());
}

void Transport::Stop()
{
    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::StartMovement %s (%s) stop moves, period %u",
        GetObjectGuid().GetString().c_str(),
        GetName(),
        GetPeriod()
        );
    if (m_moveGen)
        m_moveGen->Interrupt(*this);
    SetActiveObjectState(false);
    BuildStopMovePacket(GetMap());
}

// Return true, only if transport has correct position!
bool Transport::SetPosition(WorldLocation const& loc, bool teleport)
{
    // prevent crash when a bad coord is sent by the client
    if (!MaNGOS::IsValidMapCoord(loc.x, loc.y, loc.z, loc.orientation))
    {
        DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::SetPosition(%f, %f, %f, %f, %d) bad coordinates for transport %s!", loc.x, loc.y, loc.z, loc.orientation, teleport, GetName());
        return false;
    }

    if (teleport || GetMapId() != loc.GetMapId())
    {
        Map* oldMap = GetMap();
        Map* newMap = sMapMgr.CreateMap(loc.GetMapId(), this);

        if (!newMap)
        {
            sLog.outError("Transport::SetPosition canot create map %u for transport %s!", loc.GetMapId(), GetName());
            return false;
        }


        if (oldMap != newMap)
        {
            // Transport inserted in current map ActiveObjects list
            if (!GetTransportKit()->GetPassengers().empty())
            {
                DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::SetPosition %s notify passengers (count %u) for change map from %u to %u",GetObjectGuid().GetString().c_str(), GetTransportKit()->GetPassengers().size(), GetPosition().GetMapId(), loc.GetMapId());
                GetTransportKit()->CallForAllPassengers(NotifyMapChangeBegin(oldMap, GetPosition(), loc));
            }

            oldMap->Remove((GameObject*)this, false);
            SetMap(newMap);

            newMap->Relocation((GameObject*)this, loc);
            newMap->Add((GameObject*)this);

            // Transport inserted in current map ActiveObjects list
            if (!GetTransportKit()->GetPassengers().empty())
            {
                DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::SetPosition %s notify passengers (count %u) for finished change map to %u",GetObjectGuid().GetString().c_str(), GetTransportKit()->GetPassengers().size(), loc.GetMapId());
                GetTransportKit()->CallForAllPassengers(NotifyMapChangeEnd(newMap,loc));
            }

            DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::SetPosition %s teleported from map %u to map %u (%f, %f, %f, %f)", GetObjectGuid().GetString().c_str(), oldMap->GetId(), newMap->GetId(), loc.x, loc.y, loc.z, loc.orientation);
        }
        else if (!(GetPosition() == loc))
            GetMap()->Relocation((GameObject*)this, loc);
    }
    else if (!(GetPosition() == loc))
        GetMap()->Relocation((GameObject*)this, loc);


    return false;
}


/**
 * @addtogroup TransportSystem
 * @{
 *
 * @class TransportKit
 * This classe contains the code needed for MaNGOS to provide abstract support for GO transporter
 */

 
TransportKit::TransportKit(Transport& base)
    : TransportBase(&base), m_isInitialized(false)
{
}

TransportKit::~TransportKit()
{
    RemoveAllPassengers();
}

void TransportKit::Initialize()
{
    m_isInitialized = true;
}

void TransportKit::RemoveAllPassengers()
{
}

void TransportKit::Reset()
{
    m_isInitialized = true;
}

bool TransportKit::AddPassenger(WorldObject* passenger, Position const& transportPos)
{
    // Calculate passengers local position, if not provided
    BoardPassenger(passenger, transportPos.IsEmpty() ? CalculateBoardingPositionOf(passenger->GetPosition()) : transportPos, -1);

    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"TransportKit::AddPassenger %s boarded on %s offset %f %f %f", 
        passenger->GetObjectGuid().GetString().c_str(), GetBase()->GetObjectGuid().GetString().c_str(), transportPos.getX(), transportPos.getY(), transportPos.getZ());
}

void TransportKit::RemovePassenger(WorldObject* passenger)
{
    UnBoardPassenger(passenger);

    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"Transport::RemovePassenger %s unboarded from  %s.", 
        passenger->GetObjectGuid().GetString().c_str(), GetBase()->GetObjectGuid().GetString().c_str());
}

// Helper function to undo the turning of the vehicle to calculate a relative position of the passenger when boarding
Position TransportKit::CalculateBoardingPositionOf(Position const& pos) const override
{
    Position l(pos);
    NormalizeRotatedPosition(pos.x - GetBase()->GetPositionX(), pos.y - GetBase()->GetPositionY(), l.x, l.y);

    l.z = pos.z - GetBase()->GetPositionZ();
    l.o = MapManager::NormalizeOrientation(pos.o - GetBase()->GetOrientation());
    return l;
}


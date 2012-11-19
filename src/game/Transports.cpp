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
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Path.h"

#include "WorldPacket.h"
#include "DBCStores.h"
#include "ProgressBar.h"
#include "ScriptMgr.h"

#include "movement/MoveSpline.h"

void MapManager::LoadTransports()
{
    QueryResult *result = WorldDatabase.Query("SELECT entry, name, period FROM transports");

    uint32 count = 0;

    if( !result )
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u transports", count );
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();

        Transport *t = new Transport;

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        std::string name = fields[1].GetCppString();
        t->m_period = fields[2].GetUInt32();

        GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);

        if(!goinfo)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template missing", entry, name.c_str());
            delete t;
            continue;
        }

        if(goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template type wrong", entry, name.c_str());
            delete t;
            continue;
        }

        // setting mapID's, binded to transport GO
        if (goinfo->moTransport.mapID)
        {
            m_mapOnTransportGO.insert(std::make_pair(goinfo->moTransport.mapID,t));
            DEBUG_LOG("Loading transport %u between %s, %s map id %u", entry, name.c_str(), goinfo->name, goinfo->moTransport.mapID);
        }

        // sLog.outString("Loading transport %d between %s, %s", entry, name.c_str(), goinfo->name);

        std::set<uint32> mapsUsed;

        if(!t->GenerateWaypoints(entry, mapsUsed))
            // skip transports with empty waypoints list
        {
            sLog.outErrorDb("Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.",goinfo->moTransport.taxiPathId);
            delete t;
            continue;
        }

        float x, y, z, o;
        uint32 mapid;
        x = t->m_WayPoints[0].x; y = t->m_WayPoints[0].y; z = t->m_WayPoints[0].z; mapid = t->m_WayPoints[0].mapid; o = 1;

        //current code does not support transports in dungeon!
        const MapEntry* pMapInfo = sMapStore.LookupEntry(mapid);
        if(!pMapInfo || pMapInfo->Instanceable())
        {
            delete t;
            continue;
        }

        // creates the Gameobject
        if (!t->Create(entry, mapid, x, y, z, o, GO_ANIMPROGRESS_DEFAULT, 0))
        {
            delete t;
            continue;
        }

        m_Transports.insert(t);

        for (std::set<uint32>::const_iterator i = mapsUsed.begin(); i != mapsUsed.end(); ++i)
            m_TransportsByMap[*i].insert(t);

        //If we someday decide to use the grid to track transports, here:
        Map* map = sMapMgr.CreateMap(mapid, t);
        t->SetMap(map);
        map->InsertObject(t);

        ++count;
    } while(result->NextRow());
    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u transports", count );
    sLog.outString( ">> Loaded " SIZEFMTD " transports with mapID's", m_mapOnTransportGO.size() );

    // check transport data DB integrity
    result = WorldDatabase.Query("SELECT gameobject.guid,gameobject.id,transports.name FROM gameobject,transports WHERE gameobject.id = transports.entry");
    if(result)                                              // wrong data found
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            std::string name = fields[2].GetCppString();
            sLog.outErrorDb("Transport %u '%s' have record (GUID: %u) in `gameobject`. Transports DON'T must have any records in `gameobject` or its behavior will be unpredictable/bugged.",entry,name.c_str(),guid);
        }
        while(result->NextRow());

        delete result;
    }
}

bool MapManager::IsTransportMap(uint32 mapid)
{
    TransportGOMap::const_iterator itr = m_mapOnTransportGO.find(mapid);
    if (itr != m_mapOnTransportGO.end())
        return true;
    return false;
}

Transport* MapManager::GetTransportByGOMapId(uint32 mapid)
{
    TransportGOMap::const_iterator itr = m_mapOnTransportGO.find(mapid);
    if (itr != m_mapOnTransportGO.end())
        return itr->second;
    return NULL;
}

Transport::Transport() : GameObject()
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
}

bool Transport::Create(uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint8 animprogress, uint16 dynamicHighValue)
{
    Relocate(x,y,z,ang);
    // instance id and phaseMask isn't set to values different from std.

    if(!IsPositionValid())
    {
        sLog.outError("Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow,x,y);
        return false;
    }

    Object::_Create(ObjectGuid(HIGHGUID_MO_TRANSPORT, guidlow));

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(guidlow);

    if (!goinfo)
    {
        sLog.outErrorDb("Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f",guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    //SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetUInt32Value(GAMEOBJECT_FLAGS, (GO_FLAG_TRANSPORT | GO_FLAG_NODESPAWN));
    SetUInt32Value(GAMEOBJECT_LEVEL, m_period);
    SetEntry(goinfo->id);

    SetDisplayId(goinfo->displayId);

    SetGoState(GO_STATE_READY);
    SetGoType(GameobjectTypes(goinfo->type));
    SetGoArtKit(0);
    SetGoAnimProgress(animprogress);

    // low part always 0, dynamicHighValue is some kind of progression (not implemented)
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 0, 0);
    SetUInt16Value(GAMEOBJECT_DYNAMIC, 1, dynamicHighValue);

    SetName(goinfo->name);

    return true;
}

enum TransportWaypointsDefines
{
    TRANSPORT_POSITION_UPDATE_DELAY           = 500,
    TRANSPORT_SPEED_UPDATE_DELAY              = 1000,
};

bool Transport::GenerateWaypoints(uint32 entry, std::set<uint32> &mapids)
{
    GameObjectInfo const* goInfo = ObjectMgr::GetGameObjectInfo(entry);

    if (!goInfo)
        return false;

    uint32 pathid = goInfo->moTransport.taxiPathId;

    if (pathid >= sTaxiPathNodesByPath.size())
        return false;

    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathid];

    bool mapChange = false;
    mapids.clear();

    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (!mapChange)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (node_i.mapid == path[i+1].mapid)
                mapids.insert(node_i.mapid);
            else
                mapChange = true;
        }
        else
            mapChange = false;
    }

    Movement::PointsArray splinepath;
    float velocity  = goInfo->moTransport.moveSpeed;
    float accelRate = goInfo->moTransport.accelRate;

    uint32 time_step = TRANSPORT_POSITION_UPDATE_DELAY;

    bool never_teleport = true; //if never teleported is cyclic path

    uint32 t = 0;
    uint32 start = 0;

    for (size_t i = 0; i < path.size(); ++i)
    {
        TaxiPathNodeEntry const& node_i = path[i];
        splinepath.push_back(G3D::Vector3(node_i.x, node_i.y, node_i.z));

        //split path in segments
        bool teleport = node_i.mapid != path[(i+1)%path.size()].mapid || (path[i].actionFlag == 1); //teleport on next node?
        if (teleport || i == path.size() - 1)
        {
            never_teleport &= !teleport;

            //re-add first node for cyclic path if never teleported
            if(i == path.size() - 1 && never_teleport)
                splinepath.push_back(G3D::Vector3(path[0].x, path[0].y, path[0].z));

            //generate spline for seg
            Movement::Spline<int32> spline;
            spline.init_spline(&splinepath[0], splinepath.size(), Movement::SplineBase::ModeCatmullrom);
            Movement::CommonInitializer init(velocity);
            spline.initLengths(init);

            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::GenerateWaypoints [%d] Generate spline for seg: %d-%d (%d-%d)", pathid, start+spline.first()-1, start+spline.last()-1, spline.first(), spline.last());

            //add first point of seg to waypoints
            m_WayPoints[t] = WayPoint(path[start].mapid, path[start].x, path[start].y, path[start].z, false, path[start].arrivalEventID, path[start].departureEventID);
            t += path[start].delay * IN_MILLISECONDS;

//            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::GenerateWaypoints [%d] %d T: %d, x: %f, y: %f, z: %f, t:%d", pathid, path[start].index, t, path[start].x, path[start].y, path[start].z, false);

            //sample the segment
            for (int32 point_Idx = spline.first(); point_Idx < spline.last(); ++point_Idx)
            {
                uint32 node_Idx = (start + point_Idx) % path.size();

                float u = 1.0f;
                int32 seg_time = spline.length(point_Idx, point_Idx + 1);

                uint32 time_passed = time_step;
                int32 accelTime = 0;
                do
                {
                    if (seg_time > 0)
                    {
                        u = (float)time_passed / (float)seg_time;
                    }
                    else
                        break;

                    if (u >= 1.0f)
                        break;

                    Movement::Location c;
                    spline.evaluate_percent(point_Idx, u, c);

                    if (path[node_Idx].actionFlag == 2)
                        accelTime += time_step * accelRate / velocity * 10;     //deceleration
                    else if (node_Idx > 1 && path[node_Idx-1].actionFlag == 2)
                        accelTime -= time_step * accelRate / velocity * 10;     //acceleration

                    //add sample to waypoints
                    m_WayPoints[t + time_passed] = WayPoint(node_i.mapid, c.x, c.y, c.z, false, 0);
//                    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::GenerateWaypoints [%d] T: %d (%u, %i), x: %f, y: %f, z: %f, t:%d ", pathid, t + time_passed + accelTime, time_passed, accelTime, c.x, c.y, c.z, teleport);

                    time_passed += time_step;
                } while (u < 1.0f);
                t += seg_time;

                //add point to waypoints (if isn't first)
                if (node_Idx > 0)
                {
                    bool teleport_on_this_node = ((point_Idx == (spline.last() - 1)) ? teleport : false);   //if is the last node of segment -> teleport
                    teleport_on_this_node |= ((node_Idx == (path.size() -1)) ? !never_teleport : false);    //if is the last node of path, teleport if teleported at least one time 
                    m_WayPoints[t] = WayPoint(path[node_Idx].mapid, path[node_Idx].x, path[node_Idx].y, path[node_Idx].z, teleport_on_this_node, path[node_Idx].arrivalEventID, path[node_Idx].departureEventID);
//                    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::GenerateWaypoints [%d] %d T: %d, x: %f, y: %f, z: %f, t:%d", pathid, path[node_Idx].index, t, path[node_Idx].x, path[node_Idx].y, path[node_Idx].z, teleport_on_this_node);
                    t += path[node_Idx].delay * IN_MILLISECONDS;
                }
            }
            splinepath.clear();
            start = i + 1;
        }
    }

    m_next = m_WayPoints.begin();                           // will used in MoveToNextWayPoint for init m_curr
    MoveToNextWayPoint();                                   // m_curr -> first point
    MoveToNextWayPoint();                                   // skip first point

    m_nextNodeTime = m_curr->first;
    m_pathTime = t;

    return true;
}

void Transport::MoveToNextWayPoint()
{
    m_curr = m_next;

    ++m_next;
    if (m_next == m_WayPoints.end())
        m_next = m_WayPoints.begin();
}

void Transport::TeleportTransport(uint32 newMapid, float x, float y, float z)
{
    Map* oldMap = GetMap();
    Relocate(x, y, z);

    for(PlayerSet::iterator itr = m_passengers.begin(); itr != m_passengers.end();)
    {
        PlayerSet::iterator it2 = itr;
        ++itr;

        Player *plr = *it2;
        if(!plr)
        {
            m_passengers.erase(it2);
            continue;
        }

        if (plr->isDead() && !plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        {
            plr->ResurrectPlayer(1.0);
        }
        plr->TeleportTo(newMapid, x, y, z, GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NODELAY);

        //WorldPacket data(SMSG_811, 4);
        //data << uint32(0);
        //plr->GetSession()->SendPacket(&data);
    }

    //we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    //player far teleport would try to create same instance, but we need it NOW for transport...
    //correct me if I'm wrong O.o
    Map* newMap = sMapMgr.CreateMap(newMapid, this);
    SetMap(newMap);

    if (oldMap != newMap)
    {
        UpdateForMap(oldMap);
        oldMap->EraseObject(this);
        UpdateForMap(newMap);
        newMap->InsertObject(this);
    }
}

bool Transport::AddPassenger(Player* passenger)
{
    if (m_passengers.find(passenger) == m_passengers.end())
    {
        DETAIL_LOG("Player %s boarded transport %s.", passenger->GetName(), GetName());
        m_passengers.insert(passenger);
    }
    return true;
}

bool Transport::RemovePassenger(Player* passenger)
{
    if (m_passengers.erase(passenger))
        DETAIL_LOG("Player %s removed from transport %s.", passenger->GetName(), GetName());
    return true;
}

void Transport::Update( uint32 update_diff, uint32 /*p_time*/)
{
    if (m_WayPoints.size() <= 1)
        return;

    m_timer = WorldTimer::getMSTime() % m_period;
    while (((m_timer - m_curr->first) % m_pathTime) > ((m_next->first - m_curr->first) % m_pathTime))
    {

        DoEventIfAny(*m_curr,true);

        MoveToNextWayPoint();

        DoEventIfAny(*m_curr,false);

        // first check help in case client-server transport coordinates de-synchronization
        if (m_curr->second.mapid != GetMapId() || m_curr->second.teleport)
        {
            TeleportTransport(m_curr->second.mapid, m_curr->second.x, m_curr->second.y, m_curr->second.z);
        }
        else
        {
            Relocate(m_curr->second.x, m_curr->second.y, m_curr->second.z);
        }

        /*
        for(PlayerSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
        {
            PlayerSet::const_iterator it2 = itr;
            ++itr;
            //(*it2)->SetPosition( m_curr->second.x + (*it2)->GetTransOffsetX(), m_curr->second.y + (*it2)->GetTransOffsetY(), m_curr->second.z + (*it2)->GetTransOffsetZ(), (*it2)->GetTransOffsetO() );
        }
        */

        m_nextNodeTime = m_curr->first;

        if (m_curr == m_WayPoints.begin())
            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, " ************ BEGIN ************** %s", GetName());

        DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "%s moved to %f %f %f %d", GetName(), m_curr->second.x, m_curr->second.y, m_curr->second.z, m_curr->second.mapid);
    }
}

void Transport::UpdateForMap(Map const* targetMap)
{
    Map::PlayerList const& pl = targetMap->GetPlayers();
    if (pl.isEmpty())
        return;

    if (GetMapId()==targetMap->GetId())
    {
        for(Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
        {
            if(this != itr->getSource()->GetTransport())
            {
                UpdateData transData;
                BuildCreateUpdateBlockForPlayer(&transData, itr->getSource());
                WorldPacket packet;
                transData.BuildPacket(&packet);
                itr->getSource()->SendDirectMessage(&packet);
            }
        }
    }
    else
    {
        UpdateData transData;
        BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket out_packet;
        transData.BuildPacket(&out_packet);

        for(Map::PlayerList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr)
            if(this != itr->getSource()->GetTransport())
                itr->getSource()->SendDirectMessage(&out_packet);
    }
}

void Transport::DoEventIfAny(WayPointMap::value_type const& node, bool departure)
{
    if (uint32 eventid = departure ? node.second.departureEventID : node.second.arrivalEventID)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Taxi %s event %u of node %u of %s \"%s\") path", departure ? "departure" : "arrival", eventid, node.first, GetGuidStr().c_str(), GetName());

        if (!sScriptMgr.OnProcessEvent(eventid, this, this, departure))
            GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
    }
}

void Transport::BuildStartMovePacket(Map const* targetMap)
{
    SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_ACTIVE);
    UpdateForMap(targetMap);
}

void Transport::BuildStopMovePacket(Map const* targetMap)
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_READY);
    UpdateForMap(targetMap);
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
    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::StartMovement %s (%s) start moves, period %u/%u",
        GetObjectGuid().GetString().c_str(),
        GetName(),
        m_pathTime,
        GetPeriod()
        );
    SetActiveObjectState(true);
    BuildStartMovePacket(GetMap());
}
 
void Transport::Stop()
{
    DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "Transport::StartMovement %s (%s) stop moves, period %u/%u",
        GetObjectGuid().GetString().c_str(),
        GetName(),
        m_pathTime,
        GetPeriod()
        );
    SetActiveObjectState(false);
    BuildStopMovePacket(GetMap());
}

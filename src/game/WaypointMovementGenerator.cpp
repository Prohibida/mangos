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

#include <ctime>

#include "WaypointMovementGenerator.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "WaypointManager.h"
#include "WorldPacket.h"
#include "ScriptMgr.h"
#include "Transports.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

#include <cassert>

//-----------------------------------------------//
void WaypointMovementGenerator<Creature>::LoadPath(Creature &creature)
{
    DETAIL_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "LoadPath: loading waypoint path for %s", creature.GetGuidStr().c_str());

    i_path = sWaypointMgr.GetPath(creature.GetGUIDLow());

    // We may LoadPath() for several occasions:

    // 1: When creature.MovementType=2
    //    1a) Path is selected by creature.guid == creature_movement.id
    //    1b) Path for 1a) does not exist and then use path from creature.GetEntry() == creature_movement_template.entry

    // 2: When creature_template.MovementType=2
    //    2a) Creature is summoned and has creature_template.MovementType=2
    //        Creators need to be sure that creature_movement_template is always valid for summons.
    //        Mob that can be summoned anywhere should not have creature_movement_template for example.

    // No movement found for guid
    if (!i_path)
    {
        i_path = sWaypointMgr.GetPathTemplate(creature.GetEntry());

        // No movement found for entry
        if (!i_path)
        {
            sLog.outErrorDb("WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u) doesn't have waypoint path",
                creature.GetName(), creature.GetEntry(), creature.GetGUIDLow());
            return;
        }
    }

    // Initialize the i_currentNode to point to the first node
    if (i_path->empty())
        return;
    i_currentNode = i_path->begin()->first;
}

void WaypointMovementGenerator<Creature>::Initialize(Creature &creature)
{
    LoadPath(creature);
    creature.addUnitState(UNIT_STAT_ROAMING|UNIT_STAT_ROAMING_MOVE);
}

void WaypointMovementGenerator<Creature>::Finalize(Creature &creature)
{
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

void WaypointMovementGenerator<Creature>::Interrupt(Creature &creature)
{
    creature.InterruptMoving();
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

void WaypointMovementGenerator<Creature>::Reset(Creature &creature)
{
    creature.addUnitState(UNIT_STAT_ROAMING|UNIT_STAT_ROAMING_MOVE);
    StartMoveNow(creature);
}

void WaypointMovementGenerator<Creature>::OnArrived(Creature& creature)
{
    if (!i_path || i_path->empty())
        return;

    if (m_isArrivalDone)
        return;

    creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
    m_isArrivalDone = true;

    WaypointPath::const_iterator currPoint = i_path->find(i_currentNode);
    MANGOS_ASSERT(currPoint != i_path->end());
    WaypointNode const& node = currPoint->second;

    if (node.script_id)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Creature movement start script %u at point %u for %s.", node.script_id, i_currentNode, creature.GetGuidStr().c_str());
        creature.GetMap()->ScriptsStart(sCreatureMovementScripts, node.script_id, &creature, &creature);
    }

    // We have reached the destination and can process behavior
    if (WaypointBehavior* behavior = node.behavior)
    {
        if (behavior->emote != 0)
            creature.HandleEmote(behavior->emote);

        if (behavior->spell != 0)
            creature.CastSpell(&creature, behavior->spell, false);

        if (behavior->model1 != 0)
            creature.SetDisplayId(behavior->model1);

        if (behavior->textid[0])
        {
            // Not only one text is set
            if (behavior->textid[1])
            {
                // Select one from max 5 texts (0 and 1 already checked)
                int i = 2;
                for(; i < MAX_WAYPOINT_TEXT; ++i)
                {
                    if (!behavior->textid[i])
                        break;
                }

                creature.MonsterSay(behavior->textid[rand() % i], LANG_UNIVERSAL);
            }
            else
                creature.MonsterSay(behavior->textid[0], LANG_UNIVERSAL);
        }
    }

    // Inform script
    MovementInform(creature);
    Stop(node.delay);
}

void WaypointMovementGenerator<Creature>::StartMoveNow(Creature& creature)
{
    i_nextMoveTime.Reset(0);
    creature.clearUnitState(UNIT_STAT_WAYPOINT_PAUSED);
    StartMove(creature);
}

void WaypointMovementGenerator<Creature>::StartMove(Creature& creature)
{
    if (!i_path || i_path->empty())
        return;

    if (Stopped(creature))
        return;

    WaypointPath::const_iterator currPoint = i_path->find(i_currentNode);
    MANGOS_ASSERT(currPoint != i_path->end());

    if (WaypointBehavior* behavior = currPoint->second.behavior)
    {
        if (behavior->model2 != 0)
            creature.SetDisplayId(behavior->model2);
        creature.SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
    }

    if (m_isArrivalDone)
    {
        ++currPoint;
        if (currPoint == i_path->end())
            currPoint = i_path->begin();

        i_currentNode = currPoint->first;
    }

    m_isArrivalDone = false;

    creature.addUnitState(UNIT_STAT_ROAMING_MOVE);

    WaypointNode const& nextNode = currPoint->second;;
    Movement::MoveSplineInit<Unit*> init(creature);
    init.MoveTo(nextNode.x, nextNode.y, nextNode.z, true);

    if (nextNode.orientation != 100 && nextNode.delay != 0)
        init.SetFacing(nextNode.orientation);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE) && !creature.IsLevitating(), false);
    init.Launch();
}

bool WaypointMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
    // Waypoint movement can be switched on/off
    // This is quite handy for escort quests and other stuff
    if (creature.hasUnitState(UNIT_STAT_NOT_MOVE))
    {
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
    {
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    if (Stopped(creature))
    {
        if (CanMove(diff, creature))
            StartMove(creature);
    }
    else
    {
        if (creature.IsStopped())
            Stop(STOP_TIME_FOR_PLAYER);
        else if (creature.movespline->Finalized())
        {
            OnArrived(creature);
            StartMove(creature);
        }
    }
    return true;
}

void WaypointMovementGenerator<Creature>::MovementInform(Creature &creature)
{
    if (creature.AI())
        creature.AI()->MovementInform(WAYPOINT_MOTION_TYPE, i_currentNode);
}

bool WaypointMovementGenerator<Creature>::GetResetPosition(Creature&, float& x, float& y, float& z) const
{
    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    WaypointPath::const_iterator currPoint = i_path->find(i_currentNode);
    MANGOS_ASSERT(currPoint != i_path->end());

    x = currPoint->second.x; y = currPoint->second.y; z = currPoint->second.z;
    return true;
}

bool WaypointMovementGenerator<Creature>::Stopped(Creature& u)
{
    return !i_nextMoveTime.Passed() || u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED);
}

bool WaypointMovementGenerator<Creature>::CanMove(int32 diff, Creature& u)
{
    i_nextMoveTime.Update(diff);
    if (i_nextMoveTime.Passed() && u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED))
        i_nextMoveTime.Reset(1);

    return i_nextMoveTime.Passed() && !u.hasUnitState(UNIT_STAT_WAYPOINT_PAUSED);
}

void WaypointMovementGenerator<Creature>::AddToWaypointPauseTime(int32 waitTimeDiff)
{
    if (!i_nextMoveTime.Passed())
    {
        // Prevent <= 0, the code in Update requires to catch the change from moving to not moving
        int32 newWaitTime = i_nextMoveTime.GetExpiry() + waitTimeDiff;
        i_nextMoveTime.Reset(newWaitTime > 0 ? newWaitTime : 1);
    }
}

//----------------------------------------------------//
uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (i_currentNode >= i_path->size())
        return i_path->size();

    uint32 curMapId = (*i_path)[i_currentNode].mapid;

    for(uint32 i = i_currentNode; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].mapid != curMapId)
            return i;
    }

    return i_path->size();
}

void FlightPathMovementGenerator::_Initialize(Player &player)
{
    _Reset(player);
}

void FlightPathMovementGenerator::_Finalize(Player & player)
{
    if (player.m_taxi.empty())
    {
        // update z position to ground and orientation for landing point
        // this prevent cheating with landing  point at lags
        // when client side flight end early in comparison server side
        player.StopMoving(true);
    }
}

void FlightPathMovementGenerator::_Interrupt(Player & player)
{
}

#define PLAYER_FLIGHT_SPEED        32.0f

void FlightPathMovementGenerator::_Reset(Player & player)
{
    Movement::MoveSplineInit<Unit*> init(player);
    uint32 end = GetPathAtMapEnd();
    for (uint32 i = GetCurrentNode(); i != end; ++i)
    {
        G3D::Vector3 vertice((*i_path)[i].x,(*i_path)[i].y,(*i_path)[i].z);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(GetCurrentNode());
    init.SetFly();
    init.SetVelocity(PLAYER_FLIGHT_SPEED);
    init.Launch();
}

bool FlightPathMovementGenerator::Update(Player &player, const uint32 &diff)
{
    uint32 pointId = (uint32)player.movespline->currentPathIdx();
    if (pointId > i_currentNode)
    {
        bool departureEvent = true;
        do
        {
            DoEventIfAny(player,(*i_path)[i_currentNode],departureEvent);
            if (pointId == i_currentNode)
                break;
            i_currentNode += (uint32)departureEvent;
            departureEvent = !departureEvent;
        } while(true);
    }

    return !(player.movespline->Finalized() || i_currentNode >= (i_path->size()-1));
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (i_path->empty())
        return;

    uint32 map0 = (*i_path)[0].mapid;

    for (size_t i = 1; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].mapid != map0)
        {
            i_currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.departureEventID : node.arrivalEventID)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node.index, node.path, player.GetName());
        StartEvents_Event(player.GetMap(), eventid, &player, &player, departure);
    }
}

bool FlightPathMovementGenerator::GetResetPosition(Player&, float& x, float& y, float& z) const
{
    const TaxiPathNodeEntry& node = (*i_path)[i_currentNode];
    x = node.x; y = node.y; z = node.z;
    return true;
}

//----------------------------------------------------//
uint32 TransportPathMovementGenerator::GetPathAtMapEnd(bool withAnchorage) const
{
    if (GetCurrentNode() >= i_path->size())
        return i_path->size();

    uint32 curMapId = (*i_path)[GetCurrentNode()].mapid;

    for (uint32 i = GetCurrentNode(); i < i_path->size(); ++i)
    {
        if ((*i_path)[i].mapid != curMapId)
            return i;
        if (withAnchorage && (i > GetCurrentNode()) && ((*i_path)[i].delay > 0) && ((i +1) < i_path->size()))
            return (i+1);
    }

    return i_path->size();
}

void TransportPathMovementGenerator::Initialize(Transport& go)
{
    m_anchorageTimer.SetInterval(0);
    m_anchorageTimer.Reset();
    TaxiPathNodeEntry const* nextNode = &(*i_path)[0];
    WorldLocation newLoc = WorldLocation(nextNode->mapid, nextNode->x, nextNode->y, nextNode->z, go.GetOrientation(), go.GetPhaseMask(), go.GetInstanceId());
    go.GetMap()->Relocation(((GameObject*)&go), newLoc);
}

void TransportPathMovementGenerator::Finalize(Transport& go)
{
    //go.StopMoving();
}

void TransportPathMovementGenerator::Interrupt(Transport& go)
{
    if (!go.movespline->Finalized())
    {
        WorldLocation loc = go.GetPosition();
        loc = go.movespline->ComputePosition();
        go.movespline->_Interrupt();
        go.SetPosition(loc, false);
    }
}

void TransportPathMovementGenerator::Reset(Transport& go)
{
    Movement::MoveSplineInit<GameObject*> init(go);
    uint32 end = GetPathAtMapEnd(true);
    G3D::Vector3 vertice(go.GetPosition());
    for (uint32 i = GetCurrentNode(); i != end; ++i)
    {
        G3D::Vector3 vertice((*i_path)[i].x,(*i_path)[i].y,(*i_path)[i].z);
        init.Path().push_back(vertice);
    }
    //go.SetOrientation();
    init.SetFirstPointId(GetCurrentNode());
    init.SetVelocity(go.GetGOInfo()->moTransport.moveSpeed);
    init.SetFly();
    init.Launch();
    DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::Reset %s path resetted, start %u end %u", go.GetObjectGuid().GetString().c_str(), GetCurrentNode(), end);
}

bool TransportPathMovementGenerator::Update(Transport& go, uint32 const& diff)
{
    uint32 pointId = (uint32)go.movespline->currentPathIdx();
    bool anchorage = !m_anchorageTimer.Passed();
    bool movementFinished = go.movespline->Finalized();

    // Normal movement (also arriving to anchorage)
    if (!anchorage && (pointId > GetCurrentNode()))
    {
        DoEventIfAny(go,(*i_path)[GetCurrentNode()], true);
        MoveToNextNode();
        DoEventIfAny(go,(*i_path)[GetCurrentNode()], false);
        if (!movementFinished)
            return true;
    }

    // Anchorage delay
    if (anchorage)
    {
        m_anchorageTimer.Update(diff);
        if (m_anchorageTimer.Passed())
        {
            DoEventIfAny(go,(*i_path)[GetCurrentNode()], true);
            Reset(go);
            DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"TransportPathMovementGenerator::Update %s finish delay movement",go.GetObjectGuid().GetString().c_str());
            m_anchorageTimer.SetInterval(0);
            m_anchorageTimer.Reset();
            return false;
        }
        return true;
    }

    if (movementFinished && !anchorage)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::Update path part finished, %s point %u node %u", go.GetObjectGuid().GetString().c_str(), pointId, GetCurrentNode());
        bool isPathBreak = IsPathBreak();
        // End of current path part (teleporting or anchoring coming soon)
        if ((*i_path)[GetCurrentNode()].delay > 0)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES,"TransportPathMovementGenerator::Update %s start delay movement to %u",go.GetObjectGuid().GetString().c_str(), (*i_path)[GetCurrentNode()].delay);
            m_anchorageTimer.SetInterval((*i_path)[GetCurrentNode()].delay * IN_MILLISECONDS);
            m_anchorageTimer.Reset();
            return false;
        }
        else if (isPathBreak)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::Update %s point %u node %u - path break (current map %u, next map %u)", go.GetObjectGuid().GetString().c_str(), pointId, GetCurrentNode(), go.GetMapId(), GetCurrentNodeEntry()->mapid);
            TaxiPathNodeEntry const* nextNode = GetCurrentNodeEntry();
            WorldLocation newLoc = WorldLocation(nextNode->mapid, nextNode->x, nextNode->y, nextNode->z, go.GetOrientation(), go.GetPhaseMask(), go.GetInstanceId());
            go.SetPosition(newLoc, true);
            MoveToNextNode();
            Reset(go);
            return false;
        }
        else
        {
            //MoveToNextNode();
            Reset(go);
        }
    }

    // Old method for movement calculation - used for create correctives. FIXME - need remove after found true method for acceleration calculation...
    uint32 m_timer = WorldTimer::getMSTime() % go.GetPeriod();
    while (((m_timer - m_curr->first) % m_pathTime) > ((m_next->first - m_curr->first) % m_pathTime))
    {

        MoveToNextWayPoint();

        float distance = m_curr->second.loc.GetMapId() == go.GetMapId() ? go.GetDistance(m_curr->second.loc) : -1.0f;
        if (distance < 0.0f || distance > DEFAULT_VISIBILITY_DISTANCE / 2.0)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::Update %s must be moved to %f %f %f %d %s, but really in %f %f %f %d, distance=%f",
                go.GetObjectGuid().GetString().c_str(), m_curr->second.loc.x, m_curr->second.loc.y, m_curr->second.loc.z, m_curr->second.loc.GetMapId(), m_curr == m_WayPoints.begin() ? "(begin move)" : "",
                go.GetPositionX(), go.GetPositionY(), go.GetPositionZ(), go.GetMapId(), distance
                );
            Interrupt(go);
            if (go.SetPosition(m_curr->second.loc, m_curr->second.teleport))
            {
                if (!go.GetTransportKit()->IsInitialized())
                    go.GetTransportKit()->Initialize();
                else
                // Update passenger positions
                    go.GetTransportKit()->Update(diff);
            }
            Reset(go);
        }
    }

    // FIXME - true after debug stage
    return false;
}

void TransportPathMovementGenerator::DoEventIfAny(Transport& go, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.departureEventID : node.arrivalEventID)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::DoEventIfAny %s activate taxi %s event %u (node %u)", go.GetGuidStr().c_str(), departure ? "departure" : "arrival", eventid, node.index);

        if (!sScriptMgr.OnProcessEvent(eventid, &go, &go, departure))
            go.GetMap()->ScriptsStart(sEventScripts, eventid, &go, &go);
    }
}

TaxiPathNodeEntry const* TransportPathMovementGenerator::GetCurrentNodeEntry() const
{
    return &(*i_path)[GetCurrentNode()];
}

TaxiPathNodeEntry const* TransportPathMovementGenerator::GetNextNodeEntry() const
{
    if (i_currentNode >= (i_path->size()-1))
        return &(*i_path)[0];
    else
        return &(*i_path)[GetCurrentNode()+1];
}

void TransportPathMovementGenerator::MoveToNextNode()
{
    if (i_currentNode >= (i_path->size()-1))
        i_currentNode = 0;
    else
        ++i_currentNode;
}

bool TransportPathMovementGenerator::IsPathBreak() const
{
    return GetCurrentNode() >= GetPathAtMapEnd() ||
        GetCurrentNodeEntry()->mapid !=  GetNextNodeEntry()->mapid;
}

struct keyFrame
{
    explicit keyFrame(TaxiPathNodeEntry const& _node) : node(&_node),
        distSinceStop(-1.0f), distUntilStop(-1.0f), distFromPrev(-1.0f), tFrom(0.0f), tTo(0.0f)
    {
    }

    TaxiPathNodeEntry const* node;

    float distSinceStop;
    float distUntilStop;
    float distFromPrev;
    float tFrom, tTo;
};

bool TransportPathMovementGenerator::GenerateWaypoints()
{
    TaxiPathNodeList const& path = *i_path;

    std::vector<keyFrame> keyFrames;
    int mapChange = 0;
    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (mapChange == 0)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (node_i.mapid == path[i+1].mapid)
            {
                keyFrame k(node_i);
                keyFrames.push_back(k);
            }
            else
            {
                mapChange = 1;
            }
        }
        else
        {
            --mapChange;
        }
    }

    int lastStop = -1;
    int firstStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].distFromPrev = 0;
    if (keyFrames[0].node->actionFlag == 2)
    {
        lastStop = 0;
    }

    // find the rest of the distances between key points
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if ((keyFrames[i].node->actionFlag == 1) || (keyFrames[i].node->mapid != keyFrames[i-1].node->mapid))
        {
            keyFrames[i].distFromPrev = 0;
        }
        else
        {
            keyFrames[i].distFromPrev =
                sqrt(pow(keyFrames[i].node->x - keyFrames[i - 1].node->x, 2) +
                    pow(keyFrames[i].node->y - keyFrames[i - 1].node->y, 2) +
                    pow(keyFrames[i].node->z - keyFrames[i - 1].node->z, 2));
        }
        if (keyFrames[i].node->actionFlag == 2)
        {
            // remember first stop frame
            if(firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
    }

    uint32 m_time = 0;

    float tmpDist = 0;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    for (int i = int(keyFrames.size()) - 1; i >= 0; i--)
    {
        int j = (i + (firstStop+1)) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].node->actionFlag == 2)
            tmpDist = 0;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i].distSinceStop < (30 * 30 * 0.5f))
            keyFrames[i].tFrom = sqrt(2 * keyFrames[i].distSinceStop);
        else
            keyFrames[i].tFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;

        if (keyFrames[i].distUntilStop < (30 * 30 * 0.5f))
            keyFrames[i].tTo = sqrt(2 * keyFrames[i].distUntilStop);
        else
            keyFrames[i].tTo = ((keyFrames[i].distUntilStop - (30 * 30 * 0.5f)) / 30) + 30;

        keyFrames[i].tFrom *= IN_MILLISECONDS;
        keyFrames[i].tTo *= IN_MILLISECONDS;
    }

    //    for (int i = 0; i < keyFrames.size(); ++i) {
    //        sLog.outString("%f, %f, %f, %f, %f, %f, %f", keyFrames[i].x, keyFrames[i].y, keyFrames[i].distUntilStop, keyFrames[i].distSinceStop, keyFrames[i].distFromPrev, keyFrames[i].tFrom, keyFrames[i].tTo);
    //    }

    // Now we're completely set up; we can move along the length of each waypoint at 100 ms intervals
    // speed = max(30, t) (remember x = 0.5s^2, and when accelerating, a = 1 unit/s^2
    uint32 t = 0;
    bool teleport = false;
    if (keyFrames[keyFrames.size() - 1].node->mapid != keyFrames[0].node->mapid)
        teleport = true;

    WayPoint pos(keyFrames[0].node->mapid, keyFrames[0].node->x, keyFrames[0].node->y, keyFrames[0].node->z, teleport,
        keyFrames[0].node->arrivalEventID, keyFrames[0].node->departureEventID);
    m_WayPoints[0] = pos;
    t += keyFrames[0].node->delay * IN_MILLISECONDS;

    uint32 cM = keyFrames[0].node->mapid;
    for (size_t i = 0; i < keyFrames.size() - 1; ++i)
    {
        float d = 0;
        float tFrom = keyFrames[i].tFrom;
        float tTo = keyFrames[i].tTo;

        // keep the generation of all these points; we use only a few now, but may need the others later
        if (((d < keyFrames[i + 1].distFromPrev) && (tTo > 0)))
        {
            while ((d < keyFrames[i + 1].distFromPrev) && (tTo > 0))
            {
                tFrom += 100;
                tTo -= 100;

                if (d > 0)
                {
                    float newX, newY, newZ;
                    newX = keyFrames[i].node->x + (keyFrames[i + 1].node->x - keyFrames[i].node->x) * d / keyFrames[i + 1].distFromPrev;
                    newY = keyFrames[i].node->y + (keyFrames[i + 1].node->y - keyFrames[i].node->y) * d / keyFrames[i + 1].distFromPrev;
                    newZ = keyFrames[i].node->z + (keyFrames[i + 1].node->z - keyFrames[i].node->z) * d / keyFrames[i + 1].distFromPrev;

                    bool teleport = false;
                    if (keyFrames[i].node->mapid != cM)
                    {
                        teleport = true;
                        cM = keyFrames[i].node->mapid;
                    }

                    //                    sLog.outString("T: %d, D: %f, x: %f, y: %f, z: %f", t, d, newX, newY, newZ);
                    WayPoint pos(keyFrames[i].node->mapid, newX, newY, newZ, teleport);
                    if (teleport)
                        m_WayPoints[t] = pos;
                }

                if (tFrom < tTo)                            // caught in tFrom dock's "gravitational pull"
                {
                    if (tFrom <= 30000)
                    {
                        d = 0.5f * (tFrom / 1000) * (tFrom / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tFrom - 30000) / 1000);
                    }
                    d = d - keyFrames[i].distSinceStop;
                }
                else
                {
                    if (tTo <= 30000)
                    {
                        d = 0.5f * (tTo / 1000) * (tTo / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tTo - 30000) / 1000);
                    }
                    d = keyFrames[i].distUntilStop - d;
                }
                t += 100;
            }
            t -= 100;
        }

        if (keyFrames[i + 1].tFrom > keyFrames[i + 1].tTo)
            t += 100 - ((long)keyFrames[i + 1].tTo % 100);
        else
            t += (long)keyFrames[i + 1].tTo % 100;

        bool teleport = false;
        if ((keyFrames[i + 1].node->actionFlag == 1) || (keyFrames[i + 1].node->mapid != keyFrames[i].node->mapid))
        {
            teleport = true;
            cM = keyFrames[i + 1].node->mapid;
        }

        WayPoint pos(keyFrames[i + 1].node->mapid, keyFrames[i + 1].node->x, keyFrames[i + 1].node->y, keyFrames[i + 1].node->z, teleport,
            keyFrames[i + 1].node->arrivalEventID, keyFrames[i + 1].node->departureEventID);

        //        sLog.outString("T: %d, x: %f, y: %f, z: %f, t:%d", t, pos.x, pos.y, pos.z, teleport);

        //if (teleport)
        m_WayPoints[t] = pos;

        t += keyFrames[i + 1].node->delay * IN_MILLISECONDS;
    }
    m_pathTime = t;
    DEBUG_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "TransportPathMovementGenerator::GenerateWaypoints %u waypoints generated from %u keyframes, for total time %u", m_WayPoints.size(), keyFrames.size(), m_pathTime);

    m_next = m_WayPoints.begin();                           // will used in MoveToNextWayPoint for init m_curr
    MoveToNextWayPoint();                                   // m_curr -> first point
    MoveToNextWayPoint();                                   // skip first point

}

void TransportPathMovementGenerator::MoveToNextWayPoint()
{
    m_curr = m_next;

    ++m_next;
    if (m_next == m_WayPoints.end())
        m_next = m_WayPoints.begin();
}

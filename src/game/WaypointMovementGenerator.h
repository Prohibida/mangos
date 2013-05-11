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

#ifndef MANGOS_WAYPOINTMOVEMENTGENERATOR_H
#define MANGOS_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "Transports.h"
#include "WaypointManager.h"
#include "WorldLocation.h"
#include "DBCStructure.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS  // 3 Minutes

class GameObject;

template<class T, class P>
class MANGOS_DLL_SPEC PathMovementBase
{
    public:
        PathMovementBase() : i_currentNode(0) {}
        virtual ~PathMovementBase() {};

        // template pattern, not defined .. override required
        void LoadPath(T &);
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        P i_path;
        uint32 i_currentNode;
};

/** WaypointMovementGenerator loads a series of way points
 * from the DB and apply it to the creature's movement generator.
 * Hence, the creature will move according to its predefined way points.
 */

template<class T>
class MANGOS_DLL_SPEC WaypointMovementGenerator;

template<>
class MANGOS_DLL_SPEC WaypointMovementGenerator<Creature>
: public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
public PathMovementBase<Creature, WaypointPath const*>
{
    public:
        WaypointMovementGenerator(Creature &) : i_nextMoveTime(0), m_isArrivalDone(false) {}
        ~WaypointMovementGenerator() { i_path = NULL; }
        void Initialize(Creature &u);
        void Interrupt(Creature &);
        void Finalize(Creature &);
        void Reset(Creature &u);
        bool Update(Creature &u, const uint32 &diff);

        void MovementInform(Creature &);

        MovementGeneratorType GetMovementGeneratorType() const { return WAYPOINT_MOTION_TYPE; }

        const char* Name() const { return "<Waypoint>"; }

        // now path movement implmementation
        void LoadPath(Creature &c);

        bool GetResetPosition(Creature&, float& x, float& y, float& z) const;

        void AddToWaypointPauseTime(int32 waitTimeDiff);

        uint32 getLastReachedWaypoint() const { return m_isArrivalDone ? i_currentNode + 1 : i_currentNode; }

    private:
        void Stop(int32 time) { i_nextMoveTime.Reset(time); }
        bool Stopped(Creature& u);
        bool CanMove(int32 diff, Creature& u);

        void OnArrived(Creature&);
        void StartMove(Creature&);

        void StartMoveNow(Creature& creature);

        ShortTimeTracker i_nextMoveTime;
        bool m_isArrivalDone;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class MANGOS_DLL_SPEC FlightPathMovementGenerator
: public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
public PathMovementBase<Player,TaxiPathNodeList const*>
{
    public:
        explicit FlightPathMovementGenerator(TaxiPathNodeList const& pathnodes, uint32 startNode = 0)
        {
            i_path = &pathnodes;
            i_currentNode = startNode;
        }
        virtual void Initialize(Player &u) {_Initialize(u);};
        virtual void Finalize(Player &u)   {_Finalize(u);};
        virtual void Interrupt(Player &u)  {_Interrupt(u);};
        virtual void Reset(Player &u)      {_Reset(u);};

        bool Update(Player &, const uint32 &);
        MovementGeneratorType GetMovementGeneratorType() const { return FLIGHT_MOTION_TYPE; }

        TaxiPathNodeList const& GetPath() { return *i_path; }
        uint32 GetPathAtMapEnd() const;
        bool HasArrived() const { return (i_currentNode >= i_path->size()); }
        void SetCurrentNodeAfterTeleport();
        void SkipCurrentNode() { ++i_currentNode; }
        void DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure);
        bool GetResetPosition(Player&, float& x, float& y, float& z) const;

    protected:
        void _Initialize(Player &);
        void _Finalize(Player &);
        void _Interrupt(Player &);
        void _Reset(Player &);

};

/** TransportPathMovementGenerator generates movement of the MO_TRANSPORT and elevators for the paths
 * and hence generates ground and activities.
 */
class MANGOS_DLL_SPEC TransportPathMovementGenerator
    : public PathMovementBase<Transport,TaxiPathNodeList const*>
{
    public:
        explicit TransportPathMovementGenerator(TaxiPathNodeList const& pathnodes, uint32 startNode = 0)
        {
            i_path = &pathnodes;
            i_currentNode = startNode;
            GenerateWaypoints();
        }

        virtual void Initialize(Transport& go);
        virtual void Finalize(Transport& go);
        virtual void Interrupt(Transport& go);
        virtual void Reset(Transport &go);

        bool Update(Transport&, const uint32&);
        MovementGeneratorType GetMovementGeneratorType() const { return FLIGHT_MOTION_TYPE; }

        struct WayPoint
        {
            WayPoint() : loc(WorldLocation()), teleport(false) {}
            WayPoint(uint32 _mapid, float _x, float _y, float _z, bool _teleport, uint32 _arrivalEventID = 0, uint32 _departureEventID = 0)
                : loc(_mapid, _x, _y, _z, 0.0f), teleport(_teleport),
                arrivalEventID(_arrivalEventID), departureEventID(_departureEventID)
            {
            }
            WorldLocation loc;
            bool teleport;
            uint32 arrivalEventID;
            uint32 departureEventID;
        };
        typedef std::map<uint32, WayPoint> WayPointMap;
        WayPointMap::const_iterator GetCurrent() const { return m_curr; }
        WayPointMap::const_iterator GetNext() const    { return m_next; }
        bool GenerateWaypoints();
        void MoveToNextWayPoint();                          // move m_next/m_cur to next points

        TaxiPathNodeList const& GetPath() { return *i_path; }
        uint32 GetPathAtMapEnd(bool withAnchorage = false) const;
        bool HasArrived() const { return (i_currentNode >= i_path->size()); }
        void MoveToNextNode();
        void DoEventIfAny(Transport& go, TaxiPathNodeEntry const& node, bool departure);
        TaxiPathNodeEntry const* GetCurrentNodeEntry() const;
        TaxiPathNodeEntry const* GetNextNodeEntry() const;
        bool IsPathBreak() const;


    private:
        IntervalTimer   m_anchorageTimer;
        WayPointMap m_WayPoints;

        WayPointMap::const_iterator m_curr;
        WayPointMap::const_iterator m_next;
        uint32 m_pathTime;
};

#endif

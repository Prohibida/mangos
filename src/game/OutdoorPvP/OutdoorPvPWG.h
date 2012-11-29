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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef WORLD_PVP_WG
#define WORLD_PVP_WG

#include "../Player.h"
#include "../Creature.h"
#include "../GameObject.h"
#include "../ObjectMgr.h"

struct OutdoorPvPGameObjectBuildingWG;
typedef std::set<OutdoorPvPGameObjectBuildingWG*> GameObjectBuilding;

struct OutdoorPvPWorkShopWG;
typedef std::set<OutdoorPvPWorkShopWG*> WorkShop;

struct OutdoorPvPGraveYardWG;
typedef std::set<OutdoorPvPGraveYardWG*> GraveYard;

enum
{
    MAPID_ID_WINTERGRASP                = 571,
    // GraveYard
    GRAVEYARD_ID_ALLIANCE               = 1332,
    GRAVEYARD_ID_HORDE                  = 1331,
    GRAVEYARD_ID_KEEP                   = 1285,
    // Factions
    NEUTRAL_FACTION                     = 35,
    // Go factions
    GO_FACTION_A                        = 1732,
    GO_FACTION_H                        = 1735,
    // Npc factions
    NPC_FACTION_A                       = 1891,
    NPC_FACTION_H                       = 1979,
    // World States
    WS_BATTLE_ACTIVE_POS                = 3710,
    WS_BATTLE_ACTIVE_NEG                = 3801,
    WS_CLOCK_1                          = 3781,
    WS_CLOCK_2                          = 4354,
    WS_VEHICLE_COUNT_H                  = 3490,
    WS_VEHICLE_COUNT_MAX_H              = 3491,
    WS_VEHICLE_COUNT_A                  = 3680,
    WS_VEHICLE_COUNT_MAX_A              = 3681,
    WS_DEFENDER_TEAM                    = 3802,
    WS_ATTACKER_TEAM                    = 3803,
};

enum GameObjectArtKits
{
    GO_ARTKIT_BANNER_ALLIANCE               = 2,
    GO_ARTKIT_BANNER_HORDE                  = 1,
    GO_ARTKIT_BANNER_NEUTRAL                = 21,
};

enum GameObjectId
{
    GO_TITAN_RELIC                              = 192829,
    GO_TELEPORT                                 = 190763,
    GO_VEHICLE_TELEPORT                         = 192951,
    GO_FACTORY_BANNER_NE                        = 190475,
    GO_FACTORY_BANNER_NW                        = 190487,
    GO_FACTORY_BANNER_SE                        = 194959,
    GO_FACTORY_BANNER_SW                        = 194962,
    GO_BUILDING_NE                              = 192031,
    GO_BUILDING_NW                              = 192030,
    GO_BUILDING_SE                              = 192033,
    GO_BUILDING_SW                              = 192032,
    GO_BUILDING_KEEP_WEST                       = 192028,
    GO_BUILDING_KEEP_EAST                       = 192029,
};

enum eWGEvents
{
    EVENT_FACTORY_NE_PROGRESS_ALLIANCE = 19610,
    EVENT_FACTORY_NE_PROGRESS_HORDE = 19609,
    EVENT_FACTORY_NW_PROGRESS_ALLIANCE = 19612,
    EVENT_FACTORY_NW_PROGRESS_HORDE = 19611,
    EVENT_FACTORY_SE_PROGRESS_ALLIANCE = 21565,
    EVENT_FACTORY_SE_PROGRESS_HORDE = 21563,
    EVENT_FACTORY_SW_PROGRESS_ALLIANCE = 21562,
    EVENT_FACTORY_SW_PROGRESS_HORDE = 21560,
};

enum eWGSpell
{
    SPELL_TELEPORT_DALARAN                       = 53360,
    SPELL_TOWER_CONTROL                          = 62064,
    SPELL_RECRUIT                                = 37795,
    SPELL_CORPORAL                               = 33280,
    SPELL_LIEUTENANT                             = 55629,
    SPELL_SPIRITUAL_IMMUNITY                     = 58729,
    SPELL_TENACITY                               = 58549,
    SPELL_TENACITY_VEHICLE                       = 59911,
    SPELL_VEHICLE_TELEPORT                       = 49759,
};

enum eWGAchievements
{
    ACHIEVEMENTS_WIN_WG                          = 1717,
    ACHIEVEMENTS_WIN_WG_100                      = 1718,
    ACHIEVEMENTS_WG_TOWER_DESTROY                = 1727,
};

enum eWGGameObjectBuildingType
{
    WORLD_PVP_WG_OBJECTTYPE_DOOR,
    WORLD_PVP_WG_OBJECTTYPE_TITANRELIC,
    WORLD_PVP_WG_OBJECTTYPE_WALL,
    WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST,
    WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER,
    WORLD_PVP_WG_OBJECTTYPE_TOWER,
};

enum eWGGameObjectState
{
    WORLD_PVP_WG_OBJECTSTATE_NONE,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_DESTROY,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_HORDE_DESTROY,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DAMAGE,
    WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_DESTROY,
};

enum eWGWorkShopType
{
    WORLD_PVP_WG_WORKSHOP_NE,
    WORLD_PVP_WG_WORKSHOP_NW,
    WORLD_PVP_WG_WORKSHOP_SE,
    WORLD_PVP_WG_WORKSHOP_SW,
    WORLD_PVP_WG_WORKSHOP_KEEP_WEST,
    WORLD_PVP_WG_WORKSHOP_KEEP_EAST,
};

enum eWGNpc
{
    NPC_DEMOLISHER_ENGINEER_A        = 30499,
    NPC_DEMOLISHER_ENGINEER_H        = 30400,
    NPC_SPIRIT_GUIDE_A               = 31842,
    NPC_SPIRIT_GUIDE_H               = 31841,
    NPC_TURRER                       = 28366,
    NPC_GUARD_H                      = 30739,
    NPC_GUARD_A                      = 30740,
    NPC_VIERON_BLAZEFEATHER          = 31102,
    NPC_BOWYER_RANDOLPH              = 31052,
    NPC_HOODOO_MASTER_FU_JIN         = 31101,
    NPC_SORCERESS_KAYLANA            = 31051,
    NPC_CHAMPION_ROS_SLAI            = 39173,
    NPC_MARSHAL_MAGRUDER             = 39172,
    NPC_COMMANDER_DARDOSH            = 31091,
    NPC_COMMANDER_ZANNETH            = 31036,
    NPC_TACTICAL_OFFICER_KILRATH     = 31151,
    NPC_TACTICAL_OFFICER_AHBRAMIS    = 31153,
    NPC_SIEGESMITH_STRONGHOOF        = 31106,
    NPC_SIEGE_MASTER_STOUTHANDLE     = 31108,
    NPC_PRIMALIST_MULFORT            = 31053,
    NPC_ANCHORITE_TESSA              = 31054,
    NPC_LIEUTENANT_MURP              = 31107,
    NPC_SENIOR_DEMOLITIONIST_LEGOSO  = 31109,
    //Vehicles
    NPC_CATAPULT                     = 27881,
    NPC_DEMOLISHER                   = 28094,
    NPC_SIEGE_ENGINE_A               = 28312,
    NPC_SIEGE_ENGINE_H               = 32627,
};

struct WorldPvPWGBuildingSpawnData
{
    uint32 entry;
    uint32 WorldState;
    uint32 type;
    uint32 textid;
};

#define WG_MAX_OBJ 32
const WorldPvPWGBuildingSpawnData WGGameObjectBuillding[WG_MAX_OBJ] = {
    // Wall
    // Entry WS       type                        NameID
    { 190219, 3749, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190220, 3750, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191795, 3764, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191796, 3772, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191799, 3762, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191800, 3766, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191801, 3770, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191802, 3751, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191803, 3752, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191804, 3767, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191806, 3769, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191807, 3759, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191808, 3760, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191809, 3761, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190369, 3753, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190370, 3758, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190371, 3754, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190372, 3757, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190374, 3755, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 190376, 3756, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    // Tower of keep
    { 190221, 3711, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190373, 3713, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190377, 3714, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    { 190378, 3712, WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER, 0 },
    // Wall (with passage)
    { 191797, 3765, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191798, 3771, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    { 191805, 3768, WORLD_PVP_WG_OBJECTTYPE_WALL, 0 },
    // South tower
    { 190356, 3704, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    { 190357, 3705, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    { 190358, 3706, WORLD_PVP_WG_OBJECTTYPE_TOWER, 0 },
    // Door of forteress
    { 190375, 3763, WORLD_PVP_WG_OBJECTTYPE_DOOR, 0 },
    // Last door
    { 191810, 3773, WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST, 0 },
};

struct WorldPvPWGWorkShopDataBase
{
    uint32 entry;
    uint32 worldstate;
    uint32 type;
    uint32 textid;
    uint32 GraveYardId;
};

#define WG_MAX_WORKSHOP  6

const WorldPvPWGWorkShopDataBase WGWorkShopDataBase[WG_MAX_WORKSHOP] = {
    { 192031,3701,WORLD_PVP_WG_WORKSHOP_NE,0,1329 },
    { 192030,3700,WORLD_PVP_WG_WORKSHOP_NW,0,1330 },
    { 192033,3703,WORLD_PVP_WG_WORKSHOP_SE,0,1333 },
    { 192032,3702,WORLD_PVP_WG_WORKSHOP_SW,0,1334   },
    { 192028,3698,WORLD_PVP_WG_WORKSHOP_KEEP_WEST,0,0 },
    { 192029,3699,WORLD_PVP_WG_WORKSHOP_KEEP_EAST,0,0 }
};

struct VehicleSummonDataWG
{
    uint32 guida;
    uint32 guidh;
    float x;
    float y;
    float z;
    float o;
};

const VehicleSummonDataWG CoordVehicleSummon[6] = {
    { 530040, 530041, 5391.12f , 2986.23f , 413.298f , 3.18008f },
    { 530042, 530043, 5389.81f , 2719.13f , 413.133f , 3.14081f },
    { 530044, 530045, 4950.19f , 2389.96f , 324.269f , 1.41764f },
    { 530046, 530047, 4954.49f , 3385.17f , 380.993f , 4.35896f },
    { 530048, 530049, 4359.86f , 2347.38f , 380.103f , 6.12532f },
    { 530050, 530051, 4359.86f , 2347.38f , 380.103f , 6.12532f },
};

class MANGOS_DLL_SPEC OutdoorPvPWG : public OutdoorPvP
{
    public:
        OutdoorPvPWG();

        bool InitBattlefield(); // Init BattleField zone

        void HandleCreatureCreate(Creature* pCreature);
        void HandleGameObjectCreate(GameObject* pGo);
        void HandleCreatureDeath(Creature* pCreature);

        void Update(uint32 uiDiff);

        void HandlePlayerEnterZone(Player* pPlayer, bool isMainZone);
        void HandlePlayerLeaveZone(Player* pPlayer, bool isMainZone);
        void HandlePlayerKillInsideArea(Player* pPlayer, Unit* pVictim);

        bool HandleGameObjectUse(Player* pPlayer, GameObject* pGo);
        bool HandleEvent(uint32 uiEventId,GameObject* pGo);
        void EventPlayerDamageGO(Player *player, GameObject* target_obj, uint32 eventId, uint32 spellId);

        uint32 GetDefender()  { return defender; } // Get Defender team
        uint32 GetAttacker()  { return attacker; } // Get Attack team

        void SetBannerArtKit(GameObject* Go,uint32 uiArtkit);
        void UpdateCounterVehicle(bool init);

        // GameObjects
        ObjectGuid gBanner[4]; // Save work shop banners
        ObjectGuid gBuilding[6]; // Save work shop building
        ObjectGuid gBandT[32]; // Save keep buildings, keep towers and south towers
        uint8 GetCountBuildingAndTowers(uint32 id); // Get count or number for gBandT variable

        bool CanUseRelic;

        //World states
        void UpdateMainWS(bool status);
        void UpdateTeamWS(uint32 team);
        void UpdateVehicleCountWG();
        void UpdateWSWorkShop();

        //Auras
        void UpdateAura(Player* pPlayer);
        void UpdateTenacityStack();
        void AddAuraResurrect(Player* pPlayer);

        //Towers
        void AddDamagedTower(uint32 team);
        void AddBrokenTower(uint32 team, Player* pPlayer);
        void BrokenWallOrTower(uint32 team,Player* pPlayer);

        //Others
        void DoCompleteOrIncrementAchievement(uint32 achievement, Player *player);
        void CompleOneObjetiveQuest(Player* pPlayer,uint32 id);
        void AddDataWhenWin();
        void RecolatePlayers();

        //Vehicles
        void CreateVehicle(Creature* pCreature,uint32 npc_entry);
        void AddVehicle(Creature* pCreature,uint32 team);
        void DeleteVehicle(Creature* pCreature,uint32 team);

        //Players
        Player* GetPlayerInZone();
        Player* GetPlayersAlliance();
        Player* GetPlayersHorde();
        uint32 CountPlayersAlliance();
        uint32 CountPlayersHorde();

    protected:
        uint32 m_Timer; // In second, use in timer for updating the world state timer and controling timer phase

    private:
        // config
        bool activate; // check if it is active or not
        uint32 startdefenderteam; // Get the team which strat denfend the keep
        bool Changeteamifdisable; // If BattleFIeld isn't inactive, use it for check if change control team zone by timer

        bool m_bIsBattleStarted; // Use for start the world state timer in battle phase
        uint32 defender; // save defender team
        uint32 attacker; // save attack team

        void Install(); //Prepare GraveYard, buildings, towers, WorkShop and others for inting the zone
        void UpdateTeleport(GameObject* pGo); // Update faction
        void UpdateKeepTurret(Creature* pTurret); // Update faction
        void PrepareKeepNpc(Creature* pCreature,uint32 team); // Save in variable and hiding if it's nesessary
        void PrepareKeepGo(GameObject* pGo,uint32 team); // Save in variable and hiding if it's nesessary
        void PrepareOutKeepNpc(Creature* pCreature,uint32 team); // Save in variable and hiding if it's nesessary
        bool get_map; // Use for checking get m_Map
        Map* m_Map; // Save variable Map
        void ChangeControlZone(); // Use only when Changeteamifdisable = true fro changing control zone
        void StartBattle(); // Start the battle
        void EndBattle(bool titan); // End the battle

        uint32 m_tenacityStack;
        uint32 VehicleCountA;
        uint32 VehicleCountH;
        uint32 VehicleCountMaxA;
        uint32 VehicleCountMaxH;

        GuidSet m_sZonePlayers;
        GuidSet m_sZonePlayersAlliance;
        GuidSet m_sZonePlayersHorde;

        //Timers
        uint32 TimeChangeControl; // Save time for changing control
        uint32 TimeBattle; // Save time for battle phase
        uint32 TimeControl; // Save time for control phase
        uint32 UpdateTimer; // varible for update world states

        //OjectGuid GameObjects and Creatures
        ObjectGuid Relic;
        ObjectGuid VehcileTeleport1;
        ObjectGuid VehcileTeleport2;
        std::list<ObjectGuid> KeepCreatureA;
        std::list<ObjectGuid> KeepCreatureH;
        std::list<ObjectGuid> m_KeepGameObjectA;
        std::list<ObjectGuid> m_KeepGameObjectH;
        std::list<ObjectGuid> OutKeepCreatureA;
        std::list<ObjectGuid> OutKeepCreatureH;
        std::list<ObjectGuid> TeleportGameObject;
        std::list<ObjectGuid> TurretCreature;
        std::list<ObjectGuid> m_vehicleA;
        std::list<ObjectGuid> m_vehicleH;

        //Graveyard
        OutdoorPvPGraveYardWG* gyNe;
        OutdoorPvPGraveYardWG* gyNw;
        OutdoorPvPGraveYardWG* gySe;
        OutdoorPvPGraveYardWG* gySw;
        OutdoorPvPGraveYardWG* gyBuilding;

        //Keep and towers
        GameObjectBuilding BuildingsInZone;
        OutdoorPvPGameObjectBuildingWG* Building[32];
        uint32 DamageTowerAtt;
        uint32 DamageTowerDef;
        uint32 DestroyTowerAtt;
        uint32 DestroyTowerDef;

        //WorkShop
        std::list<OutdoorPvPWorkShopWG*>WorkShopList;
        OutdoorPvPWorkShopWG* wsNe;
        OutdoorPvPWorkShopWG* wsNw;
        OutdoorPvPWorkShopWG* wsSe;
        OutdoorPvPWorkShopWG* wsSw;
        OutdoorPvPWorkShopWG* wsKe;
        OutdoorPvPWorkShopWG* wsKw;
};

struct OutdoorPvPGameObjectBuildingWG
{

    uint32 m_Team;
    OutdoorPvPWG *m_WG;
    uint8 count;
    uint32 m_Type;
    uint32 m_TextId;
    uint32 TowerCount;
    Map* Mmap;
    bool map;

    std::list<ObjectGuid> TurretCreature;
    std::list<ObjectGuid> m_GameObjectA;
    std::list<ObjectGuid> m_GameObjectH;
    std::list<ObjectGuid> m_CreatureA;
    std::list<ObjectGuid> m_CreatureH;

    OutdoorPvPGameObjectBuildingWG(OutdoorPvPWG *WG)
    {
        m_WG = WG;
        m_Team = 0;
        count = 0;
        TowerCount = 0;
        m_Type = 0;
        m_TextId = 0;
        Mmap = 0;
        map = false;
    }

    void StartBattle()
    {
      RebuildGo();
      Mmap->GetGameObject(m_WG->gBandT[count])->SetTeam(Team(m_Team));

      //Only towers
      UpdateTurret(true);
      UpdateCreature(true);
      UpdateGameObject(true);
    }

    void EndBattle(bool changeteam)
    {
        RebuildGo();

        if(changeteam)
        {
          switch (m_Type)
          {
            case WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR:
            case WORLD_PVP_WG_OBJECTTYPE_WALL:
                m_Team = m_WG->GetDefender();
                break;
            case WORLD_PVP_WG_OBJECTTYPE_TOWER:
                m_Team = m_WG->GetAttacker();
                break;
            default:
                m_Team = TEAM_NONE;
                break;
          }
        }

      if(map)
        Mmap->GetGameObject(m_WG->gBandT[count])->SetTeam(Team(m_Team));

      //Only towers
      UpdateTurret(false);
      UpdateCreature(false);
      UpdateGameObject(false);

    }

    void Damaged()
    {
        if (m_Type == WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER)
            m_WG->AddDamagedTower(m_WG->GetDefender());
        else if (m_Type == WORLD_PVP_WG_OBJECTTYPE_TOWER)
            m_WG->AddDamagedTower(m_WG->GetAttacker());
    }

    void Destroyed(Player* pPlayer)
    {
        switch (m_Type)
        {
            case WORLD_PVP_WG_OBJECTTYPE_TOWER:
            case WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER:
                m_WG->AddBrokenTower(m_Team,pPlayer);
                break;
            case WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST:
                m_WG->CanUseRelic = true;
                break;
        }

        m_WG->BrokenWallOrTower(m_Team,pPlayer);
    }

    void Init(uint32 i, uint32 type, uint32 textid)
    {
        count = i;

        m_Type = type;

        m_TextId = textid;

        switch (m_Type)
        {
            case WORLD_PVP_WG_OBJECTTYPE_KEEP_TOWER:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR_LAST:
            case WORLD_PVP_WG_OBJECTTYPE_DOOR:
            case WORLD_PVP_WG_OBJECTTYPE_WALL:
                m_Team = m_WG->GetDefender();
                break;
            case WORLD_PVP_WG_OBJECTTYPE_TOWER:
                m_Team = m_WG->GetAttacker();
                break;
            default:
                m_Team = TEAM_NONE;
                break;
        }

        switch (count)
        {
            case 27:
                TowerCount = 1;
                break;
            case 28:
                TowerCount = 2;
                break;
            case 29:
                TowerCount = 3;
                break;
        }
    }

    void RebuildGo()
    {
        if(map)
           Mmap->GetGameObject(m_WG->gBandT[count])->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
    }

    void AddTurret(Creature* pTurret)
    {
        if(!map)
        {
            Mmap = pTurret->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
            pTurret->setFaction(35);
            pTurret->SetVisibility(VISIBILITY_OFF);

            TurretCreature.push_back(pTurret->GetObjectGuid());
        }
    }

    void AddCreature(Creature* pCreature, uint32 team)
    {
        if(!map)
        {
            Mmap = pCreature->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
                if(m_Team == ALLIANCE)
                   m_CreatureA.push_back(pCreature->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_CreatureH.push_back(pCreature->GetObjectGuid());

                pCreature->setFaction(35);
                pCreature->SetVisibility(VISIBILITY_OFF);
        }
    }

    void AddGameObject(GameObject* pGo,uint32 team)
    {
        if(!map)
        {
            Mmap = pGo->GetMap();
            map = true;
        }

        if(TowerCount > 0)
        {
                pGo->SetPhaseMask(100,true);

                if(m_Team == ALLIANCE)
                   m_GameObjectH.push_back(pGo->GetObjectGuid());
                else if(m_Team == HORDE)
                   m_GameObjectA.push_back(pGo->GetObjectGuid());
        }
    }

    void UpdateTurret(bool activate) // Only for towers
    {
        if(TowerCount == 0)
           return;

        if(activate)
        {
           for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
           {
               if(Creature* pTurret = Mmap->GetCreature((*itr)))
               {
                 pTurret->Respawn();
                 pTurret->SetVisibility(VISIBILITY_ON);

                 if(m_Team == ALLIANCE)
                     pTurret->setFaction(NPC_FACTION_A);
                 else if(m_Team == HORDE)
                     pTurret->setFaction(NPC_FACTION_H);
               }
           }
        }
        else
        {
           for (std::list<ObjectGuid>::iterator itr = TurretCreature.begin(); itr != TurretCreature.end(); ++itr)
           {
               if(Creature* pTurret = Mmap->GetCreature((*itr)))
               {
                 pTurret->Respawn();
                 pTurret->setFaction(35);
                 pTurret->SetVisibility(VISIBILITY_OFF);
               }
           }
        }

    }

    void UpdateCreature(bool activate)
    {
        if(TowerCount == 0)
           return;

        if(m_Team == ALLIANCE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
           {
               if(Creature* pCreature = Mmap->GetCreature((*itr)))
               {
                   if(activate)
                   {
                       pCreature->Respawn();
                       pCreature->SetVisibility(VISIBILITY_ON);
                   }
                   else
                   {
                       pCreature->Respawn();
                       pCreature->setFaction(35);
                       pCreature->SetVisibility(VISIBILITY_OFF);
                   }
               }
           }

           for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
           {
               if(Creature* pCreature = Mmap->GetCreature((*itr)))
               {
                       pCreature->Respawn();
                       pCreature->setFaction(35);
                       pCreature->SetVisibility(VISIBILITY_OFF);
               }
           }

        }
        else if(m_Team == HORDE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
           {
               if(Creature* pCreature = Mmap->GetCreature((*itr)))
               {
                   if(activate)
                   {
                       pCreature->Respawn();
                       pCreature->SetVisibility(VISIBILITY_ON);
                   }
                   else
                   {
                       pCreature->Respawn();
                       pCreature->setFaction(35);
                       pCreature->SetVisibility(VISIBILITY_OFF);
                   }
               }
           }

           for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
           {
               if(Creature* pCreature = Mmap->GetCreature((*itr)))
               {
                       pCreature->Respawn();
                       pCreature->setFaction(35);
                       pCreature->SetVisibility(VISIBILITY_OFF);
               }
           }
        }
    }

    void UpdateGameObject(bool activate)
    {
        if(m_Team == ALLIANCE)
           return;

        if(m_Team == ALLIANCE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
           {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                   if(activate)
                       pGo->SetPhaseMask(1,true);
                   else
                      pGo->SetPhaseMask(100,true);
               }
           }

           for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
           {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                  pGo->SetPhaseMask(100,true);
               }
           }
        }
        else
        {
           for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
           {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                   if(activate)
                       pGo->SetPhaseMask(1,true);
                   else
                      pGo->SetPhaseMask(100,true);
               }
           }

           for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
           {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                  pGo->SetPhaseMask(100,true);
               }
           }
        }

    }
};

struct OutdoorPvPGraveYardWG
{
    OutdoorPvPWG* m_WG;
    uint32 m_team;
    uint32 m_id;
    Map* Mmap;
    bool map;

    ObjectGuid SpiritGuieA;
    ObjectGuid SpiritGuieH;

    OutdoorPvPGraveYardWG(OutdoorPvPWG * WG)
    {
        m_WG = WG;
        m_team = 0;
        m_id = 0;
        Mmap = 0;
        map = false;
        SpiritGuieA.Clear();
        SpiritGuieH.Clear();
    }

    void Init(uint32 id,uint32 team)
    {
       m_id = id;
       m_team = team;
    }

    uint32 GetId()
    {
      return m_id;
    }

    void AddSpiritGuide(Creature* pCreature,uint32 team)
    {
        if (!pCreature)
            return;

        if(!map)
        {
            Mmap = pCreature->GetMap();
            map = true;
        }

        if (m_team == ALLIANCE)
            SpiritGuieA = pCreature->GetObjectGuid();
        else if (m_team == HORDE)
            SpiritGuieH = pCreature->GetObjectGuid();

        if (m_id == GRAVEYARD_ID_KEEP)
        {
           if (SpiritGuieA || SpiritGuieH)
              Start();
        }

        pCreature->SetVisibility(VISIBILITY_OFF);
    }

    void Start()
    {
        if(m_team == ALLIANCE)
        {
            Mmap->GetCreature(SpiritGuieA)->SetVisibility(VISIBILITY_ON);
        }
        else
        {
            Mmap->GetCreature(SpiritGuieH)->SetVisibility(VISIBILITY_ON);
        }
    }

    void End(bool changeteam)
    {
        if(m_team == ALLIANCE)
        {
            Mmap->GetCreature(SpiritGuieA)->SetVisibility(VISIBILITY_OFF);
        }
        else
        {
            Mmap->GetCreature(SpiritGuieH)->SetVisibility(VISIBILITY_OFF);
        }

       if (changeteam)
       {
           m_team = (m_team == ALLIANCE) ? HORDE : ALLIANCE;
       }
    }

    uint32 GetTeam()
    {
      return m_team;
    }
};

struct OutdoorPvPWorkShopWG
{
    OutdoorPvPGraveYardWG* m_GY;
    OutdoorPvPWG *m_WG;
    uint32 m_Type;
    uint32 m_State;
    uint32 m_WorldState;
    uint32 m_TeamControl;
    uint32 m_TextId;
    uint8 number;
    Map* Mmap;
    bool map;

    std::list<ObjectGuid> m_CreatureA;
    std::list<ObjectGuid> m_CreatureH;
    std::list<ObjectGuid> m_GameObjectA;
    std::list<ObjectGuid> m_GameObjectH;

    OutdoorPvPWorkShopWG(OutdoorPvPWG * WG)
    {
        m_WG = WG;
        m_GY = 0;
        m_Type = 0;
        m_State = 0;
        m_WorldState = 0;
        m_TeamControl = 0;
        m_TextId = 0;
        number = 0;
        Mmap = 0;
        map = false;
    }

    uint32 GetType()
    {
       return m_Type;
    }

    void SetGraveYard(OutdoorPvPGraveYardWG* GY)
    {
      m_GY = GY;
    }

    void AddCreature(Creature* pCreature, uint32 team)
    {
        if(!map)
        {
           Mmap = pCreature->GetMap();
           map = true;
        }

        if(m_TeamControl == ALLIANCE)
           m_CreatureA.push_back(pCreature->GetObjectGuid());
        else if(m_TeamControl == HORDE)
           m_CreatureH.push_back(pCreature->GetObjectGuid());

        pCreature->setFaction(35);
        pCreature->SetVisibility(VISIBILITY_OFF);
    }

    void AddGameObject(GameObject* pGo, uint32 team)
    {
        if(!map)
        {
           Mmap = pGo->GetMap();
           map = true;
        }

        if(m_TeamControl == ALLIANCE)
            m_GameObjectH.push_back(pGo->GetObjectGuid());
        else if(m_TeamControl == HORDE)
           m_GameObjectA.push_back(pGo->GetObjectGuid());

        pGo->SetPhaseMask(100,true);
    }

    void Init(uint8 n,uint32 worldstate, uint32 type, uint32 text)
    {
        number = n;
        m_WorldState = worldstate;
        m_Type = type;
        m_TextId = text;
        m_State = WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_INTACT;

         if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
             m_TeamControl = m_WG->GetAttacker();
         else
             m_TeamControl = m_WG->GetDefender();
    }

    void StartBattle()
    {
        switch(m_TeamControl)
        {
            case ALLIANCE:
                m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                break;
            case HORDE:
                m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                break;
        }

        m_WG->SendUpdateWorldState(m_WorldState, m_State);
        Rebuild(true);

        if (m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
        {
           m_GY->Start();
           sObjectMgr.SetGraveYardLinkTeam(m_GY->GetId(), ZONE_ID_WINTERGRASP, Team(m_TeamControl));
        }
    }

    void EndBattle(bool changeteam)
    {
        if (changeteam)
        {
            m_TeamControl = (m_TeamControl == ALLIANCE) ? HORDE : ALLIANCE;
        }

        m_State = WORLD_PVP_WG_OBJECTSTATE_NEUTRAL_INTACT;
        ChangeControl(m_TeamControl,true);

    }

    void Rebuild(bool activate)
    {

        if (m_TeamControl == ALLIANCE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
           {
             if(Creature* pCreature = Mmap->GetCreature((*itr)))
             {
                 pCreature->Respawn();
                 if(!activate)
                 {
                     pCreature->setFaction(35);
                     pCreature->SetVisibility(VISIBILITY_OFF);
                 }
             }
           }

            for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
            {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                   if(activate)
                     pGo->SetPhaseMask(1,true);
                   else
                     pGo->SetPhaseMask(100,true);
               }
            }
        }
        else if(m_TeamControl == HORDE)
        {
           for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
           {
             if(Creature* pCreature = Mmap->GetCreature((*itr)))
             {
                 pCreature->Respawn();
                 if(!activate)
                 {
                     pCreature->setFaction(35);
                     pCreature->SetVisibility(VISIBILITY_OFF);
                 }
             }
           }

            for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
            {
               if(GameObject* pGo = Mmap->GetGameObject((*itr)))
               {
                   if(activate)
                     pGo->SetPhaseMask(1,true);
                   else
                     pGo->SetPhaseMask(100,true);
               }
            }
        }

        if(map)
        {
           if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
           {
              gBanner->Respawn();
              if(activate)
              {
                if(m_TeamControl == ALLIANCE)
                  m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_ALLIANCE);
                else
                  m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_HORDE);
              }
              else
               m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_NEUTRAL);
           }

          if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
             pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
        }

        if(!activate && m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
        {
            m_GY->End(false);
            sObjectMgr.RemoveGraveYardLink(m_GY->GetId(), ZONE_ID_WINTERGRASP, Team(m_TeamControl));
        }
    }

    void ChangeControl(uint32 team, bool endbattle)
    {
        switch (team)
        {
            case ALLIANCE:
                  if(m_TeamControl != ALLIANCE)
                  {
                    m_TeamControl = ALLIANCE;

                    if(map)
                    {
                       for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                              pCreature->Respawn();
                              pCreature->setFaction(35);
                              pCreature->SetVisibility(VISIBILITY_OFF);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                             pCreature->Respawn();
                             if(endbattle)
                             {
                               pCreature->setFaction(35);
                               pCreature->SetVisibility(VISIBILITY_OFF);
                             }
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                             pGo->SetPhaseMask(100,true);
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                              if(endbattle)
                                 pGo->SetPhaseMask(100,true);
                              else
                                 pGo->SetPhaseMask(1,true);
                          }

                       }

                      if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
                      {
                          if(endbattle)
                             m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_NEUTRAL);
                          else
                             m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_ALLIANCE);

                      }


                      if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
                         pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
                    }

                    if(!endbattle)
                    {
                      m_State = WORLD_PVP_WG_OBJECTSTATE_ALLIANCE_INTACT;
                      m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    }

                    if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
                    {
                        m_GY->End(true);
                        sObjectMgr.RemoveGraveYardLink(m_GY->GetId(), ZONE_ID_WINTERGRASP, HORDE);
                        if(!endbattle)
                        {
                          m_GY->Start();
                          sObjectMgr.SetGraveYardLinkTeam(m_GY->GetId(), ZONE_ID_WINTERGRASP, ALLIANCE);
                        }
                    }
                  }
                  else if(endbattle)
                    Rebuild(false);
                  break;
            case HORDE:
                  if(m_TeamControl != HORDE)
                  {
                    m_TeamControl = HORDE;

                    if(map)
                    {
                       for (std::list<ObjectGuid>::iterator itr = m_CreatureA.begin(); itr != m_CreatureA.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                             pCreature->Respawn();
                             pCreature->setFaction(35);
                             pCreature->SetVisibility(VISIBILITY_OFF);
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_CreatureH.begin(); itr != m_CreatureH.end(); ++itr)
                       {
                          if(Creature* pCreature = Mmap->GetCreature((*itr)))
                          {
                             pCreature->Respawn();
                             if(endbattle)
                             {
                               pCreature->setFaction(35);
                               pCreature->SetVisibility(VISIBILITY_OFF);
                             }
                          }
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectA.begin(); itr != m_GameObjectA.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                             pGo->SetPhaseMask(100,true);
                       }

                       for (std::list<ObjectGuid>::iterator itr = m_GameObjectH.begin(); itr != m_GameObjectH.end(); ++itr)
                       {
                          if(GameObject* pGo = Mmap->GetGameObject((*itr)))
                          {
                              if(endbattle)
                               pGo->SetPhaseMask(100,true);
                              else
                               pGo->SetPhaseMask(1,true);
                          }

                       }


                      if(GameObject* gBanner = Mmap->GetGameObject(m_WG->gBanner[number]))
                      {
                          if(endbattle)
                            m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_NEUTRAL);
                          else
                            m_WG->SetBannerArtKit(gBanner,GO_ARTKIT_BANNER_HORDE);
                      }


                      if(GameObject* pGo =  Mmap->GetGameObject(m_WG->gBuilding[number]))
                         pGo->Rebuild(((Unit*)m_WG->GetPlayerInZone()));
                    }

                    if(!endbattle)
                    {
                      m_State = WORLD_PVP_WG_OBJECTSTATE_HORDE_INTACT;
                      m_WG->SendUpdateWorldState(m_WorldState, m_State);
                    }

                    if(m_Type < WORLD_PVP_WG_WORKSHOP_KEEP_WEST)
                    {
                        m_GY->End(true);
                        sObjectMgr.RemoveGraveYardLink(m_GY->GetId(), ZONE_ID_WINTERGRASP, ALLIANCE);
                        if(!endbattle)
                        {
                          m_GY->Start();
                          sObjectMgr.SetGraveYardLinkTeam(m_GY->GetId(), ZONE_ID_WINTERGRASP, HORDE);
                        }
                    }
                  }
                  else if(endbattle)
                    Rebuild(false);
                  break;
        }
    }

};

#endif

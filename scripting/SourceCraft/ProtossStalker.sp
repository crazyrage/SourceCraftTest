 /**
 * vim: set ai et ts=4 sw=4 :
 * File: ProtossStalker.sp
 * Description: The Protoss Stalker race for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#include <tf2_meter>
#define REQUIRE_EXTENSIONS

#include "sc/SourceCraft"
#include "sc/MissileAttack"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/Teleport"
#include "sc/dissolve"
#include "sc/shields"
#include "sc/freeze"
#include "sc/sounds"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

new const String:spawnWav[]             = "sc/pdrwht07.wav";
new const String:deathWav[]             = "sc/pdrdth00.wav";
new const String:teleportWav[]          = "sc/ptemov00.wav";

new const String:g_MissileAttackSound[] = "sc/pdrfir00.wav";

new raceID, immunityID, speedID, shieldsID;
new missileID, teleportID,  disrupterID;

new g_MissileAttackChance[]             = { 5, 10, 15, 25, 35 };
float g_MissileAttackPercent[]      = { 0.15, 0.30, 0.40, 0.50, 0.70 };

float g_TeleportDistance[]          = { 0.0, 300.0, 500.0, 800.0, 1500.0 };

float g_SpeedLevels[]               = { 0.80, 0.90, 0.95, 1.00, 1.05 };

float g_InitialShields[]            = { 0.05, 0.10, 0.25, 0.50, 0.75 };
float g_ShieldsPercent[][2]         = { {0.05, 0.10},
                                            {0.10, 0.20},
                                            {0.15, 0.30},
                                            {0.20, 0.40},
                                            {0.25, 0.50} };

int g_disrupterRace = -1;

bool cfgAllowTeleport;

public Plugin myinfo = 
{
    name = "SourceCraft Race - Protoss Stalker",
    author = "-=|JFH|=-Naris",
    description = "The Protoss Stalker race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public void OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.stalker.phrases.txt");
    LoadTranslations("sc.teleport.phrases.txt");

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public void OnSourceCraftReady()
{
    raceID      = CreateRace("stalker", -1, -1, 21, .energy_rate=2.0,
                             .faction=Protoss, .type=Cybernetic,
                             .parent="zealot");

    shieldsID   = AddUpgrade(raceID, "shields", .energy=1.0, .cost_crystals=10);
    speedID     = AddUpgrade(raceID, "speed", .cost_crystals=0);
    missileID   = AddUpgrade(raceID, "ground_weapons", .energy=2.0, .cost_crystals=20);
    immunityID  = AddUpgrade(raceID, "immunity", .cost_crystals=0);

    // Ultimate 1
    teleportID = AddUpgrade(raceID, "blink", 1, .energy=30.0, .cooldown=2.0,
                            .cost_crystals=30);

    cfgAllowTeleport = bool:GetConfigNum("allow_teleport", true);
    if (cfgAllowTeleport)
    {
        GetConfigFloatArray("range",  g_TeleportDistance, sizeof(g_TeleportDistance),
                            g_TeleportDistance, raceID, teleportID);
    }
    else
    {
        SetUpgradeDisabled(raceID, teleportID, true);
        LogMessage("Disabling Protoss Stalker:Blink due to configuration: sc_allow_teleport=%d",
                   cfgAllowTeleport);
    }

    // Ultimate 2
    disrupterID = AddUpgrade(raceID, "disrupter", 2, 4,1, .energy=120.0,
                             .cooldown=30.0, .accumulated=true, .cost_crystals=50);

    // Get Configuration Data
    GetConfigFloatArray("shields_amount", g_InitialShields, sizeof(g_InitialShields),
                        g_InitialShields, raceID, shieldsID);

    for (int level =0; level < sizeof(g_ShieldsPercent); level++)
    {
        char key[32];
        Format(key, sizeof(key), "shields_percent_level_%d", level);
        GetConfigFloatArray(key, g_ShieldsPercent[level], sizeof(g_ShieldsPercent[]),
                            g_ShieldsPercent[level], raceID, shieldsID);
    }

    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, speedID);

    GetConfigArray("chance", g_MissileAttackChance, sizeof(g_MissileAttackChance),
                   g_MissileAttackChance, raceID, missileID);

    GetConfigFloatArray("damage_percent", g_MissileAttackPercent, sizeof(g_MissileAttackPercent),
                        g_MissileAttackPercent, raceID, missileID);
}

public void OnMapStart()
{
    SetupSmokeSprite();
    SetupHaloSprite();
    SetupLightning();
    SetupBlueGlow();
    SetupRedGlow();
    SetupSpeed();

    SetupTeleport(teleportWav);
    //SetupDeniedSound();

    SetupMissileAttack(g_MissileAttackSound);

    SetupSound(deathWav);
    SetupSound(spawnWav);
}

public void OnPlayerAuthed(client)
{
    ResetTeleport(client);
}

public Action OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        ResetShields(client);
        ResetTeleport(client);
        SetSpeed(client,-1.0, true);

        // Turn off Immunities
        int immunity_level =GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, false);
    }
    else
    {
        if (g_disrupterRace < 0)
            g_disrupterRace = FindRace("disrupter");

        if (oldrace == g_disrupterRace &&
            GetCooldownExpireTime(client, raceID, disrupterID) <= 0.0)
        {
            CreateCooldown(client, raceID, disrupterID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
    return Plugin_Continue;
}

public Action OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        if (IsValidClientAlive(client))
        {
            PrepareAndEmitSoundToAll(spawnWav, client);

            // Turn on Immunities
            int immunity_level =GetUpgradeLevel(client,raceID,immunityID);
            DoImmunity(client, immunity_level, true);

            int speed_level = GetUpgradeLevel(client,raceID,speedID);
            SetSpeedBoost(client, speed_level, true, g_SpeedLevels);

            int shields_level = GetUpgradeLevel(client,raceID,shieldsID);
            SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);
        }
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public void OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade == immunityID)
            DoImmunity(client, new_level, true);
        else if (upgrade==speedID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade==shieldsID)
        {
            SetupShields(client, new_level, g_InitialShields,
                         g_ShieldsPercent, .upgrade=true);
        }
    }
}

public void OnItemPurchase(client,item)
{
    int race =GetRace(client);
    if (race == raceID && IsValidClientAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (item == g_bootsItem)
        {
            int speed_level = GetUpgradeLevel(client,race,speedID);
            SetSpeedBoost(client, speed_level, true, g_SpeedLevels);
        }
    }
}

public void OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsValidClientAlive(client))
    {
        switch (arg)
        {
            case 4,3,2:
            {
                int disrupter_level = GetUpgradeLevel(client,race,disrupterID);
                if (disrupter_level > 0)
                {
                    if (pressed)
                        SummonDisrupter(client);
                }
                else
                {
                    int blink_level = GetUpgradeLevel(client,race,teleportID);
                    if (blink_level && cfgAllowTeleport)
                    {
                        float blink_energy=GetUpgradeEnergy(raceID,teleportID) * (5.0-float(blink_level));
                        TeleportCommand(client, race, teleportID, blink_level, blink_energy,
                                        pressed, g_TeleportDistance, teleportWav);
                    }
                }
            }
            default:
            {
                int blink_level = GetUpgradeLevel(client,race,teleportID);
                if (blink_level && cfgAllowTeleport)
                {
                    float blink_energy=GetUpgradeEnergy(raceID,teleportID) * (5.0-float(blink_level));
                    TeleportCommand(client, race, teleportID, blink_level, blink_energy,
                                    pressed, g_TeleportDistance, teleportWav);
                }
                else if (pressed)
                {
                    int disrupter_level = GetUpgradeLevel(client,race,disrupterID);
                    if (disrupter_level > 0)
                        SummonDisrupter(client);
                }
            }
        }
    }
}

public void OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        PrepareAndEmitSoundToAll(spawnWav, client);

        ResetTeleport(client);

        int immunity_level =GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        int speed_level = GetUpgradeLevel(client,raceID,speedID);
        SetSpeedBoost(client, speed_level, true, g_SpeedLevels);

        int shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);
    }
}

public Action OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, damage, absorbed, bool:from_sc)
{
    if (!from_sc && attacker_index > 0 &&
        attacker_index != victim_index &&
        attacker_race == raceID)
    {
        int blink_level = GetUpgradeLevel(attacker_index,raceID,teleportID);
        if (blink_level && cfgAllowTeleport)
            TeleporterAttacked(attacker_index,raceID,teleportID);

        int weapons_level =GetUpgradeLevel(attacker_index,raceID,missileID);
        if (weapons_level > 0)
        {
            if (MissileAttack(raceID, missileID, weapons_level, event, damage+absorbed, victim_index,
                              attacker_index, victim_index, false, sizeof(g_MissileAttackChance),
                              g_MissileAttackPercent, g_MissileAttackChance, g_MissileAttackSound,
                              "sc_ground_weapons"))
            {
                return Plugin_Handled;
            }
        }
    }
    return Plugin_Continue;
}

public void OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID)
    {
        PrepareAndEmitSoundToAll(deathWav,victim_index);
        DissolveRagdoll(victim_index, 0.1);
    }
    else
    {
        if (g_disrupterRace < 0)
            g_disrupterRace = FindRace("disrupter");

        if (victim_race == g_disrupterRace &&
            GetCooldownExpireTime(victim_index, raceID, disrupterID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, disrupterID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
}

DoImmunity(client, level, bool:value)
{
    SetImmunity(client,Immunity_MotionTaking, (value && level >= 1));
    SetImmunity(client,Immunity_ShopItems, (value && level >= 2));
    SetImmunity(client,Immunity_Ultimates, (value && level >= 3));
    SetImmunity(client,Immunity_Upgrades, (value && level >= 4));

    if (value && IsValidClientAlive(client))
    {
        float start[3];
        GetClientAbsOrigin(client, start);

        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start,30.0,60.0,Lightning(),HaloSprite(),
                              0, 1, 2.0, 10.0, 0.0 ,color, 10, 0);
        TE_SendEffectToAll();
    }
}

SummonDisrupter(client)
{
    if (g_disrupterRace < 0)
        g_disrupterRace = FindRace("disrupter");

    if (g_disrupterRace < 0)
    {
        char upgradeName[64];
        GetUpgradeName(raceID, disrupterID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        LogError("***The Protoss Disrupter race is not Available!");
        PrepareAndEmitSoundToClient(client,deniedWav);
    }
    else if (GetRestriction(client,Restriction_NoUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        char upgradeName[64];
        GetUpgradeName(raceID, disrupterID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "PreventedFromSummoningReaver");
        PrepareAndEmitSoundToClient(client,deniedWav);
    }
    else if (CanInvokeUpgrade(client, raceID, disrupterID))
    {
        float clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 40.0; // Adjust position to the middle

        TE_SetupSmoke(clientLoc,SmokeSprite(),8.0,2);
        TE_SendEffectToAll();

        TE_SetupGlowSprite(clientLoc,
                           (GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                           5.0,40.0,255);
        TE_SendEffectToAll();

        ChangeRace(client, g_disrupterRace, true, false, true);
    }
}


/**
 * vim: set ai et ts=4 sw=4 :
 * File: ZergInfestor.sp
 * Description: The Zerg Infestor race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <lib/sm_gas>

#include "sc/MindControl"
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/HealthParticle"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/DarkSwarm"
#include "sc/plugins"
#include "sc/weapons"
#include "sc/burrow"
#include "sc/Plague"
#include "sc/sounds"
#include "sc/armor"

#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:g_PlagueSound[] = "sc/zdeblo01.wav";

new const String:g_ArmorName[]  = "Carapace";
float g_InitialArmor[]      = { 0.05, 0.10, 0.25, 0.50, 0.75 };
float g_ArmorPercent[][2]   = { {0.00, 0.10},
                                    {0.00, 0.20},
                                    {0.10, 0.30},
                                    {0.20, 0.40},
                                    {0.30, 0.50} };

float g_ConsumePercent[]    = { 0.10, 0.18, 0.28, 0.38, 0.48 };
float g_PlagueRange[]       = { 300.0, 400.0, 550.0, 700.0, 900.0 };
float g_DarkSwarmRange[]    = { 300.0, 400.0, 600.0, 800.0, 1000.0 };
float g_CorruptionRange[]   = { 250.0, 500.0, 700.0, 1000.0, 1500.0 };

new raceID, carapaceID, regenerationID, consumeID, burrowID;
new darkSwarmID, plagueID, swarmID, corruptionID;

new m_gasAllocation[MAXPLAYERS+1];
float m_ConsumeEnemyTime[MAXPLAYERS+1];

public Plugin myinfo = 
{
    name = "SourceCraft Race - Zerg Infestor",
    author = "-=|JFH|=-Naris",
    description = "The Zerg race Infestor for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public void OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.infestor.phrases.txt");
    LoadTranslations("sc.protector.phrases.txt");

    GetGameType();

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public void OnSourceCraftReady()
{
    raceID          = CreateRace("infestor", -1, -1, 29, 100.0, -1.0, 1.0,
                                 Zerg, Biological, "defiler");

    regenerationID  = AddUpgrade(raceID, "regeneration", 0, 0, .cost_crystals=10);
    carapaceID      = AddUpgrade(raceID, "armor", 0, 0, .cost_crystals=5);

    consumeID       = AddUpgrade(raceID, "consume", 0, 0, .energy=1.0,
                                 .cost_crystals=20);

    // Ultimate 2
    burrowID        = AddBurrowUpgrade(raceID, 2, 0, 1, 1);

    // Ultimate 1
    darkSwarmID     = AddUpgrade(raceID, "dark_swarm", 1, 0, .energy=90.0,
                                .cooldown=10.0, .cost_crystals=30);

    // Ultimate 1
    plagueID        = AddUpgrade(raceID, "disease", 1, 1, .energy=90.0,
                                 .cooldown=10.0, .cost_crystals=30);

    // Ultimate 3
    swarmID         = AddUpgrade(raceID, "swarm", 3, 0, .cooldown=2.0,
                                 .cost_crystals=30);

    // Ultimate 4
    corruptionID    = AddUpgrade(raceID, "corruption", 4, 4, .energy=10.0, .cost_crystals=40,
                                 .desc = (IsMindControlAvailable()) ? "%infestor_corruption_desc"
                                         : "%infestor_corruption_nocontrol_desc");

    // Disable/alter inapplicable upgrades
    if (!IsGasAvailable())
    {
        SetUpgradeDisabled(raceID, swarmID, true);
        LogMessage("Disabling Zerg Infestor:Swarm Infestation due to sm_gas is not available");
    }

    if (GameType != tf2)
    {
        SetUpgradeDisabled(raceID, corruptionID, true);
        LogMessage("Disabling Zerg Infestor:Enduring Corruption due to gametype != tf2");
    }

    // Get Configuration Data
    GetConfigFloatArray("armor_amount", g_InitialArmor, sizeof(g_InitialArmor),
                        g_InitialArmor, raceID, carapaceID);

    for (int level =0; level < sizeof(g_ArmorPercent); level++)
    {
        char key[32];
        Format(key, sizeof(key), "armor_percent_level_%d", level);
        GetConfigFloatArray(key, g_ArmorPercent[level], sizeof(g_ArmorPercent[]),
                            g_ArmorPercent[level], raceID, carapaceID);
    }

    GetConfigFloatArray("damage_percent", g_ConsumePercent, sizeof(g_ConsumePercent),
                        g_ConsumePercent, raceID, consumeID);

    GetConfigFloatArray("range", g_DarkSwarmRange, sizeof(g_DarkSwarmRange),
                        g_DarkSwarmRange, raceID, darkSwarmID);

    GetConfigFloatArray("range", g_PlagueRange, sizeof(g_PlagueRange),
                        g_PlagueRange, raceID, plagueID);

    GetConfigFloatArray("range", g_CorruptionRange, sizeof(g_CorruptionRange),
                        g_CorruptionRange, raceID, corruptionID);
}

public void OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "sm_gas"))
        IsGasAvailable(true);
    else if (StrEqual(name, "MindControl"))
        IsMindControlAvailable(true);
}

public void OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "sm_gas"))
        m_GasAvailable = false;
    else if (StrEqual(name, "MindControl"))
        m_MindControlAvailable = false;
}

public void OnMapStart()
{
    SetupBeamSprite();
    SetupHaloSprite();

    SetupErrorSound();
    SetupRechargeSound();
    SetupDeniedSound();

    SetupDarkSwarm();
    SetupPlague(g_PlagueSound);
}

public void OnMapEnd()
{
    for (int index =1;index<=MaxClients;index++)
    {
        ResetPlague(index);
        ResetDarkSwarm(index);
    }
}

public void OnPlayerAuthed(client)
{
    m_gasAllocation[client] = 0;
    m_ConsumeEnemyTime[client] = 0.0;
}

public void OnClientDisconnect(client)
{
    ResetPlague(client);
    ResetDarkSwarm(client);
    ResetProtected(client);
}

public Action OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        if (m_GasAvailable)
            TakeGas(client);

        ResetArmor(client);
        SetHealthRegen(client, 0.0);
        ResetDarkSwarm(client);
    }
    return Plugin_Continue;
}

public Action OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        m_gasAllocation[client] = 0;

        int swarm_level =GetUpgradeLevel(client,raceID,swarmID);
        SetupSwarmInfestation(client, swarm_level);

        int regeneration_level =GetUpgradeLevel(client,raceID,regenerationID);
        SetHealthRegen(client, float(regeneration_level+1));

        int carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public void OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==swarmID)
            SetupSwarmInfestation(client, new_level);
        else if (upgrade==regenerationID)
            SetHealthRegen(client, float(new_level+1));
        else if (upgrade==carapaceID)
        {
            SetupArmor(client, new_level, g_InitialArmor,
                       g_ArmorPercent, g_ArmorName,
                       .upgrade=true);
        }
    }
}

public void OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (pressed && race==raceID && IsValidClientAlive(client))
    {
        switch (arg)
        {
            case 4:
            {
                int corruption_level =GetUpgradeLevel(client,race,corruptionID);
                if (GetGameType() == tf2 && corruption_level > 0)
                    Corruption(client, corruption_level);
            }
            case 3:
            {
                SwarmInfestation(client);
            }
            case 2:
            {
                int burrow_level =GetUpgradeLevel(client,race,burrowID);
                Burrow(client, burrow_level+1);
            }
            default:
            {
                int plague_level =GetUpgradeLevel(client,race,plagueID);
                if (plague_level > 0)
                {
                    Plague(client, race, plagueID, plague_level,
                           UltimatePlague|EnsnaringPlague|ExplosivePlague|FatalPlague|InfectiousPlague,
                           false, g_PlagueRange, g_PlagueSound, "sc_disease");
                }
                else
                {
                    int dark_swarm_level =GetUpgradeLevel(client,race,darkSwarmID);
                    DarkSwarm(client, race, darkSwarmID, dark_swarm_level, g_DarkSwarmRange);
                }
            }
        }
    }
}

// Events
public void OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_gasAllocation[client] = 0;
        m_ConsumeEnemyTime[client] = 0.0;

        int swarm_level =GetUpgradeLevel(client,raceID,swarmID);
        SetupSwarmInfestation(client, swarm_level);

        int regeneration_level =GetUpgradeLevel(client,raceID,regenerationID);
        SetHealthRegen(client, float(regeneration_level+1));

        int carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);
    }
}

public Action OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, damage, absorbed, bool:from_sc)
{
    if (!from_sc && attacker_index > 0 &&
        attacker_index != victim_index &&
        attacker_race == raceID)
    {
        if (ConsumeEnemy(damage + absorbed, attacker_index, victim_index))
            return Plugin_Handled;
    }

    return Plugin_Continue;
}

public Action OnPlayerAssistEvent(Handle:event, victim_index, victim_race,
                                  assister_index, assister_race, damage,
                                  absorbed)
{
    if (assister_race == raceID)
    {
        if (ConsumeEnemy(damage + absorbed, assister_index, victim_index))
            return Plugin_Handled;
    }

    return Plugin_Continue;
}

public void OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    ResetProtected(victim_index);
    m_gasAllocation[victim_index] = 0;
}

bool:ConsumeEnemy(damage, index, victim_index)
{
    int level = GetUpgradeLevel(index, raceID, consumeID);
    if (level > 0 && GetRandomInt(1,10) <= 9 &&
        !GetRestriction(index, Restriction_NoUpgrades) &&
        !GetRestriction(index, Restriction_Stunned))
    {
        bool victimIsNPC    = (victim_index > MaxClients);
        bool victimIsPlayer = !victimIsNPC && IsValidClientAlive(victim_index) &&
                                  !GetImmunity(victim_index,Immunity_HealthTaking) &&
                                  !GetImmunity(victim_index,Immunity_Upgrades) &&
                                  !IsInvulnerable(victim_index);

        if (victimIsPlayer || victimIsNPC)
        {
            float lastTime = m_ConsumeEnemyTime[index];
            float interval = GetGameTime() - lastTime;
            if ((lastTime == 0.0 || interval > 0.25) &&
                CanInvokeUpgrade(index, raceID, consumeID, .notify=false))
            {
                float start[3];
                GetClientAbsOrigin(index, start);
                start[2] += 1620;

                float end[3];
                GetClientAbsOrigin(index, end);
                end[2] += 20;

                static const color[4] = { 255, 10, 25, 255 };
                TE_SetupBeamPoints(start, end, BeamSprite(), HaloSprite(),
                                   0, 1, 3.0, 20.0,10.0,5,50.0,color,255);
                TE_SendEffectToAll();
                FlashScreen(index,RGBA_COLOR_GREEN);
                FlashScreen(victim_index,RGBA_COLOR_RED);

                m_ConsumeEnemyTime[index] = GetGameTime();

                int leechhealth = RoundFloat(float(damage)*g_ConsumePercent[level]);
                if (leechhealth <= 0)
                    leechhealth = 1;

                if (IsClientInGame(index) && IsPlayerAlive(index))
                {
                    int health = GetClientHealth(index) + leechhealth;
                    if (health <= GetMaxHealth(index))
                    {
                        ShowHealthParticle(index);
                        SetEntityHealth(index,health);

                        char upgradeName[NAME_STRING_LENGTH];
                        GetUpgradeName(raceID, consumeID, upgradeName, sizeof(upgradeName), index);

                        if (victimIsPlayer)
                        {
                            DisplayMessage(index, Display_Damage, "%t", "YouHaveLeechedFrom",
                                           leechhealth, victim_index, upgradeName);
                        }
                        else
                        {
                            DisplayMessage(index, Display_Damage, "%t", "YouHaveLeeched",
                                           leechhealth, upgradeName);
                        }
                    }
                }

                if (victimIsPlayer)
                {
                    int victim_health = GetClientHealth(victim_index);
                    if (victim_health <= leechhealth)
                        KillPlayer(victim_index, index, "sc_consume");
                    else
                    {
                        SetEntityHealth(victim_index, victim_health - leechhealth);

                        if (GameType != tf2 || GetMode() != MvM)
                        {
                            int entities = EntitiesAvailable(200, .message="Reducing Effects");
                            if (entities > 50)
                                CreateParticle("blood_impact_red_01_chunk", 0.1, victim_index, Attach, "head");
                        }

                        char upgradeName[NAME_STRING_LENGTH];
                        GetUpgradeName(raceID, consumeID, upgradeName, sizeof(upgradeName), victim_index);
                        DisplayMessage(victim_index, Display_Injury, "%t", "HasLeeched",
                                       index, leechhealth, upgradeName);
                    }
                }
                else
                {
                    DamageEntity(victim_index, leechhealth, index, DMG_GENERIC, "sc_consume");
                    DisplayDamage(index, victim_index, leechhealth, "sc_consume");
                }

                return true;
            }
        }
    }
    return false;
}

SetupSwarmInfestation(client, level)
{
    if (m_GasAvailable)
    {
        int amount = ((level+1)*2) - m_gasAllocation[client];
        if (amount > 0)
        {
            m_gasAllocation[client] += amount;
            GiveGas(client, amount, .everyone=1);
        }
    }
}

SwarmInfestation(client)
{
    if (m_GasAvailable)
    {
        if (GetRestriction(client,Restriction_NoUltimates) ||
            GetRestriction(client,Restriction_Stunned))
        {
            char upgradeName[NAME_STRING_LENGTH];
            GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
            PrepareAndEmitSoundToClient(client,deniedWav);
        }
        else if (CanInvokeUpgrade(client, raceID, swarmID, false))
        {
            if (GameType == tf2)
            {
                if (TF2_IsPlayerDisguised(client))
                    TF2_RemovePlayerDisguise(client);
            }

            if (GasAttack(client))
            {
                ChargeForUpgrade(client, raceID, swarmID);
                int remaining = HasGas(client);
                if (remaining == 0)
                {
                    DisplayMessage(client,Display_Ultimate, "%t",
                                   "UsedLastSwarmInfestation");
                }
                else
                {
                    float cooldown = GetUpgradeCooldown(raceID, swarmID);
                    if (cooldown > 0.0)
                    {
                        DisplayMessage(client, Display_Ultimate, "%t", "UsedSwarmInfestationMustWait", remaining, cooldown);
                        CreateTimer(cooldown,AllowSwarmInfestation,GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
                        EnableGas(client, false);
                    }
                    else
                    {
                        DisplayMessage(client, Display_Ultimate, "%t", "UsedSwarmInfestation", remaining);
                    }
                }
            }
            else if (HasGas(client) == 0)
            {
                DisplayMessage(client ,Display_Ultimate, "%t", "SwarmInfestationDepleted");
            }
            else // if (!IsGasEnabled(client))
            {
                char upgradeName[NAME_STRING_LENGTH];
                GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "IsNotReady", upgradeName);
                PrepareAndEmitSoundToClient(client,deniedWav);
            }
        }
    }
    else
    {
        char upgradeName[NAME_STRING_LENGTH];
        GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
        PrintHintText(client, "%t", "IsNotAvailable", upgradeName);
        PrepareAndEmitSoundToClient(client,deniedWav);
    }
}

public Action AllowSwarmInfestation(Handle:timer,any:userid)
{
    int index = GetClientOfUserId(userid);
    if (index > 0 && m_GasAvailable)
    {
        EnableGas(index, true);
        if (IsClientInGame(index) && IsPlayerAlive(index))
        {
            if (GetRace(index) == raceID)
            {
                char upgradeName[NAME_STRING_LENGTH];
                GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), index);
                DisplayMessage(index, Display_Ultimate, "%t", "IsReady", upgradeName);
                PrintHintText(index, "%t", "IsReady", upgradeName);
                PrepareAndEmitSoundToClient(index, rechargeWav);
            }
        }                
    }                
    return Plugin_Stop;
}

Corruption(client, level)
{
    if (GetRestriction(client,Restriction_NoUltimates) ||
        GetRestriction(client,Restriction_Stunned))
    {
        char upgradeName[NAME_STRING_LENGTH];
        GetUpgradeName(raceID, corruptionID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        PrepareAndEmitSoundToClient(client,deniedWav);
    }
    else if (CanInvokeUpgrade(client, raceID, corruptionID, false))
    {
        if (GameType == tf2)
        {
            if (TF2_IsPlayerDisguised(client))
                TF2_RemovePlayerDisguise(client);
        }

        float range = g_CorruptionRange[level];
        int target = TraceAimTarget(client);
        if (target >= 0)
        {
            float clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);

            float targetLoc[3];
            TR_GetEndPosition(targetLoc);

            if (IsPointInRange(clientLoc,targetLoc,range))
            {
                if (IsValidEdict(target) && IsValidEntity(target) &&
                    TF2_GetExtObjectType(target) != TFExtObject_Unknown)
                {
                    int builder = GetEntPropEnt(target, Prop_Send, "m_hBuilder");
                    if (builder > 0 && !GetImmunity(builder,Immunity_Ultimates))
                    {
                        Handle pack;
                        if (CreateDataTimer(0.5, CorruptionTimer, pack, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE) &&
                            pack != INVALID_HANDLE)
                        {
                            ChargeForUpgrade(client, raceID, corruptionID);

                            WritePackCell(pack, EntIndexToEntRef(target));
                            WritePackCell(pack, client);
                            WritePackCell(pack, level);
                            WritePackFloat(pack, GetEngineTime() + (float(level) * 5.0));

                            int targetTeam = GetEntProp(target, Prop_Send, "m_iTeamNum");
                            if (targetTeam != GetClientTeam(client))
                            {
                                new r,b,g;
                                if (TFTeam:targetTeam == TFTeam_Red)
                                {
                                    r = 255;
                                    b = 100;
                                    g = 60;
                                }
                                else
                                {
                                    r = 0;
                                    b = 255;
                                    g = 100;
                                }
                                SetEntityRenderColor(target, r, b, g, 255);
                            }
                        }
                    }
                    else
                    {
                        PrepareAndEmitSoundToClient(client,deniedWav);
                        DisplayMessage(client, Display_Ultimate,
                                       "%t", "TargetIsImmune");
                    }
                }
                else
                {
                    PrepareAndEmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetInvalid");
                }
            }
            else
            {
                PrepareAndEmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetIsTooFar");
            }
        }
        else
        {
            PrepareAndEmitSoundToClient(client,deniedWav);
        }
    }
}

public Action CorruptionTimer(Handle:timer,Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        int ref =ReadPackCell(pack);
        int client =ReadPackCell(pack);
        int level =ReadPackCell(pack);
        float endTime=ReadPackFloat(pack);

        int target = EntRefToEntIndex(ref);
        if (target > 0 && IsValidEntity(target) && IsValidEdict(target))
        {
            int health = GetEntProp(target, Prop_Send, "m_iHealth");
            int team = GetEntProp(target, Prop_Send, "m_iTeamNum");
            if (team == GetClientTeam(client))
            {
                int max_health = GetEntProp(target, Prop_Data, "m_iMaxHealth");
                if (health < max_health)
                {
                    health += 10 * level;
                    if (health > max_health)
                        health = max_health;

                    SetEntityHealth(target, health);
                    if (GetEngineTime() < endTime)
                        return Plugin_Continue;
                }
            }
            else
            {
                int amount = 10 * level;
                health -= amount;
                if (health > 50 + amount)
                {
                    SetEntityHealth(target, health);
                    if (GetEngineTime() < endTime)
                        return Plugin_Continue;
                    else
                        SetEntityRenderColor(target, 255, 255, 255, 255);
                }
                else
                {
                    SetEntityRenderColor(target, 255, 255, 255, 255);
                    if (m_MindControlAvailable)
                        ReplaceObject(client, target);
                }
            }
        }
    }
    return Plugin_Stop;
}

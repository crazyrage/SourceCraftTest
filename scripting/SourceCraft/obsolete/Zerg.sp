/**
 * vim: set ai et ts=4 sw=4 :
 * File: Zerg.sp
 * Description: The Zerg race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <lib/hgrsource.inc>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/util"
#include "sc/range"
#include "sc/trace"
#include "sc/weapons"
#include "sc/maxhealth"

new String:errorWav[] = "sourcecraft/perror.mp3";
new String:deniedWav[] = "sourcecraft/buzz.wav";

new raceID, glandsID, regenerationID, healingID, tentacleID;

int g_haloSprite;
int g_lightningSprite;

public Plugin myinfo = 
{
    name = "SourceCraft Race - Zerg",
    author = "-=|JFH|=-Naris",
    description = "The Zerg race for SourceCraft.",
    version = "1.0.0.0",
    url = "http://jigglysfunhouse.net/"
};

public void OnPluginStart()
{
    GetGameType();

    HookEvent("player_spawn",PlayerSpawnEvent);

    CreateTimer(3.0,Regeneration,INVALID_HANDLE,TIMER_REPEAT);
}

public void OnSourceCraftReady()
{
    raceID          = CreateRace("Zerg", "zerg",
                                 "You are now part of the Zerg.",
                                 "You will be part of the Zerg when you die or respawn.",
                                 32);

    glandsID        = AddUpgrade(raceID,"Adrenal Glands", "adrenal_glands", "Increases Melee Attack Damage");
    regenerationID  = AddUpgrade(raceID,"Regeneration", "regeneration", "Regenerates your Health.");
    healingID       = AddUpgrade(raceID,"Healing Aura", "healing", "Heals all of your teammates in range (It does NOT heal you).");
    tentacleID      = AddUpgrade(raceID,"Tentacles", "tentacles", "Reach out and grab an opponent.", true); // Ultimate

    ControlHookGrabRope(true);
    HookGrab(OnGrab);
    HookDrop(OnDrop);
}

public void OnMapStart()
{
    g_lightningSprite = SetupModel("materials/sprites/lgtning.vmt");
    if (g_lightningSprite == -1)
        SetFailState("Couldn't find lghtning Model");

    g_haloSprite = SetupModel("materials/sprites/halo01.vmt");
    if (g_haloSprite == -1)
        SetFailState("Couldn't find halo Model");

    SetupSound(errorWav, true, true);
    SetupSound(deniedWav, true, true);
}

public void OnRaceSelected(client,Handle:player,oldrace,race)
{
    if (race != oldrace)
    {
        if (oldrace == raceID)
            TakeGrab(client);
        else if (race == raceID)
            Tentacles(client, player, GetUpgradeLevel(player,race,tentacleID));
    }
}

public Action Regeneration(Handle:timer)
{
    int maxplayers =GetMaxClients();
    for(int client =1;client<=maxplayers;client++)
    {
        if(IsClientInGame(client))
        {
            if(IsPlayerAlive(client))
            {
                Handle player=GetPlayerHandle(client);
                if(player != INVALID_HANDLE && GetRace(player) == raceID)
                {
                    int regeneration_level =GetUpgradeLevel(player,raceID,regenerationID);
                    if (regeneration_level)
                    {
                        int newhp =GetClientHealth(client)+regeneration_level;
                        int maxhp =GetMaxHealth(client);
                        if(newhp<=maxhp)
                            SetEntityHealth(client,newhp);
                    }

                    int healing_aura_level =GetUpgradeLevel(player,raceID,healingID);
                    if (healing_aura_level)
                    {
                        int num =healing_aura_level*5;
                        float range=1.0;
                        switch(healing_aura_level)
                        {
                            case 1:
                                range=300.0;
                            case 2:
                                range=450.0;
                            case 3:
                                range=650.0;
                            case 4:
                                range=800.0;
                        }
                        int count =0;
                        float indexLoc[3];
                        float clientLoc[3];
                        GetClientAbsOrigin(client, clientLoc);
                        int team = GetClientTeam(client);
                        for (int index =1;index<=maxplayers;index++)
                        {
                            if (index != client && IsClientInGame(index) &&
                                IsPlayerAlive(index) && GetClientTeam(index) == team)
                            {
                                Handle player_check=GetPlayerHandle(index);
                                if (player_check != INVALID_HANDLE)
                                {
                                    GetClientAbsOrigin(index, indexLoc);
                                    if (IsPointInRange(clientLoc,indexLoc,range))
                                    {
                                        if (TraceTarget(client, index, clientLoc, indexLoc))
                                        {
                                            int health =GetClientHealth(index);
                                            int max =GetMaxHealth(index);
                                            if (health < max)
                                            {
                                                HealPlayer(index,healing_aura_level*5,health,max);

                                                new color[4] = { 0, 0, 255, 255 };
                                                TE_SetupBeamLaser(client,index,g_lightningSprite,g_haloSprite,
                                                                  0, 1, 3.0, 10.0,10.0,5,50.0,color,255);
                                                TE_SendToAll();

                                                if (++count > num)
                                                    break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

public void OnUltimateCommand(client,Handle:player,race,bool:pressed)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        if (pressed)
            Grab(client);
        else
            Drop(client);
    }
}

public Action OnGrab(client, target)
{
    if (target != client && IsClientInGame(target) && IsPlayerAlive(target))
    {
        if ( GetClientTeam(client) != GetClientTeam(target))
        {
            Handle player_check=GetPlayerHandle(target);
            if (player_check != INVALID_HANDLE)
            {
                if (!GetImmunity(player_check,Immunity_Ultimates))
                {
                    SetOverrideGravity(player_check, 0.0);
                    return Plugin_Continue;
                }
                else
                {
                    EmitSoundToClient(client,errorWav);
                    PrintToChat(client,"%c[SourceCraft] %cTarget is %cimmune%c to ultimates!",
                            COLOR_GREEN,COLOR_DEFAULT,COLOR_TEAM,COLOR_DEFAULT);
                }
            }
            else
                EmitSoundToClient(client,deniedWav);
        }
        else
        {
            EmitSoundToClient(client,errorWav);
            PrintToChat(client,"%c[SourceCraft] %cTarget is a teammate!",
                        COLOR_GREEN,COLOR_DEFAULT);
        }
    }
    else
        EmitSoundToClient(client,deniedWav);

    return Plugin_Stop;
}

public Action OnDrop(client, target)
{
    Handle player_check=GetPlayerHandle(target);
    if (player_check != INVALID_HANDLE)
    {
        SetOverrideGravity(player_check, -1.0);
    }
    return Plugin_Continue;
}

public void OnUpgradeLevelChanged(client,Handle:player,race,upgrade,old_level,new_level)
{
    if (race == raceID && GetRace(player) == raceID)
    {
        if (upgrade==3 && (new_level <= 0 || IsPlayerAlive(client)))
            Tentacles(client, player, new_level);
    }
}

// Events
public PlayerSpawnEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    int userid =GetEventInt(event,"userid");
    int client =GetClientOfUserId(userid);
    if (client)
    {
        Handle player=GetPlayerHandle(client);
        if (player != INVALID_HANDLE)
        {
            if (GetRace(player) == raceID)
                Tentacles(client, player, GetUpgradeLevel(player,raceID,tentacleID));
        }
    }
}

public Action OnPlayerHurtEvent(Handle:event,victim_index,Handle:victim_player,victim_race,
                                attacker_index,Handle:attacker_player,attacker_race,
                                assister_index,Handle:assister_player,assister_race,
                                damage)
{
    bool changed=false;
    if (attacker_index && attacker_race == raceID && attacker_index != victim_index)
    {
        char weapon[64];
        bool is_equipment=GetWeapon(event,attacker_index,weapon,sizeof(weapon));
        if (IsMelee(weapon, is_equipment, attacker_index, victim_index))
        {
            if (AdrenalGlands(damage, victim_index, victim_player,
                              attacker_index, attacker_player))
            {
                changed = true;
            }
        }
    }
    return changed ? Plugin_Changed : Plugin_Continue;
}


public bool:AdrenalGlands(damage, victim_index, Handle:victim_player, index, Handle:player)
{
    int adrenal_glands_level =GetUpgradeLevel(player,raceID,glandsID);
    if (adrenal_glands_level)
    {
        if (!GetImmunity(victim_player,Immunity_HealthTake) &&
            !TF2_IsPlayerInvuln(victim_index))
        {
            float percent;
            switch(adrenal_glands_level)
            {
                case 1:
                    percent=0.15;
                case 2:
                    percent=0.35;
                case 3:
                    percent=0.55;
                case 4:
                    percent=0.75;
            }

            int amount =RoundFloat(float(damage)*percent);
            int newhp =GetClientHealth(victim_index)-amount;
            if (newhp <= 0)
            {
                newhp=0;
                DisplayKill(index, victim_index, "adrenal_glands", "Adrenal Glands", amount);
            }
            else
                DisplayDamage(index, victim_index, "adrenal_glands", "Adrenal Glands", amount);

            SetEntityHealth(victim_index,newhp);

            float Origin[3];
            GetClientAbsOrigin(victim_index, Origin);
            Origin[2] += 5;

            TE_SetupSparks(Origin,Origin,255,1);
            TE_SendToAll();
            return true;
        }
    }
    return false;
}

public Tentacles(client, Handle:player, level)
{
    if (level > 0)
    {
        new duration, Float:range;
        switch(level)
        {
            case 1:
            {
                duration=2;
                range=500.0; //350.0;
            }
            case 2:
            {
                duration=5;
                range=1500.0; //500.0;
            }
            case 3:
            {
                duration=10;
                range=2500.0; //750.0;
            }
            case 4:
            {
                duration=20;
                range=3500.0; //1500.0;
            }
        }
        GiveGrab(client,duration,range,0.0,1);
    }
    else
        TakeGrab(client);
}

/**
 * vim: set ai et ts=4 sw=4 :
 * File: MindControl.sp
 * Description: The MindControl upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#include <tf2_stocks>
#include <tf2_player>
#include <tf2_objects>

#include <libtf2/remote>

#include "sc/SourceCraft"
#include "sc/sounds"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

enum command { update, remove, reset, find_controller, find_builder };

new const String:controlWav[] = "sc/pteSum00.wav";

Handle m_StolenObjectList[MAXPLAYERS+1] = { INVALID_HANDLE, ... };

public Plugin myinfo = 
{
    name = "SourceCraft Upgrade - MindControl",
    author = "-=|JFH|=-Naris",
    description = "The MindControl upgrade for SourceCraft.",
    version=SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
    // Only load when running TF2
    if (GetGameType() == tf2)
    {
        // Register Natives
        CreateNative("MindControl",Native_MindControl);
        CreateNative("ControlObject",Native_ControlObject);
        CreateNative("ReplaceObject",Native_ReplaceObject);
        CreateNative("ResetMindControlledObjects",Native_ResetMindControlledObjs);
        RegPluginLibrary("MindControl");
        return APLRes_Success;
    }
    else
    {
        strcopy(error, err_max, "Cannot Load MindControl on mods other than TF2");
        return APLRes_SilentFailure;
    }
}

public void OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.mind_control.phrases.txt");

    if (GetGameType() == tf2)
    {
        if(!HookEvent("object_removed", ObjectDestroyed))
            SetFailState("Could not hook the object_removed event.");

        if(!HookEvent("object_destroyed", ObjectDestroyed))
            SetFailState("Could not hook the object_destroyed event.");

        if(!HookEventEx("player_death",CorrectDeathEvent, EventHookMode_Pre))
            SetFailState("Could not hook the player_death pre event.");
    }
}

public void OnMapStart()
{
    SetupLightning();
    SetupHaloSprite();
    SetupSmokeSprite();
    SetupBlueGlow();
    SetupRedGlow();

    SetupErrorSound();
    SetupDeniedSound();

    SetupSound(controlWav);
}

public void OnClientDisconnect(client)
{
    ResetMindControlledObjects(client, false);
}

public ObjectDestroyed(Handle:event,const String:name[],bool:dontBroadcast)
{
    int index = GetClientOfUserId(GetEventInt(event,"userid"));
    int obj = GetEventInt(event,"index");
    new TFExtObjectType:type = TFExtObjectType:GetEventInt(event,"objecttype");
    ProcessMindControlledObjects(remove, obj, index, type, INVALID_HANDLE);
}

public Action CorrectDeathEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    int attacker = GetClientOfUserId(GetEventInt(event,"attacker"));
    if (attacker > 0)
    {
        char weapon[64];
        GetEventString(event, "weapon", weapon, sizeof(weapon));

        if (StrEqual(weapon, "obj_sentrygun"))
        {
            int controller = ProcessMindControlledObjects(find_controller, -1, attacker, TFExtObject_Sentry);
            if (controller != attacker && IsValidClient(controller))
            {
                SetEventInt(event,"attacker",GetClientUserId(controller));
                return Plugin_Changed;
            }
        }
    }
    return Plugin_Continue;
}

bool:MindControl(client, Float:range, percent, &builder, &TFExtObjectType:type, bool:replace=false)
{
    if (GetGameType() == tf2)
    {
        switch (TF2_GetPlayerClass(client))
        {
            case TFClass_Spy:
            {
                if (TF2_IsPlayerCloaked(client) ||
                    TF2_IsPlayerDeadRingered(client))
                {
                    PrepareAndEmitSoundToClient(client,deniedWav);
                    return false;
                }
                else if (TF2_IsPlayerDisguised(client))
                    TF2_RemovePlayerDisguise(client);
            }
            case TFClass_Scout:
            {
                if (TF2_IsPlayerBonked(client))
                {
                    PrepareAndEmitSoundToClient(client,deniedWav);
                    return false;
                }
            }
        }
    }

    int target = TraceAimTarget(client);
    if (target >= 0)
    {
        float clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);

        float targetLoc[3];
        TR_GetEndPosition(targetLoc);

        if (IsPointInRange(clientLoc,targetLoc,range))
        {
            float distance=GetVectorDistance(clientLoc,targetLoc);
            if (GetRandomFloat(1.0,100.0) <= float(percent) * (1.0 - (distance / range) + 0.20))
            {
                return replace ? ReplaceObject(client, target, builder, type)
                               : ControlObject(client, target, builder, type);
            }
            else
            {
                PrepareAndEmitSoundToClient(client,errorWav); // Chance check failed.
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

    return false;
}

bool:ReplaceObject(client, target, &builder=0, &TFExtObjectType:type=TFExtObject_Unknown)
{
    if (IsValidEntity(target) && IsValidEdict(target))
    {
        type = TF2_GetExtObjectType(target, true);
        if (type != TFExtObject_Unknown)
        {
            //Check to see if the object is still being built
            int placing = GetEntProp(target, Prop_Send, "m_bPlacing");
            int building = GetEntProp(target, Prop_Send, "m_bBuilding");
            float complete = GetEntPropFloat(target, Prop_Send, "m_flPercentageConstructed");
            if (placing == 0 && building == 0 && complete >= 1.0)
            {
                //Find the owner of the object m_hBuilder holds the client index 1 to Maxplayers
                builder = GetEntPropEnt(target, Prop_Send, "m_hBuilder");

                if (builder > 0 && !GetImmunity(builder,Immunity_Ultimates))
                {
                    int team = GetClientTeam(client);
                    if (GetEntProp(target, Prop_Send, "m_iTeamNum") != team)
                    {
                        float pos[3];
                        GetEntPropVector(target, Prop_Send, "m_vecOrigin", pos);

                        float angles[3];
                        GetEntPropVector(target, Prop_Send, "m_angRotation", angles);

                        int iMaxHealth = GetEntProp(target, Prop_Data, "m_iMaxHealth");
                        int iHealth = GetEntProp(target, Prop_Send, "m_iHealth");
                        int iLevel = GetEntProp(target, Prop_Send, "m_iUpgradeLevel");

                        AcceptEntityInput(target, "kill");

                        int obj;
                        switch (type)
                        {
                            case TFExtObject_Sentry:
                            {
                                obj = BuildSentry(client, pos, angles, iLevel, false, false,
                                                  false, iHealth, iMaxHealth);
                            }
                            case TFExtObject_MiniSentry:
                            {
                                obj = BuildSentry(client, pos, angles, iLevel, false, true,
                                                  false, iHealth, iMaxHealth);
                            }
                            case TFExtObject_Teleporter, TFExtObject_TeleporterEntry:
                            {
                                obj = BuildTeleporterEntry(client, pos, angles, iLevel, false,
                                                           iHealth, iMaxHealth);
                            }
                            case TFExtObject_TeleporterExit:
                            {
                                obj = BuildTeleporterExit(client, pos, angles, iLevel, false,
                                                          iHealth, iMaxHealth);
                            }
                            case TFExtObject_Dispenser, TFExtObject_Amplifier, TFExtObject_RepairNode:
                            {
                                obj = BuildDispenser(client, pos, angles, iLevel, false,
                                                     iHealth, iMaxHealth, .type=type);
                            }
                        }
                        return (obj > 0);
                    }
                    else
                    {
                        PrepareAndEmitSoundToClient(client,errorWav);
                        DisplayMessage(client, Display_Ultimate,
                                       "%t", "TargetBelongsToTeammate");
                    }
                }
                else
                {
                    PrepareAndEmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetIsImmune");
                }
            }
            else
            {
                PrepareAndEmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetNotComplete");
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
        PrepareAndEmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "TargetInvalid");
    }
    return false;
}


bool:ControlObject(client, target, &builder=0, &TFExtObjectType:type=TFExtObject_Unknown)
{
    if (IsValidEntity(target) && IsValidEdict(target))
    {
        type = TF2_GetExtObjectType(target);
        if (type != TFExtObject_Unknown)
        {
            //Check to see if the object is still being built
            int placing = GetEntProp(target, Prop_Send, "m_bPlacing");
            int building = GetEntProp(target, Prop_Send, "m_bBuilding");
            float complete = GetEntPropFloat(target, Prop_Send, "m_flPercentageConstructed");
            if (placing == 0 && building == 0 && complete >= 1.0)
            {
                //Find the owner of the object m_hBuilder holds the client index 1 to Maxplayers
                builder = GetEntPropEnt(target, Prop_Send, "m_hBuilder");

                if (builder > 0 && !GetImmunity(builder,Immunity_Ultimates))
                {
                    int team = GetClientTeam(client);
                    if (GetEntProp(target, Prop_Send, "m_iTeamNum") != team)
                    {
                        // Check to see if this target has already been controlled.
                        builder = ProcessMindControlledObjects(update, target, builder, type);

                        // Change the builder to client
                        // But, that seems to cause crashes when
                        // the "real" owner pushes the destruct button :(
                        //SetEntPropEnt(target, Prop_Send, "m_hBuilder", client);

                        //paint red or blue
                        SetEntProp(target, Prop_Send, "m_nSkin", (team==3)?1:0);

                        //Change TeamNum
                        SetVariantInt(team);
                        AcceptEntityInput(target, "TeamNum", -1, -1, 0);

                        //Same thing again but we are changing SetTeam
                        SetVariantInt(team);
                        AcceptEntityInput(target, "SetTeam", -1, -1, 0);

                        //If the gun is controlled, disable it.
                        if (type == TFExtObject_Sentry &&
                            GetEntProp(target, Prop_Send, "m_bPlayerControlled"))
                        {
                            SetEntProp(target, Prop_Send, "m_bDisabled", 1);
                        }

                        PrepareAndEmitSoundToAll(controlWav,target);

                        new color[4] = { 0, 0, 0, 255 };
                        if (team == 3)
                            color[2] = 255; // Blue
                        else
                            color[0] = 255; // Red

                        float clientLoc[3];
                        GetClientAbsOrigin(client, clientLoc);

                        float targetLoc[3];
                        GetEntPropVector(target, Prop_Send, "m_vecOrigin", targetLoc);

                        TE_SetupBeamPoints(clientLoc, targetLoc, Lightning(), HaloSprite(),
                                           0, 1, 2.0, 10.0,10.0,2,50.0,color,255);
                        TE_SendEffectToAll();

                        TE_SetupSmoke(targetLoc,SmokeSprite(),8.0,2);
                        TE_SendEffectToAll();

                        TE_SetupGlowSprite(targetLoc,(team == 3) ? BlueGlow() : RedGlow(),
                                           5.0,5.0,255);
                        TE_SendEffectToAll();

                        float splashDir[3];
                        splashDir[0] = 0.0;
                        splashDir[1] = 0.0;
                        splashDir[2] = 100.0;
                        TE_SetupEnergySplash(targetLoc, splashDir, true);

                        int target_ref = EntIndexToEntRef(target);
                        Handle timer = INVALID_HANDLE;
                        if (type == TFExtObject_Sentry)
                        {
                            timer = CreateTimer(0.1, CheckSentries, target_ref,
                                                TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                        }

                        // Create the Tracking Package
                        Handle pack = CreateDataPack();
                        WritePackCell(pack, target_ref);
                        WritePackCell(pack, builder);
                        WritePackCell(pack, _:timer);
                        WritePackCell(pack, _:type);

                        // And add it to the list
                        if (m_StolenObjectList[client] == INVALID_HANDLE)
                            m_StolenObjectList[client] = CreateArray();

                        PushArrayCell(m_StolenObjectList[client], pack);

                        return true;
                    }
                    else
                    {
                        PrepareAndEmitSoundToClient(client,errorWav);
                        DisplayMessage(client, Display_Ultimate,
                                       "%t", "TargetBelongsToTeammate");
                    }
                }
                else
                {
                    PrepareAndEmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetIsImmune");
                }
            }
            else
            {
                PrepareAndEmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetNotComplete");
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
        PrepareAndEmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "TargetInvalid");
    }
    return false;
}

public Action CheckSentries(Handle:timer,any:ref)
{
    int obj = EntRefToEntIndex(ref);
    if (obj > 0 && IsValidEdict(obj) && IsValidEntity(obj))
    {
        // disable the sentry if it is controlled.
        if (GetEntProp(obj, Prop_Send, "m_bPlayerControlled"))
            SetEntProp(obj, Prop_Send, "m_bDisabled", 1);
        else            
            SetEntProp(obj, Prop_Send, "m_bDisabled", 0);

        return Plugin_Continue;
    }

    ProcessMindControlledObjects(remove, .timer=timer);
    return Plugin_Stop;
}

ProcessMindControlledObjects(command:cmd, obj=-1, builder=-1,
                             TFExtObjectType:type=TFExtObject_Unknown,
                             Handle:timer=INVALID_HANDLE)
{
    if (obj > 0 || builder > 0)
    {
        for (int client =1;client<=MaxClients;client++)
        {
            if (m_StolenObjectList[client] != INVALID_HANDLE)
            {
                int size = GetArraySize(m_StolenObjectList[client]);
                for (int index = 0; index < size; index++)
                {
                    Handle pack = GetArrayCell(m_StolenObjectList[client], index);
                    if (pack != INVALID_HANDLE)
                    {
                        ResetPack(pack);
                        int pack_ref = ReadPackCell(pack);
                        int pack_target = EntRefToEntIndex(pack_ref);
                        int pack_builder = ReadPackCell(pack);
                        Handle pack_timer = Handle:ReadPackCell(pack);
                        new TFExtObjectType:pack_type = TFExtObjectType:ReadPackCell(pack);

                        bool found;
                        if (obj > 0)
                            found = (obj == pack_target);
                        else if (timer != INVALID_HANDLE)
                            found = (timer == pack_timer);
                        else
                            found = (builder == pack_builder && type == pack_type);

                        if (found)
                        {
                            if (cmd == remove || pack_target < 0)
                            {
                                CloseHandle(pack);
                                RemoveFromArray(m_StolenObjectList[client], index);
                                if (pack_timer != INVALID_HANDLE && pack_target > 0)
                                    KillTimer(pack_timer);
                            }
                            else if (cmd == reset)
                            {
                                CloseHandle(pack);
                                RemoveFromArray(m_StolenObjectList[client], index);
                                ResetObject(-1, pack_target, pack_builder, false);
                                if (pack_timer != INVALID_HANDLE && pack_target > 0)
                                    KillTimer(pack_timer);
                            }
                            else if (cmd == update)
                            {
                                // Update the tracking package
                                CloseHandle(pack);
                                pack = CreateDataPack();
                                WritePackCell(pack, pack_ref);
                                WritePackCell(pack, -1);
                                WritePackCell(pack, _:pack_timer);
                                WritePackCell(pack, _:type);
                                SetArrayCell(m_StolenObjectList[client], index, pack);
                            }
                            return (cmd == find_controller) ? index : pack_builder;
                        }
                    }
                }
            }
        }
    }
    return (cmd == find_controller) ? 0 : builder;
}

ResetMindControlledObjects(client, bool:kill)
{
    if (m_StolenObjectList[client] != INVALID_HANDLE)
    {
        int size = GetArraySize(m_StolenObjectList[client]);
        for (int index = 0; index < size; index++)
        {
            Handle pack = GetArrayCell(m_StolenObjectList[client], index);
            if (pack != INVALID_HANDLE)
            {
                ResetPack(pack);
                int target = EntRefToEntIndex(ReadPackCell(pack));
                int builder = ReadPackCell(pack);
                Handle timer = Handle:ReadPackCell(pack);
                CloseHandle(pack);

                ResetObject(client, target, builder, kill);
                //SetArrayCell(m_StolenObjectList[client], index, INVALID_HANDLE);
                if (timer != INVALID_HANDLE && target > 0)
                    KillTimer(timer);
            }
        }
        ClearArray(m_StolenObjectList[client]);
        CloseHandle(m_StolenObjectList[client]);
        m_StolenObjectList[client] = INVALID_HANDLE;
    }
}

ResetObject(client, target, builder, bool:kill)
{
    if (target > 0 && IsValidEntity(target) && IsValidEdict(target))
    {
        // Do we still own it?
        if (client <= 0 || GetEntPropEnt(target, Prop_Send, "m_hBuilder") ==  client)
        {
            // Is the round not ending and the builder valid?
            // (still around and still an engie)?
            if (kill || !IsValidClientNotSpec(builder) ||
                TF2_GetPlayerClass(builder) != TFClass_Engineer)
            {
                AcceptEntityInput(target, "Kill", -1, -1, 0);
                //RemoveEdict(target); // Remove the object.
            }
            else
            {
                // Give it back.
                int team = GetClientTeam(builder);

                // Change the builder back
                SetEntPropEnt(target, Prop_Send, "m_hBuilder", builder);

                //paint red or blue
                SetEntProp(target, Prop_Send, "m_nSkin", (team==3)?1:0);

                //Change TeamNum
                SetVariantInt(team);
                AcceptEntityInput(target, "TeamNum", -1, -1, 0);

                //Same thing again but we are changing SetTeam
                SetVariantInt(team);
                AcceptEntityInput(target, "SetTeam", -1, -1, 0);

                //Make sure the gun is enabled, in case it was controlled.
                SetEntProp(target, Prop_Send, "m_bDisabled", 0);
            }
        }
    }
}

public int Native_MindControl(Handle plugin, int numParams)
{
    int client = GetNativeCell(1);
    float range = Float:GetNativeCell(2);
    int percent = GetNativeCell(3);
    int builder = GetNativeCellRef(4);
    new TFExtObjectType:type = GetNativeCellRef(5);
    bool replace = GetNativeCell(6);
    bool success = MindControl(client,range,percent, builder, type, replace);
    if (success)
    {
        SetNativeCellRef(4, builder);
        SetNativeCellRef(5, type);
    }
    return success;
}

public int Native_ControlObject(Handle plugin, int numParams)
{
    int client = GetNativeCell(1);
    int target = GetNativeCell(2);
    int builder = GetNativeCellRef(3);
    new TFExtObjectType:type = GetNativeCellRef(4);
    bool success = ControlObject(client,target, builder, type);
    if (success)
    {
        SetNativeCellRef(3, builder);
        SetNativeCellRef(4, type);
    }
    return success;
}

public int Native_ReplaceObject(Handle plugin, int numParams)
{
    int client = GetNativeCell(1);
    int target = GetNativeCell(2);
    int builder = GetNativeCellRef(3);
    new TFExtObjectType:type = GetNativeCellRef(4);
    bool success = ReplaceObject(client,target, builder, type);
    if (success)
    {
        SetNativeCellRef(3, builder);
        SetNativeCellRef(4, type);
    }
    return success;
}

public int Native_ResetMindControlledObjs(Handle plugin, int numParams)
{
    int client = GetNativeCell(1);
    bool kill = bool:GetNativeCell(2);
    ResetMindControlledObjects(client,kill);
}

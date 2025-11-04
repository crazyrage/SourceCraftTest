#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

// Start TF2-first; add gates for other games later
#include <tf2>
#include <tf2_stocks>

#include "sourcecraft2"
#include "core/core"

public Plugin myinfo =
{
    name = "SourceCraft 2",
    author = "Clean-room remake",
    description = "1:1 behavior remake on SM 1.12 (scaffold)",
    version = "0.1.0",
    url = ""
};

enum struct PlayerState
{
    int level;
    int xp;
    int crystals;
    int vespene;
    float energy;
    bool initialized;
}

PlayerState g_Player[MAXPLAYERS + 1];

public void OnPluginStart()
{
    RegConsoleCmd("sc_menu", Cmd_OpenMenu);

    HookEvent("player_spawn", Event_PlayerSpawn, EventHookMode_Post);
    HookEvent("player_death", Event_PlayerDeath, EventHookMode_Post);

    SC2_InitCore();
}

public void OnClientDisconnect(int client)
{
    if (client > 0 && client <= MaxClients)
    {
        g_Player[client] = PlayerState{0, 0, 0, 0, 0.0, false};
    }
}

public void OnClientPutInServer(int client)
{
    if (client > 0 && client <= MaxClients)
    {
        g_Player[client] = PlayerState{0, 0, 0, 0, 0.0, false};
    }
}

public Action Cmd_OpenMenu(int client, int argc)
{
    if (client <= 0 || !IsClientInGame(client))
        return Plugin_Handled;

    SC2_OpenMainMenu(client);
    return Plugin_Handled;
}

public void Event_PlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    if (client <= 0) return;

    if (!g_Player[client].initialized)
    {
        g_Player[client].initialized = true;
        SC2_ApplyRacePassives(client);
    }
}

public void Event_PlayerDeath(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));
    int attacker = GetClientOfUserId(event.GetInt("attacker"));
    SC2_HandleDeath(client, attacker);
}

// Optional: entity observation hooks as needed later
public void OnEntityCreated(int entity, const char[] classname) { }
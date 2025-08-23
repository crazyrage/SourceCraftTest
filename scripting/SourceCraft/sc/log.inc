/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: log.inc
 * Description: Writes message to console and log.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _log_included
 #endinput
#endif
#define _log_included

stock Log(const String:message[])
{
    PrintToServer(message);
    LogMessage(message);
}

stock Error(const String:message[])
{
    PrintToServer(message);
    LogError(message);
}

stock Fail(const String:message[])
{
    PrintToServer(message);
    SetFailState(message);
}

stock LogEventDamage(Handle:event,damage,const String:source[], race_id)
{
    int userid =GetEventInt(event,"userid");
    int victim =GetClientOfUserId(userid);
    int victim_player =GetPlayerHandle(victim);
    int victim_race =(victim_player != INVALID_HANDLE) ? GetRace(victim_player) : -1;

    int attacker_uid =GetEventInt(event,"attacker");
    int attacker =GetClientOfUserId(attacker_uid);
    int attacker_player =GetPlayerHandle(attacker);
    int attacker_race =(attacker_player != INVALID_HANDLE) ? GetRace(attacker_player) : -1;

    int assister_uid = GetEventInt(event,"assister");
    int assister =GetClientOfUserId(assister_uid);
    int assister_player =GetPlayerHandle(assister);
    int assister_race =(assister_player != INVALID_HANDLE) ? GetRace(assister_player) : -1;

    if (race_id == victim_race   || race_id == attacker_race ||
        race_id == assister_race || race_id < 0)
    {
        int health =0;
        char team[64];
        char steamID[64];
        char attacker_weapon[64];
        if (attacker)
        {
            GetClientAuthId(attacker, AuthId_Steam2, steamID, sizeof(steamID));
            GetTeamName(GetClientTeam(attacker), team, sizeof(team));
            GetClientWeapon(attacker, attacker_weapon, sizeof(attacker_weapon));
            health=GetClientHealth(attacker);
        }
        else
        {
            team[0]=0;
            attacker_weapon[0]=0;
            strcopy(steamID,sizeof(steamID), "World");
        }

        char victimID[64];
        GetClientAuthId(victim, AuthId_Steam2, victimID, sizeof(victimID));

        char victimTeam[64];
        GetTeamName(GetClientTeam(victim), victimTeam, sizeof(victimTeam));

        int victimHealth =GetClientHealth(victim);

        char event_weapon[64];
        GetEventString(event,"weapon",event_weapon,sizeof(event_weapon));

        int event_damage = GetEventInt(event, "damage");

        if (assister)
        {
            char assisterID[64];
            GetClientAuthId(assister, AuthId_Steam2, assisterID, sizeof(assisterID));

            char assisterTeam[64];
            GetTeamName(GetClientTeam(assister), assisterTeam, sizeof(assisterTeam));

            char assister_weapon[64];
            GetClientWeapon(assister, assister_weapon, sizeof(assister_weapon));

            int assisterHealth =GetClientHealth(assister);

            LogMessage("[%s] \"%N<%d><%s><%s>[%d]\"+\"%N<%d><%s><%s>[%d]\" damaged \"%N<%d><%s><%s>[%d]\" for %d(%d) with %s(%s+%s)",
                       source, attacker, attacker, steamID, team, health,
                       assister, assister,assisterID, assisterTeam, assisterHealth,
                       victim, victim, victimID, victimTeam, victimHealth,
                       damage, event_damage, event_weapon, attacker_weapon, assister_weapon);

            LogToGame("[%s] \"%N<%d><%s><%s>[%d]\"+\"%N<%d><%s><%s>[%d]\" damaged \"%N<%d><%s><%s>[%d]\" for %d(%d) with %s(%s+%s)",
                      source, attacker, attacker, steamID, team, health,
                      assister, assister,assisterID, assisterTeam, assisterHealth,
                      victim, victim, victimID, victimTeam, victimHealth,
                      damage, event_damage, event_weapon, attacker_weapon, assister_weapon);
        }
        else
        {
            LogMessage("[%s] \"%N<%d><%s><%s>[%d]\" damaged \"%N<%d><%s><%s>[%d]\" for %d(%d) with %s(%s)",
                       source, attacker, attacker, steamID, team, health,
                       victim, victim, victimID, victimTeam, victimHealth,
                       damage, event_damage, event_weapon, attacker_weapon);
            LogToGame("[%s] \"%N<%d><%s><%s>[%d]\" damaged \"%N<%d><%s><%s>[%d]\" for %d(%d) with %s(%s)\n",
                      source, attacker, attacker, steamID, team, health,
                      victim, victim, victimID, victimTeam, victimHealth,
                      damage, event_damage, event_weapon, attacker_weapon);
        }
    }
}

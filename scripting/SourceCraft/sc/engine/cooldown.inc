/**
 * vim: set ai et ts=4 sw=4 :
 * File: cooldown.inc
 * Description: Cooldown processing
 * Author(s): -=|JFH|=-Naris
 */

#include "sc/cooldown"

// Handles
Handle g_OnCooldownExpiredHandle = INVALID_HANDLE;
Handle m_PlayerCooldownTrie[MAXPLAYERS+1];
Handle m_PlayerCooldownArray[MAXPLAYERS+1];

// Place to stash the last cooldown type
int m_CooldownExpireRace;
int m_CooldownExpireClient;
int m_CooldownExpireUpgrade;
new CooldownType:m_CooldownExpireType;

Float:GetCooldownExpireTime(client, race, upgrade, &Handle:pack=INVALID_HANDLE)
{
    Handle trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        char key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);

        if (GetTrieValue(trie, key, pack))
        {
            ResetPack(pack);
            int index = ReadPackCell(pack);
            int userid = ReadPackCell(pack);
            if (GetClientOfUserId(userid) == index)
            {
                ReadPackCell(pack); // race
                ReadPackCell(pack); // upgrade
                return ReadPackFloat(pack); // expireTime
            }
        }
    }
    return 0.0;
}

CooldownType:GetCooldownType(client, race, upgrade)
{
    if (client == m_CooldownExpireClient &&
        race == m_CooldownExpireRace &&
        upgrade == m_CooldownExpireUpgrade)
    {
        return m_CooldownExpireType;
    }
    else
    {
        Handle pack = INVALID_HANDLE;
        float cooldownExpires = GetCooldownExpireTime(client, race, upgrade, pack);
        if (pack != INVALID_HANDLE && cooldownExpires != 0.0)
            return CooldownType:ReadPackCell(pack); // CooldownType:type
        else        
            return Cooldown_None;
    }
}

bool:HasCooldownExpired(client, race, upgrade, bool:notify=true)
{
    TraceInto("Cooldown", "HasCooldownExpired", "client=%d:%N, race=%d, upgrade=%d, notify=%d", \
              client, ValidClientIndex(client), race, upgrade, notify);

    Handle pack = INVALID_HANDLE;
    float currentTime = GetGameTime();
    float cooldownExpires = GetCooldownExpireTime(client, race, upgrade, pack);
    if (cooldownExpires > currentTime)
    {
        if (notify)
        {
            ReadPackCell(pack); // CooldownType:type
            ReadPackCell(pack); // Handle:timerHandle

            char name[NAME_STRING_LENGTH];
            ReadPackString(pack, name, sizeof(name));

            if (!name[0])
            {
                Handle raceHandle=GetRaceHandle(race);
                int upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                if (upgradeCategory > 1)
                    Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                else if (upgradeCategory == 1)
                    strcopy(name, sizeof(name), "Ultimate");
                else
                    strcopy(name, sizeof(name), "Upgrade");
            }

            PrepareAndEmitSoundToClient(client,deniedWav);

            float remaining = cooldownExpires - currentTime;
            float minutes = remaining / 60.0;
            if (minutes > 1.0)
            {
                int seconds = RoundToFloor(FloatFraction(minutes) * 60.0);
                if (FlipBit_IsSet(g_ClientDisplayBits[client] & Display_Ultimate_Hints))
                {
                    DisplayHint(client, HintSlot_NotReady, true, 5.0, "%t", "WontBeReadyForMinutes",
                                name, RoundToFloor(minutes), seconds);
                }
                else
                {
                    DisplayMessage(client, Display_Ultimate, "%t", "WontBeReadyForMinutes",
                                   name, RoundToFloor(minutes), seconds);
                }
                TraceCat("Cooldown", "HasCooldownExpired: Cooldown has not expired! %f < %f, %f minutes (%f seconds) remaining", \
                         currentTime, cooldownExpires, minutes, seconds);
            }
            else
            {
                if (FlipBit_IsSet(g_ClientDisplayBits[client] & Display_Ultimate_Hints))
                {
                    DisplayHint(client, HintSlot_NotReady, true, 5.0, "%t", "WontBeReadyForSeconds",
                                name, remaining);
                }
                else
                {
                    DisplayMessage(client, Display_Ultimate, "%t", "WontBeReadyForSeconds",
                                   name, remaining);
                }
                TraceCat("Cooldown", "HasCooldownExpired: Cooldown has not expired! %f < %f, %f seconds remaining", \
                         currentTime, cooldownExpires, remaining);
            }
        }
        else
        {
            TraceCat("Cooldown", "HasCooldownExpired: Cooldown has not expired! %f < %f, %f seconds remaining", \
                     currentTime, cooldownExpires, cooldownExpires - currentTime);
        }
        TraceReturn("expired=false");
        return false;
    }
    else
    {
        TraceReturn("expired=true");
        return true;
    }
}

CreateCooldown(client, race, upgrade, Float:cooldown, const String:name[],
               CooldownType:type, const String:expireSound[])
{
    TraceInto("Cooldown", "CreateCooldown", "client=%d:%N, race=%d, upgrade=%d", \
              client, ValidClientIndex(client), race, upgrade);

    if (cooldown > 0.0)
    {
        if (name[0] && (type & Cooldown_CreateNotify) == Cooldown_CreateNotify)
        {
            float minutes = cooldown / 60.0;
            if (minutes > 1.0)
            {
                int seconds = RoundToFloor(FloatFraction(minutes) * 60.0);
                if (FlipBit_IsSet(g_ClientDisplayBits[client] & Display_Ultimate_Hints))
                {
                    DisplayHint(client, HintSlot_NotReady, true, 5.0, "%t", "MustWaitMinutes",
                                RoundToFloor(minutes), seconds, name);
                }
                else
                {
                    DisplayMessage(client, Display_Ultimate, "%t", "MustWaitMinutes",
                                   RoundToFloor(minutes), seconds, name);
                }
            }
            else
            {
                if (FlipBit_IsSet(g_ClientDisplayBits[client] & Display_Ultimate_Hints))
                {
                    DisplayHint(client, HintSlot_NotReady, true, 5.0,
                                "%t", "MustWaitSeconds", cooldown, name);
                }
                else
                {
                    DisplayMessage(client, Display_Ultimate, "%t", "MustWaitSeconds",
                                   cooldown, name);
                }
            }
        }

        if ((type & Cooldown_AutoReset) == Cooldown_AutoReset)
        {
            if (cooldown > 30.0)
            {
                type &= ~(Cooldown_ResetOnDeath  |
                          Cooldown_ResetOnSpawn);

                if (cooldown > 60.0)
                    type &= ~Cooldown_ResetOnChange;
                else                    
                    type |= Cooldown_ResetOnChange;
            }
            else
            {
                type |= (Cooldown_ResetOnChange |
                         Cooldown_ResetOnDeath);
            }
        }            

        Handle pack;
        Handle timer = CreateDataTimer(cooldown,CooldownExpireTimer,pack,TIMER_FLAG_NO_MAPCHANGE);

        TraceCat("Array", "CreateDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                 client, race, upgrade, timer, pack);

        if (pack != INVALID_HANDLE)
        {
            WritePackCell(pack, client);
            WritePackCell(pack, GetClientUserId(client));
            WritePackCell(pack, race);
            WritePackCell(pack, upgrade);
            WritePackFloat(pack, GetGameTime() + cooldown);
            WritePackCell(pack, _:type);
            WritePackCell(pack, _:timer);
            WritePackString(pack, name);
            WritePackString(pack, expireSound);

            Handle array = m_PlayerCooldownArray[client];
            if (array == INVALID_HANDLE)
            {
                array = m_PlayerCooldownArray[client] = CreateArray();

                TraceCat("Array", "CreateArray m_PlayerCooldownArray[%d]=0x%08x", \
                             client,array);
            }

            Handle trie = m_PlayerCooldownTrie[client];
            if (trie == INVALID_HANDLE)
            {
                trie = m_PlayerCooldownTrie[client] = CreateTrie();

                TraceCat("Array", "CreateTrie m_PlayerCooldownTrie[%d]=0x%08x", \
                             client,trie);
            }

            char key[SHORT_STRING_LENGTH];
            Format(key, sizeof(key), "%d-%d", race, upgrade);

            // Check if there is already a cooldown for this client/race/upgrade.
            Handle oldPack;
            if (GetTrieValue(trie, key, oldPack))
            {
                // If there is, delete the old one
                // and replace it with the new one.

                ResetPack(oldPack);
                ReadPackCell(oldPack); // index
                ReadPackCell(oldPack); // userid
                ReadPackCell(oldPack); // race
                ReadPackCell(oldPack); // upgrade
                ReadPackFloat(oldPack); // expireTime
                ReadPackCell(oldPack); // CooldownType:type
                Handle oldTimerHandle = Handle:ReadPackCell(oldPack);

                int item = FindValueInArray(array, oldPack);
                if (item >= 0)
                    SetArrayCell(array, item, pack);
                else
                    PushArrayCell(array, pack);

                if (oldTimerHandle != INVALID_HANDLE)
                {
                    KillTimer(oldTimerHandle, true);

                    TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, oldTimerHandle, oldPack);
                }
                else
                {
                    CloseHandle(oldPack);

                    TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, oldTimerHandle, oldPack);
                }
            }
            else
            {
                PushArrayCell(array, pack);
            }

            SetTrieValue(trie, key, pack, true);
        }
    }

    TraceReturn();
}

public Action CooldownExpireTimer(Handle:timer,any:pack)
{
    ResetPack(pack);
    int index = ReadPackCell(pack);
    int userid = ReadPackCell(pack);
    int race = ReadPackCell(pack);
    int upgrade = ReadPackCell(pack);

    TraceInto("Cooldown", "CooldownExpireTimer", \
              "timer=0x%08x, pack=0x%08x, userid=%d, index=%d:%N, race=%d, upgrade=%d", \
              timer, pack, userid, index, ValidClientIndex(index), race, upgrade);

    Handle trie = m_PlayerCooldownTrie[index];
    if (trie != INVALID_HANDLE)
    {
        char key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);
        RemoveFromTrie(trie, key);
    }

    Handle array = m_PlayerCooldownArray[index];
    if (array != INVALID_HANDLE)
    {
        int item = FindValueInArray(array, pack);
        if (item >= 0)
            RemoveFromArray(array, item);
    }

    if (GetClientOfUserId(userid) == index)
    {
        ReadPackFloat(pack); // expireTime
        new CooldownType:type = CooldownType:ReadPackCell(pack);
        ReadPackCell(pack); // Handle:timerHandle

        // Stash the cooldown type
        m_CooldownExpireRace    = race;
        m_CooldownExpireClient  = index;
        m_CooldownExpireUpgrade = upgrade;
        m_CooldownExpireType    = type;

        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnCooldownExpiredHandle);
        Call_PushCell(index);
        Call_PushCell(race);
        Call_PushCell(upgrade);
        Call_PushCell(true);
        Call_Finish(result);

        if ((type & (Cooldown_ExpireNotify|Cooldown_AlwaysNotify)) != Cooldown_TimerOnly &&
            result == Plugin_Continue)
        {
            if (IsClientInGame(index) && IsPlayerAlive(index))
            {
                if ((type & Cooldown_AlwaysNotify) != Cooldown_TimerOnly ||
                    GetRace(index) == race)
                {
                    char name[NAME_STRING_LENGTH];
                    ReadPackString(pack, name, sizeof(name));

                    char expireSound[LONG_STRING_LENGTH];
                    ReadPackString(pack, expireSound, sizeof(expireSound));

                    if (!name[0])
                    {
                        Handle raceHandle=GetRaceHandle(race);
                        int upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                        if (upgradeCategory > 1)
                            Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                        else if (upgradeCategory == 1)
                            strcopy(name, sizeof(name), "Ultimate");
                        else
                            strcopy(name, sizeof(name), "Upgrade");
                    }

                    if (expireSound[0])
                    {
                        PrepareAndEmitSoundToClient(index, expireSound);
                    }
                    else
                    {
                        PrepareAndEmitSoundToClient(index, rechargeWav);
                    }

                    if (FlipBit_IsSet(g_ClientDisplayBits[index] & Display_Ultimate_Hints))
                    {
                        DisplayHint(index, HintSlot_NotReady, true);
                        DisplayHint(index, HintSlot_Expired, false,
                                    5.0, "%t", "IsReady", name);
                    }
                    else
                    {
                        DisplayMessage(index, Display_Ultimate,
                                       "%t", "IsReady", name);
                    }
                }
            }
        }
    }

    SetTraceCategory("Array");
    TraceReturn("EndDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                index, race, upgrade, timer, pack);

    return Plugin_Stop;
}

CooldownSpawn(client)
{
    int race = GetRace(client);
    if (race > 0)
    {
        Handle raceHandle = GetRaceHandle(race);
        int upgradeCount =GetUpgradeCount(raceHandle);
        for (int upgrade =0;upgrade<upgradeCount;upgrade++)
        {
            new CooldownType:type = GetUpgradeCooldownType(raceHandle,upgrade);
            if ((type & Cooldown_CreateOnSpawn) == Cooldown_CreateOnSpawn)
            {
                float cooldown = GetUpgradeCooldownDuration(raceHandle,upgrade);
                if (cooldown > 0.0)
                {
                    char name[64];
                    GetUpgradeName(raceHandle, upgrade, name, sizeof(name), client);
                    CreateCooldown(client, race, upgrade, cooldown, name, type, "");
                }
            }
        }
    }
}

CooldownEvent(client, CooldownType:resetBits)
{
    TraceInto("Cooldown", "CooldownEvent", "client=%d:%N, resetBits=0x%02x", \
              client, ValidClientIndex(client), resetBits);

    Handle array = m_PlayerCooldownArray[client];
    if (array != INVALID_HANDLE)
    {
        // Iterate backwards since we will be removing elements.
        for (int i = GetArraySize(array)-1; i >= 0 ; i--)
        {
            Handle pack = Handle:GetArrayCell(array, i);
            ResetPack(pack);

            ReadPackCell(pack); // index
            int userid = ReadPackCell(pack);
            int race = ReadPackCell(pack);
            int upgrade = ReadPackCell(pack);
            float expireTime = ReadPackFloat(pack);
            new CooldownType:type = CooldownType:ReadPackCell(pack);
            Handle timerHandle = Handle:ReadPackCell(pack);

            bool remove = (GetClientOfUserId(userid) != client);
            if (remove || ((type & resetBits) != Cooldown_TimerOnly &&
                           expireTime > GetGameTime()))
            {
                new Action:result = Plugin_Continue;
                Call_StartForward(g_OnCooldownExpiredHandle);
                Call_PushCell(client);
                Call_PushCell(race);
                Call_PushCell(upgrade);
                Call_PushCell(false);
                Call_Finish(result);

                if (remove || result == Plugin_Continue)
                {
                    Handle trie = m_PlayerCooldownTrie[client];
                    if (trie != INVALID_HANDLE)
                    {
                        char key[SHORT_STRING_LENGTH];
                        Format(key, sizeof(key), "%d-%d", race, upgrade);
                        RemoveFromTrie(trie, key);
                    }

                    RemoveFromArray(array, i);

                    if (timerHandle != INVALID_HANDLE)
                    {
                        KillTimer(timerHandle, true);

                        TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                                 client, race, upgrade, timerHandle, pack);
                    }
                    else
                    {
                        CloseHandle(pack);

                        TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                                 client, race, upgrade, timerHandle, pack);
                    }
                }
            }
        }
    }

    TraceReturn();
}

CooldownDisconnect(client)
{
    TraceInto("Cooldown", "CooldownDisconnect", "client=%d:%N", \
              client, ValidClientIndex(client));

    Handle trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        ClearTrie(trie);
        CloseHandle(trie);
        m_PlayerCooldownTrie[client] = INVALID_HANDLE;

        TraceCat("Array", "CloseTrie m_PlayerCooldownTrie[%d]=0x%08x", \
                 client,trie);
    }

    Handle array = m_PlayerCooldownArray[client];
    if (array != INVALID_HANDLE)
    {
        // Iterate backwards for consistency.
        for (int i = GetArraySize(array)-1; i >= 0 ; i--)
        {
            Handle pack = Handle:GetArrayCell(array, i);
            ResetPack(pack);
            ReadPackCell(pack); // index
            ReadPackCell(pack); // userid
            int race = ReadPackCell(pack); // race
            int upgrade = ReadPackCell(pack); // upgrade
            ReadPackFloat(pack); // expireTime
            ReadPackCell(pack); // CooldownType:type
            Handle timerHandle = Handle:ReadPackCell(pack);

            #if !defined _TRACE
                #pragma unused race, upgrade
            #endif

            if (timerHandle != INVALID_HANDLE)
            {
                KillTimer(timerHandle, true);

                TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                         client, race, upgrade, timerHandle, pack);
            }
            else
            {
                CloseHandle(pack);

                TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                         client, race, upgrade, timerHandle, pack);
            }
        }

        ClearArray(array);
        CloseHandle(array);
        m_PlayerCooldownArray[client] = INVALID_HANDLE;

        TraceCat("Array", "CloseArray m_PlayerCooldownArray[%d]=0x%08x", \
                 client,array);
    }

    TraceReturn();
}

ResetAllCooldowns()
{
    for (int i = 1; i <= MaxClients; i++)
        CooldownDisconnect(i);
}

ResetCooldown(client, race, upgrade, bool:notify)
{
    TraceInto("Cooldown", "ResetCooldown", "client=%d:%N, race=$d, upgrade=%d, notify=%d", \
              client, ValidClientIndex(client), race, upgrade, notify);

    Handle trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        char key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);

        Handle pack;
        if (GetTrieValue(trie, key, pack))
        {
            ResetPack(pack);
            ReadPackCell(pack); // index
            ReadPackCell(pack); // userid
            ReadPackCell(pack); // race
            ReadPackCell(pack); // upgrade
            ReadPackFloat(pack); // expireTime
            new CooldownType:type = CooldownType:ReadPackCell(pack);
            Handle timerHandle = Handle:ReadPackCell(pack);

            new Action:result = Plugin_Continue;
            Call_StartForward(g_OnCooldownExpiredHandle);
            Call_PushCell(client);
            Call_PushCell(race);
            Call_PushCell(upgrade);
            Call_PushCell(false);
            Call_Finish(result);

            if (result == Plugin_Continue)
            {
                if ((type & (Cooldown_ExpireNotify|Cooldown_AlwaysNotify)) != Cooldown_TimerOnly)
                {
                    if (IsClientInGame(client) && IsPlayerAlive(client))
                    {
                        if (notify || (type & Cooldown_AlwaysNotify) != Cooldown_TimerOnly)
                        {
                            char name[NAME_STRING_LENGTH];
                            ReadPackString(pack, name, sizeof(name));

                            char expireSound[LONG_STRING_LENGTH];
                            ReadPackString(pack, expireSound, sizeof(expireSound));

                            if (!name[0])
                            {
                                Handle raceHandle=GetRaceHandle(race);
                                int upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                                if (upgradeCategory > 1)
                                    Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                                else if (upgradeCategory == 1)
                                    strcopy(name, sizeof(name), "Ultimate");
                                else
                                    strcopy(name, sizeof(name), "Upgrade");
                            }

                            if (expireSound[0])
                            {
                                PrepareAndEmitSoundToClient(client, expireSound);
                            }
                            else
                            {
                                PrepareAndEmitSoundToClient(client, rechargeWav);
                            }

                            if (FlipBit_IsSet(g_ClientDisplayBits[client] & Display_Ultimate_Hints))
                            {
                                DisplayHint(client, HintSlot_NotReady, true);
                                DisplayHint(client, HintSlot_Expired, true,
                                            5.0, "%t", "IsReady", name);
                            }
                            else
                            {
                                DisplayMessage(client, Display_Ultimate,
                                               "%t", "IsReady", name);
                            }
                        }
                    }
                }

                RemoveFromTrie(trie, key);

                Handle array = m_PlayerCooldownArray[client];
                if (array != INVALID_HANDLE)
                {
                    int item = FindValueInArray(array, pack);
                    if (item >= 0)
                        RemoveFromArray(array, item);
                }

                if (timerHandle != INVALID_HANDLE)
                {
                    KillTimer(timerHandle, true);

                    TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, timerHandle, pack);
                }
                else
                {
                    CloseHandle(pack);

                    TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, timerHandle, pack);
                }
            }
        }
    }

    TraceReturn();
}

/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: config.inc
 * Description: This file handles configuration files
 * Author(s): Naris (Murray Wilson)
 */

#include <string_to>

Handle vecLevelConfiguration                    = INVALID_HANDLE;
Handle g_SourceCraftConfigHandle                = INVALID_HANDLE;
Handle g_ItemConfigHandle                       = INVALID_HANDLE;
int ConfigLoadedStatus:g_SourceCraftConfigLoaded    = ConfigNotLoaded;
int ConfigLoadedStatus:g_ItemConfigLoaded           = ConfigNotLoaded;

GetConfigNum(const String:key[], defvalue=0, fileId=MAIN,
             node=NONE, const String:section[]="")
{
    TraceInto("Config", "GetConfigNum", "file=%d, node=%d, section=%s, key=%s, def=%d", \
              fileId, node, section, key, defvalue);

    new value;
    if (fileId >= SHOPITEM)
    {
        new ConfigLoadedStatus:configLoaded;
        Handle raceHandle;
        Handle configHandle;
        char nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                Handle itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
            else
                nodeName[0] = '\0';
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
            else
                nodeName[0] = '\0';
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                bool nodeFound;
                if (configLoaded == ConfigNotLoaded)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        bool updateConfig = false;
                        bool sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            value = KvGetNum(configHandle, key, defvalue);
                            if (value == defvalue)
                            {
                                new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                    ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                                updateConfig = (loaded != ConfigWasLoaded);
                            }
                        }
                        else
                            updateConfig = true;

                        if (updateConfig)
                        {
                            KvSetNum(configHandle, key, value=defvalue);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = ConfigWasUpdated;
                            else
                                SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetNum(configHandle, key, value=defvalue);
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == ConfigNotLoaded)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    KvSetNum(configHandle, key, value=defvalue);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = ConfigWasUpdated;
                    else
                        SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                }
                else
                {
                    bool updateConfig = false;
                    bool sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        value = KvGetNum(configHandle, key, defvalue);
                        if (value == defvalue)
                        {
                            new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                            updateConfig = (loaded != ConfigWasLoaded);
                        }
                    }
                    else
                        updateConfig = true;

                    if (updateConfig)
                    {
                        KvSetNum(configHandle, key, value=defvalue);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }
            }
        }

        #if defined _TRACE
            char short[SHORT_STRING_LENGTH];
            if (raceHandle != INVALID_HANDLE)
                GetRaceShortName(raceHandle,short,sizeof(short));
            else
                short[0] = '\0';

            Trace("file=%d:%s, node=%s, section=%s, key=%s, value=%d, def=%d, raceHandle=0x%08x, IsLoaded=%d", \
                  fileId, short, nodeName, section, key, value, defvalue, raceHandle, \
                  (raceHandle == INVALID_HANDLE) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle));
        #endif
    }
    else if (section[0] == '\0' && StrEqual(key, "update"))
        return g_bUpdate;
    else if (section[0] == '\0' && StrEqual(key, "savexp"))
        return g_bSaveXP;
    else if (section[0] == '\0' && StrEqual(key, "min_players"))
        return g_iMinPlayers;
    else if (section[0] == '\0' && StrEqual(key, "min_ultimate"))
        return g_iMinUltimate;
    else if (section[0] == '\0' && StrEqual(key, "max_crystals"))
        return g_iMaxCrystals;
    else if (section[0] == '\0' && StrEqual(key, "max_vespene"))
        return g_iMaxVespene;
    else if (section[0] == '\0' && StrEqual(key, "use_money"))
        return g_bUseMoney;
    else if (section[0] == '\0' && StrEqual(key, "drop_xp_bias"))
        return g_iDropXPBias;
    else if (section[0] == '\0' && StrEqual(key, "max_drop_xp"))
        return g_iMaxDropXP;
    else if (section[0] == '\0' && StrEqual(key, "max_packages"))
        return g_iMaxPackages;
    else if (section[0] == '\0' && StrEqual(key, "max_drop_money"))
        return g_iMaxDropMoney;
    else if (section[0] == '\0' && StrEqual(key, "drop_money_bias"))
        return g_iDropMoneyBias;
    else if (section[0] == '\0' && StrEqual(key, "drop_crystal_bias"))
        return g_iDropCrystalBias;
    else if (section[0] == '\0' && StrEqual(key, "max_drop_crystals"))
        return g_iMaxDropCrystals;
    else if (section[0] == '\0' && StrEqual(key, "drop_pcrystals_bias"))
        return g_iDropPCrystalsBias;
    else if (section[0] == '\0' && StrEqual(key, "max_drop_pcrystals"))
        return g_iMaxDropPCrystals;
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        bool sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == ConfigWasLoaded)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = ConfigWasUpdated;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == ConfigNotLoaded || !sectionFound)
            KvSetNum(g_SourceCraftConfigHandle, key, value=defvalue);
        else
            value = KvGetNum(g_SourceCraftConfigHandle, key, defvalue);
    }

    TraceReturn("value=%d", value);
    return value;
}

Float:GetConfigFloat(const String:key[], Float:defvalue=0.0, fileId=MAIN,
                     node=NONE, const String:section[]="")
{
    TraceInto("Config", "GetConfigFloat", "file=%d, node=%d, section=%s, key=%s, def=%f", \
              fileId, node, section, key, defvalue);

    float value;
    if (fileId >= SHOPITEM)
    {
        Handle raceHandle;
        Handle configHandle;
        new ConfigLoadedStatus:configLoaded;
        char nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                Handle itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                bool nodeFound;
                if (configLoaded == ConfigNotLoaded)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        bool updateConfig = false;
                        bool sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            value = KvGetFloat(configHandle, key, defvalue);
                            if (value == defvalue)
                            {
                                new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                    ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                                updateConfig = (loaded != ConfigWasLoaded);
                            }

                            Trace("KvGetFloat sectionFound=%d, configHandle=0x%08x, file=%d, node=%s, section=%s, key=%s, value=%f, def=%f, updateConfig=%d", \
                                  sectionFound, configHandle, fileId, nodeName, section, key, value, defvalue, updateConfig);
                        }
                        else
                            updateConfig = true;

                        if (updateConfig)
                        {
                            KvSetFloat(configHandle, key, value=defvalue);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = ConfigWasUpdated;
                            else
                                SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);

                            Trace("KvSetFloat sectionFound=%d, configHandle=0x%08x, file=%d, node=%s, section=%s, key=%s, def=%f", \
                                  sectionFound, configHandle, fileId, nodeName, section, key, defvalue);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetFloat(configHandle, key, value=defvalue);

                        Trace("KvSetFloat2 nodeFound=%d, configHandle=0x%08x, file=%d, node=%s, section=%s, key=%s, def=%f", \
                              nodeFound, configHandle, fileId, nodeName, section, key, defvalue);
                    }
                    else
                    {
                        LogError("Unable to create node %s for %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == ConfigNotLoaded)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                            LogError("Unable to create %s section", section);
                    }

                    KvSetFloat(configHandle, key, value=defvalue);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = ConfigWasUpdated;
                    else
                        SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);

                    Trace("KvSetFloat3 configHandle=0x%08x, file=%d, section=%s, key=%s, def=%f", \
                          configHandle, fileId, section, key, defvalue);
                }
                else
                {
                    bool updateConfig = false;
                    bool sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        value = KvGetFloat(configHandle, key, defvalue);
                        if (value == defvalue)
                        {
                            new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                            updateConfig = (loaded != ConfigWasLoaded);
                        }
                    }
                    else
                        updateConfig = true;

                    if (updateConfig)
                    {
                        KvSetFloat(configHandle, key, value=defvalue);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);

                        Trace("KvSetFloat4 configHandle=0x%08x, file=%d, section=%s, key=%s, def=%f", \
                              configHandle, fileId, section, key, defvalue);
                    }
                }
            }
        }

        #if !defined _TRACE
            char short[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle,short,sizeof(short));
            Trace("file=%d:%s, node=%s, section=%s, key=%s, value=%f, def=%f, raceHandle=0x%08x, IsLoaded=%d", \
                  fileId, short, nodeName, section, key, value, defvalue, raceHandle, \
                  (raceHandle == INVALID_HANDLE) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle));
        #endif
    }
    else if (section[0] == '\0' && StrEqual(key, "mvm_energy_rate"))
        return g_fMvMEnergyRate;
    else if (section[0] == '\0' && StrEqual(key, "energy_rate"))
        return g_fEnergyRate;
    else if (section[0] == '\0' && StrEqual(key, "energy_factor"))
        return g_fEnergyFactor;
    else if (section[0] == '\0' && StrEqual(key, "package_duration"))
        return g_fPackageDuration;
    else if (section[0] == '\0' && StrEqual(key, "crystal_sell_rate"))
        return g_fCrystalSellRate;
    else if (section[0] == '\0' && StrEqual(key, "crystal_buy_rate"))
        return g_fCrystalBuyRate;
    else if (section[0] == '\0' && StrEqual(key, "xp_multiplier"))
        return g_fXPMultiplier;
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        bool sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == ConfigWasLoaded)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = ConfigWasUpdated;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == ConfigNotLoaded || !sectionFound)
            KvSetFloat(g_SourceCraftConfigHandle, key, value=defvalue);
        else
            value = KvGetFloat(g_SourceCraftConfigHandle, key, defvalue);
    }

    TraceReturn("value=%f", value);
    return value;
}

GetConfigString(const String:key[], String:value[], maxlength,
                const String:defvalue[]="", fileId=MAIN,
                node=NONE, const String:section[]="")
{
    TraceInto("Config", "GetConfigString", "file=%d, node=%d, section=%s, key=%s, def=%s", \
              fileId, node, section, key, defvalue);

    if (fileId >= SHOPITEM)
    {
        Handle raceHandle;
        Handle configHandle;
        new ConfigLoadedStatus:configLoaded;
        char nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                Handle itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                bool nodeFound;
                if (configLoaded == ConfigNotLoaded)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        bool updateConfig = false;
                        bool sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            KvGetString(configHandle, key, value, maxlength, defvalue);
                            if (StrEqual(value, defvalue))
                            {
                                new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                    ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                                updateConfig = (loaded != ConfigWasLoaded);
                            }
                        }
                        else
                            updateConfig = true;

                        if (updateConfig)
                        {
                            strcopy(value, maxlength, defvalue);
                            KvSetString(configHandle, key, value);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = ConfigWasUpdated;
                            else
                                SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }

                        strcopy(value, maxlength, defvalue);
                        KvSetString(configHandle, key, value);
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == ConfigNotLoaded)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    strcopy(value, maxlength, defvalue);
                    KvSetString(configHandle, key, value);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = ConfigWasUpdated;
                    else
                        SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                }
                else
                {
                    bool updateConfig = false;
                    bool sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        KvGetString(configHandle, key, value, maxlength, defvalue);
                        if (StrEqual(value, defvalue))
                        {
                            new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                            updateConfig = (loaded != ConfigWasLoaded);
                        }
                    }
                    else
                        updateConfig = true;

                    if (updateConfig)
                    {
                        strcopy(value, maxlength, defvalue);
                        KvSetString(configHandle, key, value);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }
            }
        }

        #if !defined _TRACE
            char short[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle,short,sizeof(short));
            Trace("file=%d:%s, node=%s, section=%s, key=%s, value=%s, def=%s, raceHandle=0x%08x, IsLoaded=%d", \
                  fileId, short, nodeName, section, key, value, defvalue, raceHandle, \
                  (raceHandle == INVALID_HANDLE) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle));
        #endif
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        bool sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == ConfigWasLoaded)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = ConfigWasUpdated;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == ConfigNotLoaded || !sectionFound)
        {
            strcopy(value, maxlength, defvalue);
            KvSetString(g_SourceCraftConfigHandle, key, value);
        }
        else
            KvGetString(g_SourceCraftConfigHandle, key, value, maxlength, defvalue);
    }

    TraceReturn("value=%s", value);
}

GetConfigArray(const String:key[], any:value[], size, const any:defvalue[],
               fileId=MAIN, node=NONE, const String:section[]="")
{
    TraceInto("Config", "GetConfigArray", "file=%d, node=%d, section=%s, key=%s", \
              fileId, node, section, key);

    new count = size;
    char buffer[LONG_STRING_LENGTH];
    char defstring[LONG_STRING_LENGTH];

    // Build the default string
    for (new i = 0; i < size; i++)
    {
        if (i == 0)
            Format(defstring, sizeof(defstring), "%d", defvalue[i]);
        else
            Format(defstring, sizeof(defstring), "%s %d", defstring, defvalue[i]);
    }

    if (fileId >= SHOPITEM)
    {
        Handle raceHandle;
        Handle configHandle;
        new ConfigLoadedStatus:configLoaded;
        char nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                Handle itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                bool nodeFound;
                if (configLoaded == ConfigNotLoaded)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        bool updateConfig = false;
                        bool sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                            count = StringToIntegers(buffer, value, size);

                            if (StrEqual(buffer, defstring))
                            {
                                new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                    ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                                updateConfig = (loaded != ConfigWasLoaded);
                            }
                        }
                        else
                            updateConfig = true;

                        if (updateConfig)
                        {
                            KvSetString(configHandle, key, defstring);
                            for (new i=0; i < size; i++)
                                value[i] = defvalue[i];

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = ConfigWasUpdated;
                            else
                                SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == ConfigNotLoaded)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    KvSetString(configHandle, key, defstring);
                    for (new i=0; i < size; i++)
                        value[i] = defvalue[i];

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = ConfigWasUpdated;
                    else
                        SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                }
                else
                {
                    bool updateConfig = false;
                    bool sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                        count = StringToIntegers(buffer, value, size);

                        if (StrEqual(buffer, defstring))
                        {
                            new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                            updateConfig = (loaded != ConfigWasLoaded);
                        }
                    }
                    else
                        updateConfig = true;

                    if (updateConfig)
                    {
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }
            }
        }

        #if !defined _TRACE
            char short[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle,short,sizeof(short));
            Trace("file=%d:%s, node=%s, section=%s, key=%s, value=%s, def=%s, raceHandle=0x%08x, IsLoaded=%d", \
                  fileId, short, nodeName, section, key, buffer, defstring, raceHandle, \
                  (raceHandle == INVALID_HANDLE) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle));
        #endif
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        bool sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == ConfigWasLoaded)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = ConfigWasUpdated;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == ConfigNotLoaded || !sectionFound)
        {
            KvSetString(g_SourceCraftConfigHandle, key, defstring);
            for (new i=0; i < size; i++)
                value[i] = defvalue[i];
        }
        else
        {
            KvGetString(g_SourceCraftConfigHandle, key, buffer, sizeof(buffer), defstring);
            count = StringToIntegers(buffer, value, size);
        }
    }

    TraceReturn("count=%d, buffer=%s", count, buffer);
    return count;
}

GetConfigFloatArray(const String:key[], Float:value[], size,
                    const Float:defvalue[], fileId=MAIN,
                    node=NONE, const String:section[]="")
{
    TraceInto("Config", "GetConfigFloatArray", "file=%d, node=%d, section=%s, key=%s", \
              fileId, node, section, key);

    new count = size;
    char buffer[LONG_STRING_LENGTH];
    char defstring[LONG_STRING_LENGTH];

    // Build the default string
    for (new i = 0; i < size; i++)
    {
        if (i == 0)
            Format(defstring, sizeof(defstring), "%f", defvalue[i]);
        else
            Format(defstring, sizeof(defstring), "%s %f", defstring, defvalue[i]);
    }

    if (fileId >= SHOPITEM)
    {
        Handle raceHandle;
        Handle configHandle;
        new ConfigLoadedStatus:configLoaded;
        char nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                Handle itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                bool nodeFound;
                if (configLoaded == ConfigNotLoaded)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        bool updateConfig = false;
                        bool sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                            count = StringToFloats(buffer, value, size);

                            if (StrEqual(buffer, defstring))
                            {
                                new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                    ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                                updateConfig = (loaded != ConfigWasLoaded);
                            }
                        }
                        else
                            updateConfig = true;

                        if (updateConfig)
                        {
                            KvSetString(configHandle, key, defstring);
                            for (new i=0; i < size; i++)
                                value[i] = defvalue[i];

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = ConfigWasUpdated;
                            else
                                SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];
                    }
                    else
                    {
                        LogError("Unable to create node %s for %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == ConfigNotLoaded)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                            LogError("Unable to create %s section", section);
                    }

                    KvSetString(configHandle, key, defstring);
                    for (new i=0; i < size; i++)
                        value[i] = defvalue[i];

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = ConfigWasUpdated;
                    else
                        SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                }
                else
                {
                    bool updateConfig = false;
                    bool sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                        count = StringToFloats(buffer, value, size);

                        if (StrEqual(buffer, defstring))
                        {
                            new ConfigLoadedStatus:loaded = (fileId == SHOPITEM)
                                ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle);

                            updateConfig = (loaded != ConfigWasLoaded);
                        }
                    }
                    else
                        updateConfig = true;

                    if (updateConfig)
                    {
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = ConfigWasUpdated;
                        else
                            SetRaceConfigIsLoaded(raceHandle,ConfigWasUpdated);
                    }
                }
            }
        }

        #if !defined _TRACE
            char short[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle,short,sizeof(short));
            Trace("file=%d:%s, node=%s, section=%s, key=%s, value=%s, def=%s, raceHandle=0x%08x, IsLoaded=%d", \
                  fileId, short, nodeName, section, key, buffer, defstring, raceHandle, \
                  (raceHandle == INVALID_HANDLE) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle));
        #endif
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        bool sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == ConfigWasLoaded)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = ConfigWasUpdated;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == ConfigNotLoaded || !sectionFound)
        {
            KvSetString(g_SourceCraftConfigHandle, key, defstring);
            for (new i=0; i < size; i++)
                value[i] = defvalue[i];
        }
        else
        {
            KvGetString(g_SourceCraftConfigHandle, key, buffer, sizeof(buffer), defstring);
            count = StringToFloats(buffer, value, size);
        }
    }

    TraceReturn("count=%d, buffer=%s", count, buffer);
    return count;
}

bool:ParseSettings()
{
    TraceInto("SourceCraft", "ParseSettings");

    LoadSettings();
    if (g_SourceCraftConfigLoaded == ConfigNotLoaded)
    {
        // Set options
        KvSetNum(g_SourceCraftConfigHandle, "update", g_bUpdate);
        KvSetNum(g_SourceCraftConfigHandle, "save_xp", g_bSaveXP);
        KvSetNum(g_SourceCraftConfigHandle, "charge_for_upgrades", g_bChargeForUpgrades);
        KvSetNum(g_SourceCraftConfigHandle, "save_upgrades", g_bSaveUpgrades);

        KvSetNum(g_SourceCraftConfigHandle, "show_upgrade_info", g_bShowUpgradeInfo);
        KvSetNum(g_SourceCraftConfigHandle, "show_disabled_races", g_bShowDisabledRaces);
        KvSetNum(g_SourceCraftConfigHandle, "race_menu_threshold", g_iRaceMenuThreshold);
        KvSetFloat(g_SourceCraftConfigHandle, "max_package_energy", g_fMaxPackageEnergy);
        KvSetFloat(g_SourceCraftConfigHandle, "req_package_energy", g_fReqPackageEnergy);
        KvSetFloat(g_SourceCraftConfigHandle, "min_package_energy", g_fMinPackageEnergy);
        KvSetFloat(g_SourceCraftConfigHandle, "energy_package_amt", g_fEnergyPackageAmt);

        KvSetNum(g_SourceCraftConfigHandle, "min_players", g_iMinPlayers);
        KvSetNum(g_SourceCraftConfigHandle, "min_ultimate", g_iMinUltimate);
        KvSetNum(g_SourceCraftConfigHandle, "max_crystals", g_iMaxCrystals);
        KvSetNum(g_SourceCraftConfigHandle, "max_vespene", g_iMaxVespene);

        KvSetFloat(g_SourceCraftConfigHandle, "energy_rate", g_fEnergyRate);
        KvSetFloat(g_SourceCraftConfigHandle, "energy_factor", g_fEnergyFactor);

        KvSetFloat(g_SourceCraftConfigHandle, "mvm_energy_rate", g_fMvMEnergyRate);
        KvSetFloat(g_SourceCraftConfigHandle, "mvm_energy_factor", g_fMvMEnergyFactor);

        KvSetNum(g_SourceCraftConfigHandle, "upgrade_crystals_cost", g_iUpgradeCrystalsCost);
        KvSetNum(g_SourceCraftConfigHandle, "upgrade_vespene_cost", g_iUpgradeVespeneCost);

        KvSetFloat(g_SourceCraftConfigHandle, "xp_multiplier", g_fXPMultiplier);

        // Package options
        KvSetFloat(g_SourceCraftConfigHandle, "package_duration", g_fPackageDuration);
        KvSetNum(g_SourceCraftConfigHandle, "max_packages", g_iMaxPackages);
        KvSetNum(g_SourceCraftConfigHandle, "max_drop_xp", g_iMaxDropXP);
        KvSetNum(g_SourceCraftConfigHandle, "drop_xp_bias", g_iDropXPBias);
        KvSetNum(g_SourceCraftConfigHandle, "max_drop_money", g_iMaxDropMoney);
        KvSetNum(g_SourceCraftConfigHandle, "drop_money_bias", g_iDropMoneyBias);
        KvSetNum(g_SourceCraftConfigHandle, "max_drop_crystals", g_iMaxDropCrystals);
        KvSetNum(g_SourceCraftConfigHandle, "drop_crystal_bias", g_iDropCrystalBias);
        KvSetNum(g_SourceCraftConfigHandle, "max_drop_pcrystals", g_iMaxDropPCrystals);
        KvSetNum(g_SourceCraftConfigHandle, "drop_pcrystals_bias", g_iDropPCrystalsBias);

        if (GameHasMoney())
        {
            KvSetNum(g_SourceCraftConfigHandle, "use_money", g_bUseMoney);
            KvSetFloat(g_SourceCraftConfigHandle, "crystal_sell_rate", g_fCrystalSellRate);
            KvSetFloat(g_SourceCraftConfigHandle, "crystal_buy_rate", g_fCrystalBuyRate);
        }
        else
        {
            g_bUseMoney=false;
            g_fCrystalSellRate=g_fCrystalBuyRate=1.0;
        }
    }
    else
    {
        // Load options
        g_bUpdate=bool:KvGetNum(g_SourceCraftConfigHandle, "update", g_bUpdate);
        g_bSaveXP=bool:KvGetNum(g_SourceCraftConfigHandle, "save_xp", g_bSaveXP);

        g_bChargeForUpgrades=bool:KvGetNum(g_SourceCraftConfigHandle, "charge_for_upgrades", g_bChargeForUpgrades);
        g_bSaveUpgrades=bool:KvGetNum(g_SourceCraftConfigHandle, "save_upgrades", g_bSaveUpgrades);

        g_bShowUpgradeInfo=bool:KvGetNum(g_SourceCraftConfigHandle, "show_upgrade_info", g_bShowUpgradeInfo);
        g_bShowDisabledRaces=bool:KvGetNum(g_SourceCraftConfigHandle, "show_disabled_races", g_bShowDisabledRaces);
        g_iRaceMenuThreshold=KvGetNum(g_SourceCraftConfigHandle, "race_menu_threshold", g_iRaceMenuThreshold);
        g_fMaxPackageEnergy=KvGetFloat(g_SourceCraftConfigHandle, "max_package_energy", g_fMaxPackageEnergy);
        g_fMinPackageEnergy=KvGetFloat(g_SourceCraftConfigHandle, "min_package_energy", g_fMinPackageEnergy);
        g_fReqPackageEnergy=KvGetFloat(g_SourceCraftConfigHandle, "req_package_energy", g_fReqPackageEnergy);
        g_fEnergyPackageAmt=KvGetFloat(g_SourceCraftConfigHandle, "energy_package_amt", g_fEnergyPackageAmt);

        g_iMinPlayers=KvGetNum(g_SourceCraftConfigHandle, "min_players", g_iMinPlayers);
        g_iMinUltimate=KvGetNum(g_SourceCraftConfigHandle, "min_ultimate", g_iMinUltimate);
        g_iMaxCrystals=KvGetNum(g_SourceCraftConfigHandle, "max_crystals", g_iMaxCrystals);
        g_iMaxVespene=KvGetNum(g_SourceCraftConfigHandle, "max_vespene", g_iMaxVespene);

        g_fEnergyRate=KvGetFloat(g_SourceCraftConfigHandle, "energy_rate", g_fEnergyRate);
        g_fEnergyFactor=KvGetFloat(g_SourceCraftConfigHandle, "energy_factor", g_fEnergyFactor);

        g_fMvMEnergyRate=KvGetFloat(g_SourceCraftConfigHandle, "mvm_energy_rate", g_fMvMEnergyRate);
        g_fMvMEnergyFactor=KvGetFloat(g_SourceCraftConfigHandle, "mvm_energy_factor", g_fMvMEnergyFactor);

        g_iUpgradeCrystalsCost=KvGetNum(g_SourceCraftConfigHandle, "upgrade_crystals_cost", g_iUpgradeCrystalsCost);
        g_iUpgradeVespeneCost=KvGetNum(g_SourceCraftConfigHandle, "upgrade_vespene_cost", g_iUpgradeVespeneCost);

        g_fXPMultiplier=KvGetFloat(g_SourceCraftConfigHandle, "xp_multiplier", g_fXPMultiplier);

        // Package options
        g_fPackageDuration=KvGetFloat(g_SourceCraftConfigHandle, "package_duration", g_fPackageDuration);
        g_iMaxPackages=KvGetNum(g_SourceCraftConfigHandle, "max_packages", g_iMaxPackages);
        g_iMaxDropXP=KvGetNum(g_SourceCraftConfigHandle, "max_drop_xp", g_iMaxDropXP);
        g_iDropXPBias=KvGetNum(g_SourceCraftConfigHandle, "drop_xp_bias", g_iDropXPBias);
        g_iMaxDropMoney=KvGetNum(g_SourceCraftConfigHandle, "max_drop_money", g_iMaxDropMoney);
        g_iDropMoneyBias=KvGetNum(g_SourceCraftConfigHandle, "drop_money_bias", g_iDropMoneyBias);
        g_iMaxDropCrystals=KvGetNum(g_SourceCraftConfigHandle, "max_drop_crystals", g_iMaxDropCrystals);
        g_iDropCrystalBias=KvGetNum(g_SourceCraftConfigHandle, "drop_crystal_bias", g_iDropCrystalBias);
        g_iMaxDropPCrystals=KvGetNum(g_SourceCraftConfigHandle, "max_drop_pcrystals", g_iMaxDropPCrystals);
        g_iDropPCrystalsBias=KvGetNum(g_SourceCraftConfigHandle, "drop_pcrystals_bias", g_iDropPCrystalsBias);

        if (GameHasMoney())
        {
            g_bUseMoney=bool:KvGetNum(g_SourceCraftConfigHandle, "use_money", g_bUseMoney);
            g_fCrystalSellRate=KvGetFloat(g_SourceCraftConfigHandle, "crystal_sell_rate", g_fCrystalSellRate);
            g_fCrystalBuyRate=KvGetFloat(g_SourceCraftConfigHandle, "crystal_buy_rate", g_fCrystalBuyRate);
        }
        else
        {
            g_bUseMoney=false;
            g_fCrystalSellRate=g_fCrystalBuyRate=1.0;
        }
    }

    // Load level configuration
    static const String:defaultLevels[][][] =
    {
        // long_term {"required_xp" , "kill_xp" }
        {
            "0 100 200 400 800 1600 3200 6400 12800 25600 51200 61200 71200 81200 91200 101200 111200",
            "6 8 10 12 14 16 20 24 28 32 40 50 60 70 80 90 100"
        },
        // short_term {"required_xp" , "kill_xp" }
        {
            "0 100 300 600 1000 2500 4000 6000 8000 10000 12000 14000 16000 18000 20000 22000 24000",
            "60 80 100 120 140 160 180 200 220 240 260 280 300 320 340 360 380"
        }
    };

    char temp[512];
    Handle longterm_required=CreateArray();
    Handle longterm_killxp=CreateArray();
    Handle shortterm_required=CreateArray();
    Handle shortterm_killxp=CreateArray();

    SetTraceCategory("Array");
    Trace("CreateArray longterm_required=%x",longterm_required);
    Trace("CreateArray longterm_killxp=%x",longterm_killxp);
    Trace("CreateArray shortterm_required=%x",shortterm_required);
    Trace("CreateArray shortterm_killxp=%x",shortterm_killxp);

    KvRewind(g_SourceCraftConfigHandle);

    bool termFound;
    bool levelsFound = KvJumpToKey(g_SourceCraftConfigHandle,"levels");
    if (levelsFound)
        termFound = KvJumpToKey(g_SourceCraftConfigHandle, "long_term");
    else
        termFound = false;

    if (!termFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (!KvJumpToKey(g_SourceCraftConfigHandle, "long_term", true))
        {
            LogError("Unable to create long_term section in configs/sourcecraft.cfg!");
            CloseHandle(g_SourceCraftConfigHandle);
            g_SourceCraftConfigHandle=INVALID_HANDLE;
            g_SourceCraftConfigLoaded=ConfigNotLoaded;
            TraceReturn("return false");
            return false;
        }
    }

    // required xp, long term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"required_xp",temp,sizeof(temp), defaultLevels[0][0]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[0][0]);
        KvSetString(g_SourceCraftConfigHandle,"required_xp", defaultLevels[0][0]);
    }

    new count = StringToIntegerArray(temp, longterm_required);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for long_term required_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=ConfigNotLoaded;
        TraceReturn("return false");
        return false;
    }

    // kill xp, long term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"kill_xp",temp,sizeof(temp), defaultLevels[0][1]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[0][1]);
        KvSetString(g_SourceCraftConfigHandle,"kill_xp", defaultLevels[0][1]);
    }

    count = StringToIntegerArray(temp, longterm_killxp);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for long_term kill_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=ConfigNotLoaded;
        TraceReturn("return false");
        return false;
    }

    if (levelsFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        termFound = KvJumpToKey(g_SourceCraftConfigHandle, "short_term");
    }
    else
        termFound = false;

    if (!termFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (!KvJumpToKey(g_SourceCraftConfigHandle, "short_term", true))
        {
            LogError("Unable to create short_term section in configs/sourcecraft.cfg!");
            CloseHandle(g_SourceCraftConfigHandle);
            g_SourceCraftConfigHandle=INVALID_HANDLE;
            g_SourceCraftConfigLoaded=ConfigNotLoaded;
            TraceReturn("return false");
            return false;
        }
    }

    // required xp, short term
    if (termFound)
    {
        KvGetString(g_SourceCraftConfigHandle,"required_xp",temp,sizeof(temp), defaultLevels[1][0]);
    }
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[1][0]);
        KvSetString(g_SourceCraftConfigHandle,"required_xp", defaultLevels[1][0]);
    }

    count = StringToIntegerArray(temp, shortterm_required);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for short_term required_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=ConfigNotLoaded;
        TraceReturn("return false");
        return false;
    }

    // kill xp, short term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"kill_xp",temp,sizeof(temp), defaultLevels[1][1]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[1][1]);
        KvSetString(g_SourceCraftConfigHandle,"kill_xp", defaultLevels[1][1]);
    }

    count = StringToIntegerArray(temp, shortterm_killxp);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for short_term kill_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=ConfigNotLoaded;
        TraceReturn("return false");
        return false;
    }

    vecLevelConfiguration=CreateArray();
    PushArrayCell(vecLevelConfiguration,longterm_required);
    PushArrayCell(vecLevelConfiguration,longterm_killxp);
    PushArrayCell(vecLevelConfiguration,shortterm_required);
    PushArrayCell(vecLevelConfiguration,shortterm_killxp);

    TraceCat("Array", "vecLevelConfiguration=%x",vecLevelConfiguration);

    // Load URLs
    bool urlFound;
    if (g_SourceCraftConfigLoaded)
    {
        KvRewind(g_SourceCraftConfigHandle);
        urlFound = KvJumpToKey(g_SourceCraftConfigHandle, "urls", false);
        if (urlFound)
        {
            KvGetString(g_SourceCraftConfigHandle, "info", g_InfoURL, sizeof(g_InfoURL), g_InfoURL);
            KvGetString(g_SourceCraftConfigHandle, "webapp", g_InfoBaseURL, sizeof(g_InfoBaseURL), g_InfoBaseURL);
            KvGetString(g_SourceCraftConfigHandle, "update", g_UpdateURL, sizeof(g_UpdateURL), g_UpdateURL);
            KvGetString(g_SourceCraftConfigHandle, "wiki", g_WikiURL, sizeof(g_WikiURL), g_WikiURL);
            KvGetString(g_SourceCraftConfigHandle, "bug", g_BugURL, sizeof(g_BugURL), g_BugURL);
        }
    }
    else
        urlFound = false;

    if (!urlFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (KvJumpToKey(g_SourceCraftConfigHandle, "urls", true))
        {
            KvSetString(g_SourceCraftConfigHandle, "info", g_InfoURL);
            KvSetString(g_SourceCraftConfigHandle, "webapp", g_InfoBaseURL);
            KvSetString(g_SourceCraftConfigHandle, "update", g_UpdateURL);
            KvSetString(g_SourceCraftConfigHandle, "wiki", g_WikiURL);
            KvSetString(g_SourceCraftConfigHandle, "bug", g_BugURL);
        }
    }

    //Ensure the config/sc directory exists
    char dir[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,dir,sizeof(dir),"configs/sc");
    if (!DirExists(dir))
    {
        if (!CreateDirectory(dir, 493)) // 0755 == "u+rwx,go+rx"
            LogError("Unable to create %s directory!", dir);
    }

    TraceReturn("return true");
    return true;
}

Handle:GetRaceConfig(Handle:raceHandle, &ConfigLoadedStatus:loaded)
{
    TraceInto("Config", "GetRaceConfig", "raceHandle=0x%08x, loaded=%d", raceHandle, loaded);

    Handle configHandle = GetRaceConfigHandle(raceHandle);
    if (configHandle == INVALID_HANDLE)
    {
        char short[SHORT_STRING_LENGTH];
        GetRaceShortName(raceHandle,short,sizeof(short));

        configHandle = LoadRaceConfig(short, loaded);
        SetRaceConfigIsLoaded(raceHandle,loaded);
        SetRaceHasBeenCompleted(raceHandle,false);
    }
    else
        loaded = GetRaceConfigIsLoaded(raceHandle);

    TraceReturn("configHandle=0x%08x, loaded=%d", configHandle, loaded);
    return configHandle;
}

Handle:LoadRaceConfig(const String:short[], &ConfigLoadedStatus:loaded)
{
    TraceInto("Config", "LoadRaceConfig", "short=%s, loaded=%d", short, loaded);

    Handle config = CreateKeyValues(short);

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,path,sizeof(path),"configs/sc/%s.cfg", short);

    loaded = ConfigLoadedStatus:FileToKeyValues(config,path);

    TraceReturn("configHandle=0x%08x, loaded=%d, path=%s", config, loaded, path);
    return config;
}

Handle:LoadItemConfig(&ConfigLoadedStatus:loaded)
{
    if (g_ItemConfigHandle == INVALID_HANDLE)
    {
        g_ItemConfigHandle = CreateKeyValues("shopitems");

        char path[PLATFORM_MAX_PATH];
        BuildPath(Path_SM,path,sizeof(path),"configs/sc/shopitems.cfg");

        g_ItemConfigLoaded = ConfigLoadedStatus:FileToKeyValues(g_ItemConfigHandle,path);
    }

    loaded = g_ItemConfigLoaded;
    return g_ItemConfigHandle;
}

SetItemConfigLoaded(ConfigLoadedStatus:loaded)
{
    g_ItemConfigLoaded = loaded;
}

SaveConfig(Handle:configHandle, const String:name[])
{
    TraceInto("Config", "SaveConfig", "configHandle=0x%08x, name=%s", configHandle, name);

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,path,sizeof(path),"configs/sc/%s.cfg", name);

    KvRewind(configHandle);
    if (!KeyValuesToFile(configHandle, path))
        LogError("Unable to create/update config file %s", path);

    TraceReturn("path=%s", path);        
}

LoadSettings()
{
    if (g_SourceCraftConfigHandle == INVALID_HANDLE)
    {
        char path[PLATFORM_MAX_PATH];
        BuildPath(Path_SM,path,sizeof(path),"configs/sourcecraft.cfg");

        g_SourceCraftConfigHandle=CreateKeyValues("SourceCraft");
        g_SourceCraftConfigLoaded=ConfigLoadedStatus:FileToKeyValues(g_SourceCraftConfigHandle,path);
    }
}

CompleteConfigs()
{
    TraceInto("Config", "CompleteConfigs");

    // Ensure all races are marked complete and saved.
    new size = GetRaceCount();
    for(new race=1;race<size;race++)
    {
        Handle raceHandle=GetRaceHandle(race);
        if (!GetRaceHasBeenCompleted(raceHandle))
            RaceComplete(race);
    }

    // Save and Close the shop item config file
    if (g_ItemConfigHandle != INVALID_HANDLE)
    {
        if (g_ItemConfigLoaded != ConfigWasLoaded)
            SaveConfig(g_ItemConfigHandle, "shopitems");

        CloseHandle(g_ItemConfigHandle);
        g_ItemConfigHandle = INVALID_HANDLE;
        g_ItemConfigLoaded = ConfigNotLoaded;
    }

    // Save and Close the main sourcraft config file
    if (g_SourceCraftConfigHandle != INVALID_HANDLE)
    {
        if (g_SourceCraftConfigLoaded != ConfigWasLoaded)
        {
            char path[PLATFORM_MAX_PATH];
            BuildPath(Path_SM,path,sizeof(path),"configs/sourcecraft.cfg");

            KvRewind(g_SourceCraftConfigHandle);
            if (KeyValuesToFile(g_SourceCraftConfigHandle, path))
                LogMessage("Settings updated in %s", path);
            else                
                LogError("Unable to save settings in %s", path);
        }

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=ConfigNotLoaded;
    }

    TraceReturn();
}


/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: adminmenus.inc
 * Description: This file handles the admin menu.
 * Author(s): Naris (Murray Wilson)
 */

Handle hTopAdminMenu = INVALID_HANDLE;
new TopMenuObject:hAdminMenu = INVALID_TOPMENUOBJECT;

InitAdminMenu()
{
    /* Account for late loading */
    Handle topmenu;
    if (LibraryExists("adminmenu") && ((topmenu = GetAdminTopMenu()) != INVALID_HANDLE))
    {
        OnAdminMenuReady(topmenu);
    }
    return true;
}

public void OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "adminmenu"))
        hTopAdminMenu = INVALID_HANDLE;
}

public void OnAdminMenuReady(Handle:topmenu)
{
    /*************************************************************/
    /* Add Administer SourceCraft option to SourceMod Admin Menu */
    /*************************************************************/

    /* Block us from being called twice */
    if (topmenu != hTopAdminMenu)
    {
        /* Save the Handle */
        hTopAdminMenu = topmenu;

        /* Add the SourceCraft category */
        hAdminMenu = AddToTopMenu(hTopAdminMenu, "sc_admin", TopMenuObject_Category,
                                  AdminCategoryHandler, INVALID_TOPMENUOBJECT);

        if (hAdminMenu != INVALID_TOPMENUOBJECT)
        {
            AddToTopMenu(hTopAdminMenu, "sc_admin_player", TopMenuObject_Item, AdminPlayerMenu,
                         hAdminMenu, "umc_changemap", ADMFLAG_SOURCECRAFT);
        }
    }
}

// Handles the SourceCraft category in the admin menu.
public AdminCategoryHandler(Handle:topmenu, TopMenuAction:action, TopMenuObject:object_id, param,
                            String:buffer[], maxlength)
{
    if (action == TopMenuAction_DisplayTitle || action == TopMenuAction_DisplayOption)
    {
        Format(buffer, maxlength, "%T", "AdministerMenu", param);
    }
}

// Handles the Admin Player item in the SourceCraft menu
public AdminPlayerMenu(Handle:topmenu, TopMenuAction:action, TopMenuObject:object_id, param, String:buffer[], maxlength)
{
    if (action == TopMenuAction_DisplayOption)
    {
        Format(buffer, maxlength, "%T", "AdministerPlayers", param);
    }
    else if (action == TopMenuAction_SelectOption)
    {
        AdminMenu(param,0);
    }
}

public Action AdminMenu(client,args)
{
    Handle adminMenu=CreateMenu(Admin_Selected);
    SetMenuExitButton(adminMenu,true);
    SetMenuTitle(adminMenu,"[SC] %T", "SelectPlayer", client);
    AddTargetsToMenu(adminMenu, client, true, false);
    DisplayMenu(adminMenu,client,MENU_TIME_FOREVER);
    return Plugin_Handled;
}

public Admin_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        new userid, target;
        char SelectionInfo[SMALL_STRING_LENGTH];
        PrepareAndEmitSoundToClient(client,buttonWav);
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        userid=StringToInt(SelectionInfo);
        if ((target = GetClientOfUserId(userid)) == 0)
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        else if (!CanUserTarget(client, target))
            CPrintToChat(client, "{green}[SC]{default} %t", "NoTarget");
        else
            Admin_Player(client,target);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Player(client,target)
{
    Handle adminMenu_Player=CreateMenu(Admin_Player_Select);
    SetMenuExitButton(adminMenu_Player,true);
    SetMenuExitBackButton(adminMenu_Player,true);
    SetMenuTitle(adminMenu_Player,"[SC] %T", "AdministerAPlayer", client, target);

    char buf[SMALL_STRING_LENGTH];
    Format(buf,sizeof(buf),"%d",target);

    int race_style = (GetRaceCount() >= 1) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED;
    int shop_style = (GetShopItemCount() >= 1) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED;
    AddMenuItemT(adminMenu_Player,buf,"ViewInfo", client);
    AddMenuItemT(adminMenu_Player,buf,"Reset", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"SetRace", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"GiveShopItem", client, shop_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeXP", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeLevel", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeCrystals", client);
    AddMenuItemT(adminMenu_Player,buf,"ChangeVespene", client);
    AddMenuItemT(adminMenu_Player,buf,"ChangeEnergy", client);

    if (GetPlayerStatus(target) == PlayerDisabled)
        AddMenuItemT(adminMenu_Player,buf,"EnablePlayer", client);
    else
        AddMenuItemT(adminMenu_Player,buf,"DisablePlayer", client);

    AddMenuItemT(adminMenu_Player,buf,"DeletePlayer", client);
    DisplayMenu(adminMenu_Player,client,MENU_TIME_FOREVER);
}

public Admin_Player_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            // What do they want to do with the player?
            switch(selection)
            {
                case 0: PlayerInfo(client,target, AdminMode|FromAdminMenu);
                case 1: Admin_Reset(client,target);
                case 2: Admin_SetRace(client,target);
                case 3: Admin_GiveShopItem(client,target);
                case 4: Admin_XP(client,target);
                case 5: Admin_Level(client,target);
                case 6: Admin_Crystals(client,target);
                case 7: Admin_Vespene(client,target);
                case 8: Admin_Energy(client,target);
                case 9: Admin_Disable(client,target);
                case 10: Admin_Delete(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel &&
             selection == MenuCancel_ExitBack)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);
        AdminMenu(client,0);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Reset(client,target)
{
    // Reset upgrades
    int race =GetRace(target);
    int upgradeCount =GetUpgradeCount(GetRaceHandle(race));
    for (int y =0;y<upgradeCount;y++)
            SetUpgradeLevel(target,race,y,0);

    if (GetDatabaseLoaded(target) != DataLoading &&
        GetUpgradeLevelCount(target, race) < GetLevel(target,race))
    {
        UpgradeMenu(target, UpgradeMode, false);
    }

    LogAction(client, target, "[SC] %L reset %L's upgrades", client, target);
    CPrintToChat(target, "{green}[SC]{default} %t", "AdminResetPlayer", client);
    CPrintToChat(client, "{green}[SC]{default} %t", "AdminResetConfirm", target);

    Admin_Player(client,target);
}

Admin_Return(Handle:menu,client)
{
    PrepareAndEmitSoundToClient(client,buttonWav);

    char SelectionInfo[SMALL_STRING_LENGTH];
    GetMenuItem(menu,1,SelectionInfo,sizeof(SelectionInfo));

    int target = StringToInt(SelectionInfo);
    if (IsValidClient(target))
        Admin_Player(client,target);
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

Admin_XP(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_XP_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);
        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give100XP",client);
        AddMenuItemT(menu,buf,"Give1000XP",client);
        AddMenuItemT(menu,buf,"Give10000XP",client);
        AddMenuItemT(menu,buf,"Remove 100 XP",client);
        AddMenuItemT(menu,buf,"Remove 1000 XP",client);
        AddMenuItemT(menu,buf,"Remove 10000 XP",client);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_XP_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            int race =GetRace(target);
            if (selection < 3) // Give XP
            {
                int xpadd;
                switch (selection)
                {
                    case 0: xpadd=100;
                    case 1: xpadd=1000;
                    case 2: xpadd=10000;
                }
                int newxp =GetXP(target,race)+xpadd;
                SetXP(target,race,newxp);

                LogAction(client, target, "[SC] %L gave %L %d XP", client, target, xpadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveXP", target, xpadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedXP", client, xpadd);

                DoLevelCheck(target);
            }
            else
            {
                int xprem;
                switch (selection)
                {
                    case 3: xprem=100;
                    case 4: xprem=1000;
                    case 5: xprem=10000;
                }
                int newxp =GetXP(target,race)-xprem;
                if (newxp<0)
                    newxp=0;

                SetXP(target,race,newxp);

                LogAction(client, target, "[SC] %L removed %d XP from %L", client, xprem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedXP", xprem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedXP", client, xprem);
            }
            Admin_XP(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_GiveShopItem(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Category_Selected);
        SetMenuTitle(menu,"[SC] %T", "SelectItem", client, target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        char lastCategory[NAME_STRING_LENGTH];
        lastCategory[0] = '\0';

        int count = 0;
        int shopItemCount = GetShopIndexCount();
        for (int x =0;x<shopItemCount;x++)
        {
            Handle itemHandle=GetItemIndexHandle(x);
            char itemCategory[NAME_STRING_LENGTH];
            GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory),client);

            if (strcmp(itemCategory, lastCategory, false) != 0)
            {
                if (itemCategory[0] == '\0')
                {
                    Format(itemCategory,sizeof(itemCategory),
                           "%T", "MiscellaneousItems", client);
                }

                char itembuf[SMALL_STRING_LENGTH];
                Format(itembuf,sizeof(itembuf),"%d,%d",target,x);
                AddMenuItem(menu,itembuf,itemCategory);
                strcopy(lastCategory, sizeof(lastCategory), itemCategory);
                count++;
            }
        }

        if (count > 0)
            DisplayMenu(menu,client,MENU_TIME_FOREVER);
        else
        {
            CancelMenu(menu);
            CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
            Admin_Return(menu,client);
        }
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
    }
}

public Admin_Category_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        if (IsValidClient(client))
        {
            PrepareAndEmitSoundToClient(client,buttonWav);

            char SelectionInfo[SMALL_STRING_LENGTH];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            char SelectionData[2][5];
            ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
            int target = StringToInt(SelectionData[0]);
            int index = StringToInt(SelectionData[1]);

            int shopItemCount = GetShopIndexCount();
            if (index >= 0 && index < shopItemCount)
            {
                char theCategory[NAME_STRING_LENGTH];
                char itemCategory[NAME_STRING_LENGTH];

                Handle itemMenu = CreateMenu(Admin_Item_Selected);
                SetMenuTitle(itemMenu,"[SC] %T", "SelectItem", client, target);
                SetMenuExitBackButton(itemMenu,true);
                SetMenuExitButton(itemMenu,true);

                GetItemCategory(GetItemIndexHandle(index), theCategory, sizeof(theCategory), client);

                int count = 0;
                for (int x =index;x<shopItemCount;x++)
                {
                    Handle itemHandle=GetItemIndexHandle(x);
                    GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory),client);
                    if (strcmp(itemCategory,theCategory,false) != 0)
                        break;
                    else
                    {
                        char itembuf[SMALL_STRING_LENGTH];
                        Format(itembuf,sizeof(itembuf),"%d,%d",
                               target,GetItemId(itemHandle));

                        char itemName[NAME_STRING_LENGTH];
                        GetItemName(itemHandle, itemName, sizeof(itemName),client);
                        AddMenuItem(itemMenu,itembuf,itemName);
                        count++;
                    }
                }

                if (count > 0)
                    DisplayMenu(itemMenu,client,MENU_TIME_FOREVER);
                else
                {
                    CancelMenu(itemMenu);
                    CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                    Admin_Return(menu,client);
                }
            }
            else
            {
                CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                Admin_Return(menu,client);
            }
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public Admin_Item_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        char SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
        int target = StringToInt(SelectionData[0]);
        if (IsValidClient(target))
        {
            int item =StringToInt(SelectionData[1]);
            Handle itemHandle=GetShopItemHandle(item);
            
            char itemName[NAME_STRING_LENGTH];
            GetItemName(itemHandle, itemName, sizeof(itemName),client);

            int num = GetOwnsItem(target,item);
            int max = GetItemMax(itemHandle);
            if (num < max || max < 0)
            {
                SetOwnsItem(target,item,num+1);

                int result;
                Call_StartForward(g_OnItemPurchaseHandle);
                Call_PushCell(target);
                Call_PushCell(item);
                Call_Finish(result);

                LogAction(client, target, "[SC] %L gave %L a %s", client, target, itemName);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveItem", target, itemName);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedItem", itemName, client);
            }
            else
            {
                if (num > 1)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerOwnsMax", num, itemName);
                }
                else if (num == 1)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerOwnsItem", itemName);
                }
                else // if (num == 0)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "ItemNotAvailable", itemName);
                }
            }
            Admin_Player(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            PrepareAndEmitSoundToClient(client,buttonWav);

            char SelectionInfo2[SMALL_STRING_LENGTH];
            GetMenuItem(menu,1,SelectionInfo2,sizeof(SelectionInfo2));
            Admin_GiveShopItem(client,StringToInt(SelectionInfo2));
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_SetRace(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Faction_Selected);
        SetMenuTitle(menu,"[SC] %T", "SelectRace", client, "", target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        int indexCount = GetRaceIndexCount();
        new Faction:lastFaction = Unassigned;
        for(int index =1; index < indexCount; index++)
        {
            Handle raceHandle = GetRaceIndexHandle(index);
            new Faction:faction   = GetRaceFaction(raceHandle);
            if (faction != lastFaction)
            {
                lastFaction = faction;
                if (index < indexCount)
                {
                    Handle nextHandle   = GetRaceIndexHandle(index+1);
                    new Faction:nextFaction = GetRaceFaction(nextHandle);
                    if (nextFaction != faction)
                    {
                        char item[ITEM_STRING_LENGTH];
                        GetRaceMenuItem(item, sizeof(item), client, raceHandle, 0, false);
                        if (item[0] != 0)
                        {
                            char rbuf[SMALL_STRING_LENGTH];
                            Format(rbuf,sizeof(rbuf),"%d,%d",target,GetRaceId(raceHandle));
                            AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
                            continue;
                        }
                    }
                }

                char name[NAME_STRING_LENGTH];
                GetFactionName(faction,name,sizeof(name), client);
                if (name[0] == '\0')
                    Format(name, sizeof(name), "%T", "Miscellaneous", client);

                char item[ITEM_STRING_LENGTH];
                Format(item, sizeof(item), "%T", "FactionItem", client, name);

                char rbuf[SMALL_STRING_LENGTH];
                Format(rbuf,sizeof(rbuf),"%d,%d",target,-index);
                AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
            }
        }
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Faction_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        char SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
        int target = StringToInt(SelectionData[0]);
        int index = StringToInt(SelectionData[1]);
        if (index < 0)
            AdminFactionMenu(client, target, -index);
        else            
            Admin_SetRace_Select(menu,MenuAction:action,client,selection);
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public Admin_SetRace_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        char SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);

        int target = StringToInt(SelectionData[0]);
        if (IsValidClient(target))
        {
            int race =StringToInt(SelectionData[1]);
            SetRace(target, race, false);
            char racename[NAME_STRING_LENGTH];
            GetRaceName(GetRaceHandle(race),racename,sizeof(racename),client);

            LogAction(client, target, "[SC] %L set %L's race to %s", client, target, racename);
            CPrintToChat(client, "{green}[SC]{default} %t", "AdminSetRace", target, racename);
            CPrintToChat(target, "{green}[SC]{default} %t", "PlayersRaceWasSet", client, racename);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            PrepareAndEmitSoundToClient(client,buttonWav);

            char SelectionInfo2[SMALL_STRING_LENGTH];
            GetMenuItem(menu,1,SelectionInfo2,sizeof(SelectionInfo2));
            Admin_SetRace(client,StringToInt(SelectionInfo2));
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

AdminFactionMenu(client, target, start)
{
    int indexCount = GetRaceIndexCount();
    if (start > 0 && start < indexCount)
    {
        Handle menu      = CreateMenu(Admin_SetRace_Select);
        new Faction:theFaction = GetRaceFaction(GetRaceIndexHandle(start));

        char factionName[NAME_STRING_LENGTH];
        GetFactionName(theFaction,factionName,sizeof(factionName), client);

        SetMenuTitle(menu,"[SC] %T", "SelectRace", client, factionName, target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        for(int index = start; index < indexCount; index++)
        {
            Handle raceHandle=GetRaceIndexHandle(index);
            if (GetRaceFaction(raceHandle) != theFaction)
                break;
            else
            {
                char item[ITEM_STRING_LENGTH];
                GetRaceMenuItem(item, sizeof(item), client, raceHandle, 0, false);
                if (item[0] != 0)
                {
                    char rbuf[SMALL_STRING_LENGTH];
                    Format(rbuf,sizeof(rbuf),"%d,%d",target,GetRaceId(raceHandle));
                    AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
                }
            }
        }
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
}

Admin_Level(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Level_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"GiveLevel",client);
        AddMenuItemT(menu,buf,"RemoveLevel",client);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Level_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            int race =GetRace(target);
            if (selection==0)
            {
                // Give a level
                int newlevel = GetLevel(target,race)+1;
                if (newlevel > GetRaceMaxLevel(GetRaceHandle(race)))
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerAtMaxLevel", target);
                }
                else
                {
                    LogAction(client, target, "[SC] %L gave %L a level", client, target);
                    CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveLevel", target);
                    CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedLevel", client);

                    ResetLevel(target,race,newlevel);
                }
            }
            else
            {
                // Remove a level
                int newlevel = GetLevel(target,race)-1;
                if (newlevel < 0)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerHasNoLevels", target);
                }
                else
                {
                    LogAction(client, target, "[SC] %L removed a level from %L", client, target);
                    CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedLevel", target);
                    CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedLevel", client);

                    ResetLevel(target,race,newlevel);
                }
            }
            Admin_Level(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Crystals(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Crystals_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Crystal",client);
        AddMenuItemT(menu,buf,"Give5Crystals",client);
        AddMenuItemT(menu,buf,"Give10Crystals",client);
        AddMenuItemT(menu,buf,"Give25Crystals",client);
        AddMenuItemT(menu,buf,"Remove1Crystal",client);
        AddMenuItemT(menu,buf,"Remove5Crystals",client);
        AddMenuItemT(menu,buf,"Remove10Crystals",client);
        AddMenuItemT(menu,buf,"Remove25Crystals",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Crystals_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 4) // Give crystals
            {
                int credadd;
                switch (selection)
                {
                    case 0: credadd=1;
                    case 1: credadd=5;
                    case 2: credadd=10;
                    case 3: credadd=25;
                }
                int newcred = GetCrystals(target)+credadd;
                if (newcred > g_iMaxCrystals && g_iMaxCrystals > 0)
                    newcred = g_iMaxCrystals;
                SetCrystals(target,newcred);

                LogAction(client, target, "[SC] %L gave %L %d crystals", client, target, credadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveCrystals", target, credadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedCrystals", credadd, client);

                Admin_Crystals(client,target);
            }
            else
            {
                int credrem;
                switch (selection)
                {
                    case 4: credrem=1;
                    case 5: credrem=5;
                    case 6: credrem=10;
                    case 7: credrem=25;
                }
                int newcred = GetCrystals(target)-credrem;
                if (newcred < 0)
                    newcred = 0;
                SetCrystals(target,newcred);

                LogAction(client, target, "[SC] %L removed %d crystals from %L", client, credrem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedCrystals", credrem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedCrystals", client, credrem);

                Admin_Crystals(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Vespene(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Vespene_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Vespene",client);
        AddMenuItemT(menu,buf,"Give5Vespene",client);
        AddMenuItemT(menu,buf,"Give10Vespene",client);
        AddMenuItemT(menu,buf,"Give50Vespene",client);
        AddMenuItemT(menu,buf,"Give100Vespene",client);
        AddMenuItemT(menu,buf,"Remove1Vespene",client);
        AddMenuItemT(menu,buf,"Remove5Vespene",client);
        AddMenuItemT(menu,buf,"Remove50Vespene",client);
        AddMenuItemT(menu,buf,"Remove100Vespene",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Vespene_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 5) // Give vespene
            {
                int gasadd;
                switch(selection)
                {
                    case 0: gasadd=1;
                    case 1: gasadd=5;
                    case 2: gasadd=10;
                    case 3: gasadd=50;
                    case 4: gasadd=100;
                }
                int newgas =GetVespene(target)+gasadd;
                if (newgas > g_iMaxVespene && g_iMaxVespene > 0)
                    newgas = g_iMaxVespene;
                SetVespene(target,newgas);

                LogAction(client, target, "[SC] %L gave %L %d vespene", client, target, gasadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveVespene", target, gasadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedVespene", gasadd, client);

                Admin_Vespene(client,target);
            }
            else
            {
                int gasrem;
                switch(selection)
                {
                    case 5: gasrem=1;
                    case 6: gasrem=5;
                    case 7: gasrem=10;
                    case 8: gasrem=50;
                    case 9: gasrem=100;
                }
                int newgas = GetVespene(target)-gasrem;
                if (newgas < 0)
                    newgas = 0;
                SetVespene(target,newgas);

                LogAction(client, target, "[SC] %L removed %d vespene from %L", client, gasrem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedVespene", gasrem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedVespene", client, gasrem);

                Admin_Vespene(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Energy(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Energy_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Energy",client);
        AddMenuItemT(menu,buf,"Give5Energy",client);
        AddMenuItemT(menu,buf,"Give10Energy",client);
        AddMenuItemT(menu,buf,"Give50Energy",client);
        AddMenuItemT(menu,buf,"Give100Energy",client);
        AddMenuItemT(menu,buf,"Remove1Energy",client);
        AddMenuItemT(menu,buf,"Remove5Energy",client);
        AddMenuItemT(menu,buf,"Remove10Energy",client);
        AddMenuItemT(menu,buf,"Remove50Energy",client);
        AddMenuItemT(menu,buf,"Remove100Energy",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Energy_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 5) // Give energy
            {
                float add;
                switch(selection)
                {
                    case 0: add=1.0;
                    case 1: add=5.0;
                    case 2: add=10.0;
                    case 3: add=50.0;
                    case 4: add=100.0;
                }

                IncrementEnergy(target,add);

                LogAction(client, target, "[SC] %L gave %L %d energy", client, target, add);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveEnergy", target, add);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedEnergy", add, client);

                Admin_Energy(client,target);
            }
            else
            {
                float rem;
                switch(selection)
                {
                    case 5: rem=1.0;
                    case 6: rem=5.0;
                    case 7: rem=10.0;
                    case 8: rem=50.0;
                    case 9: rem=100.0;
                }
                float energy = GetEnergy(target)-rem;
                if (energy < 0.0)
                    energy = 0.0;
                SetEnergy(target,energy);

                LogAction(client, target, "[SC] %L removed %d energy from %L", client, rem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedEnergy", rem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedEnergy", client, rem);

                Admin_Energy(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Disable(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Disable_Confirm);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        if (GetPlayerStatus(target) == PlayerDisabled)
        {
            SetMenuTitle(menu, "[SC] %T", "ConfirmEnable", client, target);
            AddMenuItemT(menu, buf, "EnablePlayer", client);
        }
        else
        {
            SetMenuTitle(menu, "[SC] %T", "ConfirmDisable", client, target);
            AddMenuItemT(menu, buf, "DisablePlayer", client);
        }

        AddMenuItemT(menu, buf, "No", client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Disable_Confirm(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            switch (selection)
            {
                case 0: // Yes
                {
                    if (GetPlayerStatus(target) == PlayerDisabled)
                    {
                        EnablePlayer(client, target);
                        ResetPlayer(target, false);
                        LoadPlayerData(target); // Reload levels

                        LogAction(client, target, "[SC] %L re-enabled %L", client, target);
                        CPrintToChat(client, "{green}[SC]{default} %t", "AdminEnabled", target);
                        CPrintToChat(target, "{green}[SC]{default} %t", "PlayerEnabled", client);
                    }
                    else
                    {
                        DisablePlayer(client, target);
                        ResetPlayer(target, false); // Reset target

                        int race = FindRace("human"); // Make target a human
                        SetRace(target, (race >= 0) ? race : 0, false);

                        LogAction(client, target, "[SC] %L disabled %L", client, target);
                        CPrintToChat(client, "{green}[SC]{default} %t", "AdminDisabled", target);
                        CPrintToChat(target, "{green}[SC]{default} %t", "PlayerDisabled", client);
                    }

                    Admin_Return(menu,client);
                }
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Delete(client,target)
{
    if (IsValidClient(target))
    {
        Handle menu=CreateMenu(Admin_Delete_Confirm);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu,"[SC] %T", "ConfirmDelete", client, target);

        char buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu, buf, "DeletePlayer", client);
        AddMenuItemT(menu, buf, "No", client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Delete_Confirm(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareAndEmitSoundToClient(client,buttonWav);

        char SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        int target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            switch (selection)
            {
                case 0: // Yes
                {
                    DeletePlayer(client, target);
                    ResetPlayer(target, false); // Reset target

                    int race = FindRace("human"); // Make target a human
                    SetRace(target, (race >= 0) ? race : 0, false);

                    LogAction(client, target, "[SC] %L deleted %L from the database", client, target);
                    CPrintToChat(client, "{green}[SC]{default}", "AdminDeleted", target);
                    CPrintToChat(target, "{green}[SC]{default}", "PlayerDeleted", client);

                    Admin_Return(menu,client);
                }
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

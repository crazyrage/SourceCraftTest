/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: menumode.inc
 * Description: Define the MenuMode enum for menu and info displays.
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _menumode_included
 #endinput
#endif
#define _menumode_included

enum MenuMode
{
    NoMode = 0,
    InfoMode = 1,
    AdminMode = 2,
    QueryMode = 4,
    UpgradeMode = 8,
    ChangeRaceMode = 16,
    PurchaseItemMode = 32,
    FromAdminMenu = 64,
    FromMainMenu = 128,
    FromShopMenu = 256,
    FromRaceMenu = 512,
    FromFactionMenu = 1024,
    FromPlayerInfoMenu = 2048,
    FromPlayerInfo = 4096,
    FromUpgradeMenu = 8192
};

#define EnableMenuExitBack (FromAdminMenu|FromMainMenu|FromShopMenu|FromRaceMenu|FromFactionMenu|FromPlayerInfoMenu|FromPlayerInfo)

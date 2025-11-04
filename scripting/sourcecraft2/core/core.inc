#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>

// Simple in-plugin storage for races. Later this can be replaced
// with richer data structures and natives exposure via CreateNative.

static ArrayList g_Races;
static ArrayList g_RaceNames;
static ArrayList g_RaceLongNames;
static ArrayList g_RaceMaxLevels;
static ArrayList g_RaceImages;
static ArrayList g_RaceTranslations;
static ArrayList g_RaceUpgrades;

// Forward handles for lifecycle events
static Handle g_OnRaceRegisteredForward = null;
static Handle g_OnUpgradeRegisteredForward = null;

public void SC2_InitCore()
{
    if (g_Races == null)
    {
        g_Races = new ArrayList();
        g_RaceNames = new ArrayList(ByteCountToCells(64));
        g_RaceLongNames = new ArrayList(ByteCountToCells(128));
        g_RaceMaxLevels = new ArrayList();
        g_RaceImages = new ArrayList(ByteCountToCells(64));
        g_RaceTranslations = new ArrayList(ByteCountToCells(64));
        g_RaceUpgrades = new ArrayList();

        // Create forwards for lifecycle events
        g_OnRaceRegisteredForward = CreateGlobalForward("SC2_OnRaceRegistered", ET_Ignore, Param_Cell, Param_String);
        g_OnUpgradeRegisteredForward = CreateGlobalForward("SC2_OnUpgradeRegistered", ET_Ignore, Param_Cell, Param_String);
    }
}

// Internal helpers
static int FindRaceIndexByShort(const char[] shortName)
{
    if (g_RaceNames == null) return -1;
    char buffer[64];
    for (int i = 0; i < g_RaceNames.Length; i++)
    {
        g_RaceNames.GetString(i, buffer, sizeof(buffer));
        if (StrEqual(buffer, shortName, false))
            return i;
    }
    return -1;
}

// Implementations used by the core plugin itself; later we will
// expose these via CreateNative so external modules can call them.
public SC2Race SC2_RegisterRace(const char[] shortName, const char[] longName, int maxLevel, const char[] image, const char[] tfile)
{
    if (g_Races == null) SC2_InitCore();

    int existing = FindRaceIndexByShort(shortName);
    if (existing != -1)
        return view_as<SC2Race>(existing);

    int id = g_RaceNames.PushString(shortName);
    g_RaceLongNames.PushString(longName);
    g_RaceMaxLevels.Push(maxLevel);
    g_RaceImages.PushString(image);
    g_RaceTranslations.PushString(tfile);
    g_RaceUpgrades.Push(new ArrayList());

    if (tfile[0])
    {
        // Best-effort translation load; failure is non-fatal
        LoadTranslations(tfile);
    }

    // Call forward
    Call_StartForward(g_OnRaceRegisteredForward);
    Call_PushCell(view_as<int>(id));
    Call_PushString(shortName);
    Call_Finish();

    return view_as<SC2Race>(id);
}

public int SC2_RegisterUpgrade(SC2Race race, const char[] key, SC2UpgradeCategory category, int reqLevel, int maxLevel, float energyCost, float recurringEnergy, float recurringInterval, bool accumulated, const char[] bind, const char[] invoke, const char[] image)
{
    int idxRace = view_as<int>(race);
    if (g_RaceUpgrades == null || idxRace < 0 || idxRace >= g_RaceUpgrades.Length)
        return -1;

    ArrayList raceUpgrades = g_RaceUpgrades.Get(idxRace);
    // Store a tiny placeholder record for now
    int idx = raceUpgrades.Push(0);

    // Call forward
    Call_StartForward(g_OnUpgradeRegisteredForward);
    Call_PushCell(view_as<int>(race));
    Call_PushString(key);
    Call_Finish();

    return idx;
}

// Stubs referenced by SourceCraft2.sp
public void SC2_OpenMainMenu(int client)
{
    PrintToChat(client, "[SC2] Menu coming soon.");
}

public void SC2_ApplyRacePassives(int client)
{
    // No-op stub for now
}

public void SC2_HandleDeath(int victim, int attacker)
{
    // No-op stub for now
}

// Simple stub implementations for public API queries so the example
// compiles and other plugins can link at runtime later.
public int SC2_GetLevel(int client) { return 0; }
public int SC2_GetXP(int client) { return 0; }
public int SC2_GetCrystals(int client) { return 0; }
public int SC2_GetVespene(int client) { return 0; }
public bool SC2_HasUpgrade(int client, SC2Race race, const char[] key, int levelMin) { return false; }
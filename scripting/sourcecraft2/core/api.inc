#pragma semicolon 1
#pragma newdecls required

// Public API for race/shop modules to register with SourceCraft2 core.

enum SC2Race { SC2Race_Invalid = -1 };

enum SC2UpgradeCategory
{
    SC2_Active = 0,
    SC2_Passive = 1,
    SC2_Toggle = 2
};

// Register a race with the core. Returns a race handle (index).
native SC2Race SC2_RegisterRace(
    const char[] shortName,
    const char[] longName,
    int maxLevel,
    const char[] image,
    const char[] translationFile // e.g., "sc.protoss.probe.phrases.txt"
);

// Register an upgrade for a race. Returns upgrade index or -1 on error.
native int SC2_RegisterUpgrade(
    SC2Race race,
    const char[] key,
    SC2UpgradeCategory category,
    int requiredLevel,
    int maxLevel,
    float energyCost,
    float recurringEnergy,
    float recurringInterval,
    bool accumulated,
    const char[] bind,
    const char[] invoke,
    const char[] image
);

// Lifecycle hooks (optional for listeners)
forward void SC2_OnRaceRegistered(SC2Race race, const char[] shortName);
forward void SC2_OnUpgradeRegistered(SC2Race race, const char[] key);

// Player query helpers (stubs for now)
native int SC2_GetLevel(int client);
native int SC2_GetXP(int client);
native int SC2_GetCrystals(int client);
native int SC2_GetVespene(int client);
native bool SC2_HasUpgrade(int client, SC2Race race, const char[] key, int levelMin);
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include "sourcecraft2"
#include "core/api"

static SC2Race g_Race = SC2Race_Invalid;

public void OnAllPluginsLoaded()
{
    // Demonstration of how a race module will register itself.
    // Keeping filenames close to legacy lets us reuse translations later.
    g_Race = SC2_RegisterRace(
        "protoss_probe",
        "Protoss Probe",
        10,
        "protoss_probe.png",
        "sc.protoss.probe.phrases.txt"
    );

    SC2_RegisterUpgrade(g_Race, "warp_in", SC2_Active, 1, 3, 25.0, 0.0, 0.0, false, "E", "%t", "warp_in.png");
    SC2_RegisterUpgrade(g_Race, "shields", SC2_Passive, 1, 5, 0.0, 0.0, 0.0, true, "", "%t", "shields.png");
}
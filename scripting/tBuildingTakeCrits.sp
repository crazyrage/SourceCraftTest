#pragma semicolon 1
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#define VERSION 		"0.0.1"

public Plugin myinfo =
{
	name = "tBuildingTakeCrits",
	author = "Thrawn",
	description = "Critical hits do critical damage against buildings. Uses SDKHooks.",
	version = VERSION,
};

#define SOUND_CRIT		"crit_hit5.wav"

Handle g_hCvarShowParticle = INVALID_HANDLE;
Handle g_hCvarPlayCritHitSound = INVALID_HANDLE;
Handle g_hCvarModifierSentries = INVALID_HANDLE;
Handle g_hCvarModifierTeleporters = INVALID_HANDLE;
Handle g_hCvarModifierDispenser = INVALID_HANDLE;

bool g_bShowParticle;
bool g_bPlayCritHitSound;
float g_fSentryModifier;
float g_fTeleportersModifier;
float g_fDispenserModifier;

public OnPluginStart() {
	CreateConVar("sm_tbuildingtakecrits_version", VERSION, "[TF2] tBuildingsTakeCrits", FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY|FCVAR_DONTRECORD);

	g_hCvarShowParticle = CreateConVar("sm_tbuildingtakecrits_showparticle", "1", "Show particle crit text.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_hCvarPlayCritHitSound = CreateConVar("sm_tbuildingtakecrits_playsound", "1", "Play crit-hit sounds.", FCVAR_NONE, true, 0.0, true, 1.0);

	g_hCvarModifierSentries = CreateConVar("sm_tbuildingtakecrits_modifier_sentry", "1.3", "Modifier for critical hits against sentries", FCVAR_NONE, true, 0.0);
	g_hCvarModifierTeleporters = CreateConVar("sm_tbuildingtakecrits_modifier_teleporter", "1.5", "Modifier for critical hits against teleporters", FCVAR_NONE, true, 0.0);
	g_hCvarModifierDispenser = CreateConVar("sm_tbuildingtakecrits_modifier_dispenser", "1.5", "Modifier for critical hits against dispenser", FCVAR_NONE, true, 0.0);

	HookConVarChange(g_hCvarPlayCritHitSound, Cvar_Changed);
	HookConVarChange(g_hCvarModifierSentries, Cvar_Changed);
	HookConVarChange(g_hCvarModifierTeleporters, Cvar_Changed);
	HookConVarChange(g_hCvarModifierDispenser, Cvar_Changed);
	HookConVarChange(g_hCvarShowParticle, Cvar_Changed);

	HookEvent("player_builtobject", Event_PlayerBuiltObject);

	AutoExecConfig(true, "plugin.tBuildingTakeCrits");
}

public OnMapStart() {
	PrecacheSound(SOUND_CRIT, true);
}

public OnConfigsExecuted() {
	g_bShowParticle = GetConVarBool(g_hCvarShowParticle);
	g_bPlayCritHitSound = GetConVarBool(g_hCvarPlayCritHitSound);

	g_fSentryModifier = GetConVarFloat(g_hCvarModifierSentries);
	g_fTeleportersModifier = GetConVarFloat(g_hCvarModifierTeleporters);
	g_fDispenserModifier = GetConVarFloat(g_hCvarModifierDispenser);
}

public Cvar_Changed(Handle:convar, const char oldValue[], const char newValue[]) {
	OnConfigsExecuted();
}

public Action Event_PlayerBuiltObject(Handle:event, const char name[], bool:dontBroadcast) {
	new iObject = GetEventInt(event, "index");

	SDKHook(iObject, SDKHook_OnTakeDamage, OnTakeDamage);
}

public Action OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype) {
	if(attacker > 0 && attacker <= MaxClients && (damagetype & DMG_ACID)) {
		char sNetclass[32];
		GetEntityNetClass(victim, sNetclass, sizeof(sNetclass));

		float fDamageModifier = 1.0;
		if(StrEqual(sNetclass, "CObjectTeleporter"))fDamageModifier = g_fTeleportersModifier;
		else if(StrEqual(sNetclass, "CObjectSentrygun"))fDamageModifier = g_fSentryModifier;
		else if(StrEqual(sNetclass, "CObjectDispenser"))fDamageModifier = g_fDispenserModifier;

		if(fDamageModifier != 1.0) {
			damage *= fDamageModifier;

			if(IsClientInGame(attacker) && !IsFakeClient(attacker) && fDamageModifier > 1.0) {
				if(g_bPlayCritHitSound) {
					EmitSoundToClient(attacker, SOUND_CRIT);
				}

				if(g_bShowParticle) {
					float pos[3];
					GetEntPropVector(victim,Prop_Send,"m_vecOrigin",pos);
					pos[2] += 64.0;
					TE_ParticleToClient(attacker, "crit_text", pos);
				}
			}

			return Plugin_Changed;
		}
	}

	return Plugin_Continue;
}

TE_ParticleToClient(client,
			String:Name[],
            Float:origin[3]=NULL_VECTOR,
            Float:start[3]=NULL_VECTOR,
            Float:angles[3]=NULL_VECTOR,
            entindex=-1,
            attachtype=-1,
            attachpoint=-1,
            bool:resetParticles=true,
            Float:delay=0.0)
{
    // find string table
    new tblidx = FindStringTable("ParticleEffectNames");
    if (tblidx==INVALID_STRING_TABLE)
    {
        LogError("Could not find string table: ParticleEffectNames");
        return;
    }

    // find particle index
    char tmp[256];
    new count = GetStringTableNumStrings(tblidx);
    new stridx = INVALID_STRING_INDEX;
    new i;
    for (i=0; i<count; i++)
    {
        ReadStringTable(tblidx, i, tmp, sizeof(tmp));
        if (StrEqual(tmp, Name, false))
        {
            stridx = i;
            break;
        }
    }
    if (stridx==INVALID_STRING_INDEX)
    {
        LogError("Could not find particle: %s", Name);
        return;
    }

    TE_Start("TFParticleEffect");
    TE_WriteFloat("m_vecOrigin[0]", origin[0]);
    TE_WriteFloat("m_vecOrigin[1]", origin[1]);
    TE_WriteFloat("m_vecOrigin[2]", origin[2]);
    TE_WriteFloat("m_vecStart[0]", start[0]);
    TE_WriteFloat("m_vecStart[1]", start[1]);
    TE_WriteFloat("m_vecStart[2]", start[2]);
    TE_WriteVector("m_vecAngles", angles);
    TE_WriteNum("m_iParticleSystemIndex", stridx);
    if (entindex!=-1)
    {
        TE_WriteNum("entindex", entindex);
    }
    if (attachtype!=-1)
    {
        TE_WriteNum("m_iAttachType", attachtype);
    }
    if (attachpoint!=-1)
    {
        TE_WriteNum("m_iAttachmentPointIndex", attachpoint);
    }
    TE_WriteNum("m_bResetParticles", resetParticles ? 1 : 0);
    TE_SendToClient(client, delay);
}
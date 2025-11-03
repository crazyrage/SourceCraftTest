#include <sourcemod>
#include <sdkhooks>

#define PLUGIN_VERSION 	"1.8A"

Handle g_enabled = INVALID_HANDLE;
Handle g_info = INVALID_HANDLE;
Handle g_multiplier = INVALID_HANDLE;
Handle g_playermultiplier = INVALID_HANDLE;

float Damage[MAXPLAYERS+1];

Handle trieWeapons;
Handle trieAmount;

public Plugin myinfo =
{
	name = "SM Damage",
	author = "SWAT_88, sdkhooks port by AtomicStryker",
	description = "Individual damage settings for each Weapon.",
	version = PLUGIN_VERSION,
	url = "http://www.sourcemod.net/"
};

public OnPluginStart()
{
	CreateConVar("sm_damage_version", PLUGIN_VERSION,"SM Damage Version", FCVAR_NOTIFY|FCVAR_REPLICATED);
	
	g_enabled = CreateConVar("sm_damage_enabled","1");
	g_info = CreateConVar("sm_damage_info","1");
	
	g_multiplier = CreateConVar("sm_damage_defaultanydamagemultiplier","1.0", "Default Damage Multiplier for any inflicted damage", 0, true);
	
	g_playermultiplier = CreateConVar("sm_damage_defaultplayerweaponmultiplier","1.0", "Default Damage Multiplier for Player inflicted damage", 0, true);
	
	RegAdminCmd("sm_damage_playermulti", CmdDamageSet, ADMFLAG_CHEATS);
	RegAdminCmd("sm_damage_weaponmulti", CmdWeaponSet, ADMFLAG_CHEATS);
	RegAdminCmd("sm_damage_weaponamount", CmdWeaponAmountSet, ADMFLAG_CHEATS);
	RegAdminCmd("sm_damage_clear", CmdClearWeaponSettings, ADMFLAG_CHEATS);
	
	trieWeapons = CreateTrie();
	trieAmount = CreateTrie();
}

public OnClientPutInServer(client)
{
	Damage[client] = GetConVarFloat(g_playermultiplier);
	SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);
}

public OnEntityCreated(entity, const char classname[])
{
	if (StrEqual(classname, "infected") || StrEqual(classname, "witch"))
	{
		SDKHook(entity, SDKHook_OnTakeDamage, OnTakeDamage);
	}
}

public Action OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype)
{
	if(!GetConVarBool(g_enabled)) return Plugin_Handled; // is this thing even on?

	char sWeapon[32], Float:mWeapon, Float:amountWeapon;
	GetEdictClassname(inflictor, sWeapon, sizeof(sWeapon));
	
	bool changed;
	float changemulti = 1.0;
	float globalmulti = GetConVarFloat(g_multiplier);
	
	if(globalmulti != 1.0) //check for a global damage multiplier setting
	{
		if(globalmulti == 0.0) damage == 0.0; //nullify setting?
		else changemulti += globalmulti - 1.0; //else add multi
		changed = true;	
	}
	
	if (attacker > 0 && attacker <= MAXPLAYERS)
	{
		if(Damage[attacker] != globalmulti) //check for a player specific multiplier setting
		{
			if (Damage[attacker] == 0) damage == 0.0; //nullify setting?
			else changemulti += Damage[attacker] - 1.0; //else add multi
			changed = true;
		}
	}
	
	if(GetTrieValue(trieWeapons,sWeapon,mWeapon)) // check for the gun multiplier setting
	{
		if (mWeapon == 0) damage == 0.0; //nullify setting?
		else changemulti += mWeapon - 1.0; //else add multi
		changed = true;
	}
	
	if(changed && damage > 0.0) damage *= changemulti; //add all multipliers if not nullified yet
	
	if(GetTrieValue(trieAmount,sWeapon,amountWeapon)) //check for gun damage amount setting
	{
		damage += amountWeapon; //add gun specific damage amount
		changed = true;
	}
	
	if(changed) return Plugin_Changed;	
	return Plugin_Continue;
}

public Action CmdDamageSet(client, args)
{
	char player[256];
	char multiplier[20];
	
	if(GetConVarInt(g_enabled) == 0) return Plugin_Handled;
	
	if (args == 2)
	{
		GetCmdArg(1,player,255);
		GetCmdArg(2,multiplier,19);
		
		char target_name[MAX_TARGET_LENGTH];
		decl target_list[MAXPLAYERS];
		decl target_count;
		bool tn_is_ml;
		char name[256];
		
		if ((target_count = ProcessTargetString(player,client,target_list, MAXPLAYERS, 0, target_name, sizeof(target_name), tn_is_ml)) <= 0)
		{
			ReplyToTargetError(client, target_count);
			return Plugin_Handled;
		}
		
		if(StringToFloat(multiplier) < 0.0) FloatToString(0.0, multiplier, sizeof(multiplier));
		
		for (new i = 0; i < target_count; i++)
		{
			if(GetConVarBool(g_info))
			{
				GetClientName(target_list[i],name,255);
				ReplyToCommand(client,"%s %f",name,StringToFloat(multiplier));
			}
			Damage[target_list[i]] = StringToFloat(multiplier);
		}
	}
	else
	{
		ReplyToCommand(client, "Usage: sm_damage_playermulti <player> <multiplier>");
	}
	
	return Plugin_Handled;
}

public Action CmdWeaponSet(client, args)
{
	char weapon[32];
	char multiplier[20];
	
	if(!GetConVarBool(g_enabled)) return Plugin_Handled;
	
	if(args == 2)
	{
		GetCmdArg(1, weapon,sizeof(weapon));
		GetCmdArg(2, multiplier,sizeof(multiplier));
		
		if(StringToFloat(multiplier) < 0.0) FloatToString(0.0, multiplier, sizeof(multiplier));
		
		SetTrieValue(trieWeapons, weapon, StringToFloat(multiplier));
		ReplyToCommand(client, "Successfully set damage multiplier of weapon: %s to %.2f!", weapon, StringToFloat(multiplier));
	}
	else
	{
		ReplyToCommand(client, "Usage: sm_damage_weaponmulti <weapon> <multiplier>");
	}
	
	return Plugin_Handled;
}

public Action CmdWeaponAmountSet(client, args)
{
	char weapon[32], String:amount[20];
	
	if(!GetConVarBool(g_enabled)) return Plugin_Handled;
	
	if(args == 2)
	{
		GetCmdArg(1, weapon, sizeof(weapon));
		GetCmdArg(2, amount, sizeof(amount));
		
		SetTrieValue(trieAmount, weapon,StringToFloat(amount));
		ReplyToCommand(client, "Successfully set additional damage of weapon: %s to %.2f!", weapon, StringToFloat(amount));
	}
	else
	{
		ReplyToCommand(client, "Usage: sm_damage_weaponamount <weapon> <amount>");
	}
	
	return Plugin_Handled;
}

public Action CmdClearWeaponSettings(client, args)
{
	if(!GetConVarBool(g_enabled)) return Plugin_Handled;
	
	ClearTrie(trieWeapons);
	ClearTrie(trieAmount);
	ReplyToCommand(client, "Successfully cleared stored damage settings of all weapons!");
	
	return Plugin_Handled;
}

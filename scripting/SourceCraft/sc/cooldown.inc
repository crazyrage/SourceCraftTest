/**
 * vim: set ai et ts=4 sw=4 :
 * File: cooldown.inc
 * Description: Cooldown Type bits
 * Author(s): -=|JFH|=-Naris
 */

#if defined _cooldown_included
 #endinput
#endif
#define _cooldown_included

enum CooldownType
{
    Cooldown_None = 0,
    Cooldown_TimerOnly = 0,
    Cooldown_AutoReset = 1,
    Cooldown_ResetOnChange = 2,
    Cooldown_ResetOnDeath = 4,
    Cooldown_ResetOnSpawn = 8,
    Cooldown_CreateNotify = 16,
    Cooldown_ExpireNotify = 32,
    Cooldown_AlwaysNotify = 64,
    Cooldown_CreateOnSpawn = 128,
    Cooldown_SpecifiesBaseValue = 256
};

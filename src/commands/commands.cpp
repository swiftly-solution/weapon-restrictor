#include "../entrypoint.h"
#include <swiftly-ext/sdk.h>

void WeaponRestrictorManagerHelp()
{
    g_SMAPI->ConPrint("[Weapon Restrictor] Swiftly Weapon Restrictor Menu\n");
    g_SMAPI->ConPrint("[Weapon Restrictor] Usage: sw_weaponrestrictor <command>\n");
    g_SMAPI->ConPrint("[Weapon Restrictor]  reload     - Reloads the Weapon Restrictor config.\n");
}

void WeaponRestrictorReload()
{
    g_Ext.LoadRestrictions();
    g_SMAPI->ConPrint("[Weapon Restrictor] Weapon Restrictor config have been succesfully reloaded.\n");
}

void WeaponRestrictorManager(const char* subcmd)
{
    std::string sbcmd = subcmd;

    if(sbcmd.size() == 0)
    {
        WeaponRestrictorManagerHelp();
        return;
    }

    if(sbcmd == "reload")
        WeaponRestrictorReload();
    else
        WeaponRestrictorManagerHelp();
}

CON_COMMAND(sw_weaponrestrictor, "Weapon Restrictor Menu")
{
    WeaponRestrictorManager(args[1]);
}

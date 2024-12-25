#ifndef _entrypoint_h
#define _entrypoint_h

#include <string>

#include <public/igameevents.h>
#include <public/engine/igameeventsystem.h>

#include <swiftly-ext/core.h>
#include <swiftly-ext/extension.h>
#include <swiftly-ext/hooks/NativeHooks.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "utils.h"

class WeaponRestrictor : public SwiftlyExt
{
public:
    std::map<std::string, std::vector<std::string>> map_restriction;
    std::map<std::string, std::map<std::string, int>> per_team;
    std::map<std::string, std::map<std::string, int>> per_player;
    bool enable_in_warmup;
    bool limit_per_team;

    std::map<int, int> playerImmunity;

std::map<uint16_t, std::pair<std::string, GearSlot>> ItemDefIndex = {
    {1, {"weapon_deagle", GearSlot::GEAR_SLOT_PISTOL}},
    {2, {"weapon_elite", GearSlot::GEAR_SLOT_PISTOL}},
    {3, {"weapon_fiveseven", GearSlot::GEAR_SLOT_PISTOL}},
    {4, {"weapon_glock", GearSlot::GEAR_SLOT_PISTOL}},
    {7, {"weapon_ak47", GearSlot::GEAR_SLOT_RIFLE}},
    {8, {"weapon_aug", GearSlot::GEAR_SLOT_RIFLE}},
    {9, {"weapon_awp", GearSlot::GEAR_SLOT_RIFLE}},
    {10, {"weapon_famas", GearSlot::GEAR_SLOT_RIFLE}},
    {11, {"weapon_g3sg1", GearSlot::GEAR_SLOT_RIFLE}},
    {13, {"weapon_galilar", GearSlot::GEAR_SLOT_RIFLE}},
    {14, {"weapon_m249", GearSlot::GEAR_SLOT_RIFLE}},
    {16, {"weapon_m4a1", GearSlot::GEAR_SLOT_RIFLE}},
    {17, {"weapon_mac10", GearSlot::GEAR_SLOT_RIFLE}},
    {19, {"weapon_p90", GearSlot::GEAR_SLOT_RIFLE}},
    {23, {"weapon_mp5sd", GearSlot::GEAR_SLOT_RIFLE}},
    {24, {"weapon_ump45", GearSlot::GEAR_SLOT_RIFLE}},
    {25, {"weapon_xm1014", GearSlot::GEAR_SLOT_RIFLE}},
    {26, {"weapon_bizon", GearSlot::GEAR_SLOT_RIFLE}},
    {27, {"weapon_mag7", GearSlot::GEAR_SLOT_RIFLE}},
    {28, {"weapon_negev", GearSlot::GEAR_SLOT_RIFLE}},
    {29, {"weapon_sawedoff", GearSlot::GEAR_SLOT_RIFLE}},
    {30, {"weapon_tec9", GearSlot::GEAR_SLOT_PISTOL}},
    {32, {"weapon_hkp2000", GearSlot::GEAR_SLOT_PISTOL}},
    {33, {"weapon_mp7", GearSlot::GEAR_SLOT_RIFLE}},
    {34, {"weapon_mp9", GearSlot::GEAR_SLOT_RIFLE}},
    {35, {"weapon_nova", GearSlot::GEAR_SLOT_RIFLE}},
    {36, {"weapon_p250", GearSlot::GEAR_SLOT_PISTOL}},
    {38, {"weapon_scar20", GearSlot::GEAR_SLOT_RIFLE}},
    {39, {"weapon_sg556", GearSlot::GEAR_SLOT_RIFLE}},
    {40, {"weapon_ssg08", GearSlot::GEAR_SLOT_RIFLE}},
    {60, {"weapon_m4a1_silencer", GearSlot::GEAR_SLOT_RIFLE}},
    {61, {"weapon_usp_silencer", GearSlot::GEAR_SLOT_PISTOL}},
    {63, {"weapon_cz75a", GearSlot::GEAR_SLOT_PISTOL}},
    {64, {"weapon_revolver", GearSlot::GEAR_SLOT_PISTOL}}
};    

public:
    bool Load(std::string& error, SourceHook::ISourceHook *SHPtr, ISmmAPI* ismm, bool late);
    bool Unload(std::string& error);
    
    void AllExtensionsLoaded();
    void AllPluginsLoaded();

    bool OnPluginLoad(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error);
    bool OnPluginUnload(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error);


    void LoadRestrictions();

public:
    const char* GetAuthor();
    const char* GetName();
    const char* GetVersion();
    const char* GetWebsite();
};

extern WeaponRestrictor g_Ext;
DECLARE_GLOBALVARS();

#endif
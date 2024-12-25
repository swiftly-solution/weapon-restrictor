#include "entrypoint.h"

#include <fstream>
#include <swiftly-ext/files.h>
#include <swiftly-ext/sdk.h>

#include <ehandle.h>
#include "sdk/services.h"

AcquireResult Hook_CCSPlayer_ItemServices_CanAcquire(void* _this, void* econItemView, AcquireMethod acquireMethod, void* unk);

//////////////////////////////////////////////////////////////
/////////////////        Core Variables        //////////////
////////////////////////////////////////////////////////////
IGameEventSystem* g_pGameEventSystem = nullptr;
IVEngineServer2* engine = nullptr;

WeaponRestrictor g_Ext;
CUtlVector<FuncHookBase *> g_vecHooks;
CREATE_GLOBALVARS();


FuncHook<decltype(Hook_CCSPlayer_ItemServices_CanAcquire)>TCCSPlayer_ItemServices_CanAcquire(Hook_CCSPlayer_ItemServices_CanAcquire, "CCSPlayer_ItemServices_CanAcquire");

//////////////////////////////////////////////////////////////
/////////////////          Core Class          //////////////
////////////////////////////////////////////////////////////
void WeaponRestrictorError(std::string text)
{
    if (!g_SMAPI)
        return;

    g_SMAPI->ConPrintf("[Weapon Restrictor] %s\n", text.c_str());
}

EXT_EXPOSE(g_Ext);
bool WeaponRestrictor::Load(std::string& error, SourceHook::ISourceHook *SHPtr, ISmmAPI* ismm, bool late)
{
    SAVE_GLOBALVARS();
    if(!InitializeHooks()) {
        error = "Failed to initialize hooks.";
        return false;
    }

    GET_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_IFACE_CURRENT(GetEngineFactory, g_pGameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);

    LoadRestrictions();

    return true;
}

bool WeaponRestrictor::Unload(std::string& error)
{
    UnloadHooks();
    return true;
}

void WeaponRestrictor::AllExtensionsLoaded()
{

}

void WeaponRestrictor::AllPluginsLoaded()
{

}

bool WeaponRestrictor::OnPluginLoad(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error)
{
    return true;
}

bool WeaponRestrictor::OnPluginUnload(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error)
{
    return true;
}

void WeaponRestrictor::LoadRestrictions()
{
    enable_in_warmup = false;
    limit_per_team = false;
    map_restriction.clear();
    per_team.clear();
    per_player.clear();

    std::ifstream ifs(GeneratePath("addons/swiftly/configs/plugins/weapon_restrictor.json"));

    if(!ifs.is_open())
        return WeaponRestrictorError("Failed to open 'addons/swiftly/configs/plugins/weapon_restrictor.json'.");

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);

    if(document.HasParseError())
        return WeaponRestrictorError(string_format("A parsing error has been detected.\nError (offset %u): %s\n", (unsigned)document.GetErrorOffset(), GetParseError_En(document.GetParseError())));

    if (document.IsArray())
        return WeaponRestrictorError("Config file cannot be an array.");
    
    if(document.HasMember("enable_in_warmup") && document["enable_in_warmup"].IsBool())
        enable_in_warmup = document["enable_in_warmup"].GetBool();
    
    if(document.HasMember("limit_per_team") && document["limit_per_team"].IsBool())
        limit_per_team = document["limit_per_team"].GetBool();

    if (document.HasMember("map_restriction") && document["map_restriction"].IsObject()) {
        const auto& mapRestrictionObject = document["map_restriction"];

        for (auto it = mapRestrictionObject.MemberBegin(); it != mapRestrictionObject.MemberEnd(); ++it) {
            std::string mapName = it->name.GetString();
            const auto& weaponsArray = it->value;

            if (weaponsArray.IsArray()) {
                std::vector<std::string> weapons;
                for (auto& weapon : weaponsArray.GetArray()) {
                    if (weapon.IsString()) {
                        weapons.push_back(weapon.GetString());
                    }
                }
                map_restriction[mapName] = weapons;
            }
        }
    }

    if (document.HasMember("per_team") && document["per_team"].IsObject()) {
        const auto& perTeamObject = document["per_team"];
        for (auto it = perTeamObject.MemberBegin(); it != perTeamObject.MemberEnd(); ++it)
        {
            std::string weapon = it->name.GetString();
            const auto& teamObject = it->value;
            if (teamObject.IsObject())
            {
                std::map<std::string, int> teamLimits;
                if (teamObject.HasMember("t") && teamObject["t"].IsInt())
                {
                    teamLimits["t"] = teamObject["t"].GetInt();
                }
                if (teamObject.HasMember("ct") && teamObject["ct"].IsInt())
                {
                    teamLimits["ct"] = teamObject["ct"].GetInt();
                }

                per_team[weapon] = teamLimits;
            }
        }
    }

    if (document.HasMember("per_player") && document["per_player"].IsObject()) {
        const auto& perPlayerObject = document["per_player"];
        for (auto it = perPlayerObject.MemberBegin(); it != perPlayerObject.MemberEnd(); ++it)
        {
            std::string weapon = it->name.GetString();
            const auto& playerObject = it->value;
            if (playerObject.IsObject())
            {
                std::map<std::string, int> playerLimits;

                for (auto jt = playerObject.MemberBegin(); jt != playerObject.MemberEnd(); ++jt)
                {
                    std::string key = jt->name.GetString();
                    if (jt->value.IsInt())
                    {
                        playerLimits[key] = jt->value.GetInt();
                    }
                }
                per_player[weapon] = playerLimits;
            }            
        }
        
    }
}

AcquireResult Hook_CCSPlayer_ItemServices_CanAcquire(void* _this, void* econItemView, AcquireMethod acquireMethod, void* unk)
{
    CHandle<CEntityInstance> controller = SDKGetProp<CHandle<CEntityInstance>>(((CPlayer_ItemServices*)_this)->m_pPawn, "CBasePlayerPawn", "m_hController");

    int playerid = controller.GetEntryIndex() - 1;

    if(g_Ext.playerImmunity.count(playerid))
        return AcquireResult::ALLOWED;

    void* gameRules = GetCCSGameRules();

    bool IsWarmUp = SDKGetProp<bool>(gameRules, "CCSGameRules","m_bWarmupPeriod");

    if(IsWarmUp && !g_Ext.enable_in_warmup)
        return AcquireResult::ALLOWED;

    std::string mapName = engine->GetServerGlobals()->mapname.ToCStr();
    uint16_t itemidx = SDKGetProp<uint16_t>(econItemView, "CEconItemView", "m_iItemDefinitionIndex");
    std::string itemWeaponName = GetWeaponNameByIdx(itemidx);

    if (auto it = g_Ext.map_restriction.find(mapName); it != g_Ext.map_restriction.end())
    {
        for (const std::string& restrictionWeaponName : it->second)
        {
            if (itemWeaponName == restrictionWeaponName)
            {
                return AcquireResult::NOT_ALLOWED_BY_PROHIBITION;
            }
        }
    }

    if(g_Ext.limit_per_team) {
        // Implementation limitation logic

        return AcquireResult::ALLOWED; 
    }
    
    return AcquireResult::ALLOWED;
}

const char* WeaponRestrictor::GetAuthor()
{
    return "Swiftly Development Team";
}

const char* WeaponRestrictor::GetName()
{
    return "Weapon Restrictor Extension";
}

const char* WeaponRestrictor::GetVersion()
{
#ifndef VERSION
    return "Local";
#else
    return VERSION;
#endif
}

const char* WeaponRestrictor::GetWebsite()
{
    return "https://swiftlycs2.net/";
}

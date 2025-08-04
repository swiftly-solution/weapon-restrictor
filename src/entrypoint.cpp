#include "entrypoint.h"

#include <fstream>
#include <swiftly-ext/files.h>
#include <swiftly-ext/sdk.h>
#include <swiftly-ext/event.h>

#include <embedder/src/Embedder.h>

#include <ehandle.h>
#include "sdk/services.h"

dyno::ReturnAction Hook_CCSPlayer_ItemServices_CanAcquire(dyno::CallbackType type, dyno::IHook& hook);

//////////////////////////////////////////////////////////////
/////////////////        Core Variables        //////////////
////////////////////////////////////////////////////////////
IGameEventSystem* g_pGameEventSystem = nullptr;
IVEngineServer2* engine = nullptr;
ISource2Server* server = nullptr;

WeaponRestrictor g_Ext;
CREATE_GLOBALVARS();

FunctionHook CCSPlayer_ItemServices_CanAcquire("CCSPlayer_ItemServices_CanAcquire", dyno::CallbackType::Pre, Hook_CCSPlayer_ItemServices_CanAcquire, "ppup", 'u');

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
bool WeaponRestrictor::Load(std::string& error, SourceHook::ISourceHook* SHPtr, ISmmAPI* ismm, bool late)
{
    SAVE_GLOBALVARS();

    GET_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_IFACE_ANY(GetServerFactory, server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);
    GET_IFACE_CURRENT(GetEngineFactory, g_pGameEventSystem, IGameEventSystem, GAMEEVENTSYSTEM_INTERFACE_VERSION);

    LoadRestrictions();

    return true;
}

bool WeaponRestrictor::Unload(std::string& error)
{
    return true;
}

void WeaponRestrictor::AllExtensionsLoaded()
{

}

void WeaponRestrictor::AllPluginsLoaded()
{

}

void SetPlayerImmunityStatus(int playerid, bool state)
{
    if (state) {
        if (g_Ext.playerImmunity.find(playerid) == g_Ext.playerImmunity.end()) g_Ext.playerImmunity.insert(playerid);
    }
    else {
        if (g_Ext.playerImmunity.find(playerid) != g_Ext.playerImmunity.end()) g_Ext.playerImmunity.erase(playerid);
    }
}

bool WeaponRestrictor::OnPluginLoad(std::string pluginName, void* pluginState, PluginKind_t kind, std::string& error)
{
    EContext* ctx = (EContext*)pluginState;

    ADD_FUNCTION("SetPlayerImmunityStatus", [](FunctionContext* context) -> void {
        int playerid = context->GetArgumentOr<int>(0, -1);
        bool state = context->GetArgumentOr<bool>(1, false);

        SetPlayerImmunityStatus(playerid, state);
        });

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

    if (!ifs.is_open())
        return WeaponRestrictorError("Failed to open 'addons/swiftly/configs/plugins/weapon_restrictor.json'.");

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);

    if (document.HasParseError())
        return WeaponRestrictorError(string_format("A parsing error has been detected.\nError (offset %u): %s\n", (unsigned)document.GetErrorOffset(), GetParseError_En(document.GetParseError())));

    if (document.IsArray())
        return WeaponRestrictorError("Config file cannot be an array.");

    if (document.HasMember("enable_in_warmup") && document["enable_in_warmup"].IsBool())
        enable_in_warmup = document["enable_in_warmup"].GetBool();

    if (document.HasMember("limit_per_team") && document["limit_per_team"].IsBool())
        limit_per_team = document["limit_per_team"].GetBool();

    if (document.HasMember("map_restriction") && document["map_restriction"].IsObject()) {
        const auto& mapRestrictionObject = document["map_restriction"];

        for (auto it = mapRestrictionObject.MemberBegin(); it != mapRestrictionObject.MemberEnd(); ++it) {
            std::string mapName = it->name.GetString();
            const auto& weaponsArray = it->value;

            if (weaponsArray.IsArray()) {
                std::set<std::string> weapons;
                for (auto& weapon : weaponsArray.GetArray()) {
                    if (weapon.IsString()) {
                        weapons.insert(weapon.GetString());
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

int GetPlayerCount() {
    if (!engine) return 0;
    if (!engine->GetServerGlobals()) return 0;

    int count = 0;

    for (int i = 0; i < engine->GetServerGlobals()->maxClients; i++) {
        auto controller = GameEntitySystem()->GetEntityInstance(CEntityIndex(i + 1));
        if (!controller) continue;
        count++;
    }

    return count;
}

int CountActiveWeaponsOnPlayers(uint16_t item_idx) {
    if (!engine) return 0;
    if (!engine->GetServerGlobals()) return 0;

    int count = 0;

    for (int i = 0; i < engine->GetServerGlobals()->maxClients; i++) {
        auto controller = GameEntitySystem()->GetEntityInstance(CEntityIndex(i + 1));
        if (!controller) continue;

        CHandle<CEntityInstance> pawn = SDKGetProp<CHandle<CEntityInstance>>(controller, "CBasePlayerController", "m_hPawn");
        if (!pawn.IsValid()) continue;

        void* pPawn = (void*)pawn.Get();
        if (!pPawn) continue;

        void* weaponServices = SDKGetProp<void*>(pPawn, "CBasePlayerPawn", "m_pWeaponServices");
        if (!weaponServices) continue;

        CUtlVector<CHandle<CEntityInstance>>* myWeapons = SDKGetPropPtr<CUtlVector<CHandle<CEntityInstance>>>(weaponServices, "CPlayer_WeaponServices", "m_hMyWeapons");

        FOR_EACH_VEC(*myWeapons, j)
        {
            void* cbaseweapon = (*myWeapons)[j].Get();
            if (!cbaseweapon) continue;
            void* attrManager = SDKGetProp<void*>(cbaseweapon, "CEconEntity", "m_AttributeManager");
            if (!attrManager) continue;
            void* itemView = SDKGetProp<void*>(attrManager, "CAttributeContainer", "m_Item");
            if (!itemView) continue;
            uint16_t itemidx = SDKGetProp<uint16_t>(itemView, "CEconItemView", "m_iItemDefinitionIndex");

            if (itemidx == item_idx) count++;
        }
    }

    return count;
}

int CountActiveWeaponsOnPlayersWithTeam(uint16_t item_idx, uint8_t teamnum) {
    if (!engine) return 0;
    if (!engine->GetServerGlobals()) return 0;

    int count = 0;

    for (int i = 0; i < engine->GetServerGlobals()->maxClients; i++) {
        auto controller = GameEntitySystem()->GetEntityInstance(CEntityIndex(i + 1));
        if (!controller) continue;

        CHandle<CEntityInstance> pawn = SDKGetProp<CHandle<CEntityInstance>>(controller, "CBasePlayerController", "m_hPawn");
        if (!pawn.IsValid()) continue;

        void* pPawn = (void*)pawn.Get();
        if (!pPawn) continue;

        uint8_t team = SDKGetProp<uint8_t>(pPawn, "CBaseEntity", "m_iTeamNum");
        if (team != teamnum) continue;

        void* weaponServices = SDKGetProp<void*>(pPawn, "CBasePlayerPawn", "m_pWeaponServices");
        if (!weaponServices) continue;

        CUtlVector<CHandle<CEntityInstance>>* myWeapons = SDKGetPropPtr<CUtlVector<CHandle<CEntityInstance>>>(weaponServices, "CPlayer_WeaponServices", "m_hMyWeapons");

        FOR_EACH_VEC(*myWeapons, j)
        {
            void* cbaseweapon = (*myWeapons)[j].Get();
            if (!cbaseweapon) continue;
            void* attrManager = SDKGetProp<void*>(cbaseweapon, "CEconEntity", "m_AttributeManager");
            if (!attrManager) continue;
            void* itemView = SDKGetProp<void*>(attrManager, "CAttributeContainer", "m_Item");
            if (!itemView) continue;
            uint16_t itemidx = SDKGetProp<uint16_t>(itemView, "CEconItemView", "m_iItemDefinitionIndex");

            if (itemidx == item_idx) count++;
        }
    }

    return count;
}

AcquireResult CheckAcquire(void* _this, void* econItemView, AcquireMethod acquireMethod, void* unk, int& playerid, int& iidx, int& limit, std::string& weapon_name)
{
    CHandle<CEntityInstance> controller = SDKGetProp<CHandle<CEntityInstance>>(((CPlayer_ItemServices*)_this)->m_pPawn, "CBasePlayerPawn", "m_hController");

    playerid = controller.GetEntryIndex() - 1;

    if (g_Ext.playerImmunity.find(playerid) != g_Ext.playerImmunity.end())
        return AcquireResult::ALLOWED;

    void* gameRules = GetCCSGameRules();

    bool IsWarmUp = SDKGetProp<bool>(gameRules, "CCSGameRules", "m_bWarmupPeriod");

    if (IsWarmUp && !g_Ext.enable_in_warmup)
        return AcquireResult::ALLOWED;

    std::string mapName = engine->GetServerGlobals()->mapname.ToCStr();
    uint16_t itemidx = SDKGetProp<uint16_t>(econItemView, "CEconItemView", "m_iItemDefinitionIndex");
    std::string itemWeaponName = GetWeaponNameByIdx(itemidx);
    iidx = (int)itemidx;
    weapon_name = itemWeaponName;
    limit = 0;

    if (auto it = g_Ext.map_restriction.find(mapName); it != g_Ext.map_restriction.end())
        if (it->second.find(itemWeaponName) != it->second.end())
            return AcquireResult::NOT_ALLOWED_BY_MAP;

    if (g_Ext.limit_per_team) {
        if (g_Ext.per_team.find(itemWeaponName) == g_Ext.per_team.end()) return AcquireResult::ALLOWED;
        auto teamid = SDKGetProp<uint8_t>(((CPlayer_ItemServices*)_this)->m_pPawn, "CBaseEntity", "m_iTeamNum");

        auto limits = g_Ext.per_team[itemWeaponName];
        int weaponLimit = (teamid == 2 ? limits["t"] : limits["ct"]);
        if (weaponLimit == -1) return AcquireResult::ALLOWED;

        limit = weaponLimit;

        int itemCount = CountActiveWeaponsOnPlayersWithTeam(itemidx, teamid);
        if (itemCount >= weaponLimit) return AcquireResult::NOT_ALLOWED_BY_PROHIBITION;
    }
    else {
        if (g_Ext.per_player.find(itemWeaponName) == g_Ext.per_player.end()) return AcquireResult::ALLOWED;
        int itemCount = CountActiveWeaponsOnPlayers(itemidx);
        auto limits = g_Ext.per_player[itemWeaponName];

        int weaponLimit = (limits.find("default") != limits.end() ? limits["default"] : -1);
        int playerCount = GetPlayerCount();

        for (auto it = limits.begin(); it != limits.end(); ++it)
        {
            if (it->first == "default") continue;

            try {
                if (std::stoi(it->first.c_str()) < playerCount)
                    weaponLimit = it->second;
                else
                    break;
            }
            catch (std::exception& e) {}
        }

        if (weaponLimit == -1) return AcquireResult::ALLOWED;

        limit = weaponLimit;

        if (itemCount >= weaponLimit) return AcquireResult::NOT_ALLOWED_BY_PROHIBITION;
    }

    return AcquireResult::ALLOWED;
}

dyno::ReturnAction Hook_CCSPlayer_ItemServices_CanAcquire(dyno::CallbackType type, dyno::IHook& hook)
{
    int playerid, itemidx, limit;
    std::string weapon_name;
    AcquireResult res = CheckAcquire(hook.getArgument<void*>(0), hook.getArgument<void*>(1), hook.getArgument<AcquireMethod>(2), hook.getArgument<void*>(3), playerid, itemidx, limit, weapon_name);

    if (res != AcquireResult::ALLOWED) {
        std::any val;
        TriggerEvent("weapon-restrictor.ext", "OnItemAcquireFail", { playerid, itemidx, limit, weapon_name }, val);
    }
    hook.setReturn(res);
    return dyno::ReturnAction::Supercede;
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
    return "https://swiftlys2.net/";
}

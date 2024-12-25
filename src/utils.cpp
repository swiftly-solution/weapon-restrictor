#include "utils.h"
#include <public/entity2/entitysystem.h>
#include <swiftly-ext/sdk.h>
#include "entrypoint.h"

inline void* UTIL_FindEntityByClassname(const char* name)
{
    CEntityIdentity* pEntity = GameEntitySystem()->m_EntityList.m_pFirstActiveEntity;

    for (; pEntity; pEntity = pEntity->m_pNext)
    {
        if (!strcmp(pEntity->m_designerName.String(), name))
            return pEntity->m_pInstance;
    };

    return nullptr;
}

void* GetCCSGameRules()
{
    static void* gameRules = nullptr;

    if (!gameRules) {
        void* entity = UTIL_FindEntityByClassname("cs_gamerules");
        if (entity) {
            gameRules = SDKGetProp<void*>(entity, "CCSGameRulesProxy", "m_pGameRules");
        }
    }

    return gameRules;
}

GearSlot GetSlotByIdx(uint16_t itemidx)
{
    if (auto it = g_Ext.ItemDefIndex.find(itemidx); it != g_Ext.ItemDefIndex.end()) {
        return it->second.gearSlot;
    }
    return GearSlot::GEAR_SLOT_INVALID;
}

std::string GetWeaponNameByIdx(uint16_t itemidx)
{
    if (auto it = g_Ext.ItemDefIndex.find(itemidx); it != g_Ext.ItemDefIndex.end()) {
        return it->second.weaponName;
    }
    return "unknown";
}
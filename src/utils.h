#ifndef _utils_h
#define _utils_h

enum class AcquireResult : unsigned int {
    ALLOWED = 0,
    INVALID_ITEM,
    ALREADY_OWNER,
    ALREADY_PURCHASED,
    REACHED_GRENADE_TYPE_LIMIT,
    REACHED_GRENADE_TOTAL_LIMIT,
    NOT_ALLOWED_BY_TEAM,
    NOT_ALLOWED_BY_MAP,
    NOT_ALLOWED_BY_MODE,
    NOT_ALLOWED_FOR_PURCHASE,
    NOT_ALLOWED_BY_PROHIBITION
};

enum class AcquireMethod : unsigned int {
    PICKUP = 0,
    BUY = 1,
};

enum class GearSlot : int  {
    GEAR_SLOT_INVALID = -1,
    GEAR_SLOT_RIFLE = 0,
    GEAR_SLOT_PISTOL,
    GEAR_SLOT_KNIFE,
    GEAR_SLOT_GRENADES,
    GEAR_SLOT_C4,
    GEAR_SLOT_RESERVED_SLOT6,
    GEAR_SLOT_RESERVED_SLOT7,
    GEAR_SLOT_RESERVED_SLOT8,
    GEAR_SLOT_RESERVED_SLOT9,
    GEAR_SLOT_RESERVED_SLOT10,
    GEAR_SLOT_RESERVED_SLOT11,
    GEAR_SLOT_BOOSTS,
    GEAR_SLOT_UTILITY = 12,
    GEAR_SLOT_COUNT = 13,
    GEAR_SLOT_FIRST = 0,
    GEAR_SLOT_LAST = 12
};

void* UTIL_FindEntityByClassname(const char* name);
void* GetCCSGameRules();

GearSlot GetSlotByIdx(uint16_t itemidx);
std::string GetWeaponNameByIdx(uint16_t itemidx);

template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
    int size_s = snprintf(nullptr, 0, format.c_str(), args...) + 1;
    if (size_s <= 0)
        return "";

    size_t size = static_cast<size_t>(size_s);
    char* buf = new char[size];
    snprintf(buf, size, format.c_str(), args...);
    std::string out = std::string(buf, buf + size - 1);
    delete buf;
    return out;
}

#endif
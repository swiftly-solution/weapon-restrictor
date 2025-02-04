AddEventHandler("OnItemAcquireFail", function (event, playerid, itemidx, limit, weapon_name)
    local player = GetPlayer(playerid)
    if not player then return EventResult.Continue end

    if not player:GetVar("weapon_restrictor.lastinteraction." .. itemidx) then
        player:SetVar(
            "weapon_restrictor.lastinteraction." .. itemidx, GetTime() - 500)
    end

    if GetTime() - player:GetVar("weapon_restrictor.lastinteraction." .. itemidx) >= 500 then
        ReplyToCommand(playerid, config:Fetch("weapon_restrictor.prefix"),
            FetchTranslation("weapon_restrict.cannot_pickup"):gsub("{LIMIT}", limit):gsub("{WEAPON_NAME}", weapon_name))
        CBaseEntity(player:CCSPlayerController():ToPtr()):EmitSound("Vote.Failed", 1.0, 0.75)
    end

    player:SetVar("weapon_restrictor.lastinteraction." .. itemidx, GetTime())

    return EventResult.Handled
end)
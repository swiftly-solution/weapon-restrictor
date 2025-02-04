export("SetPlayerImmunity", function(playerid, immunitystatus)
    local player = GetPlayer(playerid)
    if not player then return end

    SetImmunityStatus(playerid, immunitystatus == true)
end)
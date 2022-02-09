-- adaption for DA62 panel

-- link datarefs
flap_led = create_dataref_table("mrusk/panel/flap", "IntArray")
gear_led = create_dataref_table("mrusk/panel/gear", "IntArray")
deice_led = create_dataref_table("mrusk/panel/deice", "IntArray")
dataref("flap_ratio", "sim/flightmodel2/controls/flap1_deploy_ratio")
gear_ratio = dataref_table("sim/flightmodel2/gear/deploy_ratio")
dataref("gear_unsafe", "sim/cockpit/warnings/annunciators/gear_unsafe")

-- initialize tables
flap_led[0] = 0
flap_led[1] = 0
flap_led[2] = 0
gear_led[0] = 0
gear_led[1] = 0
gear_led[2] = 0
gear_led[3] = 0
deice_led[0] = 0
deice_led[1] = 0
deice_led[2] = 0

-- Deice only with DA62
if (PLANE_ICAO == "DA62") then
    dataref("deice_norm", "aerobask/deice/lt_norm")
    dataref("deice_high", "aerobask/deice/lt_high")
    dataref("deice_max", "aerobask/deice/lt_max")
else
    deice_high = 0
    deice_max = 0
    deice_norm = 0
end

-- handle everything
function mrusk_panel_handle()
    -- handle flaps
    if (flap_ratio < 0.05) then
        flap_led[0] = 1
        flap_led[1] = 0
        flap_led[2] = 0
    elseif (flap_ratio < 0.45) then
        flap_led[0] = 1
        flap_led[1] = 1
        flap_led[2] = 0
    elseif (flap_ratio < 0.55) then
        flap_led[0] = 0
        flap_led[1] = 1
        flap_led[2] = 0
    elseif (flap_ratio < 0.95) then
        flap_led[0] = 0
        flap_led[1] = 1
        flap_led[2] = 1
    else 
        flap_led[0] = 0
        flap_led[1] = 0
        flap_led[2] = 1
    end

    -- handle gear
    for i = 0, 2 do
        if (gear_ratio[i] > 0.99) then
            gear_led[i] = 1
        else
            gear_led[i] = 0
        end
    end
    gear_led[3] = gear_unsafe

    -- deice
    deice_led[0] = deice_norm
    deice_led[1] = deice_high
    deice_led[2] = deice_max

end

do_every_draw("mrusk_panel_handle()")
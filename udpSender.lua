-- BeamNG Lua script to send car data to ESP32
-- Place in: Documents/BeamNG.drive/lua/vehicle/extensions/auto/udpSender.lua

local M = {}

local ESP32_IP = "192.168.XXX.X" -- Replace with your ESP32's IP
local ESP32_PORT = 4444

local function onExtensionLoaded()
    -- Debugging message to make sure the script works lol
    print("ESP32 Dashboard extension loaded")
    print("---")
    print("---")
    print("---")
    print("---")
    print("---")
end

local function updateGFX(dt)
    local electrics = electrics.values
    if not electrics then return end

    local speed = (electrics.wheelspeed or 0) * 3.6
    local rpm = electrics.rpm or 0
    local gear = electrics.gear or 0
    local fuel = (electrics.fuel or 1) * 100
    local throttle = electrics.throttle or 0
    local brake = electrics.brake or 0

    strData1 =
        string.format("%.1f", speed) .. ',' ..
        string.format("%d", math.floor(rpm)) .. ',' ..
        string.format("%d", gear) .. ',' ..
        string.format("%.1f", fuel) .. ',' ..
        string.format("%.2f", throttle) .. ',' ..
        string.format("%.2f", brake)

    local socket = require "socket"
    local udp = socket.udp()
    udp:settimeout(0)
    udp:setoption('reuseaddr', true)
    udp:setsockname('*', 0)
    udp:setpeername(ESP32_IP, ESP32_PORT)
    udp:send(strData1)
    udp:close()
end

-- Register callbacks
M.onExtensionLoaded = onExtensionLoaded
M.updateGFX = updateGFX

return M

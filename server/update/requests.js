const { db } = require('./database')
const dateFormat = require("dateformat");

let sql = "";

const log = async (ip, action, mac = null) => {
    sql = "INSERT INTO logs SET ?";
    const data = { ip, action, mac };
    await db.query(sql, data);

    if (mac) updateStatus(mac, "online");
}

const deviceExists = async (mac) => {
    sql = "SELECT mac from devices WHERE mac=?";
    const result = await db.query(sql, mac);

    if (!result[0].length) return false;
    return true;
}

const getLatestFirmwareId = async (type) => {
    sql = "SELECT id FROM firmware WHERE type = ? ORDER BY created_at DESC LIMIT 1";
    const result = await db.query(sql, type);

    if (!result[0].length) return null;

    return result[0][0].id;
}

const updateStatus = async (mac, status) => {
    sql = "UPDATE devices SET status=?, updated_at=curtime() WHERE mac=?";
    await db.query(sql, [status, mac]);
}

const checkForDeviceUpdate = async (mac) => {
    const types = ["spiffs", "firmware"];
    for (i = 0; i < 2; i++) {
        sql = `
        SELECT firmware.created_at AS build, firmware.type, firmware.path, firmware.version
            FROM devices
        JOIN firmware
            ON target_${types[i]} = firmware.id
        WHERE 
            (target_${types[i]} != current_${types[i]} OR current_${types[i]} IS NULL)
            AND target_${types[i]} IS NOT NULL
            AND status != 'updating_${i==0 ? types[1] : types[0]}'
            AND mac=?
            AND curtime() >= update_start
            AND curtime() <= update_end
        `;
        const result = await db.query(sql, mac);

        if (result[0].length) {
            updateStatus(mac, `updating_${types[i]}`);
            return { ...result[0][0], build: dateFormat(result[0][0].build, "ddmmHHMM") };
        }
    }
    return false;
}

const registerWinder = async (mac) => {
    const spiffs = await getLatestFirmwareId("spiffs");
    const firmware = await getLatestFirmwareId("firmware");

    data = { mac, target_firmware: firmware, target_spiffs: spiffs };

    sql = "INSERT IGNORE INTO devices SET ?";
    await db.query(sql, data);
}

const completeUpdate = async (mac) => {
    console.log("complte");
    sql = `UPDATE devices SET current_spiffs = target_spiffs WHERE mac = ? AND status = 'updating_spiffs'`;
    await db.query(sql, mac);
    sql = `UPDATE devices SET current_firmware = target_firmware WHERE mac = ? AND status = 'updating_firmware'`;
    await db.query(sql, mac);
}

module.exports = { log, deviceExists, registerWinder, checkForDeviceUpdate, completeUpdate };
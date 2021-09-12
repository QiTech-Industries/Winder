const { db } = require('./database')
const dateFormat = require("dateformat");


const log = async (ip, action, mac = null) => {
    const sql = "INSERT INTO logs SET ?";
    const data = { ip, action, mac };

    await db.query(sql, data);
}

const deviceExists = async (mac) => {
    const sql = "SELECT COUNT(*) AS count from devices WHERE mac=?";
    const result = await db.query(sql, mac);

    if (result[0][0].count > 0) return true;
    return false;
}

const getLatestFirmware = async (hardware_id, beta = false) => {
    const sql = "SELECT f1.* FROM firmware f1 LEFT JOIN firmware f2 ON (f1.type = f2.type AND f1.created_at < f2.created_at) WHERE f2.created_at IS NULL AND f1.hardware_id=? AND f1.beta=? ORDER BY type DESC";
    const result = await db.query(sql, [hardware_id, beta]);

    if (result[0].length != 2) return false;

    return {
        firmware: { ...result[0][0] },
        spiffs: { ...result[0][1] }
    }
}

const checkForDeviceUpdate = async (mac) => {
    const sql = `SELECT firmware_spiffs, firmware_firmware, beta, hardware_id FROM devices WHERE mac=? AND curtime() >= update_start AND curtime() <= update_end`;
    const result = await db.query(sql, mac);

    if (!result[0].length) return false;

    const firmware = await getLatestFirmware(result[0][0].hardware_id, result[0][0].beta);

    if (!firmware) return false;

    if (firmware.firmware.id != result[0][0].firmware_firmware) {
        return { ...firmware.firmware, build: dateFormat(firmware.firmware.created_at, "ddmmHHMM") };
    }
    if (firmware.spiffs.id != result[0][0].firmware_spiffs) {
        return { ...firmware.spiffs, build: dateFormat(firmware.spiffs.created_at, "ddmmHHMM") };
    }

    return false;
}

const registerWinderGearRatio5 = async (mac) => {
    const hardware_id = 1;
    const firmware = await getLatestFirmware(hardware_id);

    if(firmware) data = { mac, hardware_id, firmware_firmware: firmware.firmware.id };
    else data = { mac, hardware_id};

    const sql = "INSERT IGNORE INTO devices SET ?";
    await db.query(sql, data);
}

const updateVersion = async (type, versionId, mac) => {
    const sql = `UPDATE devices SET firmware_${type} = ? WHERE mac = ?`;
    await db.query(sql, [versionId, mac]);
}

module.exports = { log, deviceExists, registerWinderGearRatio5, checkForDeviceUpdate, updateVersion };
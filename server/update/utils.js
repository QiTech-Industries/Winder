const validateMac = mac => {
    const regex = /^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$/;
    return regex.test(mac);
}

const validateFirmware = json => {
    const regex = /[v][0-9].[0-9].[0-9][sf]/;
    if(json && json.version && json.type && json.path && Object.keys(json).length == 3 && (json.type == "spiffs" || json.type == "firmware") && regex.test(json.version)) return true
    return false
}

module.exports = { validateMac, validateFirmware };
const validateMac = mac => {
    const regex = /^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$/;
    return regex.test(mac);
}

module.exports = { validateMac };
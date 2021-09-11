var fs = require('fs')

const firmwareFolder = "./firmware";

latest = {
    spiffs: {
        path: "",
        version: "0.0.0",
    },
    firmware: {
        path: "",
        version: "0.0.0",
    }
}

// Return true if v1 is newer than v2
const newerVersion = (v1, v2) => {
    v1parts = v1.split('.');
    v2parts = v2.split('.');

    for (i = 0; i < 3; i++) {
        if (v1parts[i] === v2parts[i]) continue;
        if (v1parts[i] > v2parts[i]) return true;
        if (v2parts[i] > v1parts[i]) return false;

        if (i === 2) return false;
    }
}

// Split filename into version and type (spiffs/firmware)
const splitFilename = (filename) => {
    const regex = /(spiffs|filename)_(([0-9]{1,2}\.){3})bin/gm;
    let matches = [...filename.matchAll(regex)][0];

    if (matches.length == 0) {
        console.log(`Filename ${filename} does not match requirements`);
        return { type: null, version: null };
    }

    return { type: matches[1], version: matches[2].slice(0, -1) };
}

const validateVersion = (version = "") => {
    const regex = /([0-9]{1,2}\.){2}[0-9]{1,2}/gm;
    let matches = version.match(regex);
    if(matches) return true;
    return false;
}

const getLatest = () => {
    fs.readdir(firmwareFolder, (err, filenames) => {
        if (err) {
            console.log("error reading firmware folder");
            return latest;
        }
        for(filename of filenames){
            let { type, version } = splitFilename(filename);
            if (!type || !version) continue;
            if (newerVersion(version, latest[type].version)) {
                latest[type].version = version;
                latest[type].path = `${firmwareFolder}/${filename}`;
            }   
        }
        console.log(latest);
    });

    return latest;
}

module.exports = {getLatest, newerVersion, validateVersion};
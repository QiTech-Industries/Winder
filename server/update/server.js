const express = require('express')
const { log, registerWinder, checkForDeviceUpdate, deviceExists, completeUpdate } = require('./requests')
const { validateMac } = require('./utils')

const app = express();
app.disable('x-powered-by');

app.get('/:mac', async (req, res) => {
  const ip = req.get("x-real-ip");
  const mac = req.params.mac;

  if (!validateMac(mac)) {
    log(ip, "invalid");
    res.status(400).send();
    return;
  }

  // This endpoint must be deactivated as soon as all Winders run the new firmware
  // it should not be possible for devices to register themselves!
  // currently all supported devices that have not been registered by hand are
  // Winders with a 1:5 Gear ratio
  deviceExists(mac).then(exists => {
    if (!exists) {
      registerWinder(mac).then(() => {
        log(ip, "register", mac);
      });
    }
  })
  //


  checkForDeviceUpdate(mac).then(firmware => {
    if (firmware) {
      res.setHeader('X-Update', firmware.type);
      res.setHeader('X-Version', firmware.version);
      res.setHeader('X-Build', firmware.build);
      res.download(firmware.path);
      return;
    }
    else log(ip, "check", mac);
    res.status(201).send();
    return;
  });
});

app.get('/success/:mac', async (req, res) => {
  const ip = req.get("x-real-ip");
  const mac = req.params.mac;

  if (!validateMac(mac)) {
    log(ip, "invalid");
    res.status(400).send();
    return;
  }

  completeUpdate(mac);
  log(ip, "success", mac);
  res.status(201).send();
});

app.listen(5000, () => {
  console.log(`Update server started`)
})

// Require Section
////////////////////////////
const express = require('express')
const { log, registerWinder, checkForDeviceUpdate, deviceExists, completeUpdate, createFirmware } = require('./requests')
const { validateMac, validateFirmware } = require('./utils')
const multer = require('multer')
////////////////////////////

// Configure section
////////////////////////////
// Express sever setup
const app = express();
app.disable('x-powered-by');

// Multer file upload setup
const firmware = multer({
  dest: 'firmware/',
  fileFilter: (req, file, cb) => {
    // Stop file upload if wrong mimetype or ending
    if (file.mimetype !== 'application/octet-stream' || file.originalname.substr(-4) != ".bin") {
      return cb(new Error())
    }

    // Proceed upload of file
    cb(null, true)
  },
  limits: {
    fileSize: 1507328 // this is the size of spiffs partition
  }
}).single('file')
////////////////////////////

// Route Section
////////////////////////////
// Register device and send update if available
app.get('/:mac', async (req, res, next) => {
  const ip = req.get("x-real-ip");
  const mac = req.params.mac;

  if (!validateMac(mac)) return next({ code: 400, message: "invalid mac address" })

  // This endpoint must be deactivated as soon as all Winders run the new firmware
  // it should not be possible for devices to register themselves!
  // currently all supported devices that have not been registered by hand are
  // Winders with a 1:5 Gear ratio
  const exists = await deviceExists(mac);
  if (!exists) {
    await registerWinder(mac);
    log(ip, "register", mac);
  }


  await log(ip, "check", mac);
  checkForDeviceUpdate(mac).then(firmware => {
    if (firmware) {
      res.setHeader('X-Update', firmware.type);
      res.setHeader('X-Version', firmware.version);
      res.setHeader('X-Build', firmware.build);
      res.download(firmware.path);
      return;
    }
    return next({ code: 204, message: "no update available" })
  });
});

// Firmware update has been completed successfully
app.get('/success/:mac', async (req, res, next) => {
  const ip = req.get("x-real-ip");
  const mac = req.params.mac;

  if (!validateMac(mac)) return next({ code: 400, message: "invalid mac address" })

  completeUpdate(mac);
  log(ip, "success", mac);
  return next({ code: 201, message: "update success registered" })
});

// Upload new firmware release to DB
app.post('/firmware', async (req, res, next) => {
  if (req.get("Authorization") != "Bearer dewrgfvxc23534rsdf234ds126tzruj54") {
    return next({ code: 401, message: "bearer auth failed" })
  }
  firmware(req, res, (err) => {
    if (err) {
      return next({ code: 400, message: "file must be in .bin format and max 1.4mb" })
    }

    // assemble db row
    const firmware = { ...req.body, path: req.file ? req.file.path : undefined };

    // Stop release if wrong request body
    if (!validateFirmware(firmware)) {
      return next({ code: 400, message: 'version, file and type field are required in correct format' });
    }

    // Stop release if DB entry already exists
    createFirmware(firmware).then(success => {
      if (success) return next({ code: 201, message: "created new firmware release" });
      return next({ code: 400, message: "version already exists" });
    });
  });
});
////////////////////////////

// Global error handling; must be registered last
app.use((err, req, res, next) => {
  res.status(err.code);
  res.type('json');
  res.type('application/json');
  res.send(err);
});

// Start express sever
app.listen(5000, () => {
  console.log(`Update server started`)
})

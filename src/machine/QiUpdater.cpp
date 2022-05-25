#include <machine/QiUpdater.h>

QiUpdater::QiUpdater(const String macAddress) : timer(this, &QiUpdater::handle) {
    _macAddress = macAddress;
    http.collectHeaders(UPDATE_HEADERS, 4);
    timer.setInterval(UPDATE_INTERVAL);
    timer.start();
}

void QiUpdater::onBeforeUpdate(std::function<bool()> cb) { _beforeUpdateCb = cb; }

void QiUpdater::onUpdateSuccess(std::function<void(String, String, String, String)> cb) { _updateSuccessCb = cb; }

void QiUpdater::handle() {
    // Check whether we are ready for an update
    if (!_beforeUpdateCb()) return;

    // Check for and download new update
    http.begin(OTA_URL + _macAddress);
    http.GET();

    // extract headers from response
    String XUpdate = http.header(UPDATE_HEADERS[0]);
    String XBuild = http.header(UPDATE_HEADERS[1]);
    String XVersion = http.header(UPDATE_HEADERS[2]);
    String date = http.header(UPDATE_HEADERS[3]);

    // select correct partition to write update to
    int partition;
    if (XUpdate == "spiffs") {
        partition = U_SPIFFS;
    } else if (XUpdate == "firmware") {
        partition = U_FLASH;
    } else {
        logPrint(LOG_LEVEL, INFO, "[Updater] No new Update available\n");
        return;
    }

    // Check update preconditions
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        logPrint(LOG_LEVEL, ERROR, "[Updater] OTA Content-Length not defined\n");
        return;
    }
    if (!Update.begin(contentLength, partition)) {
        logPrint(LOG_LEVEL, ERROR, "[Updater] Not enough space to begin OTA\n");
        return;
    }

    logPrint(LOG_LEVEL, INFO, "[Updater] Starting update of %s\n", XUpdate);

    // Try to install new update
    Client& client = http.getStream();
    Update.writeStream(client);

    if (!Update.end()) {
        logPrint(LOG_LEVEL, ERROR, "[Updater] Error #%d\n", Update.getError());
        return;
    }
    if (!Update.isFinished()) {
        logPrint(LOG_LEVEL, ERROR, "[Updater] Update failed.");
        return;
    }

    // Update successful, notify server via get
    http.setURL(OTA_URL + "success/" + WiFi.macAddress());

    // Act on server response code
    if (http.GET() == HTTP_CODE_NO_CONTENT) {
        logPrint(LOG_LEVEL, INFO, "[Updater] Update successfully completed.\n");
        // type (spiffs, firmware), buildnumber, version identifier, date of update
        _updateSuccessCb(XUpdate, XBuild, XVersion, date);
    }
}

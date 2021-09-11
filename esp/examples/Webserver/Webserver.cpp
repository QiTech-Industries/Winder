#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <LITTLEFS.h>
 
const char* ssid = "WLAN-31CC81";
const char* password =  "5825693716601501";
 
AsyncWebServer server(80);
 
void setup(){
  Serial.begin(115200);
 
  if(!LITTLEFS.begin()){
        Serial.println("An Error has occurred while mounting LITTLEFS");
        return;
  }
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LITTLEFS, "/test.html", "text/html");
  });
 
  server.begin();
}
 
void loop(){}
#include <WiFi.h>;
#include <HTTPClient.h>;
#include <ArduinoJson.h>;

const char* ssid = "AqibLower";
const char* password = "6459693023";

int pr1SensorVal=0;
char jsonOutput[128];

void UpdateSensorValsToServer()
{

  if(WiFi.status() == WL_CONNECTED) {

    HTTPClient client;

    client.begin("http://192.168.100.46:3000/api/machine-info/update");
    
    client.addHeader("Content-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(8);

    StaticJsonDocument<CAPACITY> doc;

    JsonObject object = doc.to<JsonObject>();
    object["lab_id"] = 5;
    object["pr1"] = pr1SensorVal;
    object["pr2"] = 0;
    object["dr1"] = 0;
    object["dr2"] = 0;
    object["ss"] = 0;
    object["monitor"] = 0;
    object["lastUpdatedBy"] = "devcom";
    
    serializeJson(doc, jsonOutput);
    
    int httpCode = client.POST(String(jsonOutput));
    Serial.println("\nStatuscode: " + String(httpCode));
      
    if(httpCode > 0) {
      String payload = client.getString();

      Serial.println(payload);

      client.end();
      
    } else {
      Serial.println("Error on HTTP request");
    }
  } else {
    Serial.println("Connection Lost");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WIFI");

  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to WiFi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
}

void loop() {

/*
 * Read from serial.
 * If any string from serial is received convert it to int and save as PR1 sernsor value. Then update the server
 */
  String serialReadStr = "";
  if(Serial.available() > 0) {
    serialReadStr = Serial.readString();

    Serial.print("PR1 from serial is : ");
    Serial.println(serialReadStr);

    pr1SensorVal = serialReadStr.toInt();

    UpdateSensorValsToServer();
  }
}

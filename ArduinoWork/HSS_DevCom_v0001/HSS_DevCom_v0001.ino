#include <WiFi.h>;
#include <HTTPClient.h>;
#include <ArduinoJson.h>;

const char* ssid = "AqibLower";
const char* password = "6459693023";

char jsonOutput[128];
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

/*
 * {
  lat: 33.569929,
  lon: 73.139711,
  lab_id: 3,
  lab_order: 'CCR',
  pv_id: 1
}
{
  "name": "projects/lab-pv-system/messages/7183543520972824821"
}
*/
void loop() {
  if(WiFi.status() == WL_CONNECTED) {

    HTTPClient client;

    //client.begin("http://jsonplaceholder.typicode.com/comments?id="  + String(rnd));
    //client.begin("http://jsonplaceholder.typicode.com/posts");
    client.begin("http://192.168.100.40:3000/api/pv-info/update");
    
    client.addHeader("Content-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(5);

    StaticJsonDocument<CAPACITY> doc;

    JsonObject object = doc.to<JsonObject>();
    //object["title"] = "Suscribe to Asali";
    object["lat"] = 33.579929;
    object["lon"] = 73.1450;
    object["lab_id"] = 3;
    object["lab_order"] = "CCR";
    object["pv_id"] = 3;

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

  delay(5000);
}

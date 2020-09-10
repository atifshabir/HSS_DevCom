#include <WiFi.h>;
#include <HTTPClient.h>;
#include <ArduinoJson.h>;
#include <RCSwitch.h>

#define MainDOOR      0x13C050 //01294416  
#define MainInDOOR    0xC68346 //13009734  
#define BackDOOR      0x3FC250 //04178512  
#define GalleryDOOR   0xC6CB46 //13028166  ‬
#define DrawingDOOR   0xB3C750 //11781968  ‬
#define KitchenDOOR   0x8ED950 //09361744  ‬
#define TopDOOR       0x388750 //03704656  
#define R1LOCK        0x4B2C01 //04926465
#define R1UNLOCK      0x4B2C02 //04926466
#define R1HOMEARM     0x4B2C04 //04926468
#define R1SOS         0x4B2C08 //04926472

#define R2LOCK        919553
#define R2UNLOCK      919554
#define R2HOMEARM     919556
#define R2SOS         919560

RCSwitch mySwitch = RCSwitch();

const char* ssid = "WiFi";
const char* password = "care@i93";

int pr1SensorVal, pr2SensorVal, dr1Val, dr2Val, ssVal=0;
int monVal=0;
char jsonOutput[128];
enum state{locked, unlocked, alarmed, homeLocked, programDevice, lockedandIdle};
state CurrentState;

void UpdateSensorValsToServer()
{

  if(WiFi.status() == WL_CONNECTED) {

    HTTPClient client;

    client.begin("http://172.16.2.116:3000/api/machine-info/update");
    
    client.addHeader("Content-Type", "application/json");

    const size_t CAPACITY = JSON_OBJECT_SIZE(8);

    StaticJsonDocument<CAPACITY> doc;

    JsonObject object = doc.to<JsonObject>();
    object["lab_id"] = 1;     //HSS ID
    object["pr1"] = pr1SensorVal;
    object["pr2"] = pr2SensorVal;
    object["dr1"] = dr1Val;
    object["dr2"] = dr2Val;
    object["ss"] = ssVal;
    object["monitor"] = monVal;    //lock/unlock
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
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
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

  
  if (mySwitch.available()) {
    

    switch(mySwitch.getReceivedValue()){
      case MainDOOR:
          Serial.println("Main Door Sensor");
          pr1SensorVal = MainDOOR;
          //UpdateSensorValsToServer();
      break;
      case MainInDOOR:
          Serial.println("Main Inside Door Sensor");
      break;
      case BackDOOR:
          Serial.println("Back Door Sensor");
      break;
      case GalleryDOOR:
          Serial.println("Gallery Door Sensor");
      break;
      case DrawingDOOR:
          Serial.println("Drawing Room Door Sensor");
      break;
      case KitchenDOOR:
          Serial.println("Kitchen Door Sensor");
      break;
      case TopDOOR:
          Serial.println("Top Floor Door Sensor");
      break;
      case R2LOCK:
          CurrentState=locked;
          Serial.println("Remote 1: System Armed");
      break;
      case R2UNLOCK:
          CurrentState=unlocked;
          Serial.println("Remote 1: System Disarmed");
          //pr1SensorVal = R1UNLOCK;
          //UpdateSensorValsToServer();
          //Serial.print("PR1 from serial is : ");
          //Serial.println(pr1SensorVal);
      break;
      case R2HOMEARM:
          CurrentState=homeLocked;
          Serial.println("Remote 1: System Home Armed");
          //pr1SensorVal = R1HOMEARM;
          //UpdateSensorValsToServer();
          //Serial.print("PR1 from serial is : ");
          //Serial.println(pr1SensorVal);
      break;
      case R2SOS:
          CurrentState=alarmed;
          Serial.println("Remote 1: SOS PANIC ALARM");
          monVal=1;
          //pr1SensorVal = R1SOS;
          UpdateSensorValsToServer();
          //Serial.print("PR1 from serial is : ");
          //Serial.println(pr1SensorVal);
      break;
      
      
      default:
        Serial.print("Received Unknown ");
        Serial.println( mySwitch.getReceivedValue() );
      
    }
    //Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );

    mySwitch.resetAvailable();
  }

  
  switch(CurrentState){
    case lockedandIdle:

      break;
    case locked:
        Serial.println("System Armed");
        monVal = 1;
        CurrentState=lockedandIdle;
        UpdateSensorValsToServer();
        
      break;
      case unlocked:
        Serial.println("System Disarmed");
        monVal = 0;
        UpdateSensorValsToServer();
      break;
      case homeLocked:
        Serial.println("System HomeArmed");
        monVal = 1;
        UpdateSensorValsToServer();
      break;
      case alarmed:
        Serial.println("Alarm");
        monVal = 1;
        UpdateSensorValsToServer();
      break;
      case programDevice:

      break;
    //default:
    
  }


}

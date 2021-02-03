// Required Libraries
#include <RCSwitch.h>
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoJson.h>;
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// Defining OLED Screen Size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Data Receive Pin on ESP32 from RC Receiver
#define RC_RX_PIN 26

// Hex Codes for Door Sensor, PIR Motion Sensor and Remote Buttons (Note: One PIR Sensor Sends all Three Codes)
#define DOOR_SENSOR 0xA4FB46
#define MOTION_SENSOR_CODE1 0xE6468E
#define MOTION_SENSOR_CODE2 0xE6468F
#define MOTION_SENSOR_CODE3 0xE64686
#define REMOTE_LOCK 0xE0801
#define REMOTE_UNLOCK 0xE0802
#define REMOTE_HOMEARM 0xE0804
#define REMOTE_SOS 0xE0808

// Variables to Keep Record of Current State
bool home_lock = 0, home_unlock = 0, home_arm = 0;

// Wi-Fi SSID and Password for Server Communication
const char* ssid     = "Rehan";
const char* password = "1234554321";

// Variables to Keep Record of Sensor Triggers
int pr1_val = 0, pr2_val = 0, dr1_val = 1, dr2_val = 0, ss_val=0;
// Variable to Keep Record of Device Status on Server Side
int mon_val = 0;
// Variable to Store Home Number
int home_number = 5;

char jsonOutput[128];


// Object Declaration and Initilization for RC Receiver
RCSwitch mySwitch = RCSwitch();

// Object Declaration and Initialization for OLED Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void update_server_home_state() // Function to Send Home State to Server
{
  HTTPClient client;
  client.begin("http://148.66.129.239:3000/api/machine-info/update");
  client.addHeader("Content-Type", "application/json");
  const size_t CAPACITY = JSON_OBJECT_SIZE(9);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
    
  object["hss_id"] = "ab:12:23:45:ab:02";
  object["lab_id"] = home_number;
  object["pr1"] = pr1_val;
  object["pr2"] = pr2_val;
  object["dr1"] = dr1_val;
  object["dr2"] = dr2_val;
  object["ss"] = ss_val;
  object["monitor"] = mon_val;
  object["lastUpdatedBy"] = "lca";
        
  serializeJson(doc, jsonOutput);
    
  int httpCode = client.POST(String(jsonOutput));
  Serial.println("\nStatuscode: " + String(httpCode));
      
  String payload = client.getString();
  Serial.println(payload);
  client.end();     
}

void update_server_sensor_trigger() // Function to Send Sensor Trigger to Server
{
  HTTPClient client;
  client.begin("http://148.66.129.239:3000/api/machine-info/update");
  client.addHeader("Content-Type", "application/json");
  const size_t CAPACITY = JSON_OBJECT_SIZE(9);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
    
  object["hss_id"] = "ab:12:23:45:ab:02";
  object["lab_id"] = home_number;
  object["pr1"] = pr1_val;
  object["pr2"] = pr2_val;
  object["dr1"] = dr1_val;
  object["dr2"] = dr2_val;
  object["ss"] = ss_val;
  object["monitor"] = mon_val;
  object["lastUpdatedBy"] = "devcom";
        
  serializeJson(doc, jsonOutput);
    
  int httpCode = client.POST(String(jsonOutput));
  Serial.println("\nStatuscode: " + String(httpCode));
      
  String payload = client.getString();
  Serial.println(payload);
  client.end();     
}

void peripherals_setup() // Function to Setup Different Peripherals and Communication Protocols
{
  Wire.begin(5, 4); // I2C for OLED Display
  pinMode(RC_RX_PIN, INPUT_PULLUP); // Need to Pull Up Resistor for Interrupt to Work
  mySwitch.enableReceive(RC_RX_PIN); // Set Pin as RC Receive Pin
}

void oled_setup() // Function to Setup OLED and Display Text on Startup
{
  // Check OLED Display Connected 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000); // Delay for OLED Display to Initialize
  // Starting Text on OLED
  display.setFont(&FreeMonoBold12pt7b);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(30, 20);             
  display.println("SMART");
  display.setCursor(15, 40);
  display.println("MUHAFIZ");
  display.setCursor(15, 55);
  display.setFont();
  display.setTextSize(1.2);
  display.setTextColor(WHITE);
  display.println("WiFi Connecting...");
  display.display();
}

void wifi_connect() // Function to Setup Wifi Connectivity
{
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) 
  {
    
  }
  display.setFont(&FreeMonoBold12pt7b);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(30, 20);             
  display.println("SMART");
  display.setCursor(15, 40);
  display.println("MUHAFIZ");
  display.setCursor(15, 55);
  display.setFont();
  display.setTextSize(1.2);
  display.setTextColor(WHITE);
  display.println("WiFi Connected!");
  display.display();
  
}

void back_to_state() // Function to go Back to Original State
{
  if(home_lock == 1)
  {
    home_locked();
  }
  else if(home_unlock == 1)
  {
    home_unlocked();
  }
  else if(home_arm == 1)
  {
    home_armed();
  }
  else
  {
    wifi_connect();
  }
}
 
void home_locked() // Function to Handle Home Locked Functionality
{
  home_lock = 1;
  home_unlock = 0;
  home_arm = 0;

  display.setFont(&FreeMonoBold12pt7b);
  display.stopscroll();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.fillRect(0, 0, 128, 10, WHITE);
  display.setCursor(35, 30);             
  display.println("HOME");
  display.setCursor(22, 50);
  display.println("LOCKED");
  display.fillRect(0, 55, 128, 9, WHITE);
  display.display();
  display.startscrollleft(0x00, 0x0F);

  mon_val = 1;
  update_server_home_state();
}

void home_unlocked() // Function to Handle Home Unlocked Funtionality
{
  home_lock = 0;
  home_unlock = 1;
  home_arm = 0;

  display.setFont(&FreeMonoBold12pt7b);
  display.stopscroll();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.fillRect(0, 0, 128, 10, WHITE);
  display.setCursor(35, 30);             
  display.println("HOME");
  display.setCursor(7, 50);
  display.println("UNLOCKED");
  display.fillRect(0, 55, 128, 9, WHITE);
  display.display();
  display.startscrollleft(0x00, 0x0F);

  pr1_val = 0;
  pr2_val = 0;
  dr1_val = 0;
  dr2_val = 0;
  ss_val = 0;
  update_server_sensor_trigger();
  mon_val = 0;
  update_server_home_state();
}

void home_armed() // Function to Handle Armed Functionality
{
  home_lock = 0;
  home_unlock = 0;
  home_arm = 1;

  display.setFont(&FreeMonoBold12pt7b);
  display.stopscroll();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.fillRect(0, 0, 128, 10, WHITE);
  display.setCursor(35, 30);             
  display.println("HOME");
  display.setCursor(27, 50);
  display.println("ARMED");
  display.fillRect(0, 55, 128, 9, WHITE);
  display.display();
  display.startscrollleft(0x00, 0x0F);

  mon_val = 1;
  update_server_home_state();
}

void sos_pressed() // Function to Handle SOS Button Press 
{
  display.setFont(&FreeMonoBold18pt7b);
  display.stopscroll();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.fillRect(0, 0, 128, 10, WHITE);
  display.setCursor(35, 43);             
  display.println("SOS!");
  display.fillRect(0, 55, 128, 9, WHITE);
  display.display();
  display.startscrollleft(0x00, 0x0F);
  delay(5000);
  back_to_state();
}

void setup() 
{
  Serial.begin(9600); // Serial Communication to PC
  peripherals_setup();
  oled_setup();
  wifi_connect();
}

void loop() 
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (mySwitch.available())
    {
      switch(mySwitch.getReceivedValue())
      {
        case REMOTE_LOCK:
          home_locked();
          break;
          
        case REMOTE_UNLOCK:
          home_unlocked();
          break;
          
        case REMOTE_HOMEARM:
          home_armed();
          break;
          
        case REMOTE_SOS:
          sos_pressed();
          break;
          
        case DOOR_SENSOR:
          if(home_lock or home_arm)
          {
            dr1_val = 1;
            update_server_sensor_trigger();
            Serial.println("Door Sensor!");
          }
          break;
          
        case MOTION_SENSOR_CODE1:
          if(home_lock)
          {
            pr1_val = 1;
            update_server_sensor_trigger();
            Serial.println("Motion Detected!");
          }
          break;
          
        case MOTION_SENSOR_CODE2:
          if(home_lock)
          {
            pr1_val = 1;
            update_server_sensor_trigger();
            Serial.println("Motion Detected!");
          }
          break;
          
        case MOTION_SENSOR_CODE3:
          if(home_lock)
          {
            pr1_val = 1;
            update_server_sensor_trigger();
            Serial.println("Motion Detected!");
          }
          break;
          
        default:
          Serial.println("Unknown Code!");
          break;  
      }
      mySwitch.resetAvailable();
    } 
  }
  else
  {
    oled_setup();
    wifi_connect();
    back_to_state();
  }
}

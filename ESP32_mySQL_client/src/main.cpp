#include <Arduino.h>
#include <FS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

//define your default values here, if there are different values in config.json, they are overwritten.
char mysql_server_ip[40] = "192.168.0.30";
char mysql_server_port[6] = "3306";
char mysql_server_username[50] = "defaultUser";
char mysql_server_password[50] = "1";
char mysql_server_database[50] = "DefaultDB";
char mysql_server_table[50] = "OutdoorTemps";
char mysql_server_topic[50] = "home/outdoor/terrace/temps";
char mysql_server_identifier[50] = "TempReader1";
          
//flag for saving data
bool shouldSaveConfig = false;
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value;


const int oneWireBusMain = 17;     
OneWire oneWireMain(oneWireBusMain);
DallasTemperature sensorMain(&oneWireMain);

const int oneWireBusAux1 = 15;     
OneWire oneWireAux1(oneWireBusAux1);
DallasTemperature sensorAux1(&oneWireAux1);

const int oneWireBusAux2 = 16;     
OneWire oneWireAux2(oneWireBusAux2);
DallasTemperature sensorAux2(&oneWireAux2);
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



ulong id = 0;
float temp1;
float temp2;
float temp3;


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
    }
  }
}


void setup() {
    
    sensorMain.begin();
  sensorAux1.begin();
  sensorAux2.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Booting...");
  display.display();


    Serial.begin(115200);
    Serial.println();
    delay(100);
    
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    

    //clean FS, for testing
    //SPIFFS.format();
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around

    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);

  #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
      DynamicJsonDocument json(1024);
      auto deserializeError = deserializeJson(json, buf.get());
      serializeJson(json, Serial);
      if ( ! deserializeError ) {
#else
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
#endif
        Serial.println("\nparsed json");
        strcpy(mysql_server_ip, json["mysql_server_ip"]);
        strcpy(mysql_server_port, json["mysql_server_port"]);
        strcpy(mysql_server_username, json["mysql_server_username"]);
        strcpy(mysql_server_password, json["mysql_server_password"]);
        strcpy(mysql_server_database, json["mysql_server_database"]);
        strcpy(mysql_server_table, json["mysql_server_table"]);
        strcpy(mysql_server_topic, json["mysql_server_topic"]);
        strcpy(mysql_server_identifier, json["mysql_server_identifier"]);
        } 
        else 
        {
          Serial.println("failed to load json config");
        }
        configFile.close();
        }
      }
    } else {
      Serial.println("failed to mount FS");
    }
  //end read

    
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mysql_server_ip("server", "MySql-Server:", mysql_server_ip, 40);
  WiFiManagerParameter custom_mysql_server_port("port", "MySql-Port:", mysql_server_port, 6);
  WiFiManagerParameter custom_mysql_server_username("username", "MySql-Username:", mysql_server_username, 50);
  WiFiManagerParameter custom_mysql_server_password("password", "MySql-Password:", mysql_server_password, 50);
  WiFiManagerParameter custom_mysql_server_database("database", "MySql-Database:", mysql_server_database, 50);
  WiFiManagerParameter custom_mysql_server_table("table", "MySql-Table:", mysql_server_table, 50);
  WiFiManagerParameter custom_mysql_server_topic("topic", "MySql-Topic:", mysql_server_topic, 50);
  WiFiManagerParameter custom_mysql_server_identifier("identifier", "MySql-Identifier:", mysql_server_identifier, 50);

    
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_mysql_server_ip);
  wifiManager.addParameter(&custom_mysql_server_port);
  wifiManager.addParameter(&custom_mysql_server_username);
  wifiManager.addParameter(&custom_mysql_server_password);
  wifiManager.addParameter(&custom_mysql_server_database);
  wifiManager.addParameter(&custom_mysql_server_table);
  wifiManager.addParameter(&custom_mysql_server_topic);
  wifiManager.addParameter(&custom_mysql_server_identifier);
    
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //if (!wifiManager.autoConnect("ESP32_Config")) {
  //  staticDisplay.println("failed to connect and hit timeout");
  //  delay(3000);
  //  //reset and try again, or maybe put it to deep sleep
  //  ESP.restart();
  //  delay(5000);
  //}

  //if you get here you have connected to the WiFi
  Serial.println("connected to Wifi");


//read updated parameters
  strcpy(mysql_server_ip, custom_mysql_server_ip.getValue());
  strcpy(mysql_server_port, custom_mysql_server_port.getValue());
  strcpy(mysql_server_username, custom_mysql_server_username.getValue());
  strcpy(mysql_server_password, custom_mysql_server_password.getValue());
  strcpy(mysql_server_database, custom_mysql_server_database.getValue());
  strcpy(mysql_server_table, custom_mysql_server_table.getValue());
  strcpy(mysql_server_topic, custom_mysql_server_topic.getValue());
  strcpy(mysql_server_identifier, custom_mysql_server_identifier.getValue());
  Serial.println("The values in the file are: ");
  Serial.println("IP: "+String(mysql_server_ip));
  Serial.println("Port: "+String(mysql_server_port));
  Serial.println("Username : "+String(mysql_server_username));
  Serial.println("Password: "+String(mysql_server_password));
  Serial.println("Database: "+String(mysql_server_database));
  Serial.println("Table: "+String(mysql_server_table));
  Serial.println("Topic: "+String(mysql_server_topic));
  Serial.println("Idenfitier: "+String(mysql_server_identifier));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["mysql_server_ip"] = mysql_server_ip;
    json["mysql_server_port"] = mysql_server_port;
    json["mysql_server_username"] = mysql_server_username;
    json["mysql_server_password"] = mysql_server_password;
    json["mysql_server_database"] = mysql_server_database;
    json["mysql_server_table"] = mysql_server_table;
    json["mysql_server_topic"] = mysql_server_topic;
    json["mysql_server_identifier"] = mysql_server_identifier;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP().toString());

  client.setServer(mysql_server_ip, 1883);
  client.setCallback(callback);
}

void reconnect() {
  // Loop until we're reconnected
  if(!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.println("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float cnt;
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
   sensorMain.requestTemperatures();
    temp1 = sensorMain.getTempCByIndex(0);
    sensorAux1.requestTemperatures();
    temp2 = sensorAux1.getTempCByIndex(0);
    sensorAux2.requestTemperatures();
    temp3 = sensorAux2.getTempCByIndex(0);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("1: ");
    display.print(temp1);
    display.println("'C");
    display.print("2: ");
    if(temp2 == -127) display.println("n/A");
    else{
     display.print(temp2);
      display.println("'C");
   }
    display.print("3: ");
    if(temp3 == -127) display.println("n/A");
    else{
      display.print(temp3);
      display.println("'C");
    }


    DynamicJsonDocument json(1024);
    json["LogId"] = id;
    json["SensorData"]["Sensor1"] = temp1;
    json["SensorData"]["Sensor2"] = temp2;
    json["SensorData"]["Sensor3"] = temp3;
    char str[1024];
    serializeJsonPretty(json, str);
    display.display();
    client.publish(mysql_server_topic, str);
    id++;
    
    Serial.print("Temperatur 1: ");
    Serial.println(temp1);
  }
}

#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ir_Samsung.h> // This library is part of https://github.com/crankyoldgit/IRremoteESP8266

#ifndef LED_BUILTIN
#define LED_BUILTIN
#endif

String device_air_conditioner = "25k"; // this value you get when defining the device in front-end app
                                       // you can also check it in firestore database, change to your own device id
char ssid[] = "xxxxx";   //  your network SSID (name)
char pass[] = "xxxxx"; // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;

// MQTT info
const char *thehostname = "postman.cloudmqtt.com"; // you can use the same broker or any one that you define in your Java application should be the same here
const char *user = "xxxxx";
const char *user_password = "xxxx";
const char *id = "Wemos-Air-Conditioner";

const uint16_t kIrLed = 14;   // ESP8266 GPIO pin to use. D5 Wemos D1, this can also be changed according to your needs
IRSamsungAc ac(kIrLed); // Set the GPIO used for sending messages.


void setup()
{
  ac.begin();
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  //Setup Rest Client
  WiFi.begin(ssid, pass);

  pinMode(LED_BUILTIN, OUTPUT);

  ac.off();
  ac.setFan(kSamsungAcFanLow);
  ac.setMode(kSamsungAcCool);
  ac.setTemp(23);
  ac.setSwing(false);
  printState();
  delay(500);

  client.begin(thehostname, 16157, net);
  client.onMessage(messageReceived);
  connect();
  delay(1000);

}

void connect()
{
  Serial.print("checking wifi…");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting…");
  while (!client.connect(id, user, user_password))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  client.subscribe(device_air_conditioner + "-client"); //Air Conditioner device
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(payload);
  
  String deviceOn = json["on"];
  int thermostatTemperatureSetpoint = json["thermostatTemperatureSetpoint"];
  String thermostatMode = json["thermostatMode"];
  String currentModeSettings = json["currentModeSettings"]["mode"];

  if (topic == (device_air_conditioner + "-client"))
  { 
    if (deviceOn == "true")
    {
      Serial.println("Turning on the A/C ...");
      ac.on();
      ac.send();
      printState();
    }
    if (deviceOn == "false")
    {
      Serial.println("Turning off the A/C ...");
      ac.off();
      ac.send();
      printState();
    }
    if (thermostatTemperatureSetpoint > 0)
    {
      Serial.println("Setting temperature in A/C to ... " + thermostatTemperatureSetpoint);
      ac.setTemp(thermostatTemperatureSetpoint);
      ac.send();
      printState();
    }
    if (thermostatMode == "cool"){
      Serial.println("Setting A/C mode to ... " + thermostatMode);
      ac.setMode(kSamsungAcCool);
      ac.send();
      printState();
    }
    if (thermostatMode == "heat"){
      Serial.println("Setting A/C mode to ... " + thermostatMode);
      ac.setMode(kSamsungAcHeat);
      ac.send();
      printState();
    }
    if (currentModeSettings == "quiet"){
      Serial.println("Setting A/C mode to ... " + currentModeSettings);
      ac.setQuiet(true);
      ac.send();
      printState();
    }
    if (currentModeSettings == "auto"){
      Serial.println("Setting A/C mode to ... " + currentModeSettings);
      ac.setMode(kSamsungAcAuto);
      ac.send();
      printState();
    }
     blinkLed();
  } // end topic Air Conditioner

}

void loop()
{
  client.loop();
  if (!client.connected())
  {
    connect();
  }
  
}

void printState()
{
  // Display the settings.
  Serial.println("Samsung A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
}

void blinkLed(){
   for (int i = 0; i <= 10; i++) {
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

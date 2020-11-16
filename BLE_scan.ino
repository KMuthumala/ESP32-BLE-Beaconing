/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "WiFi.h"


const char* ssid = "KRISH";
const char* password = "123456789";
const char* mqtt_server = "mqtt.eclipse.org";
const char ledPin = 2;
const char outgoingTopic[30] = "EN3250/ESP32";
const char incomingTopic[30] = "test_in";
bool sent = LOW;
char msg[1000];
int scanTime = 3; //In seconds

BLEScan* pBLEScan;
WiFiClient espClient;
PubSubClient client(espClient);

// Setup wifi connecttion with the access point
void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());

  Serial.println();
  Serial.println("WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}

// reconnect if connection failure
void reconnectWifi() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-25";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      //client.publish(outgoingTopic, "Reconnection Succeeded!");
      client.subscribe(incomingTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Connection with MQTT server(Messege arriving)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// BLE beacon Scanning function
void scan_ble() {
  String dump = "{\"id\":11,";
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  int count = foundDevices.getCount();
  Serial.print("Devices found: ");
  //root["Devices"]=count;
  Serial.println(count);
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice d = foundDevices.getDevice(i);
    if (i != 0) {
      dump += ",";
    }
    dump += "\"";
    dump += d.getAddress().toString().c_str();
    dump += "\"";
    dump += ":";
    dump += d.getRSSI();
    delay(10);
  }
  dump += "}";
  dump.toCharArray(msg, 500000);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(200);

  sent = client.publish(outgoingTopic, msg);
  if (sent) {
    Serial.print("Published: ");
    Serial.println(msg);
  } else {
    Serial.print("Failed: ");
    Serial.println(msg);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(100);
  digitalWrite(ledPin, HIGH);

  if (!client.connected()) {
    reconnectWifi();
  }
  client.loop();
  scan_ble();
  delay(2000);
}

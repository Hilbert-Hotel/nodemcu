#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "connector.h"

const long utcOffsetInSeconds = 25200;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define LED_PIN LED_BUILTIN
#define D1 5
#define D2 4
#define D5 14
#define D6 12

char *doorStatus = "";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void logging(String function, String message)
{
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject &JSONencoder = JSONbuffer.createObject();
  timeClient.update();
  /* Creating json logging */
  JSONencoder["from"] = "mqtt";
  JSONencoder["function"] = function;
  JSONencoder["message"] = message;
  JSONencoder["timestamp"] = timeClient.getEpochTime();
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  HTTPClient http;
  http.begin(log_url, sha_1);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(JSONmessageBuffer);
  String payload = http.getString();
  Serial.println(httpCode);
  http.end();
}

void setup_wifi()
{

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  int i = 0;

  while (i < length)
  {
    msg += (char)payload[i++];
  }

  Serial.print("receive ");
  Serial.println(msg);

  if (msg == "unlock")
  {
    doorUnlock();
  }
  else if (msg == "lock")
  {
    doorLock();
  }
  else if (msg == "sound")
  {
    soundOn();
  }
  else if (msg == "status")
  {
    client.publish("doorStatus", doorStatus);
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client", mqtt_username, mqtt_password))
    {
      Serial.println("connected");

      client.publish("mqtt", "Connected");

      client.subscribe("door/1");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void doorLock()
{
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);

  doorStatus = "Lock";
  logging("doorStatus", doorStatus);
  client.publish("doorStatus", doorStatus);
}

void soundOn()
{
  logging("doorStatus", "sound");
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(D5, HIGH);
    delay(500);
    digitalWrite(D5, LOW);
    delay(500);
  }
}

void doorUnlock()
{
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  doorStatus = "Unlock";
  logging("doorStatus", doorStatus);
  client.publish("doorStatus", doorStatus);
  delay(3000);
  doorLock();
}

void setup()
{
  timeClient.begin();
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 14918);
  client.setCallback(callback);
  doorLock();
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }

  if (digitalRead(D6) == LOW)
  {
    Serial.println("btn Click");
    doorUnlock();
    delay(300);
  }
  client.loop();
}

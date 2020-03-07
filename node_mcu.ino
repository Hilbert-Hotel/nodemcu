#include <ESP8266WiFi.h>
#include "connector.h"
#include <PubSubClient.h>


const char * topic = "door"; 
#define mqtt_server "m15.cloudmqtt.com" 
#define mqtt_port 14918 


#define LED_PIN LED_BUILTIN
#define D1 5
#define D2 4
#define D5 14


char *led_status = "OFF";
char *doorStatus="";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    pinMode(D1,OUTPUT);
    pinMode(D2,OUTPUT);
    pinMode(D5,OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        }   
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

   
    client.setServer(mqtt_server, mqtt_port); 
    client.setCallback(callback);
    doorLock();
}
void doorLock(){
   doorStatus="Lock";
   digitalWrite(D1, HIGH);
   digitalWrite(D2, HIGH);
}

void soundOn(){
   for (int i = 0; i <= 5; i++) {
   digitalWrite(D5,HIGH);
   delay(500);                  
  digitalWrite(D5, LOW);
  delay(500);
   }
}

void doorUnlock(){
  doorStatus="Unlock";
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
}
 
   

void loop() {
  if (!client.connected()) {
    Serial.print("MQTT connecting...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
    client.subscribe(topic);
    Serial.println("connected");
    }
  
  else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000); 
    return;
    }
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message from ");
  Serial.println(topic);
  
  String msg = "";
  int i=0;
  
  while (i < length) {
    msg += (char)payload[i++];
    }
    
  Serial.print("receive ");
  Serial.println(msg);

  if (msg == "unlock") { 
  doorUnlock();
  } 
  else if (msg == "lock") {
  doorLock();
  }
  else if (msg=="sound"){
    soundOn();
  }
  else if(msg=="status"){
  client.publish("doorStatus", doorStatus);
  }

  
}

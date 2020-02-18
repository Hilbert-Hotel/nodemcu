#include <ESP8266WiFi.h>
#include <ezButton.h>
#include "connector.h"
 
const char* ssid = wifiName;
const char* password = wifiPassword;
 
#define ledPin LED_BUILTIN
#define D1 5
#define D2 4


//Btn
//#define D5 14
ezButton button(14);

WiFiServer server(80);

void setup() {
   
  Serial.begin(115200);
  delay(10);
 
  pinMode(ledPin, OUTPUT);
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
 // pinMode(D5,INPUT);
  digitalWrite(ledPin, LOW);
  doorLock();
 
  // Connect to WiFi network
  Serial.println();
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
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}

void doorLock(){
   digitalWrite(D1, HIGH);
   digitalWrite(D2, HIGH);
}


void doorUnlock(){
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
}
 
void loop() {
  // button.loop();
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();


 // Client Unlock door
  if (request.indexOf("/Unlock") != -1)  {
    Serial.println("UNLOCK");
    doorUnlock();
    delay(10000);
    doorLock();   

  }


  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<a href=\"/Unlock\"\"><button>Unlock Door</button></a>");
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

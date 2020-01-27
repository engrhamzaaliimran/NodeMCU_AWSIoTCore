/*
 * This Code will listen to a topic $aws/things/RPI3/shadow/update on AWS IoT Core
 * and Will turn on the corresponding LED depending upon the the value of key "color"
 * The value of this key can be "Red", "Green", "Blue" or "NoColor"
 */
#define MQTT_MAX_PACKET_SIZE 512

#include "FS.h"
#include  <ESP8266WiFi.h>
#include  <PubSubClient.h>         
#include  <NTPClient.h>             
#include  <WiFiUdp.h>
#include <String>

#define led1 14
#define led2 12
#define led3 13

void led_blink(char light)
{
  if(light=='R')
  {
  Serial.println("Red light on");
  digitalWrite(led1, !HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(400);                       // wait for a second
  digitalWrite(led1, !LOW);    // turn the LED off by making the voltage LOW
  delay(400);   
  }
  if(light=='G')
  {
  Serial.println("Green light on");
  digitalWrite(led2, !HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(400);                       // wait for a second
  digitalWrite(led2, !LOW);    // turn the LED off by making the voltage LOW
  delay(400);  
  }
  if(light=='B')
  {
  Serial.println("Blue light on");
  digitalWrite(led3, !HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(400);                       // wait for a second
  digitalWrite(led3, !LOW);    // turn the LED off by making the voltage LOW
  delay(400); 
  }
  if(light=='N')
  {
   Serial.println("No Color Found");
  }
}

const char* ssid = "XXX-AP2";
const char* password = "XXX-AP2";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* AWS_endpoint = "XXXXXXX.amazonaws.com"; //MQTT broker ip

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char * temp;
  temp=(char*)payload;//converting to char array
  String aStringObject = temp;
  int index=aStringObject.indexOf("color"); // returns the index of c in string where color occurs

  led_blink((char)payload[index+9]); //it is the first character of color key value
  
  
 for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]) ;// Pring payload content
  }
  
  Serial.println();
}
WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set  MQTT port number to 8883 as per //standard
long lastMsg = 0;
char msg[1000];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
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

  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("RPI3")) {
      Serial.println("AWS-connected");
      // Once connected, publish an announcement...
      client.publish("d2s", "hello world from nodemcu");
      // ... and resubscribe
      client.subscribe("$aws/things/MyThing/shadow/update");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  pinMode(led1, OUTPUT);//setting pins output
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  digitalWrite(led1, !LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(led2, !LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(led3, !LOW);    // turn the LED off by making the voltage LOW

  Serial.begin(9600);
  Serial.setDebugOutput(true);
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(D5, OUTPUT);
  setup_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");



    // Load CA file
    File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
    if (!ca) {
      Serial.println("Failed to open ca ");
    }
    else
    Serial.println("Success to open ca");

    delay(1000);

    if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
    else
    Serial.println("ca failed");

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

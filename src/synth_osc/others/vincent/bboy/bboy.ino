/*---------------------------------------------------------------------------------------------
    _     ____       _  ____  _     ____  _  ____
   / \   /  _ \     / |/  _ \/ \ /\/  __\/ \/  _ \
   | |   | / \|     | || / \|| | |||  \/|| || / \|
   | |_/\| |-||  /\_| || |-||| \_/||    /| || |-||
   \____/\_/ \|  \____/\_/ \|\____/\_/\_\\_/\_/ \|
  ESP8266 bundle receive from PD
  Creado por la Jauria
  REV Alex vargas Benanburg                                             *
-------------------------------------------------------------------------------------------------
  Open Sound Control (OSC) library for the ESP8266
  Example for receiving open sound control (OSC) bundles on the ESP8266
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.
  This example code is in the public domain.
--------------------------------------------------------------------------------------------- */
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "esthetopies";          // your network SSID (name)
char pass[] = "esthetopies";                    // your network password
const IPAddress ip(192, 168, 0, 127);     // IP para este dispositivo
const IPAddress gateway(192, 168, 0, 1);  // IP gateway, normalmente es la del router
const IPAddress subnet(255, 255, 255, 0); // Mascara de subred


// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 8266;        // local port to listen for UDP packets (here's where we send the packets)



OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, 0);    // turn *on* led

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());

  

}


void motor(OSCMessage &msg) {
  int pin = msg.getInt(0);
  int pwm = msg.getInt(1);
  analogWrite(pin, pwm);
  Serial.print("Motor: ");
  Serial.print(pin);
  Serial.print(" : ");
  Serial.println(pwm);
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {
      bundle.dispatch("/motor", motor);
    } else {
      error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}




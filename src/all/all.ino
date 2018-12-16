  /*  NOTES
    (1)  Do not use Serial.begin() when sending output audio,
         it seems that in that case Rx serial output may not available.
         Only used for DEBUGging purposes.
    (2)  ESP8266 I2S audio interface expects int values encoded with 16 bit depth,
         so output of updateAudio() method must be between -32768 and 32768  (2**16 = 65536).
    (3)  Oscil.setFreq() method doesn't look to like double values as argument
*/

#include <MozziGuts.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>


/// WiFi - UDP ///
const char* ssid = "TP-Link_BED2";
const char* password = "73385615";

WiFiUDP Udp;
const unsigned int localUdpPort = 8266;   // local port to listen for UDP packets
const IPAddress ip(192, 168, 0, 12);     // IP para este dispositivo
const IPAddress gateway(192, 168, 0, 1);  // IP gateway, normalmente es la del router
const IPAddress subnet(255, 255, 255, 0); // Mascara de subred
OSCErrorCode error;

void connect_wifi(){
  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_udp(){
  Serial.println("Starting UDP");
  Udp.begin(localUdpPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
}

/// Mozzi Synth /// 
#define CONTROL_RATE 8 // 64 // in Hz, must be a power of 2 

/// Kuramoto model ///

const double dT = 1.0/(float (CONTROL_RATE));     // model's time delta in seconds

int NB_NEIGHS = 6;
double in_value;
char in_type;

typedef struct {
    IPAddress ip;
    double val;
    int port;
} Node;

Node Neighs[6] = {
  {IPAddress(192, 168, 0, 11), 0.0, 8266},
  {IPAddress(192, 168, 0, 13), 0.0, 8266},
  {IPAddress(192, 168, 0, 14), 0.0, 8266},
  {IPAddress(192, 168, 0, 15), 0.0, 8266},
  {IPAddress(192, 168, 0, 113), 0.0, 5005},  // for monitoring 
  {IPAddress(192, 168, 0, 116), 0.0, 5005},  // for monitoring 
};

const double W = 1.0;  // internal frequency
const double K = 1.0;  // coupling constant
double val = 0.0;  // internal value

/// debug ///
const int PPFPRE = 10; // precision print
int BAUD_RATE = 9600;
bool DEBUG = true;

void show_params(){
    Serial.println("Kuramoto model parameters:");
    Serial.printf("\t w: ");
    Serial.println((double)(W),PPFPRE);
    Serial.printf("\t k: ");
    Serial.println((double)(K),PPFPRE);
}

void show_neighs(){
  Serial.printf("Nodes:\n", WiFi.localIP().toString().c_str(), localUdpPort);
  for (int i=0; i<NB_NEIGHS; i++){
    Serial.printf("\t %d : ip %s | port %d | val ", i, Neighs[i].ip.toString().c_str(), Neighs[i].port);
    Serial.println((double)(Neighs[i].val),PPFPRE);
  }
}

void show_this(){
  Serial.printf("\t x : ip %s | port %d | val ", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.println((double)(val), PPFPRE);
}

void show_all(){
  show_neighs();
  show_this();
}

/// main ///

// OSCMessage expected to be format as /callback int int int int int int
// with int's corresponding to amp0 freq0 amp1 freq1 amp2 freq2
void oscillators(OSCMessage &msg) {
}

void neighbor(OSCMessage &msg) {
  in_type = msg.getType(0);
  if(in_type=='i'){
    in_value = (double) msg.getInt(0);
  }
  else if(in_type=='f'){
    in_value = (double) msg.getFloat(0);
  }
  else{
    Serial.printf("Wrong type `%s` in received in neighbor callback. Must be `i` or `f`.", in_type);
    return;
  }
  if(DEBUG){
    Serial.printf("Updating neighbor %s with value ", Udp.remoteIP().toString().c_str());
    Serial.println((double)(in_value), PPFPRE);
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP() == Neighs[i].ip) {
      Neighs[i] = (Node){Neighs[i].ip, in_value, Neighs[i].port};
    }
  };
}

 
// update internal value with Kuramoto model
void update_value(){
  double tmp = 0.0;
  for (int i=0; i<NB_NEIGHS; i++){
    tmp += (K * sin(Neighs[i].val - val));
  };
  val += dT * (W + tmp);
  if(DEBUG){
    Serial.printf("New interna value ");
    Serial.print((double)(val), PPFPRE);
    Serial.println(".");
  }
}

// send internal value to neighbors
void send_value(){

  OSCMessage msg("/neighbor");
  msg.add(val);

  if(DEBUG){
    Serial.printf("Sending value ");
    Serial.print((double)(val), PPFPRE);
    Serial.println(" to  neighbors.\n");
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  for (int i=0; i<NB_NEIGHS; i++){
    Udp.beginPacket(Neighs[i].ip, Neighs[i].port);
    msg.send(Udp);
    Udp.endPacket();
  }
  msg.empty();
}

// receive incoming UDP packets
int read_buffer(){
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()){
      bundle.dispatch("/oscillators", oscillators);
      bundle.dispatch("/neighbor", neighbor);
      update_value();
      send_value();
    } else {
      error = bundle.getError();
      Serial.print("OSCBundle error: ");
      Serial.println(error);
    }
  }
}

/// main ///

// This runs CONTROL_RATE times per second.
void updateControl(){
    read_buffer();
}

// This runs on average 16384 times per second, so code here needs to be lean.
int updateAudio(){
  return 0;
}

void setup(){
  /*
      /!\ WARNING: is next line is not commented may no be audio output /!\
  */
  Serial.begin(BAUD_RATE);

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage HIGH

  connect_wifi();
  connect_udp();

  show_all();
  show_params();

  // start synth engine
  startMozzi(CONTROL_RATE);
}


void loop(){
  audioHook(); // required here, it fills output buffer (256 cells > maximum latency of about 15 ms)
}




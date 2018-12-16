  /*  NOTES
    (1)  Do not use Serial.begin() when sending output audio,
         it seems that in that case Rx serial output may not available.
         Only used for DEBUGging purposes.
    (2)  ESP8266 I2S audio interface expects int values encoded with 16 bit depth,
         so output of updateAudio() method must be between -32768 and 32768  (2**16 = 65536).
    (3)  Oscil.setFreq() method doesn't look to like float values as argument
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/cos8192_int8.h> // waveform table

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
const IPAddress ip(192, 168, 0, 14);     // IP para este dispositivo
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

// oscillators amplitudes
int8_t a0, a1, a2;

// oscillators (tables with int8_t values between -128 and 127)
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos0(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2(COS8192_DATA);

/// Kuramoto model ///
int NB_NEIGHS = 6;
float in_value;
char in_type;

typedef struct {
    IPAddress ip;
    float val;
    int port;
} Node;

Node Neighs[6] = {
  {IPAddress(192, 168, 0, 11), 0.0, 8266},
  {IPAddress(192, 168, 0, 12), 0.0, 8266},
  {IPAddress(192, 168, 0, 13), 0.0, 8266},
  {IPAddress(192, 168, 0, 15), 0.0, 8266},
  {IPAddress(192, 168, 0, 113), 0.0, 5005},  // for monitoring 
  {IPAddress(192, 168, 0, 116), 0.0, 5005},  // for monitoring 
};

const float dT = 1.0/(float (CONTROL_RATE)); // model's time delta in seconds
const float W = 1.0;  // internal frequency
const float K = 1.0;  // coupling constant
float val = 0.0;  // internal value

/// debug ///
const int PPFPRE = 10; // precision print
int BAUD_RATE = 9600;
bool DEBUG = false;

void show_params(){
    Serial.println("Kuramoto model parameters:");
    Serial.printf("\t w: ");
    Serial.println((float)(W),PPFPRE);
    Serial.printf("\t k: ");
    Serial.println((float)(K),PPFPRE);
}

void show_neighs(){
  Serial.printf("Nodes:\n", WiFi.localIP().toString().c_str(), localUdpPort);
  for (int i=0; i<NB_NEIGHS; i++){
    Serial.printf("\t %d : ip %s | port %d | val ", i, Neighs[i].ip.toString().c_str(), Neighs[i].port);
    Serial.println((float)(Neighs[i].val),PPFPRE);
  }
}

void show_this(){
  Serial.printf("\t x : ip %s | port %d | val ", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.println((float)(val), PPFPRE);
}

void show_all(){
  show_neighs();
  show_this();
}

/// main ///

// oscillators OSCMessage expected to be format as /callback int int int int int int
// with int's corresponding to amp0 freq0 amp1 freq1 amp2 freq2
void oscillators(OSCMessage &msg) {
  a0 = (int8_t) msg.getInt(0);
  aCos0.setFreq(msg.getInt(1));

  a1 = (int8_t) msg.getInt(2);
  aCos1.setFreq(msg.getInt(3));

  a2 = (int8_t) msg.getInt(4);
  aCos2.setFreq(msg.getInt(5));

  if(DEBUG){
    Serial.print("Updating a0 with value ");
    Serial.println(a0);
    Serial.print("Updating a1 with value ");
    Serial.println(a1);
    Serial.print("Updating a2 with value ");
    Serial.println(a2);
  }
}

void neighbor(OSCMessage &msg) {
  // update neighbors
  if(DEBUG){
    in_type = msg.getType(0);
    if(in_type=='f'){
      in_value = msg.getFloat(0);
    } else{
      Serial.printf("Wrong type `%s` in received in neighbor callback. Must be `i` or `f`.", in_type);
      return;
    }
    Serial.printf("Updating neighbor %s with value ", Udp.remoteIP().toString().c_str());
    Serial.println((float)(in_value), PPFPRE);
  } else{
    in_value = msg.getFloat(0);
  }

  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP() == Neighs[i].ip) {
      Neighs[i] = (Node){Neighs[i].ip, in_value, Neighs[i].port};
    }
  };
  // update internal value
  update_value();
  // send internal value to neighbors
  send_message();
}

// update internal value with Kuramoto model
void update_value(){
  float tmp = 0.0;
  for (int i=0; i<NB_NEIGHS; i++){
    tmp += (K * sin(Neighs[i].val - val));
  };
  val += dT * (W + tmp);
  if(DEBUG){
    Serial.printf("New internal value ");
    Serial.print((float)(val), PPFPRE);
    Serial.println(".");
  }
}

// send internal value to neighbors
void send_message(){
  OSCMessage msg("/neighbor");
  msg.add(val);
  if(DEBUG){
    Serial.printf("Sending value ");
    Serial.print((float)(val), PPFPRE);
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

void read_mesagge(){
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()){
      msg.dispatch("/oscillators", oscillators);
      msg.dispatch("/neighbor", neighbor);
    } else {
      error = msg.getError();
      Serial.print("Message error: ");
      Serial.println(error);
    }
  }

}

// receive incoming UDP packets
int read_buffer(){
  read_mesagge();
}

/// main ///

// This runs CONTROL_RATE times per second.
void updateControl(){
    read_buffer();
}

// This runs on average 16384 times per second, so code here needs to be lean.
int updateAudio(){
  long asig = (long)
    aCos0.next()*a0 +
    aCos1.next()*a1 +
    aCos2.next()*a2;
  asig >>= 9; // bitwise division to normalise total amplitude
  return ((int) asig)<<8; // bitwise multiplication adapt to ESP8266 range
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

  // set initial frequencies
  aCos0.setFreq(200);
  aCos1.setFreq(500);
  aCos2.setFreq(1000);

  // set initial amplitudes
  a0=a1=a2=127;

  // start synth engine
  startMozzi(CONTROL_RATE);
}


void loop(){
  audioHook(); // required here, it fills output buffer (256 cells > maximum latency of about 15 ms)
}




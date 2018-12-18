#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/cos8192_int8.h> // waveform table

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define CONTROL_RATE 32 // in Hz, must be a power of 2 

/// debug ///
const int PPFPRE = 10; // print precision 
int BAUD_RATE = 9600;
bool DEBUG = false;
bool MONITOR = true;

// WiFi - UDP
const char* ssid = "TP-Link_BED2";
const char* password = "73385615";

WiFiUDP Udp;
const unsigned int localUdpPort = 8266;   // local port to listen for UDP packets
const IPAddress ip(192, 168, 0, 15);     // IP para este dispositivo
const IPAddress gateway(192, 168, 0, 1);  // IP gateway, normalmente es la del router
const IPAddress subnet(255, 255, 255, 0); // Mascara de subred
OSCErrorCode error;

// Kuramoto model
int NB_NEIGHS = 5;
float in_value;
char in_type;

typedef struct{
    IPAddress ip;
    float val;
    int port;
    float weight;
} Node;

Node Neighs[5] ={
 {IPAddress(192, 168, 0, 10), 0.0, 8266, 1.0},
 {IPAddress(192, 168, 0, 11), 0.0, 8266, 1.0},
 {IPAddress(192, 168, 0, 12), 0.0, 8266, 1.0},
 {IPAddress(192, 168, 0, 13), 0.0, 8266, 1.0},
 {IPAddress(192, 168, 0, 14), 0.0, 8266, 1.0},
};

int NB_MONITORS = 1;
Node Monitors[1] ={
 {IPAddress(192, 168, 0, 113), 0.0, 5005},
};

float DT = 1.0/(float (CONTROL_RATE)); // model's time delta in seconds
float W = 1.0;  // internal frequency
float K = 1.0;  // coupling constant
float current_val = 0.0;  // internal value

// Mozzi Synth

// oscillator amplitude
int8_t a0;

// oscillator (table with int8_t values between -128 and 127)
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos0(COS8192_DATA);


/// helpers ///

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

void show_params(){
    Serial.println("Kuramoto model parameters:");
    Serial.printf("\t W: ");
    Serial.println(W, PPFPRE);
    Serial.printf("\t K: ");
    Serial.println(K, PPFPRE);
    Serial.printf("\t DT: ");
    Serial.println(DT, PPFPRE);
    Serial.printf("\t DEBUG: ");
    Serial.println(DEBUG);
    Serial.printf("\t MONITOR: ");
    Serial.println(MONITOR);
}

void show_neighs(){
  Serial.printf("Nodes:\n", WiFi.localIP().toString().c_str(), localUdpPort);
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      Serial.printf("\t %d : ip %s | port %d | weight ", i, Neighs[i].ip.toString().c_str(), Neighs[i].port);
      Serial.print(Neighs[i].weight, PPFPRE);
      Serial.print(" | current_val "),
      Serial.println(Neighs[i].val, PPFPRE);
    }
  }
}

void show_this(){
  Serial.printf("\t x : ip %s | port %d | weight x | current_val ", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.println(current_val, PPFPRE);
}

void show_all(){
  show_neighs();
  show_this();
}

void monitor(){
  OSCMessage msg("/monitor");
  msg.add(current_val);
  if(DEBUG){
    Serial.printf("Sending value ");
    Serial.print((float)(current_val), PPFPRE);
    Serial.println(" to  monitors:");
  }
  for (int i=0; i<NB_MONITORS; i++){
    Udp.beginPacket(Monitors[i].ip, Monitors[i].port);
    msg.send(Udp);
    Udp.endPacket();
    if(DEBUG){
      Serial.print("\t");
      Serial.println(Monitors[i].ip);
    }
  }
  msg.empty();
}


/// callbacks ///

void show_neighs_ckb(OSCMessage &msg){
  show_neighs();
}


void set_DEBUG(OSCMessage &msg){
  DEBUG = (bool) msg.getInt(0);
  Serial.print("DEBUG ");
  Serial.println(DEBUG);
}

void set_MONITOR(OSCMessage &msg){
  MONITOR = (bool) msg.getInt(0);
  if(DEBUG){
    Serial.print("MONITOR ");
    Serial.println(MONITOR);
  }
}

void set_current_val(OSCMessage &msg){
  current_val = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Current value updated with value ");
    Serial.println(current_val);
  }
}

void set_K(OSCMessage &msg){
  K = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Parameter `K` updated with value ");
    Serial.println(K);
  }
}

void set_W(OSCMessage &msg){
  W = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Parameter `W` updated with value ");
    Serial.println(W);
  }
}

void set_DT(OSCMessage &msg){
  DT = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Parameter `DT` updated with value ");
    Serial.println(DT);
  }
}

void set_amp(OSCMessage &msg){
  a0 = (int8_t) msg.getFloat(0);
  if(DEBUG){
    Serial.print("Updated a0 with value ");
    Serial.println(a0);
  }
}

void set_freq(OSCMessage &msg){
  float f0 = msg.getFloat(0);
  aCos0.setFreq(f0);
  if(DEBUG){
    Serial.print("Updated f0 with value ");
    Serial.println(f0);
  }
}

void neighbors_weights(OSCMessage &msg){
  for (int i=0; i<NB_NEIGHS; i++){
    Neighs[i].weight = msg.getFloat(i);
  }
}

void neighbor_weight_idx(OSCMessage &msg){
  int idx = msg.getInt(0);
  Neighs[idx].weight = msg.getFloat(1);
  if(DEBUG){
    Serial.printf("Updated %d-th neighbor with weight ", idx);
    Serial.println(Neighs[idx].weight);
    show_neighs();
  }
}
// main 

// update internal value
void update_value(){
  float tmp = 0.0;
  for(int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      tmp += (K * sin(Neighs[i].val - current_val));
    }
  };
  current_val += DT * (W + tmp);
  if(DEBUG){
    Serial.printf("New internal value ");
    Serial.print(current_val, PPFPRE);
    Serial.println(".");
  }
}



// send internal value to neighbors
void send_message(){
  OSCMessage msg("/neighbor");
  msg.add(current_val);
  if(DEBUG){
    Serial.printf("Sending value ");
    Serial.print(current_val, PPFPRE);
    Serial.println(" to  neighbors:");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      Udp.beginPacket(Neighs[i].ip, Neighs[i].port);
      msg.send(Udp);
      Udp.endPacket();
      if(DEBUG){
        Serial.print("\t");
        Serial.println(Neighs[i].ip);
      }
    }
  }
  msg.empty();
}

// receive value from neighbor
void neighbor(OSCMessage &msg){
  in_value = msg.getFloat(0);
  // update neighbors
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP() == Neighs[i].ip && Neighs[i].weight > 0){
      Neighs[i] = (Node){Neighs[i].ip, in_value, Neighs[i].port, Neighs[i].weight};
      if(DEBUG){
        Serial.printf("Updating neighbor %s with value ", Udp.remoteIP().toString().c_str());
        Serial.println(in_value, PPFPRE);
      }
    }
  };
  // update internal value
  update_value();
  // send internal value to neighbors
  send_message();
  // send internal value to monitors
  if(MONITOR){
    monitor();
  }
}

void parse_message(){
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0){
    while (size--){
      msg.fill(Udp.read());
    }
    if (!msg.hasError()){
      msg.dispatch("/show_neighs", show_neighs_ckb);  // nok
      msg.dispatch("/current_val", set_current_val);
      msg.dispatch("/param/K", set_K);
      msg.dispatch("/param/W", set_W);
      msg.dispatch("/param/DT", set_DT);
      msg.dispatch("/DEBUG", set_DEBUG);  // nok
      msg.dispatch("/MONITOR", set_MONITOR);  // nok
      msg.dispatch("/oscillator/freq", set_freq);
      msg.dispatch("/oscillator/amp", set_amp);  // nok
      msg.dispatch("/neighbor", neighbor);
      msg.dispatch("/neighbors_weights", neighbors_weights);  // nok
      msg.dispatch("/neighbor_weight_idx", neighbor_weight_idx);  // nok
    } else{
      error = msg.getError();
      Serial.print("Message error: ");
      Serial.println(error);
    }
  }

}
// This runs CONTROL_RATE times per second.
void updateControl(){
  parse_message();
  aCos0.setFreq((int) 100*current_val);
}

// This runs on average 16384 times per second, so code here needs to be lean.
int updateAudio(){
  return  a0*aCos0.next();
}

void setup(){
  Serial.begin(BAUD_RATE);

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);

  connect_wifi();
  connect_udp();

  show_all();
  show_params();

  // set initial frequencies
  aCos0.setFreq(500);

  // set initial amplitude
  a0=32;

  // start synth engine
  startMozzi(CONTROL_RATE);
}

void loop(){
  audioHook(); // required here, it fills output buffer (256 cells > maximum latency of about 15 ms)
}

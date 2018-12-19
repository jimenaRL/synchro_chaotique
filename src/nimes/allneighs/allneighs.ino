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
const char* SSID = "esthetopies";
const char* PWD = "esthetopies";

WiFiUDP Udp;
const unsigned int commPort = 8266;   // local port to listen for UDP packets
const int intIp = 11; // IP para este dispositivo sera ip(192, 168, 0, intIp);
const IPAddress gateway(192, 168, 0, 1);  // IP gateway, normalmente es la del router
const IPAddress subnet(255, 255, 255, 0); // Mascara de subred
OSCErrorCode error;

// model
const int NB_NEIGHS = 5;
float in_value;

typedef struct{
    int intIp;
    float val;
    float weight;
} Node;

Node Neighs[5] ={
 {10, 0.0, 1.0},
 {12, 0.0, 1.0},
 {13, 0.0, 1.0},
 {14, 0.0, 1.0},
 {15, 0.0, 1.0},
};

const int MONITOR_PORT = 5005;
int NB_MONITORS = 3;
int Monitors[3] = {111, 112, 113};


float DT = 1.0/(float (CONTROL_RATE)); // time delta
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
  Serial.printf("\nConnecting to %s ", SSID);
  WiFi.config(IPAddress(192, 168, 0, intIp), gateway, subnet);
  WiFi.begin(SSID, PWD);
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
  Udp.begin(commPort);
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
  Serial.print("Nodes:\n");
  for (int i=0; i<NB_NEIGHS; i++){
   if(Neighs[i].weight > 0){
     Serial.printf("\tintIp %d | weight ", Neighs[i].intIp);
     Serial.print(Neighs[i].weight, PPFPRE);
     Serial.print(" | current_val "),
     Serial.println(Neighs[i].val, PPFPRE);
   }
  }
}

void show_this(){
  Serial.printf("\tintIp %d | ---------- | current_val ", intIp);
  Serial.print(current_val, PPFPRE);
}

void monitor(){
  OSCMessage msg("/monitor");
  msg.add(current_val);
  if(DEBUG){
    Serial.print("Sending value ");
    Serial.print((float)(current_val), PPFPRE);
    Serial.print(" to monitors:");
  }
  for (int i=0; i<NB_MONITORS; i++){
    Udp.beginPacket(IPAddress(192, 168, 0, Monitors[i]), MONITOR_PORT);
    msg.send(Udp);
    Udp.endPacket();
    if(DEBUG){
      Serial.print(" ");
      Serial.print(Monitors[i]);
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

void neighbor_weight(OSCMessage &msg){
  int idx = msg.getInt(0);
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].intIp == idx){
      Neighs[i].weight = msg.getFloat(1);
      if(DEBUG){
        Serial.printf("Updated neighbor %d with weight ", idx);
        Serial.println(Neighs[idx].weight);
      }
    }
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
    Serial.print("Sending value ");
    Serial.print(current_val, PPFPRE);
    Serial.print(" to neighbors:");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      Udp.beginPacket(IPAddress(192, 168, 0, Neighs[i].intIp), commPort);
      msg.send(Udp);
      Udp.endPacket();
      if(DEBUG){
        Serial.print(" ");
        Serial.print(Neighs[i].intIp);
      }
    }
  }
  if(DEBUG){
    Serial.println("");
  }
  msg.empty();
}

// receive value from neighbor
void neighbor(OSCMessage &msg){
  in_value = msg.getFloat(0);
  // update neighbors
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP()[3] == Neighs[i].intIp && Neighs[i].weight > 0){
      Neighs[i] = (Node){Neighs[i].intIp, in_value, Neighs[i].weight};
      if(DEBUG){
        Serial.printf("Updating neighbor %d with value ", Neighs[i].intIp);
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
      msg.dispatch("/show_neighs", show_neighs_ckb);
      msg.dispatch("/current_val", set_current_val);
      msg.dispatch("/param/K", set_K);
      msg.dispatch("/param/W", set_W);
      msg.dispatch("/param/DT", set_DT);
      msg.dispatch("/DEBUG", set_DEBUG);
      msg.dispatch("/MONITOR", set_MONITOR);
      msg.dispatch("/oscillator/freq", set_freq);
      msg.dispatch("/oscillator/amp", set_amp);
      msg.dispatch("/neighbor", neighbor);
      msg.dispatch("/neighbor_weight", neighbor_weight);
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

  show_neighs();
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

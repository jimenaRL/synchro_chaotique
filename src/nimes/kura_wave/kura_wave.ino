#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/cos8192_int8.h> // waveform table

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define CONTROL_RATE 64 // in Hz, must be a power of 2 

/// debug ///
const int PPFPRE = 5; // print precision 
int BAUD_RATE = 9600;
bool DEBUG = true;
bool MONITOR = true;

// WiFi - UDP
const char* SSID = "esthetopies";
const char* PWD = "esthetopies";

WiFiUDP Udp;
const unsigned int COMMPORT = 8266;  // local port to listen for UDP packets
const int IP0 = 192;
const int IP1 = 168;
const int IP2 = 0;
const int selfIdIp = 10; // IP para este dispositivo sera ip(192, 168, 0, idIp);
const IPAddress gateway(IP0, IP1, IP2, 1);  // IP gateway, normalmente es la del router
const IPAddress subnet(255, 255, 255, 0); // Mascara de subred
OSCErrorCode error;

// model
const int NB_NEIGHS = 5;

typedef struct{
    int idIp;
    int nb_iter;
    float kura_val;
    float current_val;
    float previous_val;
    float weight;
} Node;

const float self_weight = 0.0;
Node Self = {selfIdIp, 0, 0.0, 10.0, 0.0, self_weight};

Node Neighs[5]={
 {11, 0, 0.0, 0.0, 0.0, 1.0},
 {12, 0, 0.0, 0.0, 0.0, 0.0},
 {13, 0, 0.0, 0.0, 0.0, 0.0},
 {14, 0, 0.0, 0.0, 0.0, 0.0},
 {15, 0, 0.0, 0.0, 0.0, 0.0},
};

const int KURA_MONITOR_PORT = 5005;
const int WAVE_MONITOR_PORT = 5006;
int NB_MONITORS = 3;
int Monitors[3] = {111, 112, 113};

// wave equaiton parameters
float C = 0.051;  // sound speed
float MU = 0.01;  // viscosity

// kuramoto model parameters
float DT = 1.0/(float (CONTROL_RATE)); // model's time delta in seconds
float W = 1.0;  // internal frequency
float K = 1.0;  // coupling constant


// Mozzi Synth

// oscillator amplitude
int8_t a0;

// oscillator (table with int8_t values between -128 and 127)
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos0(COS8192_DATA);

/// helpers ///

void connect_wifi(){
  Serial.printf("\nConnecting to %s ", SSID);
  WiFi.config(IPAddress(IP0, IP1, IP2, Self.idIp), gateway, subnet);
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
  Udp.begin(COMMPORT);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
}

void kura_monitor(){
  OSCMessage kura_msg("/kura_monitor");
  kura_msg.add(Self.kura_val);
  if(DEBUG){
    Serial.print("Sending kura value to monitors:");
  }
  for (int i=0; i<NB_MONITORS; i++){
    Udp.beginPacket(IPAddress(IP0, IP1, IP2, Monitors[i]), KURA_MONITOR_PORT);
    kura_msg.send(Udp);
    Udp.endPacket();
    if(DEBUG){
      Serial.print(" ");
      Serial.print(Monitors[i]);
    }
  }
  kura_msg.empty();
    if(DEBUG){
      Serial.println("");
    }
}

void wave_monitor(){
  OSCMessage wave_msg("/wave_monitor");
  wave_msg.add(Self.current_val);
  wave_msg.add(Self.nb_iter);
  if(DEBUG){
    Serial.print("Sending wave value and iter to monitors:");
  }
  for (int i=0; i<NB_MONITORS; i++){
    Udp.beginPacket(IPAddress(IP0, IP1, IP2, Monitors[i]), WAVE_MONITOR_PORT);
    wave_msg.send(Udp);
    Udp.endPacket();
    if(DEBUG){
      Serial.print(" ");
      Serial.print(Monitors[i]);
    }
  }
  wave_msg.empty();
    if(DEBUG){
      Serial.println("");
    }
}


/// showers ///

void show_params(){
    Serial.println("All:");
    Serial.printf("\tDEBUG: ");
    Serial.println(DEBUG);
    Serial.printf("\tMONITOR: ");
    Serial.println(MONITOR);
    Serial.println("Wave:");
    Serial.printf("\tC: ");
    Serial.println(C, PPFPRE);
    Serial.printf("\tMU: ");
    Serial.println(MU, PPFPRE);
    Serial.println("Kuramoto parameters");
    Serial.printf("\tK: ");
    Serial.println(K);
    Serial.printf("\tW: ");
    Serial.println(W);
    Serial.printf("\tDT: ");
    Serial.println(DT);
}

void show_neighs(){
  Serial.print("Neighs:\n");
  for (int i=0; i<NB_NEIGHS; i++){
   if(Neighs[i].weight > 0){
      Serial.printf("\tidIp %d | weight ", Neighs[i].idIp);
      Serial.print(Neighs[i].weight, PPFPRE);
      Serial.print(" | kura_val "),
      Serial.print(Neighs[i].kura_val, PPFPRE);
      Serial.print(" | current_val "),
      Serial.print(Neighs[i].current_val, PPFPRE);
      Serial.print(" | previous_val ");
      Serial.println(Self.previous_val, PPFPRE);
   }
  }
}

void show_this(){
  Serial.print("Self:\n");
  Serial.printf("\tidIp %d | weight ------- | kura_val ", Self.idIp);
  Serial.print(Self.kura_val, PPFPRE);
  Serial.print(" | current_val ");
  Serial.print(Self.current_val, PPFPRE);
  Serial.print(" | previous_val ");
  Serial.println(Self.previous_val, PPFPRE);
}

void show_all(){
  show_params();
  show_this();
  show_neighs();
}

void show_params_ckb(OSCMessage &msg){
  show_params();
}

void show_all_ckb(OSCMessage &msg){
  show_all();
}
void show_this_ckb(OSCMessage &msg){
  show_this();
}

void show_neighs_ckb(OSCMessage &msg){
  show_neighs();
}

/// setters ///
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

void set_K(OSCMessage &msg) {
  K = msg.getFloat(0);
  Serial.print("Parameter `K` updated with value ");
  Serial.println(K, PPFPRE);
}

void set_W(OSCMessage &msg) {
  W = msg.getFloat(0);
  Serial.print("Parameter `W` updated with value ");
  Serial.println(W, PPFPRE);
}

void set_DT(OSCMessage &msg) {
  DT = msg.getFloat(0);
  Serial.print("Parameter `DT` updated with value ");
  Serial.println(DT, PPFPRE);
}

void set_C(OSCMessage &msg){
  C = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Parameter `C` updated with value ");
    Serial.println(C, PPFPRE);
  }
}

void set_MU(OSCMessage &msg){
  MU = msg.getFloat(0);
  if(DEBUG){
    Serial.print("Parameter `MU` updated with value ");
    Serial.println(MU, PPFPRE);
  }
}

void set_amp(OSCMessage &msg){
  a0 = (int8_t) msg.getFloat(0);
  if(DEBUG){
    Serial.print("Updated a0 with value ");
    Serial.println(a0, PPFPRE);
  }
}

void set_freq(OSCMessage &msg){
  float f0 = msg.getFloat(0);
  aCos0.setFreq(f0);
  if(DEBUG){
    Serial.print("Updated f0 with value ");
    Serial.println(f0, PPFPRE);
  }
}

void set_weights(OSCMessage &msg){
  int in_idIp = msg.getInt(0);
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].idIp == in_idIp){
      Neighs[i].weight = msg.getFloat(1);
      if(DEBUG){
        Serial.printf("Updated neighbor %d with weight ", Neighs[i].idIp);
        Serial.println(Neighs[i].weight);
      }
    }
  }
}

void set_values(OSCMessage &msg) {
  int in_idIp = msg.getInt(0);
  if(in_idIp == Self.idIp){
      Self.current_val = msg.getFloat(1);
      Self.previous_val = msg.getFloat(2);
      if(DEBUG){
        Serial.print("Self ");
        Serial.print(Self.idIp);
        Serial.print(" current and previous values updated with values: ");
        Serial.print(Self.current_val, PPFPRE);
        Serial.print(" and ");
        Serial.println(Self.previous_val, PPFPRE);
      }
  }
  else{
    for (int i=0; i<NB_NEIGHS; i++){
      if(Neighs[i].idIp == in_idIp){
        Neighs[i].current_val = msg.getFloat(1);
        Neighs[i].previous_val = msg.getFloat(2);
        if(DEBUG){
          Serial.print("Neighbor ");
          Serial.print(Neighs[i].idIp);
          Serial.print(" current and previous values updated with values: ");
          Serial.print(Neighs[i].current_val, PPFPRE);
          Serial.print(" and ");
          Serial.println(Neighs[i].previous_val, PPFPRE);
        }
      }
    }
  }
}


// main 

void kura_val(OSCMessage &msg){
  float in_value = msg.getFloat(0);
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP()[3] == Neighs[i].idIp && Neighs[i].weight > 0){
      Neighs[i].kura_val = in_value;
      if(DEBUG){
        Serial.printf("Updating neighbor %d with kura value ", Neighs[i].idIp);
        Serial.println(Neighs[i].kura_val, PPFPRE);
      }
    }
  }
  kura_update();
}

void kura_update(){
  float tmp = 0.0;
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      tmp += sin(Neighs[i].kura_val - Self.kura_val);
    }
  };
  Self.kura_val += DT * (W + K * tmp);
  if(DEBUG){
    Serial.printf("New kuramoto value ");
    Serial.print((float)(Self.kura_val), PPFPRE);
    Serial.println(".");
  }
}

void kura_send(){
  OSCMessage msg("/kura_val");
  msg.add(Self.kura_val);
  if(DEBUG){
    Serial.print("Sending kura value ");
    Serial.print(Self.kura_val, PPFPRE);
    Serial.print(" to neighbors:");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      Udp.beginPacket(IPAddress(IP0, IP1, IP2, Neighs[i].idIp), COMMPORT);
      msg.send(Udp);
      Udp.endPacket();
      if(DEBUG){
        Serial.print(" ");
        Serial.print(Neighs[i].idIp);
      }
    }
  }
  if(DEBUG){
    Serial.println("");
  }
  msg.empty();
  if(MONITOR){
    kura_monitor();
  }
}

void wave_val(OSCMessage &msg){
  float in_iter = msg.getInt(0);
  float in_value = msg.getFloat(1);
  // update neighbors
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP()[3] == Neighs[i].idIp && Neighs[i].weight > 0){
      Neighs[i].previous_val = Neighs[i].current_val;
      Neighs[i].current_val = in_value;
      Neighs[i].nb_iter = in_iter;
      if(DEBUG){
        Serial.printf("Updating neighbor %d wave values : ", Neighs[i].idIp);
        Serial.print(" nb_iter "),
        Serial.print(Neighs[i].nb_iter);
        Serial.print(" current_val ");
        Serial.print(Self.current_val, PPFPRE);
        Serial.print(" previous_val ");
        Serial.println(Self.previous_val, PPFPRE);
      }
    }
  };
}

void wave_update(){
  float D2TP = 2.0 * Self.current_val - Self.previous_val;
  float D2XP_current = 0.0;
  float D2XP_previous = 0.0;
  int NB_ACT_NEIGHS = 0;
  for(int j=0; j<NB_NEIGHS; j++){
    if(Neighs[j].weight > 0){
      D2XP_previous += (Neighs[j].previous_val - Self.previous_val);
      D2XP_current += (Neighs[j].current_val - Self.current_val);
    }
  }
  Self.previous_val = Self.current_val;
  // DTD2XP_current = (D2XP_current - D2XP_previous)
  Self.current_val = D2TP + C * D2XP_current + MU * (D2XP_current - D2XP_previous);
  Self.nb_iter += 1;
  if(DEBUG){
    Serial.printf("New wave equation value ");
    Serial.print(Self.current_val, PPFPRE);
    Serial.print(" and iter ");
    Serial.println(Self.nb_iter);
  }
}

// send internal value to neighbors
void wave_send(){
  OSCMessage msg("/wave_val");
  msg.add(Self.nb_iter);
  msg.add(Self.current_val);
  if(DEBUG){
    Serial.print("Sending wave value ");
    Serial.print(Self.current_val, PPFPRE);
    Serial.print("  and iter ");
    Serial.print(Self.nb_iter);
    Serial.print(" to neighbors:");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Neighs[i].weight > 0){
      Udp.beginPacket(IPAddress(IP0, IP1, IP2, Neighs[i].idIp), COMMPORT);
      msg.send(Udp);
      Udp.endPacket();
      if(DEBUG){
        Serial.print(" ");
        Serial.print(Neighs[i].idIp);
      }
    }
  }
  if(DEBUG){
    Serial.println("");
  }
  msg.empty();
  if(MONITOR){
    wave_monitor();
  }
}

void parse_osc(){
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0){
    Serial.println("-------------------------- ");
    while (size--){
      msg.fill(Udp.read());
    }
    if (!msg.hasError()){
      msg.dispatch("/show/all", show_all_ckb);
      msg.dispatch("/show/this", show_this_ckb);
      msg.dispatch("/show/neighs", show_neighs_ckb);
      msg.dispatch("/show/params", show_params_ckb);

      msg.dispatch("/set/weights", set_weights);
      msg.dispatch("/set/values", set_values);

      msg.dispatch("/set/C", set_C);
      msg.dispatch("/set/MU", set_MU);
      msg.dispatch("/set/DEBUG", set_DEBUG);
      msg.dispatch("/set/MONITOR", set_MONITOR);
      msg.dispatch("/set/oscFreq", set_freq);
      msg.dispatch("/set/oscAmp", set_amp);

      msg.dispatch("/kura_val", kura_val);
      kura_send();

      msg.dispatch("/wave_val", wave_val);
      float cond = Self.kura_val - (int) Self.kura_val;
      Serial.print("cond: ");
      Serial.println(cond, PPFPRE);
      // wave value update half-cycle
      if(0 < cond && cond < 0.2){
        Serial.println("Should wave_send... ");
        wave_send();
      }
      if(0.5 < cond && cond < 0.7){
        Serial.println("Should wave_update... ");
        wave_update();
      }
    }else{
      error = msg.getError();
      Serial.print("Message error: ");
      Serial.println(error);
    }
    Serial.println("-------------------------- ");
  }
}

// This runs CONTROL_RATE times per second
void updateControl(){
  parse_osc();
  aCos0.setFreq((int) 100*Self.current_val);
}

// This runs on average 16384 times per second
int updateAudio(){
  return a0*aCos0.next();
}

void setup(){
  Serial.begin(BAUD_RATE);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  connect_wifi();
  connect_udp();

  show_all();

  // set initial amplitude
  a0=32;

  // start synth engine
  startMozzi(CONTROL_RATE);
}

void loop(){
  audioHook(); // required here, it fills output buffer (256 cells > maximum latency of about 15 ms)
}

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

/// WiFi - UDP /// 
const char* ssid = "jimena";
const char* password = "ababababab";

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
const int bufferSize = 255;        // size of buffer
char incomingPacket[bufferSize];   // buffer for incoming packets
byte mac[6];                       // the MAC address of SAT

/// neighbors ///
int NB_NEIGHS = 2;

typedef struct {
    IPAddress ip;
    double val;
    int port;
} Node;

Node Neighs[2] = {
  {IPAddress(192, 168, 0, 30), 0.0, 4210},
  {IPAddress(192, 168, 0, 13), 0.0, 4210},
};


/// internal ///
char val_buff[bufferSize];  // buffer to store strinf representation of internal value
const double TAU = 100;     // time delta for updating values in milliseconds
const int WIDTH = 10;       // WIDTH of for char to string conversion
const int PREC = 10;        // Precision of for char to string conversion


/// Kuramoto model parameters ///
const double dT = TAU/1000.0;     // model's time delta in seconds
const double W = 1.0;             // internal frequency
const double K = 1.0;             // coupling constant
double val = 0.0;  // internal value

/// debug ///

const int ppFpre = 10; // precision print
bool debug = false;
bool monitoring = true;

Node monitor_server = {IPAddress(192, 168, 0, 36), 0.0, 5005};


void print_mac(){
  WiFi.macAddress(mac);
  Serial.print("MAC adress: ");
  Serial.print(mac[0],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.println(mac[5],HEX);
}

void show_params(){
    Serial.println("Kuramoto model parameters:");
    Serial.printf("\t w: ");
    Serial.println((double)(W),ppFpre);
    Serial.printf("\t k: ");
    Serial.println((double)(K),ppFpre);
    Serial.printf("\t dt: ");
    Serial.println((double)(dT),ppFpre);
}

void show_neighs(){
  Serial.printf("Nodes:\n", WiFi.localIP().toString().c_str(), localUdpPort);
  for (int i=0; i<NB_NEIGHS; i++){
    Serial.printf("\t %d : ip %s | port %d | val ", i, Neighs[i].ip.toString().c_str(), Neighs[i].port);
    Serial.println((double)(Neighs[i].val),ppFpre);
  }
}

void show_this(){
  Serial.printf("\t x : ip %s | port %d | val ", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.println((double)(val), ppFpre);
}

void show_all(){
  show_neighs();
  show_this();
}

void _monitor(){
  //Serial.printf("Sending value ");
  //Serial.print((double)(val), ppFpre);
  //Serial.println(" to  monitor server.");
  Udp.beginPacket(monitor_server.ip, monitor_server.port);
  Udp.write(val_buff);
  Udp.endPacket();
}

// utils //
void connect_wifi(){
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

// core methods //

// send internal value to neighbors
void send_value(){
  // convert double val into an ASCII representation that will be stored under value_buff
  dtostrf(val, WIDTH, PREC, val_buff);
  if(debug){
    Serial.printf("Sending value ");
    Serial.print((double)(val), ppFpre);
    Serial.println(" to  neighbors.");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    Udp.beginPacket(Neighs[i].ip, Neighs[i].port);
    Udp.write(val_buff);
    Udp.endPacket();
  }
}

// update internal value with Kuramoto model
void update_value(){
  if(debug){
    Serial.printf("Updating internal value\n");
  }
  double tmp = 0.0;
  for (int i=0; i<NB_NEIGHS; i++){
    tmp += (K * sin(Neighs[i].val - val));
  };
  val += dT * (W + tmp);
}

// update neighbor value from received packet
void update_neigh(){
  double in_value = atof(incomingPacket);
  if(debug){
    Serial.printf("Updating neighbor %s with value ", Udp.remoteIP().toString().c_str(), in_value);
    Serial.print((double)(in_value), ppFpre);
    Serial.printf("\n");
  }
  for (int i=0; i<NB_NEIGHS; i++){
    if(Udp.remoteIP() == Neighs[i].ip) {
      Neighs[i] = (Node){Neighs[i].ip, in_value, Neighs[i].port};
    }
  };
}

void update_state(){
  update_neigh();
  update_value();
}

// receive incoming UDP packets
int read_buffer(){
  int packetSize = Udp.parsePacket();
  if (packetSize){
    int len = Udp.read(incomingPacket, bufferSize);
    if(debug){
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    }
    if (len > 0){
      incomingPacket[len] = '\0';   // set null character to end  string representation
    }
  }
  return packetSize;
}

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.begin(115200);
  connect_wifi();
  print_mac();
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d. \n", WiFi.localIP().toString().c_str(), localUdpPort);
  show_all();
  show_params();
}

void loop(){
  // handle incoming packets
  int packetSize = read_buffer();
  // if package received
  if(packetSize){
    // wait
    delay(TAU/2.);
    // update internal state
    update_state();
    // wait
    delay(TAU/2.);
    // send internal value to neighbors
    send_value();
    if(monitoring){
      _monitor();
      // show_all();
      // for monitoring
      digitalWrite(LED_BUILTIN, LOW);
      delay(10);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

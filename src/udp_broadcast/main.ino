#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

/// WiFi - UDP /// 
const char* ssid = "jimena";
const char* password = "ababababab";

WiFiUDP Udp;
unsigned int localUdpPort = 4210; // local port to listen on
unsigned int remoteUdpPort = 4210; // local port to talk to
char incomingPacket[255];         // buffer for incoming packets
byte mac[6];                      // the MAC address of SAT

/// neighbors /// 
int nb_neighs = 4;

typedef struct {
    IPAddress ip;
    float val;
} Node;

Node Neighs[4] = {
  {IPAddress(192, 168, 0, 17), 0.0},  // debug fake neighbor (my cel phone) 
  {IPAddress(192, 168, 0, 36), 0.0},  // debug fake neighbor (my laptop) 
  {IPAddress(192, 168, 0, 13), 0.0},
  {IPAddress(192, 168, 0, 43), 0.0},
};


/// internal /// 
const float tau = 0.001;   // frame time of server
char val_buff[10];   // buffer to store strinf representation of internal value 
const int width = 4; // width of for char to string conversion 
const int prec = 6;  // precision of for char to string conversion 


/// Kuramoto model parameters /// 
const float dt = 0.1;    // time delta for updating values
const float w = 1.0;    // frequency
const float k = 0.5;    // coupling constant
float val = 10.0; // float(random(300));  // internal value

/// debug /// 
bool debug = true;

//void typeof(String a){ Serial.println("it a String a");}
//void typeof(int a)   { Serial.println("it a int a");}
//void typeof(char* a) { Serial.println("it a char* a");}
//void typeof(float a) { Serial.println("it a float a");}


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
    Serial.println((float)(w),5); 
    Serial.printf("\t k: ");
    Serial.println((float)(k),5); 
    Serial.printf("\t dt: ");
    Serial.println((float)(dt),5); 

}

void show_neighs(){
    Serial.printf("Nodes:\n", WiFi.localIP().toString().c_str(), localUdpPort);
    for (int i=0; i<nb_neighs; i++){
      Serial.printf("\t %d : ip %s | val ", i, Neighs[i].ip.toString().c_str());
      Serial.println((float)(Neighs[i].val),1); 
    }
}

void show_this(){
      Serial.printf("\t x : ip %s | val ", WiFi.localIP().toString().c_str());
      Serial.println((float)(val), 5); 
}

void show_all(){
  show_neighs();
  show_this();
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

// send internal value to neighbors 
void send_value(){
  // converts double val into an ASCII representationthat will be stored under value_buff
  dtostrf(val, width, prec, val_buff);
  for (int i=0; i < nb_neighs; i++){
    Udp.beginPacket(Neighs[i].ip, remoteUdpPort);
    Udp.write(val_buff);
    Udp.endPacket();
  }
}

// update internal value with Kuramoto model
void update_value(){
  Serial.printf("Updating internal value.\n");
  float tmp = 0.0;
  for (int i=0; i < nb_neighs; i++){
    // /!\ /!\ /!\ sin argument angle mus be in Radians /!\ /!\ /!\ //
    tmp += (k * sin(PI*(Neighs[i].val - val)/180.0));
  };
  val += dt * (w + tmp);
}

// update neighbor value from received packet 
void update_neigh(){
  Serial.printf("Updating neighbor %s with value %s ! \n", Udp.remoteIP().toString().c_str(), incomingPacket);
  for (int i=0; i<nb_neighs; i++){
    if(Udp.remoteIP() == Neighs[i].ip) {
      Neighs[i] = (Node){IPAddress(192, 168, 0, 17), String(incomingPacket).toFloat()};
    }
  };
}

// handle incoming packets
void handle_packets(){
  int packetSize = Udp.parsePacket();
  if (packetSize){
    // receive incoming UDP packets
    if(debug){
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    }
    int len = Udp.read(incomingPacket, 255);
    if (len > 0){
      incomingPacket[len] = 0;
    }
    if(debug){
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    }
    update_neigh();
    update_value();
    send_value();
    show_all();
    // for monitoring
    if(debug){
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
    }    
  }
}

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.begin(115200);
  Serial.println();
  connect_wifi();
  print_mac();
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d. \n", WiFi.localIP().toString().c_str(), localUdpPort);
  show_all();
  show_params();
}

void loop(){
    // handle incoming packets
    handle_packets();
    // wait
    //delay(tau/2.);
    // send internal value to neighbors
    //send_value();
    // wait
    //delay(tau/2.);
    // update internal value
    //update_value();

}


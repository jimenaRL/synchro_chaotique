/*  NOTES
    (1)  Do not use Serial.begin() when sending output audio,
         it seems that in that case Rx serial output may not available.
         Only used for debugging purposes.
    (2)  ESP8266 I2S audio interface expects int values encoded with 16 bit depth,
         so output of updateAudio() method must be between -32768 and 32768  (2**16 = 65536).
    (3)  Oscil.setFreq() method doesn't look to like double values as argument
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/cos8192_int8.h> // waveform table

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

/// Mozzi Synth /// 

#define CONTROL_RATE 64 // must be a power of 2 

// oscillators amplitudes
int8_t a0, a1, a2;

// oscillators (tables with int8_t values between -128 and 127)
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos0(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos1(COS8192_DATA);
Oscil<COS8192_NUM_CELLS, AUDIO_RATE> aCos2(COS8192_DATA);


/// WiFi - UDP ///
const char* ssid = "esthetopies";
const char* password = "esthetopies";

WiFiUDP Udp;
const unsigned int localUdpPort = 8266;   // local port to listen for UDP packets
const IPAddress ip(192, 168, 0, 127);     // IP para este dispositivo
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


// OSCMessage expected to be format as /callback int int int int int int
// with int's corresponding to amp0 freq0 amp1 freq1 amp2 freq2
void callback(OSCMessage &msg) {

  a0 = (int8_t) msg.getInt(0);
  aCos0.setFreq(msg.getInt(1));

  a1 = (int8_t) msg.getInt(2);
  aCos1.setFreq(msg.getInt(3));

  a2 = (int8_t) msg.getInt(4);
  aCos2.setFreq(msg.getInt(5));

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
      bundle.dispatch("/callback", callback);
    } else {
      error = bundle.getError();
      Serial.print("OSCBundle error: ");
      Serial.println(error);
    }
  }
}

/// main ///
void updateControl(){
    read_buffer();
}

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
  //Serial.begin(115200);

  connect_wifi();
  connect_udp();

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
  audioHook(); // required here
}




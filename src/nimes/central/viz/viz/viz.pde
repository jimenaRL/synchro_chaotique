import peasy.*;
PeasyCam cam;

import oscP5.*;
import netP5.*;
OscP5 oscP5;

/* a NetAddress contains the ip address and port number of a remote location in the network. */
NetAddress myBroadcastLocation; 

ArrayList<IpVertex> vertices;
IntDict ipNum;

int STROKE = 0; // 255;
color BACKGROUND = #ffffff; // #000000;
color C0 = #48917B; // RGB(72, 145, 123)  // vert
int  A0 = 64;
color C1 = #FFC8C8; // RGB(71, 96, 143)   // blue
int  A1 = 64;
color C2 = #47608F; // RGB(255, 200, 200) // rouge
int  A2 = 128; 
int EDGE = 80;

int RADIUS = 250;
float DELTA = 0.08;
   
boolean ROTATE = false;
float SPEED = 0.005; // PI/4096.0;
float SPEEDX = SPEED/4;
float SPEEDY = SPEED/2;
float SPEEDZ = SPEED;

void setup() {
  size(1280, 800, P3D);
  cam = new PeasyCam(this, 1000000);

  noStroke();
  fill(0);
  ortho(-width, width, -height, height);

  vertices = new ArrayList<IpVertex>();
vertices.add(new IpVertex(100, 1, -5, -5));
vertices.add(new IpVertex(101, 1, -5, -3));
vertices.add(new IpVertex(102, 1, -5, -1));
vertices.add(new IpVertex(103, 1, -5, 1));
vertices.add(new IpVertex(104, 1, -5, 3));
vertices.add(new IpVertex(105, 1, -5, 5));
vertices.add(new IpVertex(106, 1, -3, -5));
vertices.add(new IpVertex(107, 1, -3, -3));
vertices.add(new IpVertex(108, 1, -3, -1));
vertices.add(new IpVertex(109, 1, -3, 1));
vertices.add(new IpVertex(110, 1, -3, 3));
vertices.add(new IpVertex(111, 1, -3, 5));
vertices.add(new IpVertex(112, 1, -1, -5));
vertices.add(new IpVertex(113, 1, -1, -3));
vertices.add(new IpVertex(114, 1, -1, -1));
vertices.add(new IpVertex(115, 1, -1, 1));
vertices.add(new IpVertex(116, 1, -1, 3));
vertices.add(new IpVertex(117, 1, -1, 5));
vertices.add(new IpVertex(118, 1, 1, -5));
vertices.add(new IpVertex(119, 1, 1, -3));
vertices.add(new IpVertex(120, 1, 1, -1));
vertices.add(new IpVertex(121, 1, 1, 1));
vertices.add(new IpVertex(122, 1, 1, 3));
vertices.add(new IpVertex(123, 1, 1, 5));
vertices.add(new IpVertex(124, 1, 3, -5));
vertices.add(new IpVertex(125, 1, 3, -3));
vertices.add(new IpVertex(126, 1, 3, -1));
vertices.add(new IpVertex(127, 1, 3, 1));
vertices.add(new IpVertex(128, 1, 3, 3));
vertices.add(new IpVertex(129, 1, 3, 5));
vertices.add(new IpVertex(130, 1, 5, -5));
vertices.add(new IpVertex(131, 1, 5, -3));
vertices.add(new IpVertex(132, 1, 5, -1));
vertices.add(new IpVertex(133, 1, 5, 1));
vertices.add(new IpVertex(134, 1, 5, 3));
vertices.add(new IpVertex(135, 1, 5, 5));
vertices.add(new IpVertex(200, 2, -5, -6));
vertices.add(new IpVertex(201, 2, -5, -4));
vertices.add(new IpVertex(202, 2, -5, -2));
vertices.add(new IpVertex(203, 2, -5, 0));
vertices.add(new IpVertex(204, 2, -5, 2));
vertices.add(new IpVertex(205, 2, -5, 4));
vertices.add(new IpVertex(206, 2, -5, 6));
vertices.add(new IpVertex(207, 2, -3, -6));
vertices.add(new IpVertex(208, 2, -3, -4));
vertices.add(new IpVertex(209, 2, -3, -2));
vertices.add(new IpVertex(210, 2, -3, 0));
vertices.add(new IpVertex(211, 2, -3, 2));
vertices.add(new IpVertex(212, 2, -3, 4));
vertices.add(new IpVertex(213, 2, -3, 6));
vertices.add(new IpVertex(214, 2, -1, -6));
vertices.add(new IpVertex(215, 2, -1, -4));
vertices.add(new IpVertex(216, 2, -1, -2));
vertices.add(new IpVertex(217, 2, -1, 0));
vertices.add(new IpVertex(218, 2, -1, 2));
vertices.add(new IpVertex(219, 2, -1, 4));
vertices.add(new IpVertex(220, 2, -1, 6));
vertices.add(new IpVertex(221, 2, 1, -6));
vertices.add(new IpVertex(222, 2, 1, -4));
vertices.add(new IpVertex(223, 2, 1, -2));
vertices.add(new IpVertex(224, 2, 1, 0));
vertices.add(new IpVertex(225, 2, 1, 2));
vertices.add(new IpVertex(226, 2, 1, 4));
vertices.add(new IpVertex(227, 2, 1, 6));
vertices.add(new IpVertex(228, 2, 3, -6));
vertices.add(new IpVertex(229, 2, 3, -4));
vertices.add(new IpVertex(230, 2, 3, -2));
vertices.add(new IpVertex(231, 2, 3, 0));
vertices.add(new IpVertex(232, 2, 3, 2));
vertices.add(new IpVertex(233, 2, 3, 4));
vertices.add(new IpVertex(234, 2, 3, 6));
vertices.add(new IpVertex(235, 2, 5, -6));
vertices.add(new IpVertex(236, 2, 5, -4));
vertices.add(new IpVertex(237, 2, 5, -2));
vertices.add(new IpVertex(238, 2, 5, 0));
vertices.add(new IpVertex(239, 2, 5, 2));
vertices.add(new IpVertex(240, 2, 5, 4));
vertices.add(new IpVertex(241, 2, 5, 6));
vertices.add(new IpVertex(300, 3, -6, -6));
vertices.add(new IpVertex(301, 3, -6, -4));
vertices.add(new IpVertex(302, 3, -6, -2));
vertices.add(new IpVertex(303, 3, -6, 0));
vertices.add(new IpVertex(304, 3, -6, 2));
vertices.add(new IpVertex(305, 3, -6, 4));
vertices.add(new IpVertex(306, 3, -6, 6));
vertices.add(new IpVertex(307, 3, -4, -6));
vertices.add(new IpVertex(308, 3, -4, -4));
vertices.add(new IpVertex(309, 3, -4, -2));
vertices.add(new IpVertex(310, 3, -4, 0));
vertices.add(new IpVertex(311, 3, -4, 2));
vertices.add(new IpVertex(312, 3, -4, 4));
vertices.add(new IpVertex(313, 3, -4, 6));
vertices.add(new IpVertex(314, 3, -2, -6));
vertices.add(new IpVertex(315, 3, -2, -4));
vertices.add(new IpVertex(316, 3, -2, -2));
vertices.add(new IpVertex(317, 3, -2, 0));
vertices.add(new IpVertex(318, 3, -2, 2));
vertices.add(new IpVertex(319, 3, -2, 4));
vertices.add(new IpVertex(320, 3, -2, 6));
vertices.add(new IpVertex(321, 3, 0, -6));
vertices.add(new IpVertex(322, 3, 0, -4));
vertices.add(new IpVertex(323, 3, 0, -2));
vertices.add(new IpVertex(324, 3, 0, 0));
vertices.add(new IpVertex(325, 3, 0, 2));
vertices.add(new IpVertex(326, 3, 0, 4));
vertices.add(new IpVertex(327, 3, 0, 6));
vertices.add(new IpVertex(328, 3, 2, -6));
vertices.add(new IpVertex(329, 3, 2, -4));
vertices.add(new IpVertex(330, 3, 2, -2));
vertices.add(new IpVertex(331, 3, 2, 0));
vertices.add(new IpVertex(332, 3, 2, 2));
vertices.add(new IpVertex(333, 3, 2, 4));
vertices.add(new IpVertex(334, 3, 2, 6));
vertices.add(new IpVertex(335, 3, 4, -6));
vertices.add(new IpVertex(336, 3, 4, -4));
vertices.add(new IpVertex(337, 3, 4, -2));
vertices.add(new IpVertex(338, 3, 4, 0));
vertices.add(new IpVertex(339, 3, 4, 2));
vertices.add(new IpVertex(340, 3, 4, 4));
vertices.add(new IpVertex(341, 3, 4, 6));
vertices.add(new IpVertex(342, 3, 6, -6));
vertices.add(new IpVertex(343, 3, 6, -4));
vertices.add(new IpVertex(344, 3, 6, -2));
vertices.add(new IpVertex(345, 3, 6, 0));
vertices.add(new IpVertex(346, 3, 6, 2));
vertices.add(new IpVertex(347, 3, 6, 4));
vertices.add(new IpVertex(348, 3, 6, 6));
vertices.add(new IpVertex(400, 4, -6, -5));
vertices.add(new IpVertex(401, 4, -6, -3));
vertices.add(new IpVertex(402, 4, -6, -1));
vertices.add(new IpVertex(403, 4, -6, 1));
vertices.add(new IpVertex(404, 4, -6, 3));
vertices.add(new IpVertex(405, 4, -6, 5));
vertices.add(new IpVertex(406, 4, -4, -5));
vertices.add(new IpVertex(407, 4, -4, -3));
vertices.add(new IpVertex(408, 4, -4, -1));
vertices.add(new IpVertex(409, 4, -4, 1));
vertices.add(new IpVertex(410, 4, -4, 3));
vertices.add(new IpVertex(411, 4, -4, 5));
vertices.add(new IpVertex(412, 4, -2, -5));
vertices.add(new IpVertex(413, 4, -2, -3));
vertices.add(new IpVertex(414, 4, -2, -1));
vertices.add(new IpVertex(415, 4, -2, 1));
vertices.add(new IpVertex(416, 4, -2, 3));
vertices.add(new IpVertex(417, 4, -2, 5));
vertices.add(new IpVertex(418, 4, 0, -5));
vertices.add(new IpVertex(419, 4, 0, -3));
vertices.add(new IpVertex(420, 4, 0, -1));
vertices.add(new IpVertex(421, 4, 0, 1));
vertices.add(new IpVertex(422, 4, 0, 3));
vertices.add(new IpVertex(423, 4, 0, 5));
vertices.add(new IpVertex(424, 4, 2, -5));
vertices.add(new IpVertex(425, 4, 2, -3));
vertices.add(new IpVertex(426, 4, 2, -1));
vertices.add(new IpVertex(427, 4, 2, 1));
vertices.add(new IpVertex(428, 4, 2, 3));
vertices.add(new IpVertex(429, 4, 2, 5));
vertices.add(new IpVertex(430, 4, 4, -5));
vertices.add(new IpVertex(431, 4, 4, -3));
vertices.add(new IpVertex(432, 4, 4, -1));
vertices.add(new IpVertex(433, 4, 4, 1));
vertices.add(new IpVertex(434, 4, 4, 3));
vertices.add(new IpVertex(435, 4, 4, 5));
vertices.add(new IpVertex(436, 4, 6, -5));
vertices.add(new IpVertex(437, 4, 6, -3));
vertices.add(new IpVertex(438, 4, 6, -1));
vertices.add(new IpVertex(439, 4, 6, 1));
vertices.add(new IpVertex(440, 4, 6, 3));
vertices.add(new IpVertex(441, 4, 6, 5));
vertices.add(new IpVertex(500, 5, -5, -5));
vertices.add(new IpVertex(501, 5, -5, -3));
vertices.add(new IpVertex(502, 5, -5, -1));
vertices.add(new IpVertex(503, 5, -5, 1));
vertices.add(new IpVertex(504, 5, -5, 3));
vertices.add(new IpVertex(505, 5, -5, 5));
vertices.add(new IpVertex(506, 5, -3, -5));
vertices.add(new IpVertex(507, 5, -3, -3));
vertices.add(new IpVertex(508, 5, -3, -1));
vertices.add(new IpVertex(509, 5, -3, 1));
vertices.add(new IpVertex(510, 5, -3, 3));
vertices.add(new IpVertex(511, 5, -3, 5));
vertices.add(new IpVertex(512, 5, -1, -5));
vertices.add(new IpVertex(513, 5, -1, -3));
vertices.add(new IpVertex(514, 5, -1, -1));
vertices.add(new IpVertex(515, 5, -1, 1));
vertices.add(new IpVertex(516, 5, -1, 3));
vertices.add(new IpVertex(517, 5, -1, 5));
vertices.add(new IpVertex(518, 5, 1, -5));
vertices.add(new IpVertex(519, 5, 1, -3));
vertices.add(new IpVertex(520, 5, 1, -1));
vertices.add(new IpVertex(521, 5, 1, 1));
vertices.add(new IpVertex(522, 5, 1, 3));
vertices.add(new IpVertex(523, 5, 1, 5));
vertices.add(new IpVertex(524, 5, 3, -5));
vertices.add(new IpVertex(525, 5, 3, -3));
vertices.add(new IpVertex(526, 5, 3, -1));
vertices.add(new IpVertex(527, 5, 3, 1));
vertices.add(new IpVertex(528, 5, 3, 3));
vertices.add(new IpVertex(529, 5, 3, 5));
vertices.add(new IpVertex(530, 5, 5, -5));
vertices.add(new IpVertex(531, 5, 5, -3));
vertices.add(new IpVertex(532, 5, 5, -1));
vertices.add(new IpVertex(533, 5, 5, 1));
vertices.add(new IpVertex(534, 5, 5, 3));
vertices.add(new IpVertex(535, 5, 5, 5));

  // mapping dictionary ip-num
  ipNum = new IntDict();
  for (int i = 0; i < vertices.size(); i++) {
    ipNum.set(str(vertices.get(i).ip), i);
  }

  /* create a new instance of oscP5. 
   * 12000 is the port number you are listening for incoming osc messages*/
  oscP5 = new OscP5(this, 12000);
  /* create a new NetAddress. a NetAddress is used when sending osc messages
   * with the oscP5.send method */
  /* the address of the osc broadcast server */
  myBroadcastLocation = new NetAddress("127.0.0.1", 32000);
}

void draw() {  
  if(ROTATE){
    cam.rotateX(SPEEDX);
    cam.rotateY(SPEEDY);
    cam.rotateZ(SPEEDZ);
  }

  background(BACKGROUND);

  // draw EDGEs 
  stroke(STROKE);
  strokeWeight(1); 
  for (IpVertex vA : vertices) {
    for (IpVertex vB : vertices) {
      if (abs(vA.xpos-vB.xpos)+abs(vA.ypos-vB.ypos)+abs(vA.zpos-vB.zpos)<=2) {
        if (vA.xpos != vB.xpos) {
          line(EDGE*vA.xpos, EDGE*vA.ypos, EDGE*vA.zpos, EDGE*vB.xpos, EDGE*vB.ypos, EDGE*vB.zpos);
        }
      }
    }
  }
  noStroke();

  for (IpVertex v : vertices) {
    v.display();
  }
}

class IpVertex { 
  float xpos, ypos, zpos, amp0, amp1, amp2, r, delta; 
  int ip;
  int alpha;
  IpVertex (int i, float x, float y, float z) {  
    ip = i; 
    xpos = x; 
    ypos = y; 
    zpos = z; 
    amp0 = 0.; 
    amp1 = 0.; 
    amp2 = 0.;
    alpha = ALPHA;
    r = RADIUS;
    delta = DELTA;
  } 

  void updateValues(float v0, float v1, float v2) { 
    amp0 = v0;
    amp1 = v1;
    amp2 = v2;
  } 
  void updateValue0(float v) { 
    amp0 = v;
  } 
  void updateValue1(float v) { 
    amp1 = v;
  } 
  void updateValue2(float v) { 
    amp2 = v;
  } 
  void display() { 
    pushMatrix();

    translate(xpos*EDGE, ypos*EDGE, zpos*EDGE);

    translate(delta*EDGE, 0, 0);

    //fill(noise(amp0) * 255, noise(amp0) * 255, noise(amp0) * 255, alpha);
    fill(C0, A0);
    sphere(r*amp0);

    translate(0, delta*EDGE, 0);
    //fill(noise(amp1) * 255, noise(amp1) * 255, noise(amp1) * 255, alpha);
    fill(C1, A1);
    sphere(r*amp1);

    translate(0, 0, delta*EDGE);
    //fill(noise(amp2) * 255, noise(amp2) * 255, noise(amp2) * 255, alpha);
    fill(C2, A2);
    sphere(r*amp2);
        
    popMatrix();
  }
};

void keyPressed() {
  if (key == ' '){
    ROTATE = !ROTATE;
  }
}


// incoming osc message are forwarded to the oscEvent method. 
void oscEvent(OscMessage theOscMessage) {

  //println("### received an osc message with addrpattern "+theOscMessage.addrPattern()+" and typetag "+theOscMessage.typetag());

  String node = theOscMessage.get(0).stringValue();  
  int vertexNum = ipNum.get(node);
  IpVertex v = vertices.get(vertexNum);
  //println("vertexNum: "+str(vertexNum)+" "+"node: "+node);

  float xA0 = theOscMessage.get(1).floatValue();  
  float xA1 = theOscMessage.get(2).floatValue();  
  float xA2 = theOscMessage.get(3).floatValue();

  //float xB0 = theOscMessage.get(4).floatValue();  
  //float xB1 = theOscMessage.get(5).floatValue();  
  //float xB2 = theOscMessage.get(6).floatValue();

  //float xC0 = theOscMessage.get(7).floatValue();  
  //float xC1 = theOscMessage.get(8).floatValue();  
  //float xC2 = theOscMessage.get(9).floatValue();

  //println(str(vertexNum)+" "+str(xA0)+" "+str(xA1)+" "+str(xA1));
  v.updateValues(xA0, xA1, xA2);
}

Playing Audio from the EPS8266



Mozzi sound synthesis library

    * ESP8266 port of the Mozzi sound synthesis library for Arduino
    * https://github.com/tfry-git/Mozzi


Audio output materials

    * ESP8266  wemos d1 mini lite
    * 2-3W 8Ohm haut-parleur
    * Option A
        * 
    * Option B
        * from ESP8266Audio Arduino library for playing audio on ESP8266 using a software-simulated delta-sigma DAC with dynamic 32x-128x oversampling. 
        * transistor NPN 2N3904
        * 220uF condensateur
        * Connect directly Rx to speaker - and speaker + to 5V.
        * Works but it audio volumen is a lot smaller than with option B.
        * You may also want to add a 220uF capacitor from USB5V to GND just to help filter out any voltage drop during high volume playback.

    * Option C
        * 1K résistance
        * 10uF condensateur
        * 10nF condensateur
        * From audio hacking on the esp8266
        * https://www.hackster.io/janost







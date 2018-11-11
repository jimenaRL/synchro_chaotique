Play audio from the EPS8266 using Mozzi sound synthesis library and OSC control


Requirements

    * ESP8266 port of the Mozzi sound synthesis library for Arduino
    * https://github.com/tfry-git/Mozzi


Materials & Connection

    * ESP8266 wemos d1 mini lite
    * 2-3W 8Ohm haut-parleur
    * Option A
        * Connect directly Rx to speaker - and speaker + to 5V.
        * Works but it audio volumen is a lot smaller than with option B.
    * Option B from [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio/)
        * transistor NPN 2N3904
        * 220uF condensateur
        * Also possible to add a 220uF capacitor from USB5V to GND just to help filter out any voltage drop during high volume playback.
        * Works quite nice.

    * Option C from [audio-hacking-on-the-esp8266](http://blog.dspsynth.eu/audio-hacking-on-the-esp8266/)
        * 1K résistance
        * 10uF condensateur
        * 10nF condensateur








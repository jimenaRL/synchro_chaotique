clear & sudo python osc_server.py -port=666 -neighbors 777 888 -verbose 0 -init 0.1 -frequency 0.3 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=777 -neighbors 888 666 -verbose 0 -init -0.1 -frequency 0.1 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=888 -neighbors 666 777 -verbose 0 -init 3.1415 -frequency 0.1 -couplings -0.5 0.2

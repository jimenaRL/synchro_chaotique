# naive

clear & sudo python osc_server.py -port=666 -neighbors 777 888 -verbose 0 -init 0 -frequency 1 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=777 -neighbors 888 666 -verbose 0 -init 0 -frequency 9 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=888 -neighbors 666 777 -verbose 0 -init 0 -frequency 1 -couplings -0.5 0.2


# runge-kutta

clear & sudo python osc_server.py -port=666 -neighbors 777 888 -method runge-kutta -verbose 0 -init 0 -frequency 1 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=777 -neighbors 888 666 -method runge-kutta -verbose 0 -init 0 -frequency 9 -couplings -0.5 0.2
clear & sudo python osc_server.py -port=888 -neighbors 666 777 -method runge-kutta -verbose 0 -init 0 -frequency 1 -couplings -0.5 0.2

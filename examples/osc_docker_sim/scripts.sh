# euler
sudo clear & sudo python osc_server.py -port=666 -neighbors 777 888 -verbose 1
sudo clear & sudo python osc_server.py -port=777 -neighbors 888 666 -verbose 1
sudo clear & sudo python osc_server.py -port=888 -neighbors 666 777 -verbose 1


# runge-kutta
sudo clear & sudo python osc_server.py -port=666 -neighbors 777 888 -method runge-kutta -verbose 1
sudo clear & sudo python osc_server.py -port=777 -neighbors 888 666 -method runge-kutta -verbose 1
sudo clear & sudo python osc_server.py -port=888 -neighbors 666 777 -method runge-kutta -verbose 1


sudo python osc_server.py -port=666 -neighbors 777 888 -verbose 1 >> 666.log
sudo python osc_server.py -port=777 -neighbors 888 666 -verbose 1 >> 777.log
sudo python osc_server.py -port=888 -neighbors 666 777 -verbose 1 >> 888.log
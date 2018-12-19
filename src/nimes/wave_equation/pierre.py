import numpy as np
from copy import deepcopy

duree=15*64
nb_oscilators =30

def f(y,z):
    ypost = deepcopy(y) #pression au tps t
    zpost = deepcopy(z) #pression au tps t-1
    deltat = 0.1
    deltax = 1./nb_oscilators
    c = 0.1
    mu = 0.01
    for i in range(len(y)):
        imoins = (i-1)%len(y)
        iplus = (i+1)%len(y)

        D2TP = 2.*z[i]-y[i]
        D2XP = -2.*z[i] + z[iplus]+z[imoins]
        DTD2XP = -2.*(z[i]-y[i]) +z[iplus]+z[imoins]-y[iplus]-y[imoins]


        #zpost[i] = D2TP + (deltat**2)*2*c**2 * D2XP/(deltax**2)+deltat*mu * DTD2XP/(deltax*deltax) 
        zpost[i] = D2TP + 0.051* D2XP +0.1*DTD2XP 
        ypost[i] =  z[i]
        #ypost[i] =  z[i]-int(z[i])
        #zpost[i] = zpost[i]-int(zpost[i])
    return (ypost, zpost)



y = np.empty((nb_oscilators, duree))
z = np.empty((nb_oscilators, duree))
#y[:,0] = 0.9*np.random.rand(nb_oscilators)
#y[:,0] += -y[:,0].mean()
y[:,0]=-1
#y[14,0]=-1
#y[16,0]=-1
y[15,0]=29
#y[:,0]=np.sin(2.*np.pi*np.linspace(1, nb_oscilators, nb_oscilators)/30.)

z[:,0] = y[:,0]
#print(z[:,0])
for idx in range(1, duree):
    (y[:, idx], z[:, idx]) = f(y[:,idx-1], z[:, idx-1])
   # z[15, idx]+=5.9*np.cos(0.005*idx)+5.9*np.cos(0.0045*idx)
#    print(z[:,idx-1])

print(y[:,duree-1])
print(z[:,duree-1])
print(y[:,duree-1].mean())
plt.imshow(z, cmap=plt.get_cmap("seismic"), aspect='auto', interpolation='none', vmin=-1, vmax=1)
#plt.imshow(abs(y-z), cmap=plt.get_cmap("seismic"), aspect='auto')


plt.subplots_adjust(bottom=0.1, right=0.8, top=0.9)
cax = plt.axes([0.85, 0.1, 0.075, 0.8])
plt.colorbar(cax=cax)
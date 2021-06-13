from os import read
import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft,ifft
from scipy import signal
import sys




def normal(v):
    max_ = np.max(v, axis = 0)
    min_ = np.min(v, axis = 0)
    return (v-min_)/(max_ - min_)




def plot(filename):
    f = open(filename)
    p=[]
    while(True):
        line = f.readline()
        if line == '':
            break
        else:
            x = line.split(",")
            pressure = int(x[1])
            accx = int(x[2])
            accy = int(x[3])
            accz = int(x[4])
            p.append(pressure)
    
            #print(pressure,accx,accy,accz)
    f.close()
    p = np.array(p)
    #p = p[0:5120]
    #plt.plot(p)
    #taps  = signal.firwin(100, [1.6, 10], pass_zero=False,fs=64)
    #pf = signal.lfilter(taps, 1.0, p)
    pf = p

    f,t,mag = signal.stft(pf , fs=64, noverlap=2048*0.9, nperseg=2048)
    print(mag.shape)
    mag = np.abs(mag)
    #mag = mag[10:1024,:]  # cut dc 
    mag[0:50,:]=0
    mag = normal(mag)
    print(mag[:, 15])
    plt.pcolormesh(mag) #plot
    
    plt.show()

if __name__ == "__main__":
 
   #print("arg :"+str(sys.argv))
   plot(sys.argv[1])

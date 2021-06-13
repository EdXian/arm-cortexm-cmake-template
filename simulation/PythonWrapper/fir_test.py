from os import read
import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import fft,ifft
from scipy import signal
import sys
import cmsisdsp as dsp

def plot(filename):
    f = open(filename)   ##read biologue data
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
    taps  = signal.firwin(100, [1.6, 10], pass_zero=False,fs=64)
    numTaps=100
    blocksize = len(p)
    firf32 = dsp.arm_fir_instance_f32()
    tmp = np.zeros(numTaps + blocksize - 1)
    dsp.arm_fir_init_f32(firf32,100, taps,tmp)
    ps = p
    ps = dsp.arm_fir_f32(firf32,ps)  #use CMSIS DSP 
    pf = signal.lfilter(taps, 1.0, p) #use SCIPY
    plt.title('FIR error')
    print( "mean error: "+str(np.sum(pf-ps)/len(pf)) )
    plt.plot(pf-ps)
    plt.show()

if __name__ == "__main__":
 
   #print("arg :"+str(sys.argv))
   plot(sys.argv[1])

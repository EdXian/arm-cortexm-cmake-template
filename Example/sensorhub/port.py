from serial import Serial
import time
import serial.tools.list_ports as ports
from scipy.fft import fft, ifft
from scipy import signal
import numpy as np
import matplotlib.pyplot as plt
com_ports = list(ports.comports()) # create a list of com ['COM1','COM2'] 
for i in com_ports:            
	print(i.device) # returns 'COMx'   
        
print("Serial start.")

import serial
import struct
print("Serial start.")
COM_PORT = '/dev/ttyACM6'
BAUD_RATES = 115200
ser = serial.Serial(COM_PORT, BAUD_RATES)
t = np.arange(128)
sig = np.sin(2*np.pi*t/16)

#sig = np.random.rand(128)
print(sig)
count = 0
#time.sleep(0.1)
#ser.write([0x56, 0xff])
#time.sleep(0.1)
#ser.write([0x56, 0xff])
#time.sleep(0.1)
try:
    for i in range(int(len(sig)/4)):
        time.sleep(0.05)
        s = struct.pack("<BBffffH",0x55,0x12,sig[count*4+0],sig[count*4+1],sig[count*4+2],sig[count*4+3],0x312)
        ser.write(s)
        data_raw = ser.read(20)
        head,len,a,b,c,d,sum=struct.unpack('<BBffffH',data_raw)
        print(count,'get data:', a,b,c,d)
        count= count+1

#    for i in range(int(len(sig)/4)):
#        data_raw = ser.read(20)
#        head,len,a,b,c,d,sum=struct.unpack('<BBffffH',data_raw)
#        print(count,'get data:', a,b,c,d)
#        count= count+1

except KeyboardInterrupt:
    ser.close()

a = signal.firwin(100,[1.6, 31.5],pass_zero=False,fs=256)
b = signal.lfilter(a, 1.0, sig)
f,t,s= signal.stft( b,window='hamming',nperseg=128,noverlap=0)

print(s)

#print(b)
#b  = np.hamming(128) *b
#y = fft(b)
#print(np.abs(y))
#plt.plot(np.abs(y))

#plt.show()


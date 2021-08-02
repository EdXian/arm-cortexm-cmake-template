from serial import Serial
import time
import serial.tools.list_ports as ports

from scipy import signal
import numpy as np
com_ports = list(ports.comports()) # create a list of com ['COM1','COM2'] 
for i in com_ports:            
	print(i.device) # returns 'COMx'   
        
print("Serial start.")

import serial
import struct
print("Serial start.")
COM_PORT = 'COM8'
BAUD_RATES = 115200
ser = serial.Serial(COM_PORT, BAUD_RATES)
t = np.arange(100)
sig = np.sin(2*np.pi*t/20)
print(sig)

try:
    for i in range(int(len(sig)/4)):
        time.sleep(0.1)
        s = struct.pack("<BBffffH",0x55,0x12,sig[i*4+0],sig[i*4+1],sig[i*4+2],sig[i*4+3],0x312)
        ser.write(s)
        while ser.in_waiting:
            data_raw = ser.read(20)
            head,len,a,b,c,d,sum=struct.unpack('<BBffffH',data_raw)
            print(i,'get data:', a,b,c,d)
except KeyboardInterrupt:
    ser.close()

a = signal.firwin(100,[1.6, 31.5],pass_zero=False,fs=256)
b = signal.lfilter(a, 1.0, sig)
print(b)


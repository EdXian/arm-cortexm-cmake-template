from serial import Serial
import time
import serial.tools.list_ports as ports
import numpy as np
com_ports = list(ports.comports()) # create a list of com ['COM1','COM2'] 
for i in com_ports:            
	print(i.device) # returns 'COMx'   
        
print("Serial start.")

import serial
import struct
print("Serial start.")
COM_PORT = '/dev/ttyACM1'
BAUD_RATES = 115200
ser = serial.Serial(COM_PORT, BAUD_RATES)
t = np.arange(100)
signal = np.sin(2*np.pi*t/20)
print(signal)

try:
    for i in range(int(len(signal)/4)):
        time.sleep(1)
        s = struct.pack("<BBffffH",0x55,0x12,signal[i*4+0],signal[i*4+1],signal[i*4+2],signal[i*4+3],0x312)
        ser.write(s)
        while ser.in_waiting:
            data_raw = ser.read(20)
            head,len,a,b,c,d,sum=struct.unpack('<BBffffH',data_raw)
            print('get data:', a,b,c,d)
except KeyboardInterrupt:
    ser.close()



#try:
#    while True:
#        time.sleep(1)
#        s = struct.pack("<BBffffH",0x55,0x12,3.14,3.14,3.14,3.14,0x312)


#        ser.write(s)
#        while ser.in_waiting:

#            data_raw = ser.read(20)
#            head,len,a,b,c,d,sum=struct.unpack('<BBffffH',data_raw)

#            #ser.write(s)
#            #data = data_raw.decode()
#            #print('getdata：', data_raw)
#            print('get data:', a,b,c,d)

#except KeyboardInterrupt:
#    ser.close()


import cmsisdsp as dsp
import numpy as np
from scipy import signal
import matplotlib.pyplot as  plt
from scipy.fft import fft, ifft
from ctypes import *
'''
sample = 64
samples = np.arange(0.0,sample,1.0)
t = np.cos(samples*2*np.pi/16)
print(len(samples))
x = dsp.dsp_stft(samples,0)
print(x)
'''
# test for hamming window
'''
x = np.zeros(51)
x[0:len(x)] = 1
y = np.hamming(51)
x = dsp.dsp_hamming_window(x,len(x))
print(x-y)
plt.plot(x-y)
plt.show()
'''

# test for normalize
'''
sample =1024
samples = np.arange(0.0,sample,1.0)
t = 3*np.cos(samples*2*np.pi/16)
t = dsp.dsp_normalize(t)
plt.plot(t)
plt.show()
'''

#test for stft (pass hamming window before do fft)
'''
sample = 1024 
samples = np.arange(0.0,sample,1.0)
signal = 3*np.cos(samples*2*np.pi/16) + np.cos(samples*2*np.pi/64)
t = dsp.dsp_stft(signal)
v=np.hamming(1024)

v = v*signal
v = fft(v)
v = np.abs(v)
v = v[0:512]
plt.plot(v-t)
plt.show()
'''


sample = 1024
samples = np.arange(0.0,sample,1.0)
signal = 3*np.cos(samples*2*np.pi/16) + np.cos(samples*2*np.pi/64)
t = dsp.dsp_reserve_freq_band(signal,100,900,58.3)
plt.plot(t)
plt.show()









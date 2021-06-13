from os import read
import numpy as np
import matplotlib.pyplot as plt

from matplotlib.colors import LogNorm

f = open('stft.txt')
sign = 0
cnt = 0
x=[]
y=[]
cnt = 0
fft_size = 512
v = np.zeros(fft_size)

def normal(v):
    max_ = np.max(v)
    min_ = np.min(v)
    return (v-min_)/(max_ - min_)

while(cnt<2000):
    s = f.readline()
    s_ =  ('==p%d==\n') % cnt
    x=[]
    if s==s_:
        read_cnt = 0
        while(read_cnt<fft_size):
            data = f.readline()
            temp=float(data) 
            x.append(temp)
            read_cnt=read_cnt+1
        
        x = np.array(x)

        x[0] = 0
        x[1] = 0
        v = np.vstack((v,x))
    else:
        break
    cnt=cnt+1

v=v[1:]
v=v[:,1:fft_size]

bpm = []
fd = open('bpm.txt')
while(True):
        line = fd.readline()
        if line == '':
            break
        else:
            bpm_value = float(line)
            bpm.append(bpm_value)
    
#print(bpm)
bpm = np.array(bpm)


bpm_g = []
fr = open('bpm_g.txt')
while(True):
    line = fr.readline()
    if line == '':
        break
    else:
        bpm_value = float(line)
        bpm_g.append(bpm_value)

bpm_g = np.array(bpm_g)


         

scenario = []
fs = open('scenario.txt')
while(True):
    line = fs.readline()
    if line == '':
        break
    else:
        sc = float(line)
        scenario.append(sc)

scenario = np.array(scenario)





plt.pcolormesh(v.T)

'''


plt.subplot(412)
plt.ylim([50,100])
plt.plot(bpm)


plt.subplot(413)
#plt.ylim([50,100])
plt.plot(bpm_g)


'''
plt.show()
f.close()
fd.close()
fr.close()
    

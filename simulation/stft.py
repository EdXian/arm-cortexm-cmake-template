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
v = np.zeros(512)

def normal(v):
    max_ = np.max(v)
    min_ = np.min(v)
    return (v-min_)/(max_ - min_)

while(cnt<1000):
    s = f.readline()
    s_ =  ('==p%d==\n') % cnt
    x=[]
    if s==s_:
        read_cnt = 0
        while(read_cnt<512):
            data = f.readline()
            temp=float(data) 
            x.append(temp)
            read_cnt=read_cnt+1
        
        x = np.array(x)
  	
        v = np.vstack((v,x))
    else:
        break
    cnt=cnt+1

v=v[1:]
print(cnt)
plt.pcolormesh(v.T)

plt.show()
f.close()
    
    

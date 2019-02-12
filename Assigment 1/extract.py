import collections
import numpy as np
import matplotlib.pyplot as plt
import statistics

f = open("ping_p.txt",'r')
dicti = {}
data = []
for line in f:
	stir = line[-9:-4]
	if stir[0] == '=':
		stir=stir[1:]
	data.append(float(stir))
	print(stir)
	if stir in dicti:
		dicti[stir] +=1
	else:
		dicti[stir]=1

m = collections.OrderedDict(sorted(dicti.items()))

print(statistics.median(data))

x=[]
y=[]

for i,j in m.items():
	x.append(i)
	y.append(j)
	print(i)

# # print((x[500]+x[501])/2)
# # print(x[])
# plt.bar(x,y)
# plt.xlabel('Latency (ms)')
# plt.ylabel('Frequency')
# # plt.title('ping -p ff00 202.141.80.14')
# plt.title('ping -n -c 1000 202.141.80.14')
# plt.xticks(np.arange(0.175, len(x), 10.0),rotation=45)
# plt.show()
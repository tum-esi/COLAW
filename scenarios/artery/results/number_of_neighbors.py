import pandas as pd 
import numpy
import matplotlib.pyplot as plt 
import io
from collections import Counter

def parse_ndarray(s):
    return numpy.fromstring(s, sep=' ') if s else None

veins_obstacles = pd.read_csv('test.csv', converters={'vecvalue': parse_ndarray})

full_proofs = veins_obstacles[(veins_obstacles.type=='vector')].vecvalue

B = Counter({1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0, 9:0, 10:0})

for row in full_proofs:
	unique, counts = numpy.unique(row, return_counts=True)
	B = B + Counter(dict(zip(unique, counts)))

print(B)

B = sorted(B.items(), key=lambda pair: pair[0])

print(B)
plt.bar(range(len(B)), [t[1] for t in B], align='center')
plt.xticks(range(len(B)), [int(t[0]) for t in B])
# # for python 2.x:
# plt.bar(range(len(D)), D.values(), align='center')  # python 2.x
# plt.xticks(range(len(D)), D.keys())  # in python 2.x

plt.show()
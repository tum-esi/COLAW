import pandas as pd 
import numpy as np 
import matplotlib.pyplot as plt 
import matplotlib.colors as mcolors
import sys
from collections import Counter
import io
from matplotlib.patches import Patch
import os
import matplotlib.ticker as ticker


# Autoscale so that xlabel also fits
from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})


def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None

# Import .csv file
veins_obstacles = pd.read_csv(str(sys.argv[1]), converters={'vecvalue': parse_ndarray})
plt_name = os.path.splitext(sys.argv[1])[0]

# Create colormap
clist = [(0, '#f9ba00ff'), (0.125, '#ffdc00ff'), (0.25, '#a2ad00ff'), (0.5, '#007c30ff'), 
         (0.7, '#00778aff'), (0.75, '#98c6eaff'), (1, '#0073cfff')]

rvb = mcolors.LinearSegmentedColormap.from_list("", clist, N=60)
sm = plt.cm.ScalarMappable(cmap=rvb, norm=plt.Normalize(vmin=1, vmax=60))
# fake up the array of the scalar mappable. Urgh…
# https://medium.com/data-science-canvas/way-to-show-colorbar-without-calling-imshow-or-scatter-4a378058316
sm._A = []

print("----------------------------------------------------------------------")
print("-------------------------FIRST ANALYSIS------------------------------")
print("----------------------------------------------------------------------")

# Get the generated proofs and the requested proofs
proof_size = veins_obstacles[(veins_obstacles.type=='vector') & (veins_obstacles.name=='proof_size')].vecvalue
proof_requested = veins_obstacles[(veins_obstacles.type=='vector') & (veins_obstacles.name=='proof_requested')].vecvalue

# Usage of the starting Counter because I don't want to loose the entries with zero count
A = Counter({0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0, 9:0, 10:0, 11:0, 12:0, 13:0, 14:0, 15:0, 16:0, 17:0, 18:0, 19:0, 20:0,
	21:0, 22:0, 23:0, 24:0, 25:0, 26:0, 27:0, 28:0, 29:0, 30:0, 31:0, 32:0, 33:0, 34:0, 35:0, 36:0, 37:0, 38:0, 39:0, 40:0, 41:0,
	42:0, 43:0, 44:0, 45:0, 46:0, 47:0, 48:0, 49:0, 50:0, 51:0, 52:0, 53:0, 54:0, 55:0, 56:0, 57:0, 58:0, 59:0, 60:0})

array = [A]*60 # Represents the x-axis of the final proof

# In parallel, go through the requested proofs and the generated proofs
# e.g. (x,y) = (15,9) means that a 15-size proof was requested and a 
# 9-size proof was actually generated. Generate a temporal Counter which 
# has an entry 9:1 and add him to the 15th row of "array"
for req, siz in zip(proof_requested, proof_size):
	for x,y in zip(req,siz):
		tmp = Counter({0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0, 9:0, 10:0, 11:0, 12:0, 13:0, 14:0, 15:0, 16:0, 17:0, 18:0, 19:0, 20:0,
			21:0, 22:0, 23:0, 24:0, 25:0, 26:0, 27:0, 28:0, 29:0, 30:0, 31:0, 32:0, 33:0, 34:0, 35:0, 36:0, 37:0, 38:0, 39:0, 40:0, 41:0,
			42:0, 43:0, 44:0, 45:0, 46:0, 47:0, 48:0, 49:0, 50:0, 51:0, 52:0, 53:0, 54:0, 55:0, 56:0, 57:0, 58:0, 59:0, 60:0})		
		unique, counts = np.unique(y, return_counts=True)
		tmp.update(Counter(dict(zip(unique, counts))))
		array[int(x)-1] = tmp + array[int(x)-1]

# Calculate total number of requested and generated proofs just for 
# duplechecking with "Second Analysis"
total_requests = 0
total_generated = 0
for idx, val in enumerate(array):
	total_requests += sum([pair[1] for pair in val.items()])
	if sum([pair[1] for pair in val.items()]) != 0:
		for key,value in val.items():
			if (value != 0) & (key != "total"):
				if key != 0:
					total_generated += value

print("Total requested location proofs: ", int(total_requests))
print("Total generated location proofs: ", int(total_generated))

# Sort the all collections of "array"
for idx, entry in enumerate(array):
	array[idx] = sorted(entry.items(), key=lambda pair: pair[0])

fig, ax = plt.subplots()
bottom = np.array(0)
# Go through all "array"-entries (collection) and draw a stacked
# bar-graph from the collection. Draw the 0-entry (failed) red and
# the rest according to the defined colormap
for idx, entry in enumerate(array):
	for data in entry:
		if data[0] == 0:
			# +1 because array[0] corresponds to requested proof size 1
			bars = ax.bar(idx+1, data[1], bottom=bottom, color='#9c0d16ff')
			bottom = bottom + np.array(data[1])
		if data[0] != 0:
			# +1 because array[0] corresponds to requested proof size 1
			bars = ax.bar(idx+1, data[1], bottom=bottom, color=rvb(data[0]/60))
			bottom = bottom + np.array(data[1])
	bottom = np.array(0)

# Colormap settings
cbar = fig.colorbar(sm)
cbar.ax.yaxis.set_label_position('left')
cbar.ax.set_ylabel('Proof size')

# Plot ticks settings
minors = np.arange(0,60)
ax.xaxis.set_minor_locator(ticker.FixedLocator(minors))
ax.xaxis.set_major_locator(ticker.FixedLocator([0,10,20,30,40,50,60]))

# Plot settings
ax.set_ylabel('Number of proofs')
ax.set_xlabel('Size of requested proofs')
legend_elements = [Patch(facecolor='#9c0d16ff', label='Failed proofs')]
ax.legend(handles=legend_elements)

# Save plot
plt.savefig(plt_name + '_relationship.pdf')
plt.savefig(plt_name + '_relationship.pgf')
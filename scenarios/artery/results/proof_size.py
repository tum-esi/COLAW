import pandas as pd 
import numpy as np 
import matplotlib.pyplot as plt 
import matplotlib.colors as mcolors
import sys
from collections import Counter
import io
from matplotlib.patches import Patch
import os
import math
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

rvb = mcolors.LinearSegmentedColormap.from_list("", clist)
sm = plt.cm.ScalarMappable(cmap=rvb, norm=plt.Normalize(vmin=1, vmax=60))
# fake up the array of the scalar mappable. Urgh…
# https://medium.com/data-science-canvas/way-to-show-colorbar-without-calling-imshow-or-scatter-4a378058316
sm._A = []

print("----------------------------------------------------------------------")
print("-------------------------SECOND ANALYSIS------------------------------")
print("----------------------------------------------------------------------")

# Get the generated proofs
proof_size = veins_obstacles[(veins_obstacles.type=='vector') & (veins_obstacles.name=='proof_size')].vecvalue

# Usage of the starting Counter because I don't want to loose the entries with zero count
B = Counter({0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0, 8:0, 9:0, 10:0, 11:0, 12:0, 13:0, 14:0, 15:0, 16:0, 17:0, 18:0, 19:0, 20:0,
	21:0, 22:0, 23:0, 24:0, 25:0, 26:0, 27:0, 28:0, 29:0, 30:0, 31:0, 32:0, 33:0, 34:0, 35:0, 36:0, 37:0, 38:0, 39:0, 40:0, 41:0,
	42:0, 43:0, 44:0, 45:0, 46:0, 47:0, 48:0, 49:0, 50:0, 51:0, 52:0, 53:0, 54:0, 55:0, 56:0, 57:0, 58:0, 59:0, 60:0})

# Go through all the generated proofs and add them to the collection
for row in proof_size:
	unique, counts = np.unique(row, return_counts=True)
	B.update(Counter(dict(zip(unique, counts))))

B.update(Counter({0.5:0})) # So that there is a bit of space between "Failed" and 1

# Sort the collection of all generated proofs
B = sorted(B.items(), key=lambda pair: pair[0])

print("Total requested location proofs: ", sum([pair[1] for pair in B]))
print("Total generated location proofs: ", sum([pair[1] for pair in B if pair[0] != 0]))
print("Total failed location proofs: ", B[0][1])

# Calculate mean and standard deviation
sum_of_numbers = sum(number*count for number, count in B)
count = sum(count for n, count in B)
mean = sum_of_numbers / count

total_squares = sum(number*number * count for number, count in B)
mean_of_squares = total_squares / count
variance = mean_of_squares - mean * mean
std_dev = math.sqrt(variance)

print("Mean: ", mean)
print("Standard deviation: ", std_dev)

# Vector for bar color generation
x = np.arange(len(B)).astype(float)

# Plot bar graph
fig, ax = plt.subplots()
plt.tight_layout()

# Plot the bar graph
bars = ax.bar(range(len(B)), [t[1] for t in B], align='center', color=rvb(x/60))
bars[0].set_color('#9c0d16ff') # Failed proofs
minors = np.arange(2,61)

# Plot ticks settings
ax.xaxis.set_minor_locator(ticker.FixedLocator(minors))
ax.xaxis.set_major_locator(ticker.FixedLocator([0,2,11,21,31,41,51,61]))
ticks = ax.xaxis.get_ticklabels()
ticks[0].set_rotation(90) # Set rotations
labels = ["Failed", 1, 10, 20, 30, 40, 50, 60] # Because of the 0.5 insertion have to rename the x-axis
ax.set_xticklabels(labels)

# Plot settings
ax.set_ylabel('Number of proofs')
ax.set_xlabel('Size of generated proofs', labelpad=-13)

# Save plot
plt.savefig(plt_name + '_size.pdf')
plt.savefig(plt_name + '_size.pgf')

###################################################
#### Bar graph comparing full+half with failed ####
###################################################

# Subplot no. 1
# plt.subplot(grid[1,2])

# import tikzplotlib
# tikzplotlib.save(plt_name + '_size.tex')

# full_proofs = sum(veins_obstacles[(veins_obstacles.name=='#full_proofs')].value)
# half_proofs = sum(veins_obstacles[(veins_obstacles.name=='#half_proofs')].value)
# failed_proofs = sum(veins_obstacles[(veins_obstacles.name=='#failed_proofs')].value)

# p1 = plt.bar(1, full_proofs, color='#0073cfff')
# p2 = plt.bar(1, half_proofs, bottom=full_proofs, color='#007c30ff')

# bottom = np.array(0)
# for data in B:
# 	p1 = plt.bar(2, data[1], bottom=bottom, color=rvb(data[0]/60), width=0.5)
# 	bottom = bottom + np.array(data[1])

# p2 = plt.bar(3, failed_proofs, color='#9c0d16ff', width=0.5)

# print("Total location proof requests: ", int(full_proofs+half_proofs+failed_proofs))
# print("Total generated location proofs: ", int(full_proofs+half_proofs))

# plt.ylabel('Number of Proofs')
# plt.xticks(range(1, 5), ("", "Success", "Failed", ""))
# plt.savefig(plt_name + '_comparison.pdf')
# plt.savefig(plt_name + '_comparison.pgf')

'''
==============
Visualize the vector fields 
Layout: 
1.) Line: layout 
2.) Line: max/min values
3.) rest of lines filled with vector data 
==============
'''

from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
import numpy as np

import os

# read vector field in 
dir_name = os.path.dirname(__file__)
dir_name += "/../Data/"
file_name = "vektorfield.dat"

full_file_path = dir_name + file_name

f = open(full_file_path, "r")

# first line describes data layout 
data_layout = f.readline()

# 2nd line 
limits = f.readline().split()

width = int(limits[0])
height = int(limits[1])
depth = int(limits[2])

u = np.arange(width * height * depth).reshape(width, height, depth)
v = np.arange(width * height * depth).reshape(width, height, depth)
w = np.arange(width * height * depth).reshape(width, height, depth)

u = u.astype('float64')
v = v.astype('float64')
w = w.astype('float64')

for z in range(depth): 
    for y in range(height):
        for x in range(width):

            # read in x,y and z component from file 
            line = f.readline()

            line = line.split()
            u[z][y][x] = float(line[0])
            v[z][y][x] = float(line[1])
            w[z][y][x] = float(line[2])


f.close()

fig = plt.figure()
ax = fig.gca(projection='3d')

# Make the grid
y, z, x = np.meshgrid(np.arange(0, width, 1),
                      np.arange(0, height, 1),
                      np.arange(0, depth, 1))

ax.quiver(x, y, z, u, v, w, normalize=True)

plt.show()
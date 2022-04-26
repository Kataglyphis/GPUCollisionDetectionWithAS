'''
==============
Visualize the particles 
Layout: 
1. Line: layout 
2. Line: dim values
3. - 3+(dim.x*dim.y*dim.z) line: filled with particle data 

struct Particle {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    vec4 color;
}

==============
'''

import matplotlib.pyplot as plt
import numpy as np

import os

# read vector field in 
dir_name = os.path.dirname(__file__)
dir_name += "/../Data/"
file_name = "particles.dat"

full_file_path = dir_name + file_name

f = open(full_file_path, "r")

# first line describes data layout 
data_layout = f.readline()

# num particles in each dimension 
limits = f.readline().split()

width = int(limits[0])
height = int(limits[1])
depth = int(limits[2])

u = np.arange(width * height * depth).reshape(width, height, depth)
v = np.arange(width * height * depth).reshape(width, height, depth)
w = np.arange(width * height * depth).reshape(width, height, depth)

fig = plt.figure()
ax = fig.add_subplot(projection='3d')

for z in range(depth): 
    for y in range(height):
        for x in range(width):

            # read in x,y and z component from position from file 
            line = f.readline()

            line = line.split()
            u[x][y][z] = float(line[0])
            v[x][y][z] = float(line[1])
            w[x][y][z] = float(line[2])

            ax.scatter(u[x][y][z], v[x][y][z], w[x][y][z], marker='o')

            # handle velocity 
            line = f.readline()

            # handle acceleration
            line = f.readline()

            # handle color
            line = f.readline()

            # skip on empty line 
            line = f.readline()

f.close()

ax.set_xlabel('Position X')
ax.set_ylabel('Position Y')
ax.set_zlabel('Position Z')

plt.show()
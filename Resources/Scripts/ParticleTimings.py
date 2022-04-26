from cProfile import label
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

# Data for plotting
t = np.array([32768,65536,131072,262144,524288,1048576])
s = np.array([0.103,0.185,0.35,0.67,1.27, 2.48])

# t = np.array([1,2,3,4])
# s = np.array([1,4,9,16])
fig, ax2 = plt.subplots()
# ax1.plot(t, s)
ax2.scatter(t,s, marker='x')
ax2.set_ylim(0, 3)
ax2.set_xlim(0, 1100000)
ax2.set(xlabel='# particles', ylabel='time (ms)',
       title='Particle simulation timings')
ax2.grid()

fig.savefig("particle_timings.png")
plt.show()

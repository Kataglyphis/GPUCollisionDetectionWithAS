import matplotlib.pyplot as plt
import numpy as np
import csv 
import os 
from operator import itemgetter 

# read vector field in 
dir_name = os.path.dirname(__file__)
dir_name += "/../Data/"
file_name = dir_name + "workGroupExp.csv"

# fix workgroup size indices for our csv file 
WORK_GROUP_SIZE_DX_INDEX        = 0
WORK_GROUP_SIZE_DY_INDEX        = 1
WORK_GROUP_SIZE_DZ_INDEX        = 2

# number of particles in each dimension 
NUM_PARTICLES_X_INDEX           = 3
NUM_PARTICLES_Y_INDEX           = 4
NUM_PARTICLES_Z_INDEX           = 5

# all our messured timings 
TIMING_SIMULATION_PASS_INDEX    = 6
TIMING_INTEGRATION_PASS_INDEX   = 7
TIMING_COMPUTE_PASS_INDEX       = 8

# ----- UTIL FUNCTIONS FOR DEALING WITH CSV FILES -----
def read_csv_file(name, fields, rows):

    with open(name, 'r', newline='') as csvfile:
        csvreader = csv.reader(csvfile, dialect='excel', delimiter=";")
        # extracting field names through first row
        fields = next(csvreader)
        for row in csvreader:
            # print(', '.join(row))
            rows.append(row)
        return fields, rows

# --- KEEP IN MIND: We needed to replace '.' with ',' in our csv files for the floats
def get_simulation_pass_timing(file_data_row):
    return float(file_data_row[TIMING_SIMULATION_PASS_INDEX].replace(',','.'))

def get_integration_pass_timing(file_data_row):
    return float(file_data_row[TIMING_INTEGRATION_PASS_INDEX].replace(',','.'))

def get_compute_pass_timing(file_data_row):
    return float(file_data_row[TIMING_COMPUTE_PASS_INDEX].replace(',','.'))

def get_workgroup_size_x(file_data_row):
    return int(file_data_row[WORK_GROUP_SIZE_DX_INDEX])

def get_workgroup_size_y(file_data_row):
    return int(file_data_row[WORK_GROUP_SIZE_DY_INDEX])

def get_workgroup_size_z(file_data_row):
    return int(file_data_row[WORK_GROUP_SIZE_DZ_INDEX])

def get_num_particles_x(file_data_row):
    return int(file_data_row[NUM_PARTICLES_X_INDEX])

def get_num_particles_y(file_data_row):
    return int(file_data_row[NUM_PARTICLES_Y_INDEX])

def get_num_particles_z(file_data_row):
    return int(file_data_row[NUM_PARTICLES_Z_INDEX]) 

# ----- Compare compute timings -----
# --- First plot: all workgroup size combos in comparison
# first row holds data layout 
data_layout = []
file_data = []

read_csv_file(file_name, data_layout, file_data)

file_data.sort(key=itemgetter(TIMING_COMPUTE_PASS_INDEX))

fig, ax = plt.subplots()
plt.title("Timings for #" + str(len(file_data)) + " working group combos for #"   +  str( int(file_data[0][NUM_PARTICLES_X_INDEX]) * 
                                                                            int(file_data[0][NUM_PARTICLES_Y_INDEX]) *
                                                                            int(file_data[0][NUM_PARTICLES_Z_INDEX])) 
                                                                         + " particles")
plt.ylabel("Runtime (ms)")
plt.xlabel("Workgroup sizes in X,Y,Z")
ax.axes.xaxis.set_ticklabels([])
plt.tick_params(bottom=False)

workgroup_combos =  []

for i in range(len(file_data)) : 
    workgroup_combos.append(    str(get_workgroup_size_x(file_data[i])) + " " +
                                str(get_workgroup_size_y(file_data[i])) + " " +
                                str(get_workgroup_size_z(file_data[i])) + " ")

compute_timings = []
for i in range(len(file_data)):
    compute_timings.append(get_compute_pass_timing(file_data[i]))

plt.plot(workgroup_combos, compute_timings)
plt.show()

# --- Show 5 Best work group combos
fig, ax = plt.subplots()
numSamples = 10
plt.title(str(numSamples) + " best working work group combos for #"   + str( get_num_particles_x(file_data[0]) * 
                                                                        get_num_particles_y(file_data[0]) *
                                                                        get_num_particles_z(file_data[0])) 
                                                                        + " particles")
plt.ylabel("Runtime (ms)")
plt.xlabel("Workgroup sizes in X,Y,Z")
workgroup_combos =  []

for i in range(numSamples) : 
    workgroup_combos.append(    str(get_workgroup_size_x(file_data[i])) + " " +
                                str(get_workgroup_size_y(file_data[i])) + " " +
                                str(get_workgroup_size_z(file_data[i])) + " " )

compute_timings = []
for i in range(numSamples):
    compute_timings.append(get_compute_pass_timing(file_data[i]))

plt.bar(workgroup_combos, compute_timings, alpha=0.6)
plt.show()

# --- Show 5 Worst work group combos
fig, ax = plt.subplots()
plt.title(str(numSamples) + " worst working work group combos for #"   + str( get_num_particles_x(file_data[0]) * 
                                                            get_num_particles_y(file_data[0]) *
                                                            get_num_particles_z(file_data[0])) 
                                                             + " particles")
plt.ylabel("Runtime (ms)")
plt.xlabel("Workgroup sizes in X,Y,Z")
workgroup_combos =  []

for i in range(numSamples) : 
    workgroup_combos.append(    str(get_workgroup_size_x(file_data[len(file_data) - i - 1])) + " " +
                                str(get_workgroup_size_y(file_data[len(file_data) - i - 1])) + " " +
                                str(get_workgroup_size_z(file_data[len(file_data) - i - 1])) + " " )

compute_timings = []
for i in range(numSamples):
    compute_timings.append(get_compute_pass_timing(file_data[len(file_data) - i - 1]))

plt.bar(workgroup_combos, compute_timings, alpha=0.6)
plt.show()

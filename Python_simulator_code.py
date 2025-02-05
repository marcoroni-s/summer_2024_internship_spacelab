# Imports necessary libraries for our program
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Defining Constants
g = 9.81
m = 0.136
p = 1.225
A = np.pi*(0.0275**2)
C_d = 0.44
c = (1/2)*C_d*p*A
dt = 0.001 # Our time step

# Reading our Thrust Curve .csv file
tcurve = pd.read_csv('Estes_C5.csv')
timeval = tcurve['Time (s)']
thrustval = tcurve['Thrust (N)'] # Creates an array from the values in column labelled 'Thrust (N)' in our .csv file
massval = tcurve['Mass (kg)']
thrust_time = round(timeval.max(),3) # Records our thrust time and rounds it to two decimals.

# Defining function to Calculate Acceleration during Boost Phase
def f(v):
    return -g + T/m - c*v**2/m

# Creating an array of time steps from zero to the total thrust time
time = np.arange(0,thrust_time+dt,dt)

# create empty lists
yd = []
y = []
drag = []

# set initial conditions
yd.append(0)
y.append(0)
drag.append(0)
k = 0

# Loop to simulate values and create Velocity, Altitude, and Drag Lists
for i in range(len(time) - 1):
    if time[i] >= timeval[k]:
        T = (thrustval[k + 1] + thrustval[k])/2 # Averages Thrust for 2 points and considering it for all points in between
        m = m - (massval[k] - massval[k + 1]) # Accounts for reduction in mass as motor burns
        k = k + 1
    yd.append(yd[i] + f(yd[i])*dt)
    y.append(y[i] + yd[i]*dt)
    drag.append(c*(yd[i])**2)

# Using a while loop to simulate values for coast phase until rocket hits apogee
i = int(thrust_time*1000 - 1)
T = 0 # During Coast Phase Thrust is 0
while yd[i] >= 0:
    yd.append(yd[i] + f(yd[i])*dt)
    y.append(y[i] + yd[i]*dt)
    drag.append(c*(yd[i])**2)
    i = i + 1

# Redefining time to include both boost and coast phase
time = np.arange(0,len(y))/1000

# Prints Apogee in meters & feet
print('Apogee [m]:',y[i])
print('Apogee [ft]:',y[i]*3.281)

# Plots the first figure with flight altitude and velocity with respect to time
plt.figure(0)
plt.plot(time,y)
plt.plot(time,yd)
plt.xlabel('Time [s]')
plt.ylabel('Altitude [m]  |  Velocity [m/s]')
plt.legend(['Altitude [m]', 'Velocity [m/s]'])
plt.title('(Altitude & Velocity) vs Time Graph')
plt.grid(True)
plt.savefig("Rocket Trajectory Graph")

# Plots the second figure with flight drag with respect to time
plt.figure(1)
plt.plot(time, drag)
plt.title('Drag Force vs Time')
plt.xlabel('Time [seconds]')
plt.ylabel('Drag [N]')
plt.grid(True)
plt.savefig("Drag Graph")

# Shows all plots that use the matplotlib functions
plt.show()


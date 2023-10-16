import xavna
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def take_sweep(x):
    file_saved = False
    while not file_saved:
        #vna.save_meas_data_to_file(False)
        file_saved = vna.save_S21_magnitude_to_file()

        if (file_saved):
            f = open("meas21.txt", "r")
            x = []
            y = []
            for line in f:
                data = line.replace('\n','').split('\t')
                x.append(float(data[0])/1e6)
                y.append(float(data[1]))

            ax.clear()
            ax.plot(x , y)

            plt.title("S21 of the filter.")
            plt.xlabel('Frequency, MHz')
            plt.ylabel('Magnitude, dB')
            plt.xlim(start/1e6, stop/1e6)
            plt.ylim(-70, 3)

# To make it work on Debian.
#matplotlib.use("TkAgg")
vna = xavna.VNA()
if (vna.obj == 0):
    exit()

#vna.set_sweep_params(750e6, 1000e6, 201, 1)
vna.debug(True)
vna.loadSOLTCalibration("SOLT_750-1000-100.cal")
#vna.denySOLT()
start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.start_scan()
time.sleep(1)

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
while(1):
    ani = animation.FuncAnimation(fig, take_sweep, interval=100)
    plt.show()
    
vna.stop_scan()

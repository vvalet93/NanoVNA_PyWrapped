import xavna
import time
import matplotlib.pyplot as plt

vna = xavna.VNA()
vna.set_sweep_params(200e6, 500e6, 51, 1)
start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.start_scan()
time.sleep(5)
plt.ion()
fig, ax = plt.subplots()
#ax = fig.add_subplot(111)
ax.set_xlabel('Frequency, MHz')
ax.set_ylabel('Magnitude, dB')
ax.set_xlim(start/1e6, stop/1e6)
ax.set_ylim(-3, 3)
S21, = ax.plot([], [])


while(1):
    time.sleep(0.2)
    file_saved = vna.save_S21_magnitude_to_file()
    if (file_saved):
        f = open("meas21.txt", "r")
        x = []
        y = []
        for line in f:
            data = line.replace('\n','').split('\t')
            x.append(float(data[0])/1e6)
            y.append(float(data[1]))
        
        S21.set_xdata(x)
        S21.set_ydata(y)
        fig.canvas.draw()
        fig.canvas.flush_events()
    
vna.stop_scan()

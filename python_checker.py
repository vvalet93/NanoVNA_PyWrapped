import xavna


vna = xavna.VNA()
start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.set_sweep_params(200e6, 500e6, 101, 2)

start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.start_scan()

while(1):
    vna.save_meas_data_to_file("meas.txt")

vna.stop_scan()

import xavna


vna = xavna.VNA()
start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.set_sweep_params(200e6, 500e6, 51, 30)

start = vna.start_freq_hz
stop = vna.stop_freq_hz
points = vna.sweep_points_count
average = vna.average_rate

vna.start_scan()
#vna.set_sweep_params(200e6, 300e6, 51, 30)
vna.stop_scan()

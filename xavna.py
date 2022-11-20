from ctypes import *

class VNA(object):
    def __init__(self):
        self._xavnalib = cdll.LoadLibrary('./build/xavna_controller.so')
        self.obj = self._xavnalib.findVnaAndConnect()

    def start_scan(self):
        self._xavnalib.startScan(self.obj)

    def stop_scan(self):
        self._xavnalib.stopScan(self.obj)

    def disconnect(self):
        self._xavnalib.disconnect(self.obj)

    def set_sweep_params(self, start_freq, stop_freq, points, average):
        self._xavnalib.setSweepParams(self.obj, c_double(start_freq), c_double(stop_freq), c_int(points), c_int(average))

    def __del__(self):
        self.disconnect()
        self._xavnalib.dispose(self.obj)

    # Sweep parameters - are read-only properties. Change them using safe method set_sweep_params.
    @property
    def start_freq_hz(self) -> c_double:
        self._xavnalib.getStartFreqHz.restype = c_double
        return self._xavnalib.getStartFreqHz(self.obj)

    @property
    def stop_freq_hz(self) -> c_double:
        self._xavnalib.getStopFreqHz.restype = c_double
        return self._xavnalib.getStopFreqHz(self.obj)

    @property
    def step_freq_hz(self) -> c_double:
        self._xavnalib.getStepFreqHz.restype = c_double
        return self._xavnalib.getStepFreqHz(self.obj)

    @property
    def sweep_points_count(self) -> c_int:
        return self._xavnalib.getPointsCount(self.obj)

    @property
    def average_rate(self) -> c_int:
        return self._xavnalib.getAverageRate(self.obj)
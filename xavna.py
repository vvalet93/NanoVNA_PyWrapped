from ctypes import *

xavnalib = cdll.LoadLibrary('./build/xavna_controller.so')

class VNA(object):
    def __init__(self):
        self.obj = xavnalib.findVnaAndConnect()

    def startScan(self):
        xavnalib.startScan(self.obj)

    def stopScan(self):
        xavnalib.stopScan(self.obj)

    def __del__(self):
        xavnalib.disconnect(self.obj)


xavna = VNA()
xavna.startScan()
xavna.stopScan()
xavna.__del__()

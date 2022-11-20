#pragma once
#include <pthread.h>
#include <complex>
#include <vector>
#include <array>
#include <stdexcept>
#include "common.H"
#include <iostream>

using namespace std;

namespace xaxaxa {
	class VNADevice {
	public:
		// called by background thread when a single frequency measurement is done
        function<void(int freqIndex, VNARawValue val)> frequencyCompletedCallback;
        
		
		// called by background thread when a complete sweep of all frequencies is done
		function<void(const vector<VNARawValue>& vals)> sweepCompletedCallback;
		
		// called by background thread when an error occurs
		function<void(const exception& exc)> backgroundErrorCallback;
		
		
		VNADevice();
		~VNADevice();
		
		// find all xaVNA devices present
		static vector<string> findDevices();
		
		// returns the device handle as returned by xavna_open(), or NULL
		void* device();
		
		// open a vna device
		// dev: path to the serial device; if empty, it will be selected automatically
		void open(string dev);
		
		// returns whether the device is a T/R vna
		bool isTR();
		
		// returns whether the device uses the autosweep protocol
		bool isAutoSweep();
		
		// returns true if either device is a T/R vna or forceTR is true
		bool isTRMode();
		
		// start the frequency sweep background thread, which repeatedly performs
		// scans until it is stopped using stopScan()
		void startScan();
		
		// stop the background thread
		void stopScan();
		
		// whether the background thread is running
		bool isScanning();
		
		// close the vna device
		void close();
		
		// wait for one full measurement, and call cb with results
		void takeMeasurement(function<void(const vector<VNARawValue>& vals)> cb);

		// Changes sweep parameters. If sweep is running - stops it, changes parameters and run it again.
		void setSweepParams(double startFreqHz, double stopFreqHz, int points, int average);
		
		// return the frequency in Hz at an array index
		double freqAt(int i) {
			return _startFreqHz+i*_stepFreqHz;
		}
		
		// return highest power output in dBm
		int maxPower() const {
			return 10;
		}

		double getStartFreqHz(){
			return _startFreqHz;
		}

		double getStopFreqHz(){
			return _startFreqHz + ((_nPoints - 1) * _stepFreqHz);
		}

		double getStepFreqHz(){;
			return  _stepFreqHz;
		}

		int getPointsCount(){
			return  _nPoints;
		}

		int getAverageRate(){
			return  _nValues;
		}

		int getSweepDelay(){
			return  _nWait;
		}
		
		bool isDisableReference(){
			return  _disableReference;
		}

		bool isForceTR(){
			return  _forceTR;
		}

		bool isSwapPorts(){
			return  _swapPorts;
		}

		int getAtt1(){
			return _attenuation1;
		}

		int getAtt2(){
			return _attenuation2;
		}
		
		// diagnostics
		
		// called by background thread after frequencyCompletedCallback(); provides signal values
        // directly from hardware
        function<void(int freqIndex, const vector<array<complex<double>, 4> >& values)> frequencyCompletedCallback2_;

		// internal methods
		void* _mainThread();
		void* _runAutoSweep();
		
	private:
	    // internal variables
		void* _dev=NULL;
		pthread_t _pth;
		bool _threadRunning=false;
		bool _shouldExit=false;
		bool _lastDeviceIsAutosweep=false;
		volatile uint32_t _measurementCnt=0;
		function<void(const vector<VNARawValue>& vals)> _cb;
		volatile function<void(const vector<VNARawValue>& vals)>* _cb_;
		
		// frequency sweep parameters; do NOT change while background thread is running
		double _startFreqHz=200e6;	// start frequency in Hz
		double _stepFreqHz=25e6;		// step frequency in Hz
		int _nPoints=50;				// how many frequency points
		int _nValues=30;	 			// how many values to average over
		int _nWait=20;				// how many values to skip after changing frequency
		bool _disableReference = false;	// if true, do not divide by reference value
		bool _forceTR = false;		// if device is full two port vna, force T/R mode
		bool _swapPorts = false;		// only supported on full two port vna

		// rf parameters
		int _attenuation1=25, _attenuation2=25;
	};
}


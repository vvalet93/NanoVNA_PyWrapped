#include "include/xavna_cpp.H"
#include "include/xavna.h"
#include "include/platform_abstraction.H"
#include "include/workarounds.H"
#include "include/calibration.H"
#include <pthread.h>
#include <array>
#include <chrono>
#include <fstream>

using namespace std;
using namespace std::chrono;

namespace xaxaxa {
    static void* _mainThread_(void* param);
	bool _noscan = false;

	VNADevice::VNADevice() {
        _cb_ = &_cb;
        frequencyCompletedCallback2_ = [](int freqIndex, const vector<array<complex<double>, 4> >& values) {};
	}
	VNADevice::~VNADevice() {
		
	}
	vector<string> VNADevice::findDevices() {
		return xavna_find_devices();
	}
	
	void* VNADevice::device() {
		return _dev;
	}
	
	void VNADevice::open(string dev) {
		if(_dev) close();
		if(dev == "") {
			auto tmp = findDevices();
			if(tmp.size() == 0) throw runtime_error("no vna device found");
			dev = tmp[0];
		}
		_dev = xavna_open(dev.c_str());
		if(!_dev) {
			// ECONNREFUSED indicates DFU mode
			if(errno == ECONNREFUSED)
				throw logic_error("DFU mode");
			throw runtime_error(strerror(errno));
		}

		bool a = isAutoSweep();
		if(a != _lastDeviceIsAutosweep) {
			if(isAutoSweep()) {
				_nValues = 2;
				_nWait = -1;
			} else {
				_nValues = 30;
				_nWait = 20;
			}
		}
		_lastDeviceIsAutosweep = a;
	}
	bool VNADevice::isTR() {
		if(!_dev) return true;
		//return true;
		return xavna_is_tr(_dev);
	}
	bool VNADevice::isAutoSweep() {
		if(!_dev) return false;
		return xavna_is_autosweep(_dev);
	}
	bool VNADevice::isTRMode() {
		return isTR() || _forceTR;
	}
	void VNADevice::startScan() {
		if(!_dev) throw logic_error("VNADevice: you must call open() before calling startScan()");
		if(_threadRunning) return;
		_threadRunning = true;
		pthread_create(&_pth, NULL, &_mainThread_, this);
	}
	void VNADevice::stopScan() {
		if(!_threadRunning) return;
		_shouldExit = true;
		pthread_cancel(_pth);
		pthread_join(_pth, NULL);
		_shouldExit = false;
		_threadRunning = false;
	}
	bool VNADevice::isScanning() {
		return _threadRunning;
	}

    bool VNADevice::isCalibrated()
    {
        return _isCalibrated;
    }

    void VNADevice::close() {
		if(_threadRunning) stopScan();
		if(_dev != NULL) {
			xavna_close(_dev);
			_dev = NULL;
		}
	}

	void VNADevice::setSweepParams(double startFreqHz, double stopFreqHz, int points, int average) {
		bool stopWasTriggered {false};
		if (_threadRunning){
			stopScan();
			stopWasTriggered = true;
		} 
		if (_threadRunning) throw logic_error("setSweepParams: could not stop sweep.");

		_startFreqHz = startFreqHz;
		_stepFreqHz = (stopFreqHz - startFreqHz) / (points - 1);
		_nPoints = points;
		_nValues = average;

		if (stopWasTriggered) startScan();
	}
	
	void VNADevice::takeMeasurement(function<void(const vector<VNARawValue>& vals)> cb) {
		if(!_threadRunning) throw logic_error("takeMeasurement: vna scan thread must be started");
		_cb = cb;
		__sync_synchronize();
		__sync_add_and_fetch(&_measurementCnt, 1);
	}
	
	static inline void swap(VNARawValue& a, VNARawValue& b) {
		VNARawValue tmp = a;
		a = b;
		b = tmp;
	}

	bool VNADevice::loadSOLTCalibration(char *calPath){
		fstream calFile;
		calFile.open(string(calPath), ios::in);

		if (!calFile.is_open())
		{
			cout << string(calPath) + " was not found!\n";
			return false;
		}

		string line;
		vector<string> fileContent;
		while (getline(calFile,line))
		{
			fileContent.push_back(line);
		}

		if (fileContent.size() < 11)
		{
			cout << string(calPath) + " is empty or corrupted!\n";
			return false;
		}

		// New sweep settings will apply based on calibration file.
		int points;
		double startFreq, freqStep, stopFreq;

		char separator = ' '; 
		vector<string> sweepParams = stringSplit(fileContent[2], separator);
		if (sweepParams.size() < 3)
		{
			cout << string(fileContent[2]) + " wrong number of sweep parameters!\n";
			return false;
		}

		points = std::stoi(sweepParams[0]);
		startFreq = std::stod(sweepParams[1]);
		freqStep = std::stod(sweepParams[2]);
		stopFreq = startFreq + (freqStep * (points - 1));
		
		int calFileHeaderLinesNumber = 3;
		int calDataNumber = 4;
		int calculatedCalFileLength = calFileHeaderLinesNumber + calDataNumber + points;
		if (fileContent.size() != calculatedCalFileLength){
			cout << "Wrong amount of calibration data! " << fileContent.size() << ", expected: " << calculatedCalFileLength;
			return false;
		}

		std::map<std::string, CalibrationType> calTypeMap {
			{"load1", CAL_LOAD},
			{"open1", CAL_OPEN},
			{"short1", CAL_SHORT},
			{"thru", CAL_THRU}
		};

		for (int i = calFileHeaderLinesNumber; i < fileContent.size(); i++){
			if (calTypeMap.count(fileContent[i])){
				CalibrationType calType = calTypeMap[fileContent[i]];
				for (int j = i + 1; j < points + i; j++){
					vector<string> complexCalibData = stringSplit(fileContent[j], separator);
					if (sweepParams.size() < 8)
					{
						cout << string(fileContent[2]) + " wrong number of calibration complex data!\n";
						return false;
					}

					int calSize=sizeof(complex2) * points;
					_calibrationReferences[calType].resize(0);
					_calibrationReferences[calType].resize(points);
					
					vector<complex2> calData;
					complex2 port1, port2;
					port1[0] = complex<double>(std::stod(sweepParams[0]), std::stod(sweepParams[1]));
					port1[1] = complex<double>(std::stod(sweepParams[2]), std::stod(sweepParams[3]));

					port2[0] = complex<double>(std::stod(sweepParams[4]), std::stod(sweepParams[5]));
					port2[1] = complex<double>(std::stod(sweepParams[6]), std::stod(sweepParams[7]));
					
					calData.push_back(port1);
					calData.push_back(port2);
					_calibrationReferences[calType] = calData;
				}
			}
		}
		
		_isCalibrated = true;
		setSweepParams(startFreq, stopFreq, points);
		bool calibrationApplied = applySOLT();
		if (calibrationApplied)
			cout << "Calibration from file " + string(calPath) + " was successfully applied!\n";
		
		return true;
    }

    bool VNADevice::applySOLT(){
		if (_calibrationReferences.size() == 0 || !_isCalibrated){
			cout << "applySOLT: no calibration data to apply!\n";
			return false;
		}

		int nPoints = _calibrationReferences[0].size();

		for(int i=0;i<nPoints;i++) {
			_cal_coeffs[i] = SOL_compute_coefficients(
								_calibrationReferences[CAL_SHORT][i][0],
								_calibrationReferences[CAL_OPEN][i][0],
								_calibrationReferences[CAL_LOAD][i][0]);
			if(_calibrationReferences[CAL_THRU].size() != 0)
				_cal_thru[i] = _calibrationReferences[CAL_THRU][i][1];
			else _cal_thru[i] = 1.;
			
			auto x1 = _calibrationReferences[CAL_LOAD][i][0],
				y1 = _calibrationReferences[CAL_LOAD][i][1],
				x2 = _calibrationReferences[CAL_OPEN][i][0],
				y2 = _calibrationReferences[CAL_OPEN][i][1];
			
			_cal_thru_leak_r[i] = (y1-y2)/(x1-x2);
			_cal_thru_leak[i] = y2-_cal_thru_leak_r[i]*x2;
		}
		_useCalibration = true;
		return true;
    }

    void VNADevice::denySOLT(){
		_useCalibration = false;
    }

    void *VNADevice::_mainThread()
    {
        if(xavna_is_autosweep(_dev)) {
			return _runAutoSweep();
		}
		bool tr = isTRMode();
		uint32_t last_measurementCnt = _measurementCnt;
		int cnt=0;
		while(!_shouldExit) {
			vector<VNARawValue> results(_nPoints);
			for(int i=0;i<_nPoints;i++) {
				fflush(stdout);
				int ports = tr?1:2;
				
				// values is indexed by excitation #, then wave #
				// e.g. values[0][1] is wave 1 measured with excitation on port 0
				vector<array<complex<double>, 4> > values(ports);
				for(int port=0; port<ports; port++) {
					int p = _swapPorts?(1-port):port;
					if(!_noscan) {
						if(xavna_set_params(_dev, (int)round(freqAt(i)/1000.),
											(p==0?_attenuation1:_attenuation2), p, _nWait) < 0) {
							backgroundErrorCallback(runtime_error("xavna_set_params failed: " + string(strerror(errno))));
							return NULL;
						}
					}
                    if(xavna_read_values_raw(_dev, (double*)&values[port], _nValues)<0) {
						backgroundErrorCallback(runtime_error("xavna_read_values_raw failed: " + string(strerror(errno))));
						return NULL;
					}
				}
				if(_swapPorts) {
					for(int port=0;port<ports;port++) {
						swap(values[port][0], values[port][2]);
						swap(values[port][1], values[port][3]);
					}
				}
				
				VNARawValue tmp;
				if(tr) {
					if(_disableReference)
						tmp << values[0][1], 0,
						        values[0][3], 0;
					else
						tmp << values[0][1]/values[0][0], 0,
						        values[0][3]/values[0][0], 0;
				} else {
					complex<double> a0,b0,a3,b3;
					complex<double> a0p,b0p,a3p,b3p;
					a0 = values[0][0];
					b0 = values[0][1];
					a3 = values[0][2];
					b3 = values[0][3];
					a0p = values[1][0];
					b0p = values[1][1];
					a3p = values[1][2];
					b3p = values[1][3];
					
					// solving for two port S parameters given two sets of
					// observed waves (port 1 in, port 1 out, port 2 in, port 2 out)
					
					complex<double> d = 1. - (a3*a0p)/(a0*a3p);
					
					// S11M
					tmp(0,0) = ((b0/a0) - (b0p*a3)/(a3p*a0))/d;
					// S21M
					tmp(1,0) = ((b3/a0) - (b3p*a3)/(a3p*a0))/d;
					// S12M
					tmp(0,1) = ((b0p/a3p) - (b0*a0p)/(a3p*a0))/d;
					// S22M
					tmp(1,1) = ((b3p/a3p) - (b3*a0p)/(a3p*a0))/d;
				}
				//tmp(0,0) = tmp(1,0)*6.;
				/*if(abs(tmp(0,0)) > 0.5 && (arg(tmp(0,0))*180) < -90) {
					static int cnt = 0;
					if((cnt++) > 10) {
						_noscan = true;
					}
				}*/
				
                frequencyCompletedCallback(i, tmp);
                frequencyCompletedCallback2_(i, values);
                
				results[i]=tmp;
				if(_shouldExit) return NULL;
			}
			sweepCompletedCallback(results);
			
			if(_measurementCnt != last_measurementCnt) {
				__sync_synchronize();
				if(cnt == 1) {
					function<void(const vector<VNARawValue>& vals)> func
						= *(function<void(const vector<VNARawValue>& vals)>*)_cb_;
					func(results);
					cnt = 0;
					last_measurementCnt = _measurementCnt;
				} else cnt++;
			}
			
		}
		return NULL;
    }
    static complex<double> cx(const double* v) {
		return {v[0], v[1]};
	}
	void* VNADevice::_runAutoSweep() {
		uint32_t last_measurementCnt = _measurementCnt;
		int lastFreqIndex = -1;
		int measurementEndPoint = -1;
		double currChunkPoints = 16.;
		int chunkPoints = (int)currChunkPoints;
		int _nValues = this->_nValues;
		int collectDataState = 0;
		int cnt = 0;
		int currValueIndex = 0;
		vector<VNARawValue> results(_nPoints);
		vector<array<complex<double>, 4> > rawValues(1);
		rawValues[0] = {0., 0., 0., 0.};
		
		xavna_set_autosweep(_dev, _startFreqHz, _stepFreqHz, _nPoints, _nValues);
		while(!_shouldExit) {
			fflush(stdout);
			int chunkValues = chunkPoints;
			autoSweepDataPoint values[chunkValues];

			// read a chunk of values
			steady_clock::time_point t1 = steady_clock::now();
			if(xavna_read_autosweep(_dev, values, chunkValues)<0) {
				backgroundErrorCallback(runtime_error("xavna_read_autosweep failed: " + string(strerror(errno))));
				return NULL;
			}
			steady_clock::time_point t2 = steady_clock::now();
			double readChunkTimeSeconds = duration_cast<duration<double>>(t2 - t1).count();
			
			// process chunk
			for(int i=0; i<chunkValues; i++) {
				auto& value = values[i];
				array<complex<double>, 4> currRawValue =
						{cx(value.forward[0]), cx(value.reverse[0]),
						cx(value.forward[1]), cx(value.reverse[1])};

				for(int j=0; j<4; j++)
					rawValues[0][j] += currRawValue[j];

				if(value.freqIndex >= _nPoints) {
					fprintf(stderr, "warning: hw returned freqIndex (%d) >= _nPoints (%d)\n", value.freqIndex, _nPoints);
					continue;
				}

				// keep track of how many values so far for this frequency
				if(value.freqIndex != lastFreqIndex) {
					currValueIndex = 0;
					lastFreqIndex = value.freqIndex;
				} else currValueIndex++;

				// last value for this frequency point
				if(currValueIndex == _nValues - 1) {
					VNARawValue tmp;
					if(_disableReference)
						tmp << rawValues[0][1], 0,
						        rawValues[0][3], 0;
					else
						tmp << rawValues[0][1]/rawValues[0][0], 0,
						        rawValues[0][3]/rawValues[0][0], 0;
					frequencyCompletedCallback(value.freqIndex, tmp);
					frequencyCompletedCallback2_(value.freqIndex, rawValues);
					results[value.freqIndex] = tmp;
					if(value.freqIndex == _nPoints - 1)
						sweepCompletedCallback(results);
					rawValues[0] = {0., 0., 0., 0.};
				}
				if(_shouldExit) return NULL;
				if(collectDataState == 1 && value.freqIndex == 0) {
					collectDataState = 2;
				} else if(collectDataState == 2 && value.freqIndex == _nPoints - 1) {
					collectDataState = 3;
					cnt = 0;
				} else if(collectDataState == 3) {
					if(currValueIndex == _nValues - 1) {
						cnt++;
						if(cnt >= 5) {
							collectDataState = 0;
							_nValues = this->_nValues;
							xavna_set_autosweep(_dev, _startFreqHz, _stepFreqHz, _nPoints, _nValues);

							function<void(const vector<VNARawValue>& vals)> func
									= *(function<void(const vector<VNARawValue>& vals)>*)_cb_;
							func(results);
						}
					}
				}
			}
			
			if(_measurementCnt != last_measurementCnt) {
				// collect measurement requested
				last_measurementCnt = _measurementCnt;
				// when collecting measurements, use double the averaging factor
				_nValues = this->_nValues * 2;
				xavna_set_autosweep(_dev, _startFreqHz, _stepFreqHz, _nPoints, _nValues);
				collectDataState = 1;
			}
			
			// adjust chunk size to target chunkTime 50ms
			// if the read took less than 20ms, treat it as 20ms
			if(readChunkTimeSeconds < 0.02) readChunkTimeSeconds = 0.02;
			double newChunkPoints = double(chunkPoints)/readChunkTimeSeconds * 0.05;
			if(newChunkPoints > 128) newChunkPoints = 128;
			if(newChunkPoints < 8) newChunkPoints = 8;
			currChunkPoints = currChunkPoints * 0.7 + newChunkPoints * 0.3;
			chunkPoints = (int)currChunkPoints;
			//fprintf(stderr, "chunkPoints %d\n", chunkPoints);
		}
		return NULL;
	}

    static vector<string> stringSplit(string str, char separator) {
		vector<string> result;
		int startIndex = 0, endIndex = 0;
		for (int i = 0; i <= str.size(); i++) {
			if (str[i] == separator || i == str.size()) {
				endIndex = i;
				string temp;
				temp.append(str, startIndex, endIndex - startIndex);
				result.push_back(temp);
				startIndex = endIndex + 1;
			}
		}

		return result;
	}

	static void* _mainThread_(void* param) {
		return ((VNADevice*)param)->_mainThread();
	}
}

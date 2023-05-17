#include "xavna_controller.h"
#include <iostream>
#include <fstream>
#include <complex>
#include <chrono>
#include <ctime>  
#include <iomanip>

using namespace std;
using namespace std::chrono;

// TODO
// 1. Calibration.
// 2. Find a way to avoid saving data to file.

extern "C"{
    // Open a vna device
    // Device will be found and selected automatically
    VNADevice* findVnaAndConnect(){
        auto vna = new VNADevice();
 
        try
        {
            vna->open(""); // It will find device if name is not provided.
            vna->sweepCompletedCallback = [] (const vector<VNARawValue>& vals) { measurements = vals; };
            vna->frequencyCompletedCallback = [] (int freqIndex, VNARawValue val) { };
            vna->backgroundErrorCallback = [] (const exception& exc) { std::cout<<"Measurement error!\n" << exc.what(); };
            return vna;
        }
        catch (const runtime_error& error)
        {
            delete vna;
            std::cout << "Couldn't connect to VNA!\n" << error.what() <<std::endl;
            return NULL;
        }
    }

    // Close the vna device.
    void disconnect(VNADevice* vna){
        if (vna != nullptr){
            vna->close();
        }
    }

    // Dispose the object.
    void dispose(VNADevice* vna){
        if (vna != nullptr){
            delete vna;
        }
    }
     
    // Start the frequency sweep background thread, which repeatedly performs
    // scans until it is stopped using stopScan()
    void startScan(VNADevice* vna){
        if (vna != nullptr){
            vna->startScan();
        }
    }
    
    // Stop the background thread
    void stopScan(VNADevice* vna){
        if (vna != nullptr){
            vna->stopScan();
        }
    }
    
    // Whether the background thread is running
    bool isScanning(VNADevice* vna){
        if (vna != nullptr){
            return vna->isScanning();
        }
        
        return false;
    }

    // Wait for one full measurement, and call cb with results
    bool saveMeasDataToFile(VNADevice* vna){
        string filename = "meas.txt";
        if (vna != nullptr){
            controller_log("Instance is null.");
            return false;
        }
        
        if (!isScanning(vna)){
            controller_log("saveMeasDataToFile: Sweep must be run first!");
            return false;
        }

        if (measurements.size() == 0){
            controller_log("saveMeasDataToFile: Data is not available!");
            return false;
        }

        ofstream myfile (filename);
        myfile.clear();
        if (!myfile.is_open()){
            controller_log("saveMeasDataToFile: Unable to open file" + filename);
            return false;
        }
        
        for_each(measurements.begin(), measurements.end(), 
            [&myfile](const VNARawValue& val){ myfile << val(0,0) << "\t" << val(1,0) << "\t" << val(0,1) << "\t"<< val(1,1) << "\n";}
        );

        myfile.close();
        measurements.clear();
        return true;
    }

    // Save the lates measurements to file.
    bool saveS21MagnitudeToFile(VNADevice* vna){
        string filename = "meas21.txt";
        if (vna != nullptr){
            controller_log("Instance is null.");
            return false;
        }
        
        if (!isScanning(vna)){
            controller_log("saveS21MagnitudeToFile: Sweep must be run first!");
            return false;
        }
        
        if (measurements.size() == 0){
            return false;
        }

        ofstream myfile (filename);
        myfile.clear();
        if (!myfile.is_open()){
            controller_log("saveS21MagnitudeToFile: Unable to open file");
            return false;
        }

        for(auto point = 0; point < getPointsCount(vna); point++){
            myfile << vna->freqAt(point) << "\t" << 10*log(std::abs(measurements.at(point)(1,0))) << "\n";
        }
        myfile.close();
        measurements.clear();
        return true;
    }

    // Changes sweep parameters. If sweep is running - stops it, changes parameters and run it again.
	void setSweepParams(VNADevice* vna, double startFreqHz, double stopFreqHz, int points, int average){
        if (vna != nullptr){
            vna->setSweepParams(startFreqHz, stopFreqHz, points, average);
        }
    }

    // Sweep parameter variables reading.
    // Returns start frequency in Hz.
    double getStartFreqHz(VNADevice* vna){
        if (vna != nullptr){
            return vna->getStartFreqHz();
        }
        
        return 0.0;
    }

    // Returns stop frequency in Hz.
    double getStopFreqHz(VNADevice* vna){
        if (vna != nullptr){
            return vna->getStopFreqHz();
        }
        
        return 0.0;
    }

    // Returns frequency step in Hz.
    double getStepFreqHz(VNADevice* vna){
        if (vna != nullptr){
            return vna->getStepFreqHz();
        }
        
        return 0.0;
    }

    // Returns count of points.
    int getPointsCount(VNADevice* vna){
        if (vna != nullptr){
            return vna->getPointsCount();
        }
        
        return 0;
    }

    // Returns average rate.
    int getAverageRate(VNADevice* vna){
        if (vna != nullptr){
            return vna->getAverageRate();
        }
        
        return 0;
    }

    // Returns delay after sweep in ms.
    int getSweepDelay(VNADevice* vna){
        if (vna != nullptr){
            return vna->getSweepDelay();
        }
        
        return 0;
    }

    // Get attenuation of port #1
    int getAtt1(VNADevice* vna){
        if (vna != nullptr){
            return vna->getAtt1();
        }
        
        return 0;
    }

    // Get attenuation of port #2
    int getAtt2(VNADevice* vna){
        if (vna != nullptr){
            return vna->getAtt2();
        }
        
        return 0;
    }

    // Load calibration from existing file.
    bool loadSOLTCalibration(VNADevice* vna, char *filePath){
        if (vna != nullptr){
            return vna->loadSOLTCalibration(filePath);
        }
        
        return false;
    }

    // Apply measured or loaded calibraion. 
    // Called automatically after device was calibrated or calibration filed loaded.
    bool applySOLT(VNADevice* vna){
        if (vna != nullptr){
            return vna->applySOLT();
        }

        return false;
    }

    // Deny calibration without erasing calibration data. Might be applied back calling applySOLT()
    void denySOLT(VNADevice* vna){
        if (vna != nullptr){
            vna->denySOLT();
        }  
    }

    // Switch on/off debug mode.
	void debug(VNADevice* vna, bool debug)
    {
        _debug = debug;

        if (vna != nullptr){
            vna->debug(debug);
        } 
    }

    // Prints log to console if debug mode is on.
	void controller_log(std::string str)
	{
		if (_debug)
		{
			auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::cout << "[" << std::put_time(std::localtime(&time), "%T") << "] " << str << std::endl;
		}
	}
}
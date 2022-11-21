#include "xavna_controller.h"
#include <iostream>
#include <fstream>
#include <complex>

// TODO
// 1. Properly convert data to dB.
// 2. Calibration.
// 3. Debug mode. Switch off chunkPoints etc..

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
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }

        vna->close();
        std::cout << "Disconnected from VNA!" << std::endl;
    }

    // Dispose the object.
    void dispose(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }

        delete vna;
        std::cout << "Object was disposed." << std::endl;
    }
     
    // Start the frequency sweep background thread, which repeatedly performs
    // scans until it is stopped using stopScan()
    void startScan(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }
            
        vna->startScan();
    }
    
    // Stop the background thread
    void stopScan(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }

        vna->stopScan();
    }
    
    // Whether the background thread is running
    bool isScanning(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->isScanning();
    }

    // Wait for one full measurement, and call cb with results
    bool saveMeasDataToFile(VNADevice* vna){
        string filename = "meas.txt";
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        if (!isScanning(vna)){
            std::cout << "saveMeasDataToFile: Sweep must be run first!" << std::endl;
            return false;
        }

        if (measurements.size() == 0){
            std::cout << "saveMeasDataToFile: Data is not available!" << std::endl;
            return false;
        }

        ofstream myfile (filename);
        myfile.clear();
        if (!myfile.is_open()){
            cout << "saveMeasDataToFile: Unable to open file" + filename;
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
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        if (!isScanning(vna)){
            std::cout << "saveS21MagnitudeToFile: Sweep must be run first!" << std::endl;
            return false;
        }
        
        if (measurements.size() == 0){
            //std::cout << "saveS21MagnitudeToFile: Data is not available!" << std::endl;
            return false;
        }

        ofstream myfile (filename);
        myfile.clear();
        if (!myfile.is_open()){
            cout << "saveS21MagnitudeToFile: Unable to open file" + filename;
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
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }
        vna->setSweepParams(startFreqHz, stopFreqHz, points, average);
    }

    // Sweep parameter variables reading.
    double getStartFreqHz(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0.0;
        }
        
        return vna->getStartFreqHz();
    }

    double getStopFreqHz(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0.0;
        }
        
        return vna->getStopFreqHz();
    }

    double getStepFreqHz(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0.0;
        }
        
        return vna->getStepFreqHz();
    }


    int getPointsCount(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0;
        }
        
        return vna->getPointsCount();
    }


    int getAverageRate(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0;
        }
        
        return vna->getAverageRate();
    }


    int getSweepDelay(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return 0;
        }
        
        return vna->getSweepDelay();
    }

    
    bool isDisableReference(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->isDisableReference();
    }


    bool isForceTR(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->isForceTR();
    }

    bool isSwapPorts(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->isSwapPorts();
    }

    int getAtt1(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->getAtt1();
    }

    int getAtt2(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return false;
        }
        
        return vna->getAtt2();
    }
}
#include "xavna_controller.h"
#include <iostream>

// TODO
// 1. Return measurement results.
// 2. Calibration.
// 3. Debug mode. Switch off chunkPoints etc..

extern "C"{
    // Open a vna device
    // Device will be found and selected automatically
    VNADevice* findVnaAndConnect(){
        auto vna = new VNADevice();
        vna->sweepCompletedCallback = [] (const vector<VNARawValue>& vals) { };
        vna->frequencyCompletedCallback = [] (int freqIndex, VNARawValue val) { };
        vna->backgroundErrorCallback = [] (const exception& exc) { };
        
        try
        {
            vna->open(""); // It will find device if name is not provided.
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
    void takeMeasurement(VNADevice* vna){
        if (vna == nullptr){
            std::cout << "Instance is null." << std::endl;
            return;
        }
        vna->sweepCompletedCallback = [] (const vector<VNARawValue>& vals) { };

        return; // TODO
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
#include "xavna_controller.h"
#include <iostream>

// TODO
// 1. Sweep settings. READ MANUAL, do not change parameters when sweep is running!
// 2. Return measurement results.
// 3. Calibration.

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

    // Close the vna device and dispose the object.
    void disconnect(VNADevice* vna){
        vna->close();
        delete vna;
        std::cout << "Disconnected from VNA! Object was disposed." << std::endl;
    }
     
    // Start the frequency sweep background thread, which repeatedly performs
    // scans until it is stopped using stopScan()
    void startScan(VNADevice* vna){
        vna->startScan();
    }
    
    // Stop the background thread
    void stopScan(VNADevice* vna){
        vna->stopScan();
    }
    
    // Whether the background thread is running
    bool isScanning(VNADevice* vna){
        return vna->isScanning();
    }

    // Wait for one full measurement, and call cb with results
    void takeMeasurement(VNADevice* vna){
        vna->sweepCompletedCallback = [] (const vector<VNARawValue>& vals) { };

        return; // TODO
    }
}
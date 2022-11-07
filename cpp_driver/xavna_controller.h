#include "include/xavna/xavna_cpp.H"

using namespace xaxaxa;

extern "C"{
        // Open a vna device
    // Device will be found and selected automatically
    VNADevice* findVnaAndConnect();

    // Close the vna device.
    void disconnect(VNADevice* vna);

    // Dispose the object.
    void dispose(VNADevice* vna);

    // Start the frequency sweep background thread, which repeatedly performs
    // scans until it is stopped using stopScan()
    void startScan(VNADevice* vna);

    // Stop the background thread
    void stopScan(VNADevice* vna);

    // Whether the background thread is running
    bool isScanning(VNADevice* vna);

    // Wait for one full measurement, and call cb with results
    void takeMeasurement(VNADevice* vna);
}
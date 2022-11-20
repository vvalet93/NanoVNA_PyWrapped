#include "libxavna/include/xavna_cpp.H"

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

    // Save the lates measurements to file.
    void saveMeasDataToFile(VNADevice* vna);

    // Changes sweep parameters. If sweep is running - stops it, changes parameters and run it again.
	void setSweepParams(VNADevice* vna, double startFreqHz, double stopFreqHz, int points, int average);

    // Changes sweep parameters. If sweep is running - stops it, changes parameters and run it again.
	//double* getS21Magnitude(VNADevice* vna, double startFreqHz, double stopFreqHz, int points, int average);

    double getStartFreqHz(VNADevice* vna);

    double getStopFreqHz(VNADevice* vna);

    double getStepFreqHz(VNADevice* vna);

    int getPointsCount(VNADevice* vna);

    int getAverageRate(VNADevice* vna);

    int getSweepDelay(VNADevice* vna);
    
    bool isDisableReference(VNADevice* vna);

    bool isForceTR(VNADevice* vna);

    bool isSwapPorts(VNADevice* vna);

    int getAtt1(VNADevice* vna);

    int getAtt2(VNADevice* vna);

    vector<VNARawValue> measurements;
}
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

    // Saves the lates measurements to file meas.txt. Returns true if data was saved.
    bool saveMeasDataToFile(VNADevice* vna);

    // Saves S21 magnitudeto file meas21.txt. Returns true if data was saved.
    bool saveS21MagnitudeToFile(VNADevice* vna);

    // Changes sweep parameters. If sweep is running - stops it, changes parameters and run it again.
	void setSweepParams(VNADevice* vna, double startFreqHz, double stopFreqHz, int points, int average);

    // Returns start frequency in Hz.
    double getStartFreqHz(VNADevice* vna);

    // Returns stop frequency in Hz
    double getStopFreqHz(VNADevice* vna);

    // Returns frequency step in Hz
    double getStepFreqHz(VNADevice* vna);

    // Returns count of points.
    int getPointsCount(VNADevice* vna);

    // Returns average rate.
    int getAverageRate(VNADevice* vna);

    // Returns delay after sweep in ms.
    int getSweepDelay(VNADevice* vna);
    
    // Get attenuation of port #1
    int getAtt1(VNADevice* vna);

    // Get attenuation of port #2
    int getAtt2(VNADevice* vna);

    // Load calibration from existing file.
	bool loadSOLTCalibration(VNADevice* vna, char* filePath);

    // Apply measured or loaded calibraion. 
    // Called automatically after device was calibrated or calibration filed loaded.
    bool applySOLT(VNADevice* vna);

    // Deny calibration without erasing calibration data. Might be applied back calling applySOLT()
	void denySOLT(VNADevice* vna);

    // Switch on/off debug mode.
	void debug(VNADevice* vna, bool debug);

    // Prints log to console if debug mode is on.
    void controller_log(std::string str);

    bool _debug = false;

    // Measured data.
    vector<VNARawValue> measurements;
}
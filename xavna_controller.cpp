#include "xavna_controller.h"

using namespace xaxaxa;

int main(){
    auto devices = VNADevice::findDevices();
    std::cout <<"Next devices have been found:" << std::endl;
    for (auto device : devices){
        std::cout<<device<<std::endl;
    }
    
    return 0;
}
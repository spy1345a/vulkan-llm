#include "device_selector.hpp"
#include "compute_add.hpp"
#include <iostream>

int main() {
    auto sel = selectDevice(createVulkanInstance());
    if (!sel.device) {
        std::cerr << "Device selection failed\n";
        return 1;
    }

    std::cout << "Selected GPU:\n";
    printDeviceInfo(sel.properties);

    {
        ComputeAdd adder(sel.device, sel.physicalDevice, sel.queue);
        adder.setA(7.0f);
        adder.setB(5.0f);
        float result = adder.run();
        std::cout << "7 + 5 = " << result << "\n";
    }

    destroyDeviceSelection(sel);
    return 0;
}

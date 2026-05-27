#include "device_selector.hpp"
#include "arithmetic/op.hpp"
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
        Op add(sel.device, sel.physicalDevice, sel.queue, "arithmetic/shaders/add.spv");
        add.setA(7.0f);
        add.setB(5.0f);
        std::cout << "7 + 5 = " << add.run() << "\n";
    }
    {
        Op sub(sel.device, sel.physicalDevice, sel.queue, "arithmetic/shaders/sub.spv");
        sub.setA(7.0f);
        sub.setB(5.0f);
        std::cout << "7 - 5 = " << sub.run() << "\n";
    }
    {
        Op mul(sel.device, sel.physicalDevice, sel.queue, "arithmetic/shaders/mul.spv");
        mul.setA(7.0f);
        mul.setB(5.0f);
        std::cout << "7 * 5 = " << mul.run() << "\n";
    }
    {
        Op div(sel.device, sel.physicalDevice, sel.queue, "arithmetic/shaders/div.spv");
        div.setA(7.0f);
        div.setB(5.0f);
        std::cout << "7 / 5 = " << div.run() << "\n";
    }

    destroyDeviceSelection(sel);
    return 0;
}

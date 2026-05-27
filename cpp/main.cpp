#include "device_selector.hpp"
#include "arithmetic/op.hpp"
#include <iostream>
#include <vector>
#include <random>

int main() {
    auto sel = selectDevice(createVulkanInstance());
    if (!sel.device) {
        std::cerr << "Device selection failed\n";
        return 1;
    }

    std::cout << "Selected GPU:\n";
    printDeviceInfo(sel.properties);

    const uint32_t N = 1 << 20;

    std::vector<float> a(N), b(N), result(N);
    for (uint32_t i = 0; i < N; i++) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(N - i);
    }

    {
        Op op(sel.device, sel.physicalDevice, sel.queue,
              "arithmetic/shaders/add.spv", N);
        op.setA(a.data());
        op.setB(b.data());
        op.run();
        op.getResult(result.data());
        std::cout << "add  [0]=" << result[0]
                  << " [N-1]=" << result[N - 1] << "\n";
    }
    {
        Op op(sel.device, sel.physicalDevice, sel.queue,
              "arithmetic/shaders/mul.spv", N);
        op.setA(a.data());
        op.setB(b.data());
        op.run();
        op.getResult(result.data());
        std::cout << "mul  [0]=" << result[0]
                  << " [N-1]=" << result[N - 1] << "\n";
    }

    destroyDeviceSelection(sel);
    return 0;
}

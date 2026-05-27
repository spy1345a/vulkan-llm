#include "device_selector.hpp"
#include <iostream>
#include <cstring>

VkInstance createVulkanInstance() {
    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    VkInstanceCreateInfo ici{};
    ici.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &appInfo;
    VkResult res = vkCreateInstance(&ici, nullptr, &instance);
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance\n";
        return VK_NULL_HANDLE;
    }
    return instance;
}

std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    if (count > 0)
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
    return devices;
}

void printDeviceInfo(const VkPhysicalDeviceProperties& props) {
    std::cout << "  Name      : " << props.deviceName << "\n"
              << "  Type      : "
              << (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "Discrete GPU" :
                  props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "Integrated GPU" :
                  props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU    ? "Virtual GPU" :
                  props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU           ? "CPU" : "Other")
              << "\n"
              << "  API       : " << VK_API_VERSION_MAJOR(props.apiVersion) << "."
              << VK_API_VERSION_MINOR(props.apiVersion) << "."
              << VK_API_VERSION_PATCH(props.apiVersion) << "\n";
}

DeviceSelection selectDevice(VkInstance instance, const VkPhysicalDeviceProperties& preferredProps) {
    DeviceSelection sel{};
    sel.instance = instance;

    auto devices = enumeratePhysicalDevices(instance);
    if (devices.empty()) {
        std::cerr << "No Vulkan-capable devices found\n";
        return sel;
    }

    // Try to find a device matching preferred properties (e.g., discrete GPU)
    VkPhysicalDevice selected = VK_NULL_HANDLE;
    if (preferredProps.apiVersion != 0) {
        for (auto d : devices) {
            VkPhysicalDeviceProperties p;
            vkGetPhysicalDeviceProperties(d, &p);
            if (p.deviceType == preferredProps.deviceType &&
                p.apiVersion >= preferredProps.apiVersion &&
                std::strcmp(p.deviceName, preferredProps.deviceName) == 0) {
                selected = d;
                sel.properties = p;
                break;
            }
        }
    }

    // Fallback: pick first device (prefer discrete GPU)
    if (selected == VK_NULL_HANDLE) {
        int bestIdx = 0;
        for (size_t i = 0; i < devices.size(); i++) {
            VkPhysicalDeviceProperties p;
            vkGetPhysicalDeviceProperties(devices[i], &p);
            if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                bestIdx = (int)i;
                break;
            }
        }
        selected = devices[bestIdx];
        vkGetPhysicalDeviceProperties(selected, &sel.properties);
    }

    sel.physicalDevice = selected;

    // Create logical device + queue
    sel.queueFamilyIndex = 0;
    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = sel.queueFamilyIndex;
    qci.queueCount       = 1;
    qci.pQueuePriorities = &priority;

    VkDeviceCreateInfo dci{};
    dci.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos    = &qci;

    VkResult res = vkCreateDevice(selected, &dci, nullptr, &sel.device);
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create logical device\n";
        return sel;
    }

    vkGetDeviceQueue(sel.device, sel.queueFamilyIndex, 0, &sel.queue);
    return sel;
}

uint32_t findMemoryType(VkPhysicalDevice physDev, uint32_t typeBits) {
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(physDev, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++) {
        if ((typeBits & (1u << i)) &&
            (mp.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            return i;
        }
    }
    std::cerr << "No suitable memory type\n";
    return 0;
}

void destroyDeviceSelection(DeviceSelection& sel) {
    if (sel.device != VK_NULL_HANDLE)
        vkDestroyDevice(sel.device, nullptr);
    if (sel.instance != VK_NULL_HANDLE)
        vkDestroyInstance(sel.instance, nullptr);
    sel = {};
}

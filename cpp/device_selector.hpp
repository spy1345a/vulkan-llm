#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

struct DeviceSelection {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    uint32_t queueFamilyIndex;
    VkPhysicalDeviceProperties properties;
};

VkInstance createVulkanInstance();
std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);
DeviceSelection selectDevice(VkInstance instance, const VkPhysicalDeviceProperties& preferredProps = {});
void printDeviceInfo(const VkPhysicalDeviceProperties& props);
uint32_t findMemoryType(VkPhysicalDevice physDev, uint32_t typeBits);
void destroyDeviceSelection(DeviceSelection& sel);

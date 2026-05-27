#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class ComputeAdd {
public:
    ComputeAdd(VkDevice device, VkPhysicalDevice physDev, VkQueue queue,
               const char* shaderDir = "shader/");
    ~ComputeAdd();

    void setA(float val);
    void setB(float val);
    float run();

private:
    VkDevice device;
    VkPhysicalDevice physDev;
    VkQueue queue;

    VkBuffer bufferA = VK_NULL_HANDLE;
    VkBuffer bufferB = VK_NULL_HANDLE;
    VkBuffer bufferOut = VK_NULL_HANDLE;
    VkDeviceMemory memoryA = VK_NULL_HANDLE;
    VkDeviceMemory memoryB = VK_NULL_HANDLE;
    VkDeviceMemory memoryOut = VK_NULL_HANDLE;
    VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkShaderModule shaderMod = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    std::vector<uint32_t> loadSPIRV(const std::string& path);
    uint32_t findMemType(uint32_t typeBits);
    void createStorageBuffer(VkBuffer& buf, VkDeviceMemory& mem);
};

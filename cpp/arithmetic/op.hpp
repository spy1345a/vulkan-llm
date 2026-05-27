#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <cstdint>

class Op {
public:
    Op(VkDevice device, VkPhysicalDevice physDev, VkQueue queue,
       const std::string& shaderPath, uint32_t count);
    ~Op();

    void setA(const float* data);
    void setB(const float* data);
    void run();
    void getResult(float* out);

    uint32_t count() const { return count_; }

private:
    VkDevice device;
    VkPhysicalDevice physDev;
    VkQueue queue;
    uint32_t count_;
    VkDeviceSize bufSize_;

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

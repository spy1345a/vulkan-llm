#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <cstdint>

class MatMul {
public:
    MatMul(VkDevice device, VkPhysicalDevice physDev, VkQueue queue,
           const std::string& shaderPath,
           uint32_t M, uint32_t K, uint32_t N);
    ~MatMul();

    void setA(const float* data);
    void setB(const float* data);
    void run();
    void getResult(float* out);

private:
    VkDevice device;
    VkPhysicalDevice physDev;
    VkQueue queue;
    uint32_t M_, K_, N_;
    VkDeviceSize sizeA_, sizeB_, sizeC_;

    struct PushConsts { uint32_t M, K, N; };

    VkBuffer bufferA = VK_NULL_HANDLE;
    VkBuffer bufferB = VK_NULL_HANDLE;
    VkBuffer bufferC = VK_NULL_HANDLE;
    VkDeviceMemory memoryA = VK_NULL_HANDLE;
    VkDeviceMemory memoryB = VK_NULL_HANDLE;
    VkDeviceMemory memoryC = VK_NULL_HANDLE;
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
    void createStorageBuffer(VkBuffer& buf, VkDeviceMemory& mem, VkDeviceSize sz);
};

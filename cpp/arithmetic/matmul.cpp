#include "matmul.hpp"
#include "../device_selector.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

static constexpr uint32_t WG_X = 16;
static constexpr uint32_t WG_Y = 16;

std::vector<uint32_t> MatMul::loadSPIRV(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { std::cerr << "Cannot open: " << path << "\n"; return {}; }
    size_t size = f.tellg();
    if (size == 0 || size % 4 != 0) { std::cerr << "Bad SPIRV: " << path << "\n"; return {}; }
    f.seekg(0);
    std::vector<uint32_t> code(size / 4);
    f.read(reinterpret_cast<char*>(code.data()), size);
    return code;
}

uint32_t MatMul::findMemType(uint32_t typeBits) {
    return findMemoryType(physDev, typeBits);
}

void MatMul::createStorageBuffer(VkBuffer& buf, VkDeviceMemory& mem, VkDeviceSize sz) {
    VkBufferCreateInfo bi{};
    bi.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size        = sz;
    bi.usage       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device, &bi, nullptr, &buf);

    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(device, buf, &mr);

    VkMemoryAllocateInfo ai{};
    ai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize  = mr.size;
    ai.memoryTypeIndex = findMemType(mr.memoryTypeBits);
    vkAllocateMemory(device, &ai, nullptr, &mem);
    vkBindBufferMemory(device, buf, mem, 0);
}

MatMul::MatMul(VkDevice dev, VkPhysicalDevice phys, VkQueue q,
               const std::string& shaderPath,
               uint32_t M, uint32_t K, uint32_t N)
    : device(dev), physDev(phys), queue(q),
      M_(M), K_(K), N_(N),
      sizeA_(static_cast<VkDeviceSize>(M) * K * sizeof(float)),
      sizeB_(static_cast<VkDeviceSize>(K) * N * sizeof(float)),
      sizeC_(static_cast<VkDeviceSize>(M) * N * sizeof(float)) {

    createStorageBuffer(bufferA, memoryA, sizeA_);
    createStorageBuffer(bufferB, memoryB, sizeB_);
    createStorageBuffer(bufferC, memoryC, sizeC_);

    VkDescriptorSetLayoutBinding bindings[3]{};
    for (uint32_t i = 0; i < 3; i++) {
        bindings[i].binding         = i;
        bindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo dsli{};
    dsli.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsli.bindingCount = 3;
    dsli.pBindings    = bindings;
    vkCreateDescriptorSetLayout(device, &dsli, nullptr, &dsl);

    VkPushConstantRange pcr{};
    pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pcr.offset     = 0;
    pcr.size       = sizeof(PushConsts);

    VkPipelineLayoutCreateInfo pli{};
    pli.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount         = 1;
    pli.pSetLayouts            = &dsl;
    pli.pushConstantRangeCount = 1;
    pli.pPushConstantRanges    = &pcr;
    vkCreatePipelineLayout(device, &pli, nullptr, &pipelineLayout);

    auto spirv = loadSPIRV(shaderPath);
    if (spirv.empty()) { std::cerr << "Failed to load SPIRV\n"; return; }
    VkShaderModuleCreateInfo smci{};
    smci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = spirv.size() * 4;
    smci.pCode    = spirv.data();
    vkCreateShaderModule(device, &smci, nullptr, &shaderMod);

    VkComputePipelineCreateInfo cpci{};
    cpci.sType        = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cpci.layout       = pipelineLayout;
    cpci.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    cpci.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = shaderMod;
    cpci.stage.pName  = "main";
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipeline);

    VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3};
    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets       = 1;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes    = &poolSize;
    vkCreateDescriptorPool(device, &dpci, nullptr, &descPool);

    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool     = descPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts        = &dsl;
    vkAllocateDescriptorSets(device, &dsai, &descSet);

    VkDescriptorBufferInfo dbi[3]{};
    dbi[0] = {bufferA, 0, sizeA_};
    dbi[1] = {bufferB, 0, sizeB_};
    dbi[2] = {bufferC, 0, sizeC_};
    VkWriteDescriptorSet writes[3]{};
    for (uint32_t i = 0; i < 3; i++) {
        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = descSet;
        writes[i].dstBinding      = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].pBufferInfo     = &dbi[i];
    }
    vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);

    VkCommandPoolCreateInfo cpoolci{};
    cpoolci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpoolci.queueFamilyIndex = 0;
    vkCreateCommandPool(device, &cpoolci, nullptr, &cmdPool);

    VkCommandBufferAllocateInfo cbai{};
    cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool        = cmdPool;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &cbai, &cmd);
}

MatMul::~MatMul() {
    vkDeviceWaitIdle(device);
    vkDestroyCommandPool(device, cmdPool, nullptr);
    vkDestroyDescriptorPool(device, descPool, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyShaderModule(device, shaderMod, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, dsl, nullptr);
    vkFreeMemory(device, memoryA, nullptr);
    vkFreeMemory(device, memoryB, nullptr);
    vkFreeMemory(device, memoryC, nullptr);
    vkDestroyBuffer(device, bufferA, nullptr);
    vkDestroyBuffer(device, bufferB, nullptr);
    vkDestroyBuffer(device, bufferC, nullptr);
}

void MatMul::setA(const float* data) {
    void* mapped;
    vkMapMemory(device, memoryA, 0, sizeA_, 0, &mapped);
    std::memcpy(mapped, data, sizeA_);
    vkUnmapMemory(device, memoryA);
}

void MatMul::setB(const float* data) {
    void* mapped;
    vkMapMemory(device, memoryB, 0, sizeB_, 0, &mapped);
    std::memcpy(mapped, data, sizeB_);
    vkUnmapMemory(device, memoryB);
}

void MatMul::run() {
    VkCommandBufferBeginInfo cbbi{};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &cbbi);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);

    PushConsts pc{M_, K_, N_};
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(pc), &pc);

    uint32_t gx = (N_ + WG_X - 1) / WG_X;
    uint32_t gy = (M_ + WG_Y - 1) / WG_Y;
    vkCmdDispatch(cmd, gx, gy, 1);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo si{};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd;
    vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

void MatMul::getResult(float* out) {
    void* mapped;
    vkMapMemory(device, memoryC, 0, sizeC_, 0, &mapped);
    std::memcpy(out, mapped, sizeC_);
    vkUnmapMemory(device, memoryC);
}

#include "device_selector.hpp"
#include <iostream>
#include <vector>
#include <fstream>

std::vector<uint32_t> loadSPIRV(const char* path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    size_t size = f.tellg();
    f.seekg(0);
    std::vector<uint32_t> code(size / 4);
    f.read(reinterpret_cast<char*>(code.data()), size);
    return code;
}

int main() {
    auto sel = selectDevice(createVulkanInstance());
    if (!sel.device) {
        std::cerr << "Device selection failed\n";
        return 1;
    }

    VkDevice device = sel.device;
    VkQueue queue = sel.queue;
    VkPhysicalDevice physDev = sel.physicalDevice;

    std::cout << "Selected GPU:\n";
    printDeviceInfo(sel.properties);

    // buffer A (7)
    VkBufferCreateInfo bi{};
    bi.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size        = sizeof(float);
    bi.usage       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer bufferA;
    vkCreateBuffer(device, &bi, nullptr, &bufferA);
    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(device, bufferA, &mr);
    VkMemoryAllocateInfo ai{};
    ai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize  = mr.size;
    ai.memoryTypeIndex = findMemoryType(physDev, mr.memoryTypeBits);
    VkDeviceMemory memoryA;
    vkAllocateMemory(device, &ai, nullptr, &memoryA);
    vkBindBufferMemory(device, bufferA, memoryA, 0);
    void* mappedA;
    vkMapMemory(device, memoryA, 0, sizeof(float), 0, &mappedA);
    *(float*)mappedA = 7.0f;
    vkUnmapMemory(device, memoryA);

    // buffer B (5)
    VkBuffer bufferB;
    vkCreateBuffer(device, &bi, nullptr, &bufferB);
    vkGetBufferMemoryRequirements(device, bufferB, &mr);
    ai.memoryTypeIndex = findMemoryType(physDev, mr.memoryTypeBits);
    VkDeviceMemory memoryB;
    vkAllocateMemory(device, &ai, nullptr, &memoryB);
    vkBindBufferMemory(device, bufferB, memoryB, 0);
    void* mappedB;
    vkMapMemory(device, memoryB, 0, sizeof(float), 0, &mappedB);
    *(float*)mappedB = 5.0f;
    vkUnmapMemory(device, memoryB);

    // buffer Out (result)
    VkBuffer bufferOut;
    vkCreateBuffer(device, &bi, nullptr, &bufferOut);
    vkGetBufferMemoryRequirements(device, bufferOut, &mr);
    ai.memoryTypeIndex = findMemoryType(physDev, mr.memoryTypeBits);
    VkDeviceMemory memoryOut;
    vkAllocateMemory(device, &ai, nullptr, &memoryOut);
    vkBindBufferMemory(device, bufferOut, memoryOut, 0);
    void* mappedOut;
    vkMapMemory(device, memoryOut, 0, sizeof(float), 0, &mappedOut);
    *(float*)mappedOut = 0.0f;
    vkUnmapMemory(device, memoryOut);

    std::cout << "Buffers ready\n";

    // descriptor set layout
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
    VkDescriptorSetLayout dsl;
    vkCreateDescriptorSetLayout(device, &dsli, nullptr, &dsl);

    // pipeline layout
    VkPipelineLayoutCreateInfo pli{};
    pli.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount = 1;
    pli.pSetLayouts    = &dsl;
    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(device, &pli, nullptr, &pipelineLayout);

    // shader
    auto spirv = loadSPIRV("shader/add.comp.spv");
    VkShaderModuleCreateInfo smci{};
    smci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = spirv.size() * 4;
    smci.pCode    = spirv.data();
    VkShaderModule shaderMod;
    vkCreateShaderModule(device, &smci, nullptr, &shaderMod);

    // compute pipeline
    VkComputePipelineCreateInfo cpci{};
    cpci.sType        = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cpci.layout       = pipelineLayout;
    cpci.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    cpci.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = shaderMod;
    cpci.stage.pName  = "main";
    VkPipeline pipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipeline);

    std::cout << "Pipeline ready\n";

    // descriptor pool + set
    VkDescriptorPoolSize poolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3};
    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets       = 1;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes    = &poolSize;
    VkDescriptorPool descPool;
    vkCreateDescriptorPool(device, &dpci, nullptr, &descPool);

    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool     = descPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts        = &dsl;
    VkDescriptorSet descSet;
    vkAllocateDescriptorSets(device, &dsai, &descSet);

    // wire buffers to descriptor set
    VkDescriptorBufferInfo dbi[3]{};
    dbi[0] = {bufferA,   0, sizeof(float)};
    dbi[1] = {bufferB,   0, sizeof(float)};
    dbi[2] = {bufferOut, 0, sizeof(float)};
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

    // command pool + buffer
    VkCommandPoolCreateInfo cpoolci{};
    cpoolci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpoolci.queueFamilyIndex = 0;
    VkCommandPool cmdPool;
    vkCreateCommandPool(device, &cpoolci, nullptr, &cmdPool);

    VkCommandBufferAllocateInfo cbai{};
    cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool        = cmdPool;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &cbai, &cmd);

    // record commands
    VkCommandBufferBeginInfo cbbi{};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &cbbi);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descSet, 0, nullptr);
    vkCmdDispatch(cmd, 1, 1, 1);
    vkEndCommandBuffer(cmd);

    // submit
    VkSubmitInfo si{};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd;
    vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    // read result
    void* mappedResult;
    vkMapMemory(device, memoryOut, 0, sizeof(float), 0, &mappedResult);
    float result = *(float*)mappedResult;
    vkUnmapMemory(device, memoryOut);

    std::cout << "7 + 5 = " << result << "\n";

    // cleanup
    vkDestroyCommandPool(device, cmdPool, nullptr);
    vkDestroyDescriptorPool(device, descPool, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyShaderModule(device, shaderMod, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, dsl, nullptr);
    vkFreeMemory(device, memoryA, nullptr);
    vkFreeMemory(device, memoryB, nullptr);
    vkFreeMemory(device, memoryOut, nullptr);
    vkDestroyBuffer(device, bufferA, nullptr);
    vkDestroyBuffer(device, bufferB, nullptr);
    vkDestroyBuffer(device, bufferOut, nullptr);
    destroyDeviceSelection(sel);

    return 0;
}

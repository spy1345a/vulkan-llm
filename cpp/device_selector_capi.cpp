#include "device_selector.hpp"
#include "compute_add.hpp"
#include <cstring>
#include <cstdint>

extern "C" {

int ds_get_device_count() {
    VkInstance inst = createVulkanInstance();
    if (!inst) return 0;
    auto devices = enumeratePhysicalDevices(inst);
    vkDestroyInstance(inst, nullptr);
    return (int)devices.size();
}

int ds_get_device_info(int index, char* name_out, int name_max_len,
                       int* type_out, int* api_version_out) {
    VkInstance inst = createVulkanInstance();
    if (!inst) return -1;
    auto devices = enumeratePhysicalDevices(inst);
    if (index < 0 || (size_t)index >= devices.size()) {
        vkDestroyInstance(inst, nullptr);
        return -1;
    }
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(devices[index], &props);
    std::strncpy(name_out, props.deviceName, name_max_len - 1);
    name_out[name_max_len - 1] = '\0';
    if (type_out) *type_out = (int)props.deviceType;
    if (api_version_out) *api_version_out = (int)props.apiVersion;
    vkDestroyInstance(inst, nullptr);
    return 0;
}

int64_t ds_create_device(int preferred_index) {
    VkInstance inst = createVulkanInstance();
    if (!inst) return 0;

    auto devices = enumeratePhysicalDevices(inst);
    if (devices.empty()) { vkDestroyInstance(inst, nullptr); return 0; }

    VkPhysicalDevice selected;
    if (preferred_index >= 0 && (size_t)preferred_index < devices.size())
        selected = devices[preferred_index];
    else
        selected = devices[0];

    DeviceSelection* sel = new DeviceSelection{};
    sel->instance = inst;
    sel->physicalDevice = selected;
    vkGetPhysicalDeviceProperties(selected, &sel->properties);
    sel->queueFamilyIndex = 0;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = sel->queueFamilyIndex;
    qci.queueCount       = 1;
    qci.pQueuePriorities = &priority;

    VkDeviceCreateInfo dci{};
    dci.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos    = &qci;

    if (vkCreateDevice(selected, &dci, nullptr, &sel->device) != VK_SUCCESS) {
        delete sel;
        vkDestroyInstance(inst, nullptr);
        return 0;
    }
    vkGetDeviceQueue(sel->device, sel->queueFamilyIndex, 0, &sel->queue);
    return reinterpret_cast<int64_t>(sel);
}

int ds_get_selected_device_name(int64_t handle, char* out, int max_len) {
    if (!handle) return -1;
    auto* sel = reinterpret_cast<DeviceSelection*>(handle);
    std::strncpy(out, sel->properties.deviceName, max_len - 1);
    out[max_len - 1] = '\0';
    return 0;
}

int ds_get_selected_device_type(int64_t handle) {
    if (!handle) return -1;
    auto* sel = reinterpret_cast<DeviceSelection*>(handle);
    return (int)sel->properties.deviceType;
}

void ds_destroy_device(int64_t handle) {
    if (!handle) return;
    auto* sel = reinterpret_cast<DeviceSelection*>(handle);
    if (sel->device) vkDestroyDevice(sel->device, nullptr);
    if (sel->instance) vkDestroyInstance(sel->instance, nullptr);
    delete sel;
}

int64_t ca_create(int64_t sel_handle, const char* shader_dir) {
    if (!sel_handle) return 0;
    auto* sel = reinterpret_cast<DeviceSelection*>(sel_handle);
    auto* add = new ComputeAdd(sel->device, sel->physicalDevice, sel->queue, shader_dir);
    return reinterpret_cast<int64_t>(add);
}

void ca_set_a(int64_t handle, float val) {
    if (!handle) return;
    reinterpret_cast<ComputeAdd*>(handle)->setA(val);
}

void ca_set_b(int64_t handle, float val) {
    if (!handle) return;
    reinterpret_cast<ComputeAdd*>(handle)->setB(val);
}

float ca_run(int64_t handle) {
    if (!handle) return 0.0f;
    return reinterpret_cast<ComputeAdd*>(handle)->run();
}

void ca_destroy(int64_t handle) {
    if (!handle) return;
    delete reinterpret_cast<ComputeAdd*>(handle);
}

}

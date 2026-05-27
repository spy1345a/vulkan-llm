from vulkan._device import get_device_count, get_device_info, VulkanDevice

_device_list = [get_device_info(i) for i in range(get_device_count())]

def devices():
    if not _device_list:
        print("No Vulkan devices found")
        return
    for gpu in _device_list:
        print(f"  [{gpu['index']}] {gpu['name']}  ({gpu['type']})")

def select_device(index: int):
    return VulkanDevice(index)

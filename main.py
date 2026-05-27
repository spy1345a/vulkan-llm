import vulkan

vulkan.devices()

gpu = vulkan.select_device(0)
print(f"\n{gpu.name}")
gpu.close()

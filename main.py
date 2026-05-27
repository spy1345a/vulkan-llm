from vulkan import devices, select_device

devices()

gpu = select_device(0)
print(f"\n{gpu.name}")
gpu.close()

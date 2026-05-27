import ctypes
import ctypes.util
from pathlib import Path

_lib: ctypes.CDLL | None = None


def _get_lib() -> ctypes.CDLL:
    global _lib
    if _lib is not None:
        return _lib
    candidates = [
        Path(__file__).parent / "cpp" / "build" / "libvulkan_device_selector.so",
        ctypes.util.find_library("vulkan_device_selector"),
    ]
    for path in candidates:
        if path:
            try:
                _lib = ctypes.CDLL(str(path))
                break
            except OSError:
                continue
    if _lib is None:
        raise OSError("Cannot find libvulkan_device_selector.so")

    _lib.ds_get_device_count.restype = ctypes.c_int
    _lib.ds_get_device_count.argtypes = []

    _lib.ds_get_device_info.restype = ctypes.c_int
    _lib.ds_get_device_info.argtypes = [
        ctypes.c_int,
        ctypes.c_char_p,
        ctypes.c_int,
        ctypes.POINTER(ctypes.c_int),
        ctypes.POINTER(ctypes.c_int),
    ]

    _lib.ds_create_device.restype = ctypes.c_int64
    _lib.ds_create_device.argtypes = [ctypes.c_int]

    _lib.ds_get_selected_device_name.restype = ctypes.c_int
    _lib.ds_get_selected_device_name.argtypes = [ctypes.c_int64, ctypes.c_char_p, ctypes.c_int]

    _lib.ds_get_selected_device_type.restype = ctypes.c_int
    _lib.ds_get_selected_device_type.argtypes = [ctypes.c_int64]

    _lib.ds_destroy_device.restype = None
    _lib.ds_destroy_device.argtypes = [ctypes.c_int64]

    return _lib


DEVICE_TYPE_NAMES = {
    0: "Other",
    1: "Integrated GPU",
    2: "Discrete GPU",
    3: "Virtual GPU",
    4: "CPU",
}


def get_device_count() -> int:
    return _get_lib().ds_get_device_count()


def get_device_info(index: int) -> dict:
    lib = _get_lib()
    name_buf = ctypes.create_string_buffer(256)
    dev_type = ctypes.c_int()
    api_ver = ctypes.c_int()
    ret = lib.ds_get_device_info(
        index, name_buf, ctypes.c_int(256),
        ctypes.byref(dev_type), ctypes.byref(api_ver),
    )
    if ret != 0:
        raise RuntimeError(f"Failed to get device info for index {index}")
    ver = api_ver.value
    return {
        "index": index,
        "name": name_buf.value.decode(),
        "type": DEVICE_TYPE_NAMES.get(dev_type.value, "Unknown"),
        "type_code": dev_type.value,
        "api_version": f"{ver >> 22}.{(ver >> 12) & 0x3ff}.{ver & 0xfff}",
    }


class VulkanDevice:
    def __init__(self, preferred_index: int = -1):
        lib = _get_lib()
        self._handle = lib.ds_create_device(preferred_index)
        if self._handle == 0:
            raise RuntimeError("Failed to create Vulkan device")

    @property
    def name(self) -> str:
        lib = _get_lib()
        buf = ctypes.create_string_buffer(256)
        lib.ds_get_selected_device_name(self._handle, buf, 256)
        return buf.value.decode()

    @property
    def type_code(self) -> int:
        return _get_lib().ds_get_selected_device_type(self._handle)

    @property
    def type_name(self) -> str:
        return DEVICE_TYPE_NAMES.get(self.type_code, "Unknown")

    def close(self):
        if self._handle:
            _get_lib().ds_destroy_device(self._handle)
            self._handle = 0

    def __del__(self):
        self.close()

    def __repr__(self) -> str:
        return f"VulkanDevice(name='{self.name}', type='{self.type_name}')"


if __name__ == "__main__":
    count = get_device_count()
    print(f"Found {count} Vulkan device(s):\n")
    for i in range(count):
        info = get_device_info(i)
        print(f"  [{info['index']}] {info['name']}")
        print(f"       Type : {info['type']}")
        print(f"       API  : {info['api_version']}\n")

    if count > 0:
        dev = VulkanDevice()
        print(f"Selected: {dev}")
        dev.close()

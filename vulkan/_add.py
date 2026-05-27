import ctypes
from pathlib import Path

_lib = None

def _get_lib():
    global _lib
    if _lib is not None:
        return _lib
    candidates = [
        Path(__file__).resolve().parent.parent / "cpp" / "build" / "libvulkan_device_selector.so",
    ]
    for path in candidates:
        try:
            _lib = ctypes.CDLL(str(path))
            break
        except OSError:
            continue
    if _lib is None:
        raise OSError("Cannot find libvulkan_device_selector.so")

    _lib.ca_create.restype = ctypes.c_int64
    _lib.ca_create.argtypes = [ctypes.c_int64, ctypes.c_char_p]

    _lib.ca_set_a.restype = None
    _lib.ca_set_a.argtypes = [ctypes.c_int64, ctypes.c_float]

    _lib.ca_set_b.restype = None
    _lib.ca_set_b.argtypes = [ctypes.c_int64, ctypes.c_float]

    _lib.ca_run.restype = ctypes.c_float
    _lib.ca_run.argtypes = [ctypes.c_int64]

    _lib.ca_destroy.restype = None
    _lib.ca_destroy.argtypes = [ctypes.c_int64]

    return _lib


_SHADER_DIR = str(Path(__file__).resolve().parent.parent / "cpp" / "shader") + "/"

class Adder:
    def __init__(self, device_handle: int, shader_dir: str = ""):
        sdir = shader_dir if shader_dir else _SHADER_DIR
        self._handle = _get_lib().ca_create(device_handle, sdir.encode())
        if self._handle == 0:
            raise RuntimeError("Failed to create Adder")

    def set_a(self, val: float):
        _get_lib().ca_set_a(self._handle, ctypes.c_float(val))

    def set_b(self, val: float):
        _get_lib().ca_set_b(self._handle, ctypes.c_float(val))

    def run(self) -> float:
        return _get_lib().ca_run(self._handle)

    def close(self):
        if self._handle:
            _get_lib().ca_destroy(self._handle)
            self._handle = 0

    def __del__(self):
        self.close()

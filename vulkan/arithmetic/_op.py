import ctypes
from pathlib import Path

_lib = None


def _get_lib():
    global _lib
    if _lib is not None:
        return _lib
    path = Path(__file__).resolve().parent.parent.parent / "cpp" / "build" / "libvulkan_device_selector.so"
    _lib = ctypes.CDLL(str(path))

    _lib.op_create.restype = ctypes.c_int64
    _lib.op_create.argtypes = [ctypes.c_int64, ctypes.c_char_p, ctypes.c_int]

    _lib.op_set_a.restype = None
    _lib.op_set_a.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.op_set_b.restype = None
    _lib.op_set_b.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.op_run.restype = None
    _lib.op_run.argtypes = [ctypes.c_int64]

    _lib.op_get_result.restype = None
    _lib.op_get_result.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.op_count.restype = ctypes.c_int
    _lib.op_count.argtypes = [ctypes.c_int64]

    _lib.op_destroy.restype = None
    _lib.op_destroy.argtypes = [ctypes.c_int64]

    return _lib


_SHADER_DIR = str(Path(__file__).resolve().parent.parent.parent / "cpp" / "arithmetic" / "shaders") + "/"


class Op:
    def __init__(self, device_handle: int, shader_name: str, count: int):
        shader_path = _SHADER_DIR + shader_name
        self._count = count
        self._arr_type = ctypes.c_float * count
        self._handle = _get_lib().op_create(device_handle, shader_path.encode(), count)
        if self._handle == 0:
            raise RuntimeError(f"Failed to create Op for {shader_name}")

    def set_a(self, data: list):
        arr = self._arr_type(*data)
        _get_lib().op_set_a(self._handle, arr)

    def set_b(self, data: list):
        arr = self._arr_type(*data)
        _get_lib().op_set_b(self._handle, arr)

    def run(self):
        _get_lib().op_run(self._handle)

    def get_result(self) -> list:
        out = self._arr_type()
        _get_lib().op_get_result(self._handle, out)
        return list(out)

    def close(self):
        if self._handle:
            _get_lib().op_destroy(self._handle)
            self._handle = 0

    def __del__(self):
        self.close()

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

    _lib.op_destroy.restype = None
    _lib.op_destroy.argtypes = [ctypes.c_int64]

    return _lib


_SHADER_DIR = str(Path(__file__).resolve().parent.parent.parent / "cpp" / "arithmetic" / "shaders") + "/"


class Op:
    def __init__(self, device_handle: int, shader_name: str):
        self._dev_handle = device_handle
        self._shader_name = shader_name
        self._handle = 0
        self._count = 0

    def _ensure(self, count: int):
        if self._handle and self._count == count:
            return
        if self._handle:
            self.close()
        self._count = count
        path = _SHADER_DIR + self._shader_name
        self._handle = _get_lib().op_create(self._dev_handle, path.encode(), count)
        if self._handle == 0:
            raise RuntimeError(f"Failed to create Op for {self._shader_name}")

    def set_a(self, data: list):
        self._ensure(len(data))
        arr = (ctypes.c_float * len(data))(*data)
        _get_lib().op_set_a(self._handle, arr)

    def set_b(self, data: list):
        self._ensure(len(data))
        arr = (ctypes.c_float * len(data))(*data)
        _get_lib().op_set_b(self._handle, arr)

    def set(self, *arrays):
        if len(arrays) < 2:
            raise ValueError("Need at least 2 arrays")
        n = len(arrays[0])
        for a in arrays:
            if len(a) != n:
                raise ValueError("All arrays must have same length")
        self.set_a(arrays[0])
        for arr in arrays[1:]:
            self.set_b(arr)
            self.run()
            if arr is not arrays[-1]:
                tmp = self.get_result()
                self.set_a(tmp)
        return self

    def run(self):
        _get_lib().op_run(self._handle)

    def get_result(self) -> list:
        out = (ctypes.c_float * self._count)()
        _get_lib().op_get_result(self._handle, out)
        return list(out)

    def close(self):
        if self._handle:
            _get_lib().op_destroy(self._handle)
            self._handle = 0
            self._count = 0

    def __del__(self):
        self.close()

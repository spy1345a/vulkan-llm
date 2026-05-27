import ctypes
from pathlib import Path

_lib = None


def _get_lib():
    global _lib
    if _lib is not None:
        return _lib
    path = Path(__file__).resolve().parent.parent.parent / "cpp" / "build" / "libvulkan_device_selector.so"
    _lib = ctypes.CDLL(str(path))

    _lib.mm_create.restype = ctypes.c_int64
    _lib.mm_create.argtypes = [ctypes.c_int64, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int]

    _lib.mm_set_a.restype = None
    _lib.mm_set_a.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.mm_set_b.restype = None
    _lib.mm_set_b.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.mm_run.restype = None
    _lib.mm_run.argtypes = [ctypes.c_int64]

    _lib.mm_get_result.restype = None
    _lib.mm_get_result.argtypes = [ctypes.c_int64, ctypes.POINTER(ctypes.c_float)]

    _lib.mm_destroy.restype = None
    _lib.mm_destroy.argtypes = [ctypes.c_int64]

    return _lib


_SHADER_DIR = str(Path(__file__).resolve().parent.parent.parent / "cpp" / "arithmetic" / "shaders") + "/"


class MatMul:
    def __init__(self, dev, M: int, K: int, N: int):
        self._M, self._K, self._N = M, K, N
        path = _SHADER_DIR + "matmul.spv"
        self._handle = _get_lib().mm_create(dev._handle, path.encode(), M, K, N)
        if self._handle == 0:
            raise RuntimeError("Failed to create MatMul")

    def set_a(self, data: list):
        arr = (ctypes.c_float * (self._M * self._K))(*data)
        _get_lib().mm_set_a(self._handle, arr)

    def set_b(self, data: list):
        arr = (ctypes.c_float * (self._K * self._N))(*data)
        _get_lib().mm_set_b(self._handle, arr)

    def run(self):
        _get_lib().mm_run(self._handle)

    def get_result(self) -> list:
        out = (ctypes.c_float * (self._M * self._N))()
        _get_lib().mm_get_result(self._handle, out)
        return list(out)

    def close(self):
        if self._handle:
            _get_lib().mm_destroy(self._handle)
            self._handle = 0

    def __del__(self):
        self.close()

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
    def __init__(self, dev):
        self._dev = dev
        self._handle = 0
        self._M = self._K = self._N = 0
        self._flat_a = None
        self._flat_b = None

    def _ensure(self):
        if self._handle:
            return
        if self._M == 0 or self._K == 0 or self._N == 0:
            raise RuntimeError("Call set_a() and set_b() first")
        path = _SHADER_DIR + "matmul.spv"
        self._handle = _get_lib().mm_create(self._dev._handle, path.encode(),
                                            self._M, self._K, self._N)
        if self._handle == 0:
            raise RuntimeError("Failed to create MatMul")

    def set_a(self, data: list):
        self._M = len(data)
        self._K = len(data[0])
        self._flat_a = [data[r][c] for r in range(self._M) for c in range(self._K)]
        return self

    def set_b(self, data: list):
        k = len(data)
        self._N = len(data[0])
        if self._K and k != self._K:
            raise ValueError(f"A cols ({self._K}) != B rows ({k})")
        self._K = k
        self._flat_b = [data[r][c] for r in range(k) for c in range(self._N)]
        return self

    def run(self):
        self._ensure()
        arr_a = (ctypes.c_float * (self._M * self._K))(*self._flat_a)
        arr_b = (ctypes.c_float * (self._K * self._N))(*self._flat_b)
        _get_lib().mm_set_a(self._handle, arr_a)
        _get_lib().mm_set_b(self._handle, arr_b)
        _get_lib().mm_run(self._handle)
        return self

    def get_result(self):
        out = (ctypes.c_float * (self._M * self._N))()
        _get_lib().mm_get_result(self._handle, out)
        flat = list(out)
        return [[flat[r * self._N + c] for c in range(self._N)] for r in range(self._M)]

    def close(self):
        if self._handle:
            _get_lib().mm_destroy(self._handle)
            self._handle = 0

    def __del__(self):
        self.close()

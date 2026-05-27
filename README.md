# vulkan-llm

GPU-accelerated arithmetic using Vulkan compute shaders. Python + C++ bindings for running element-wise ops and matrix multiplication on your GPU.

## Quick start

```python
from vulkan import devices, select_device, Add, Sub, Mul, Div, MatMul

devices()
dev = select_device(0)

# Element-wise ops (any number of arrays)
op = Add(dev)
op.set([1, 2, 3], [4, 5, 6], [7, 8, 9])
print(op.get_result())   # [12.0, 15.0, 18.0]

# Matrix multiplication (auto-detects dimensions)
mm = MatMul(dev)
mm.set_a([[1, 2], [3, 4]])
mm.set_b([[5, 6], [7, 8]])
mm.run()
print(mm.get_result())   # [[19, 22], [43, 50]]
```

## Build

```bash
cd cpp
cmake -B build
cmake --build build
```

Requires: Vulkan SDK, CMake 3.20+, C++17, Python 3.

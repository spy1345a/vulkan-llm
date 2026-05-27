# vulkan-llm

**GPU compute on hardware everyone forgot about.**

CUDA locks AI to Nvidia. Vulkan runs on everything — that RX 580 gathering dust in your closet, the integrated Intel GPU in your laptop, the old AMD card you replaced. These chips have teraflops of compute. They just need software that talks to them.

vulkan-llm is a from-scratch Vulkan compute engine for running ML operations on **any GPU** — no CUDA, no ROCm, no vendor lock-in. If Vulkan runs on it, you can compute on it.

## What it does

- Element-wise ops (add, sub, mul, div) on GPU arrays — any number of inputs
- Matrix multiplication with auto-detected dimensions  
- Parallel dispatch across all GPU cores (256-thread workgroups)
- Python bindings via ctypes (no PyTorch dependency)

## Quick start

```python
from vulkan import devices, select_device, Add, MatMul

devices()
dev = select_device(0)

# Sum 4 arrays at once on your "obsolete" GPU
op = Add(dev)
op.set([1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12])
print(op.get_result())   # [22.0, 26.0, 30.0]

# Matrix multiply on hardware that "can't do AI"
mm = MatMul(dev)
mm.set_a([[1, 2], [3, 4]]).set_b([[5, 6], [7, 8]]).run()
print(mm.get_result())   # [[19, 22], [43, 50]]
```

The above runs on an **AMD Radeon RX 580** — a 2017 GPU worth ~$50 used that no modern AI framework talks to. It has 2304 shaders and 8 GB VRAM. That's not "e-waste" — that's a parallel compute engine waiting for software.

## Why Vulkan?

| Approach | Vendor lock | Hardware support |
|----------|-------------|-----------------|
| CUDA     | Nvidia only | RTX 20xx+ for modern ML |
| ROCm     | AMD only (Linux) | RX 5000+ officially |
| Vulkan   | **None** | **Every GPU since 2016** |

Vulkan is the only cross-vendor, cross-platform GPU compute API. Same code runs on AMD, Nvidia, Intel, Apple, ARM, and Qualcomm GPUs.

## Build

```bash
cd cpp
cmake -B build
cmake --build build
```

Requires: Vulkan SDK, CMake 3.20+, C++17, Python 3.

## Roadmap

- [x] Element-wise ops with varargs
- [x] Matrix multiplication
- [ ] Tensor views (no CPU copies between chained ops)
- [ ] Convolution for neural networks
- [ ] Half-precision (FP16) support
- [ ] Custom PyTorch extension for autograd
- [ ] Operator fusing (single dispatch for `a * b + c`)

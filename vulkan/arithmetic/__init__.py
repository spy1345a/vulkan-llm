from vulkan._device import VulkanDevice
from vulkan.arithmetic._op import Op
from vulkan.arithmetic._matmul import MatMul


class Add(Op):
    def __init__(self, dev: VulkanDevice):
        super().__init__(dev._handle, "add.spv")


class Sub(Op):
    def __init__(self, dev: VulkanDevice):
        super().__init__(dev._handle, "sub.spv")


class Mul(Op):
    def __init__(self, dev: VulkanDevice):
        super().__init__(dev._handle, "mul.spv")


class Div(Op):
    def __init__(self, dev: VulkanDevice):
        super().__init__(dev._handle, "div.spv")

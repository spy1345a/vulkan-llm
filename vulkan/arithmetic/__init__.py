from vulkan.arithmetic._op import Op


class Add(Op):
    def __init__(self, device_handle: int, count: int = 1):
        super().__init__(device_handle, "add.spv", count)


class Sub(Op):
    def __init__(self, device_handle: int, count: int = 1):
        super().__init__(device_handle, "sub.spv", count)


class Mul(Op):
    def __init__(self, device_handle: int, count: int = 1):
        super().__init__(device_handle, "mul.spv", count)


class Div(Op):
    def __init__(self, device_handle: int, count: int = 1):
        super().__init__(device_handle, "div.spv", count)

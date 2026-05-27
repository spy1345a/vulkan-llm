from vulkan.arithmetic._op import Op


class Add(Op):
    def __init__(self, device_handle: int):
        super().__init__(device_handle, "add.spv")


class Sub(Op):
    def __init__(self, device_handle: int):
        super().__init__(device_handle, "sub.spv")


class Mul(Op):
    def __init__(self, device_handle: int):
        super().__init__(device_handle, "mul.spv")


class Div(Op):
    def __init__(self, device_handle: int):
        super().__init__(device_handle, "div.spv")

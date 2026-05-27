from vulkan import devices, select_device, Add, Sub, Mul, Div

devices()

dev = select_device(0)

a = [1, 2, 3, 4, 5]
b = [10, 20, 30, 40, 50]

op = Add(dev)
op.set_a(a)
op.set_b(b)
op.run()
print(f"add: {a} + {b} = {op.get_result()}")
op.close()

op = Sub(dev)
op.set_a([10, 20, 30])
op.set_b([3, 5, 7])
op.run()
print(f"sub: {op.get_result()}")
op.close()

op = Mul(dev)
op.set_a([2, 3, 4])
op.set_b([5, 6, 7])
op.run()
print(f"mul: {op.get_result()}")
op.close()

op = Div(dev)
op.set_a([10, 20, 30])
op.set_b([2, 4, 5])
op.run()
print(f"div: {op.get_result()}")
op.close()

dev.close()

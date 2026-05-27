from vulkan import devices, select_device, Add, Sub, Mul, Div

devices()

dev = select_device(0)

op_add = Add(dev._handle)
op_add.set_a(7)
op_add.set_b(5)
print(f"7 + 5 = {op_add.run()}")
op_add.close()

op_sub = Sub(dev._handle)
op_sub.set_a(7)
op_sub.set_b(5)
print(f"7 - 5 = {op_sub.run()}")
op_sub.close()

op_mul = Mul(dev._handle)
op_mul.set_a(7)
op_mul.set_b(5)
print(f"7 * 5 = {op_mul.run()}")
op_mul.close()

op_div = Div(dev._handle)
op_div.set_a(7)
op_div.set_b(5)
print(f"7 / 5 = {op_div.run()}")
op_div.close()

dev.close()

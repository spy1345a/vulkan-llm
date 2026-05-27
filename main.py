from vulkan import devices, select_device, Add, Sub, Mul, Div, MatMul

devices()

dev = select_device(0)

# --- varargs: add 3 arrays ---
op = Add(dev)
op.set([1, 2, 3], [4, 5, 6], [7, 8, 9])
print(f"1+2+3 + 4+5+6 + 7+8+9 = {op.get_result()}")
op.close()

# --- varargs: sub 3 arrays ---
op = Sub(dev)
op.set([10, 20, 30], [1, 2, 3], [4, 5, 6])
print(f"10-1-4, 20-2-5, 30-3-6 = {op.get_result()}")
op.close()

# --- matmul with auto-detect ---
A = [[1, 2],
     [3, 4]]
B = [[5, 6],
     [7, 8]]

mm = MatMul(dev)
mm.set_a(A).set_b(B).run()
print(f"\nA * B = {mm.get_result()}")
mm.close()

# --- 3x3 matmul ---
A3 = [[1, 2, 3],
      [4, 5, 6]]
B3 = [[7, 8],
      [9, 10],
      [11, 12]]

mm3 = MatMul(dev)
mm3.set_a(A3).set_b(B3).run()
print(f"\n2x3 * 3x2 = {mm3.get_result()}")
mm3.close()

dev.close()

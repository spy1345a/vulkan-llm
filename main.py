from vulkan import devices, select_device, Add, Sub, Mul, Div, MatMul

devices()

dev = select_device(0)

# --- element-wise ops ---
op = Add(dev)
op.set_a([1, 2, 3])
op.set_b([10, 20, 30])
op.run()
print(f"add: {op.get_result()}")
op.close()

# --- matrix multiply ---
# A = [[1, 2],
#      [3, 4]]
# B = [[5, 6],
#      [7, 8]]
# C = A * B = [[19, 22],
#              [43, 50]]

M, K, N = 2, 2, 2
A = [1, 2,  3, 4]        # row-major: M×K
B = [5, 6,  7, 8]        # row-major: K×N

mm = MatMul(dev, M, K, N)
mm.set_a(A)
mm.set_b(B)
mm.run()
res = mm.get_result()
print(f"matmul {M}x{K} * {K}x{N}:")
print(f"  [[{res[0]}, {res[1]}],")
print(f"   [{res[2]}, {res[3]}]]")
mm.close()

dev.close()

from vulkan import devices, select_device, Add, Sub, Mul, Div
import time

devices()

dev = select_device(0)

N = 10_000_000
a = [float(i) for i in range(N)]
b = [float(N - i) for i in range(N)]

t0 = time.perf_counter()

op = Add(dev._handle, N)
op.set_a(a)
op.set_b(b)
op.run()
res = op.get_result()
op.close()

t1 = time.perf_counter()

print(f"add  N={N}  time={t1-t0:.4f}s  check={res[0]} {res[N-1]}")
print(f"      expected: 0+{N}={N}, {N-1}+1={N}")
dev.close()

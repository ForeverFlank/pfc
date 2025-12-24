import os
import numpy as np
import matplotlib.pyplot as plt

mat = np.zeros(shape=(16, 16))

def count_bytes(dir):
    with open(dir, "rb") as f:
        while byte := f.read(1):
            num = int.from_bytes(byte)
            upper = (num & 0xF0) >> 4
            lower = num & 0x0F
            mat[upper][lower] += 1

root_dir = "src/mini_v3/asm/"
for file in os.listdir(root_dir):
    if file.endswith(".bin"):
        count_bytes(os.path.join(root_dir, file))

plt.imshow(mat)
plt.show()
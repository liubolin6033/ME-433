import numpy as np
import matplotlib.pyplot as plt

# displacement normalized from -1 to 1
x = np.linspace(-1, 1, 1000)

# desired force normalized from -1 to 1
force = np.sin(8 * np.pi * x)

plt.figure(figsize=(8,4))

plt.plot(x, force, linewidth=2)

plt.xlabel("Normalized Displacement")
plt.ylabel("Normalized Desired Force")

plt.title("Detent Haptic Effect")

plt.grid(True)

plt.ylim([-1.1,1.1])

plt.savefig("detent_force_curve.png", dpi=300)

plt.show()
import seaborn as s
import matplotlib.pyplot as plt
import numpy as np


def get_data(filename):
    data = []
    with open(filename) as f:
        space_size = int(f.readline())
        for _ in range(space_size * space_size):
            value = float(f.readline())
            data.append(value)

    data = np.array(data)
    max_value = max(data)
    data = data.reshape((space_size, space_size))
    return (data, max_value)


data, max_value = get_data("heat-data/0")
plt.subplot(2, 2, 1)
s.heatmap(data, vmin=0.0, vmax=max_value)

data, max_value = get_data("heat-data/1")
plt.subplot(2, 2, 2)
s.heatmap(data, vmin=0.0, vmax=max_value)

data, max_value = get_data("heat-data/2")
plt.subplot(2, 2, 3)
s.heatmap(data, vmin=0.0, vmax=max_value)

data, max_value = get_data("heat-data/3")
plt.subplot(2, 2, 4)
s.heatmap(data, vmin=0.0, vmax=max_value)

plt.show()

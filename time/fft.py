import matplotlib.pyplot as plt


def show_time(data):
    x_data = [t[0] for t in data]
    computation = [t[1] for t in data]
    transmission = [t[2] for t in data]
    total = [t[1] + t[2] for t in data]

    comp_plot, = plt.plot(x_data, computation)
    trans_plot, = plt.plot(x_data, transmission)
    total_plot, = plt.plot(x_data, total)

    plt.legend([comp_plot, trans_plot, total_plot], [
        "Computation", "Transmission", "Total"])


data = [
    (1, 0, 2),
    (2, 2, 3),
    (4, 4, 1),
]

plt.subplot(1, 2, 1)
plt.ylabel("time (s)")
plt.xlabel("number of processes")
show_time(data)

data = [
    (1, 0, 2),
    (2, 2, 3),
    (4, 10, 1),
]

plt.subplot(1, 2, 2)
plt.ylabel("time (s)")
plt.xlabel("size of input data")
show_time(data)

plt.show()

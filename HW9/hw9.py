import csv
import numpy as np
import matplotlib.pyplot as plt


def read_csv(filename):
    time = []
    signal = []

    with open(filename, "r") as file:
        reader = csv.reader(file)
        for row in reader:
            time.append(float(row[0]))
            signal.append(float(row[1]))

    return time, signal


def compute_fft(signal, sample_rate):
    N = len(signal)
    fft_vals = np.abs(np.fft.fft(signal))
    freqs = np.fft.fftfreq(N, 1 / sample_rate)

    half_N = N // 2
    return freqs[:half_N], fft_vals[:half_N]


def moving_average(signal, X):
    filtered = []

    for i in range(len(signal)):
        if i < X:
            filtered.append(signal[i])
        else:
            avg = sum(signal[i - X:i]) / X
            filtered.append(avg)

    return filtered


def iir_filter(signal, A, B):
    filtered = [signal[0]]

    for i in range(1, len(signal)):
        new_val = A * filtered[i - 1] + B * signal[i]
        filtered.append(new_val)

    return filtered


def plot_filter(time, signal, filtered_signal, sample_rate, signal_name, filter_type, parameter_text, save_name):
    freqs, fft_vals = compute_fft(signal, sample_rate)
    freqs_f, fft_filtered = compute_fft(filtered_signal, sample_rate)

    plt.figure(figsize=(10, 6))

    # Time domain
    plt.subplot(2, 1, 1)
    plt.plot(time, signal, label="Raw", color="black")
    plt.plot(time, filtered_signal, label="Filtered", color="red")
    plt.xlabel("Time (s)")
    plt.ylabel("Signal")
    plt.title(f"{signal_name} {filter_type}, {parameter_text}")
    plt.legend()

    # FFT
    plt.subplot(2, 1, 2)
    plt.plot(freqs, fft_vals, label="Raw", color="black")
    plt.plot(freqs_f, fft_filtered, label="Filtered", color="red")
    plt.xlim(0, 200)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title(f"{signal_name} FFT Comparison, {filter_type}")
    plt.legend()

    plt.tight_layout()
    plt.savefig(save_name, dpi=300)
    plt.close()


files = [
    "sigA.csv",
    "sigB.csv",
    "sigC.csv",
    "sigD.csv"
]

MAF_X = 100

IIR_A = 0.9
IIR_B = 0.1

for filename in files:
    signal_name = filename.replace(".csv", "")

    time, signal = read_csv(filename)

    sample_rate = len(time) / time[-1]
    print(signal_name, "sample rate:", sample_rate, "Hz")

    maf_signal = moving_average(signal, MAF_X)

    plot_filter(
        time,
        signal,
        maf_signal,
        sample_rate,
        signal_name,
        "Moving Average Filter",
        f"X = {MAF_X}",
        f"{signal_name}_MAF.png"
    )

    iir_signal = iir_filter(signal, IIR_A, IIR_B)

    plot_filter(
        time,
        signal,
        iir_signal,
        sample_rate,
        signal_name,
        "IIR Filter",
        f"A = {IIR_A}, B = {IIR_B}",
        f"{signal_name}_IIR.png"
    )
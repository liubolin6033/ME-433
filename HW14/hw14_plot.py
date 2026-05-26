import serial
import time
import numpy as np
import matplotlib.pyplot as plt
import csv

PORT = "/dev/tty.usbmodem101"
BAUD = 115200
SAMPLES = 800


def main():
    ser = serial.Serial(PORT, BAUD, timeout=3)
    time.sleep(2)

    ser.reset_input_buffer()

    ser.write(f"{SAMPLES}\n".encode())

    times = []
    raw_values = []
    filtered_values = []

    reading_data = False

    while True:
        line = ser.readline().decode(errors="ignore").strip()

        if not line:
            continue

        print(line)

        if line == "time_ms,raw,filtered":
            reading_data = True
            continue

        if line == "END":
            break

        if reading_data:
            parts = line.split(",")

            if len(parts) == 3:
                try:
                    t = float(parts[0])
                    raw = float(parts[1])
                    filtered = float(parts[2])

                    times.append(t)
                    raw_values.append(raw)
                    filtered_values.append(filtered)

                except ValueError:
                    pass

    ser.close()

    times = np.array(times)
    raw_values = np.array(raw_values)
    filtered_values = np.array(filtered_values)

    time_s = times / 1000.0

    with open("hw14_data.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time_s", "raw", "filtered"])

        for t, raw, filtered in zip(time_s, raw_values, filtered_values):
            writer.writerow([t, raw, filtered])

    plt.figure()
    plt.plot(time_s, raw_values, label="Raw")
    plt.plot(time_s, filtered_values, label="IIR Filtered")
    plt.xlabel("Time (s)")
    plt.ylabel("HX711 Value")
    plt.title("HX711 Force Sensor Data")
    plt.legend()
    plt.grid(True)
    plt.savefig("hw14_time_data.png", dpi=300)
    plt.show()

    dt = np.mean(np.diff(time_s))
    fs = 1.0 / dt

    print(f"Estimated sample rate: {fs:.2f} Hz")
    print(f"Nyquist frequency: {fs / 2:.2f} Hz")

    raw_zero_mean = raw_values - np.mean(raw_values)
    filtered_zero_mean = filtered_values - np.mean(filtered_values)

    raw_fft = np.abs(np.fft.rfft(raw_zero_mean))
    filtered_fft = np.abs(np.fft.rfft(filtered_zero_mean))
    freqs = np.fft.rfftfreq(len(time_s), d=dt)

    plt.figure()
    plt.plot(freqs, raw_fft, label="Raw FFT")
    plt.plot(freqs, filtered_fft, label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title("FFT of Raw and Filtered HX711 Data")
    plt.xlim(0, fs / 2)
    plt.legend()
    plt.grid(True)
    plt.savefig("hw14_fft.png", dpi=300)
    plt.show()

    print("Saved files:")
    print("hw14_data.csv")
    print("hw14_time_data.png")
    print("hw14_fft.png")


if __name__ == "__main__":
    main()
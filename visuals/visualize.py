import argparse
import numpy as np
import matplotlib.pyplot as plt


def main(csv_path: str) -> None:
    data = np.loadtxt(csv_path, delimiter=",", skiprows=1)

    frames = data[:, 0]
    angles = data[:, 1]
    t_absolute = data[:, 2]

    Q1 = np.percentile(angles, 25)
    Q3 = np.percentile(angles, 75)
    IQR = Q3 - Q1

    LOWER = Q1 - 1.5 * IQR
    UPPER = Q3 + 1.5 * IQR

    angles_clean = angles.copy()
    angles_clean[(angles < LOWER) | (angles > UPPER)] = 0.0

    mask = angles_clean != 0.0

    frames = frames[mask]
    angles_clean = angles_clean[mask]
    t_absolute = t_absolute[mask]

    t_delta = np.diff(t_absolute, prepend=t_absolute[0])
    t_delta[0] = 0.0

    time_mean = np.mean(t_delta)
    time_median = np.median(t_delta)
    time_stdev = np.std(t_delta, ddof=1) if len(t_delta) > 1 else 0.0
    time_min = np.min(t_delta)
    time_max = np.max(t_delta)

    print("=== Frame Processing Time Statistics ===")
    print(f"Mean:    {time_mean:.6f} s")
    print(f"Median:  {time_median:.6f} s")
    print(f"Stddev:  {time_stdev:.6f} s")
    print(f"Min:     {time_min:.6f} s")
    print(f"Max:     {time_max:.6f} s at frame {frames[np.argmax(t_delta)]}")
    print()

    plt.figure(figsize=(12, 8))

    plt.subplot(2, 2, 1)
    plt.plot(t_absolute, angles_clean, marker="o")
    plt.xlabel("Absolute Time (s)")
    plt.ylabel("Slip Angle (deg)")
    plt.title("Slip Angle Over Time")
    plt.grid(True)

    plt.subplot(2, 2, 2)
    plt.plot(frames, angles_clean, marker="o")
    plt.xlabel("Frame")
    plt.ylabel("Slip Angle (deg)")
    plt.title("Slip Angle vs. Frame")
    plt.grid(True)

    plt.subplot(2, 2, 3)
    plt.hist(t_delta[1:], bins=10)
    plt.xlabel("Frame Processing Time (s)")
    plt.ylabel("Count")
    plt.title("Distribution of Frame Processing Times")

    plt.subplot(2, 2, 4)
    plt.plot(frames, t_delta, marker="o")
    plt.xlabel("Frame")
    plt.ylabel("Processing Time (s)")
    plt.title("Frame Processing Time (Δt)")
    plt.grid(True)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Visualize slip angle and processing time statistics from a CSV file.")
    parser.add_argument("--csv", required=True, help="Path to the CSV file containing the data.")
    args = parser.parse_args()

    main(args.csv)
'''
Quick script to visualize slip angle and processing time statistics from stats.csv
'''
import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt("../stats.csv", delimiter=",", skiprows=1)

frames = data[:, 0]
angles = data[:, 1]
t_absolute = data[:, 2]

# remove outliers from the data
Q1 = np.percentile(angles, 25)
Q3 = np.percentile(angles, 75)
IQR = Q3 - Q1

LOWER = Q1 - 1.5 * IQR
UPPER = Q3 + 1.5 * IQR

angles_clean = angles.copy()
angles_clean[(angles < LOWER) | (angles > UPPER)] = 0.0

# filter out zero angles (outliers OR frames with no slip angle calculated)
mask = angles_clean != 0.0

frames = frames[mask]
angles_clean = angles_clean[mask]
t_absolute = t_absolute[mask]
# t_delta = t_delta[mask]

# processing time for each frame is t_i - t_(i-1)
t_delta = np.diff(t_absolute, prepend=t_absolute[0])
t_delta[0] = 0.0   # no previous frame

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

# 1. Slip angle over absolute time
plt.subplot(2, 2, 1)
plt.plot(t_absolute, angles_clean, marker='o')
plt.xlabel("Absolute Time (s)")
plt.ylabel("Slip Angle (deg)")
plt.title("Slip Angle Over Time")
plt.grid(True)

# 2. Slip angle vs. frame
plt.subplot(2, 2, 2)
plt.plot(frames, angles_clean, marker='o')
plt.xlabel("Frame")
plt.ylabel("Slip Angle (deg)")
plt.title("Slip Angle vs. Frame")
plt.grid(True)

# 3. Histogram of processing times
plt.subplot(2, 2, 3)
plt.hist(t_delta[1:], bins=10)  # ignore frame 0
plt.xlabel("Frame Processing Time (s)")
plt.ylabel("Count")
plt.title("Distribution of Frame Processing Times")

# 4. Processing time vs. frame
plt.subplot(2, 2, 4)
plt.plot(frames, t_delta, marker='o')
plt.xlabel("Frame")
plt.ylabel("Processing Time (s)")
plt.title("Frame Processing Time (Δt)")
plt.grid(True)

plt.tight_layout()
plt.show()
# Slip Angle Project

## Quick Start

Make sure you have OpenCV installed:
- **macOS**: Use Homebrew: `brew install opencv`
- **Linux**: Use your package manager: `sudo apt install libopencv-dev`
- **Windows**: Install OpenCV manually or via vcpkg.

1. After cloning the project, create a build directory and navigate into it:

```bash
mkdir build
cd build
```

2. Run CMake to configure the project:

```bash
cmake ..
```

3. Build the project:

```bash
cmake --build .
```

4. Run the project:

```bash
/path/to/slip-angle /path/to/original.mp4 /path/to/output.avi
```

## Advanced Options

- To use a system-installed OpenCV, pass `-DBUILD_OPENCV=OFF` and specify the OpenCV directory if needed:

```bash
cmake .. -DBUILD_OPENCV=OFF -DOpenCV_DIR=/path/to/opencv/lib/cmake/opencv4
```

- To build OpenCV locally (this takes longer):

```bash
cmake .. -DBUILD_OPENCV=ON
```

- For Windows, use the following commands in a Developer Command Prompt:

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build .
```

## Notes

- If CMake can't find OpenCV, set `-DOpenCV_DIR=` or `-DOpenCV_ROOT=`.
- Building OpenCV locally places files under `build/_deps` and may take significant time.

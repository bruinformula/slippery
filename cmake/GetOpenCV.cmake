include(FetchContent)
include(ExternalProject)


# Option to control whether we build OpenCV ourselves
option(BUILD_OPENCV "Build OpenCV from source instead of using system version" OFF)

# Allow user to specify a custom OpenCV root (install location)
set(OpenCV_ROOT "" CACHE PATH "Path to a custom OpenCV installation")

if(BUILD_OPENCV)
    message(STATUS "Building OpenCV from source...")

    set(OPENCV_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/opencv-install)
    set(OPENCV_BINARY_DIR  ${CMAKE_BINARY_DIR}/_deps/opencv-build)
    set(OPENCV_SOURCE_DIR  ${CMAKE_BINARY_DIR}/_deps/opencv-src)

    include(ExternalProject)
    ExternalProject_Add(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.12.0
        GIT_PROGRESS TRUE
        PREFIX ${CMAKE_BINARY_DIR}/_deps/opencv
        SOURCE_DIR ${OPENCV_SOURCE_DIR}
        BINARY_DIR ${OPENCV_BINARY_DIR}
        INSTALL_DIR ${OPENCV_INSTALL_DIR}

        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${OPENCV_INSTALL_DIR}
            -DBUILD_DOCS=OFF
            -DBUILD_EXAMPLES=OFF
            -DBUILD_TESTS=OFF
            -DBUILD_PERF_TESTS=OFF
            -DBUILD_NEW_PYTHON_SUPPORT=OFF
            -DBUILD_WITH_DEBUG_INFO=OFF
            -DBUILD_WITH_STATIC_CRT=OFF
            -DBUILD_FAT_JAVA_LIB=OFF
            -DWITH_FFMPEG=OFF
            -DACCELERATE_NEW_LAPACK=OFF
    )

    # Tell find_package where to look after build
    set(OpenCV_DIR ${OPENCV_INSTALL_DIR}/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    message(STATUS "Using locally built OpenCV: ${OpenCV_INCLUDE_DIRS}")

elseif(OpenCV_ROOT)
    message(STATUS "Using custom OpenCV installation at: ${OpenCV_ROOT}")
    #set(OpenCV_DIR "${OpenCV_ROOT}/lib/cmake/opencv2")
    find_package(OpenCV REQUIRED)
    message(STATUS "Found OpenCV: ${OpenCV_INCLUDE_DIRS}")

else()
    message(STATUS "Using system-installed OpenCV")
    find_package(OpenCV REQUIRED)
    message(STATUS "Found OpenCV: ${OpenCV_INCLUDE_DIRS}")
endif()

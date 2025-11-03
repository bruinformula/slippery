#find_package(OpenCV REQUIRED)

add_executable(slip-angle main.cpp)
target_link_libraries(slip-angle PRIVATE ${OpenCV_LIBS})
target_include_directories(slip-angle PRIVATE ${OpenCV_INCLUDE_DIRS})


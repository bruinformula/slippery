
add_executable(slip-angle main.cpp)

target_link_libraries(slip-angle PRIVATE ${OpenCV_LIBS} slippery_lib)
target_include_directories(slip-angle PRIVATE ${OpenCV_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include") 


include("${CMAKE_SOURCE_DIR}/cmake/GetOpenCV.cmake")

file(GLOB_RECURSE slippery_lib_CXX_FILES CONFIGURE_DEPENDS *.cpp)

add_library(slippery_lib STATIC ${slippery_lib_CXX_FILES})

target_include_directories(slippery_lib PUBLIC "${CMAKE_SOURCE_DIR}/include" )

target_link_libraries(slippery_lib PRIVATE ${OpenCV_LIBS})
target_include_directories(slippery_lib PRIVATE ${OpenCV_INCLUDE_DIRS})
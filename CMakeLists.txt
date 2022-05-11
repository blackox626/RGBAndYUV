cmake_minimum_required(VERSION 3.22)
project(RGBAndYUV C)

set(CMAKE_C_STANDARD 99)

set(LIBJPEGTURBO_DIR  /opt/libjpeg-turbo)

# 头文件搜索路径
include_directories(${LIBJPEGTURBO_DIR}/include/)

# 动态链接库或静态链接库的搜索路径
link_directories(${LIBJPEGTURBO_DIR}/lib/)

add_executable(RGBAndYUV main.c)

#链接库
target_link_libraries(RGBAndYUV libturbojpeg.dylib)

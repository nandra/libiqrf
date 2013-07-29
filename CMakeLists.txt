project(iqrf C)

cmake_minimum_required(VERSION 2.8.1)
enable_testing()

# use local module (for Findlibusb-1.0.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(libusb-1.0 REQUIRED)
if (NOT LIBUSB_1_FOUND)
	message(FATAL_ERROR "libusb-1.0 not installed. (try apt-get install libusb-1.0-0-dev)")
endif (NOT LIBUSB_1_FOUND)

add_subdirectory(src)

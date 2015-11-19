project(iqrf)

set (iqrf_version_major 0)
set (iqrf_version_minor 1)
set (iqrf_version_patch 2)
set (iqrf_soversion 0)
set (iqrf_postfix "")

set (iqrf_headers
	iqrf.h
	spi.h
	usb.h
)

set (iqrf_sources
	iqrf.c
	spi.c
	usb.c
)

include("GNUInstallDirs")
include_directories ("${LIBUSB_1_INCLUDE_DIRS}")

add_library(iqrf SHARED ${iqrf_sources} ${iqrf_headers})
target_link_libraries(iqrf ${LIBUSB_1_LIBRARIES})

set_target_properties(iqrf PROPERTIES
	VERSION "${iqrf_version_major}.${iqrf_version_minor}"
	SOVERSION "${iqrf_soversion}"
)

install(TARGETS iqrf DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES iqrf.h DESTINATION include)

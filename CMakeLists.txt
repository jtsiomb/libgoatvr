cmake_minimum_required(VERSION 3.0)
project(libgoatvr)

include(GNUInstallDirs)

set(SO_MAJOR 1)
set(SO_MINOR 1)

file(GLOB src "src/*.cc")
file(GLOB hdr "src/*.h")
file(GLOB pubhdr "include/*.h")

add_library(libgoatvr SHARED ${src} ${hdr} ${pubhdr})
add_library(libgoatvr-static STATIC ${src} ${hdr} ${pubhdr})

set_target_properties(libgoatvr PROPERTIES VERSION ${SO_MAJOR}.${SO_MINOR})
set_target_properties(libgoatvr PROPERTIES SOVERSION ${SO_MAJOR})

target_include_directories(libgoatvr PUBLIC include)
target_include_directories(libgoatvr-static PUBLIC include)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
endif()

install(TARGETS libgoatvr
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS libgoatvr-static
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${pubhdr} DESTINATION include)

add_subdirectory(examples/goatvr_sdl)

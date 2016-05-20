cmake_minimum_required(VERSION 3.0)
project(libgoatvr)

include(GNUInstallDirs)

find_package(OpenGL)

option(mod_oculus "Build Oculus (libovr) module" ON)
# TODO
#option(mod_oculus_old "Build old Oculus SDK 0.5 module" OFF)
#option(mod_openvr "Build Valve OpenVR module" OFF)
#option(mod_spaceball "Build spaceball module" OFF)
#option(mod_glstereo "Build stereoscopic OpenGL module" ON)

set(SO_MAJOR 1)
set(SO_MINOR 1)

file(GLOB src "src/*.cc")
file(GLOB hdr "src/*.h")
file(GLOB pubhdr "include/*.h")

# this will generate a file called reg_modules.cc which we'll add to the build
if(WIN32)
	execute_process(COMMAND configure.bat WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
else()
	execute_process(COMMAND ./configure WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
list(APPEND src reg_modules.cc)

if(mod_oculus)
	list(APPEND mod_libs LibOVR)
else()
	list(REMOVE_ITEM src "src/mod_oculus.cc")
	list(REMOVE_ITEM hdr "src/mod_oculus.h")
endif()

add_library(libgoatvr SHARED ${src} ${hdr} ${pubhdr})
add_library(libgoatvr-static STATIC ${src} ${hdr} ${pubhdr})

set_target_properties(libgoatvr PROPERTIES VERSION ${SO_MAJOR}.${SO_MINOR})
set_target_properties(libgoatvr PROPERTIES SOVERSION ${SO_MAJOR})

target_include_directories(libgoatvr PUBLIC include)
target_include_directories(libgoatvr-static PUBLIC include)

target_link_libraries(libgoatvr libgmath ${mod_libs} ${OPENGL_LIBRARIES})
target_link_libraries(libgoatvr-static libgmath ${mod_libs} ${OPENGL_LIBRARIES})

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

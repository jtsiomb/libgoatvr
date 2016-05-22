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
	add_definitions(-DUSE_MOD_OCULUS)
	list(APPEND mod_libs LibOVR)
endif()

add_library(goatvr SHARED ${src} ${hdr} ${pubhdr})
add_library(goatvr-static STATIC ${src} ${hdr} ${pubhdr})

set_target_properties(goatvr PROPERTIES VERSION ${SO_MAJOR}.${SO_MINOR})
set_target_properties(goatvr PROPERTIES SOVERSION ${SO_MAJOR})

if(MSVC)
	set_target_properties(goatvr PROPERTIES PREFIX "lib")
	set_target_properties(goatvr PROPERTIES IMPORT_PREFIX "lib")
	set_target_properties(goatvr-static PROPERTIES PREFIX "lib")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996 /wd4305")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall")
endif()

target_include_directories(goatvr PUBLIC include)
target_include_directories(goatvr-static PUBLIC include)

find_library(gmath_lib NAMES gmath libgmath)
#if(WIN32)
#	set(gmath_lib libgmath)
#else()
#	set(gmath_lib gmath)
#endif()

target_link_libraries(goatvr ${gmath_lib} ${mod_libs} ${OPENGL_LIBRARIES})
target_link_libraries(goatvr-static ${gmath_lib} ${mod_libs} ${OPENGL_LIBRARIES})

install(TARGETS goatvr
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS goatvr-static
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${pubhdr} DESTINATION include)

add_subdirectory(examples/goatvr_sdl)

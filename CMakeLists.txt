cmake_minimum_required(VERSION 3.0)
project(libgoatvr)

include(GNUInstallDirs)

find_package(OpenGL)

option(build_examples "Build example programs" ON)

if(WIN32)
	set(mod_oculus_default ON)
	set(mod_oculus_old_default OFF)
else()
	set(mod_oculus_default OFF)
	set(mod_oculus_old_default ON)
endif()

option(mod_oculus "Build Oculus SDK module" ${mod_oculus_default})
option(mod_openvr "Build Valve OpenVR module" ON)
option(mod_oculus_old "Build old Oculus SDK 0.5 module" ${mod_oculus_old_default})

set(SO_MAJOR 1)
set(SO_MINOR 1)

file(GLOB src "src/*.cc")
file(GLOB hdr "src/*.h")
file(GLOB pubhdr "include/*.h")

# this will generate a file called reg_modules.cc which we'll add to the build
if(WIN32)
	execute_process(COMMAND configure.bat WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	set(def libgoatvr.def)
else()
	execute_process(COMMAND ./configure WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
list(APPEND src reg_modules.cc)

add_library(goatvr SHARED ${src} ${def} ${hdr} ${pubhdr})
add_library(goatvr-static STATIC ${src} ${hdr} ${pubhdr})

set_target_properties(goatvr PROPERTIES CXX_STANDARD 11)
set_target_properties(goatvr-static PROPERTIES CXX_STANDARD 11)

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

include_directories(/usr/local/include)

find_library(gmath_lib NAMES gmath libgmath)
#if(WIN32)
#	set(gmath_lib libgmath)
#else()
#	set(gmath_lib gmath)
#endif()

if(mod_oculus)
	add_definitions(-DUSE_MOD_OCULUS)
	list(APPEND mod_libs LibOVR)
endif()
if(mod_openvr)
	add_definitions(-DUSE_MOD_OPENVR)
	list(APPEND mod_libs openvr_api)
endif()
if(mod_oculus_old)
	add_definitions(-DUSE_MOD_OCULUS_OLD)
	list(APPEND mod_libs OVR dl)
endif()

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

if(build_examples)
	add_subdirectory(examples/goatvr_sdl)
endif()

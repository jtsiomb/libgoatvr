# fix fail to link with SDL2_LIBRARIES on debian due to trailing whitespace
cmake_policy(SET CMP0004 OLD)

file(GLOB src "src/*.c")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

if(NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall")
endif()

add_executable(goatvr_sdl ${src})
target_include_directories(goatvr_sdl PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(goatvr_sdl goatvr-static
	${OPENGL_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_LIBRARIES})

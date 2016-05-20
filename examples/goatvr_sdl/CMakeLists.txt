file(GLOB src "src/*.c")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

if(NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall")
endif()

add_executable(goatvr_sdl ${src})
target_include_directories(goatvr_sdl PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(goatvr_sdl libgoatvr-static
	${OPENGL_LIBRARIES} ${SDL2_LIBRARY})

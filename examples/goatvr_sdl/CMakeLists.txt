file(GLOB src "src/*.c")

if(NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall")
endif()

add_executable(goatvr_sdl ${src})
target_link_libraries(goatvr_sdl libgoatvr-static)

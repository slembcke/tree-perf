cmake_minimum_required(VERSION 3.1)
set(CMAKE_C_STANDARD 11)

add_subdirectory(chipmunk)
add_subdirectory(box2d)

add_executable(tree-perf
	main.c
	brute_test.c
	chipmunk_test.c
	box2d_test.cpp
)

target_include_directories(tree-perf PRIVATE
	chipmunk/include
)

target_link_libraries(tree-perf
	chipmunk
	box2d
)

add_custom_command(
	TARGET tree-perf POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/blue_noise_placement_256_0.png ${CMAKE_BINARY_DIR}/blue_noise_placement_256_0.png
)

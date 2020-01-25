# https://github.com/g-truc/glm/pull/966
find_package(glm REQUIRED)

set(SOURCE_FILES
	fft.h
	export_util.h
	main.cpp
)

add_executable(caustics_textures ${SOURCE_FILES})
target_link_libraries(caustics_textures glm::glm)

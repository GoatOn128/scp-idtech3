set(Q3MAP_SOURCES
	brush_primit.cpp
	brush.cpp
	bsp.cpp
	facebsp.cpp
	fog.cpp
	glfile.cpp
	leakfile.cpp
	light_trace.cpp
	light.cpp
	lightmaps.cpp
	map.cpp
	mesh.cpp
	misc_model.cpp
	nodraw.cpp
	patch.cpp
	portals.cpp
	prtfile.cpp
	shaders.cpp
	surface.cpp
	terrain.cpp
	tjunction.cpp
	tree.cpp
	vis.cpp
	visflow.cpp
	writebsp.cpp
	${CMAKE_SOURCE_DIR}/tools/common/aselib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/bspfile.cpp
	${CMAKE_SOURCE_DIR}/tools/common/cmdlib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/imagelib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/mathlib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/mutex.cpp
	${CMAKE_SOURCE_DIR}/tools/common/polylib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/scriplib.cpp
	${CMAKE_SOURCE_DIR}/tools/common/threads.cpp
	pakstuff.cpp
	jpeglib.cpp
	q3map_main.cpp)

add_executable(q3map ${Q3MAP_SOURCES})
find_package(Threads REQUIRED)
target_include_directories(q3map PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/tools/common")
target_link_libraries(q3map PRIVATE idlib Threads::Threads)
target_compile_features(q3map PRIVATE cxx_std_14)
target_compile_definitions(q3map PRIVATE NOMINMAX)
configure_target(q3map)
target_precompile_headers(q3map PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/tools/precompiled.h>")

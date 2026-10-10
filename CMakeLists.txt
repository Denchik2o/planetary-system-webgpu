cmake_minimum_required(VERSION 3.20)
project(planetary-system-webgpu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- SDL3 ----------
find_package(SDL3 REQUIRED CONFIG)

# ---------- wgpu-native (как в репозитории практик) ----------
find_path(WGPU_NATIVE_INCLUDE_DIR
    NAMES webgpu.h wgpu.h
    HINTS ${WGPU_NATIVE_ROOT}/include/webgpu
    REQUIRED)
find_library(WGPU_NATIVE_LIBRARY
    NAMES wgpu_native
    HINTS ${WGPU_NATIVE_ROOT}/lib
    REQUIRED)

add_library(wgpu-native UNKNOWN IMPORTED)
set_target_properties(wgpu-native PROPERTIES
    IMPORTED_LOCATION "${WGPU_NATIVE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${WGPU_NATIVE_INCLUDE_DIR}"
)
if(APPLE)
    target_link_libraries(wgpu-native INTERFACE
        "-framework Metal"
        "-framework QuartzCore"
        "-framework Foundation"
        "-framework CoreFoundation"
    )
endif()

# ---------- math ----------
add_library(math INTERFACE)
target_include_directories(math INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_sources(math PRIVATE
    include/math/detail/alloca.hpp
    include/math/affine.hpp
    include/math/projection.hpp
    include/math/math.hpp
    include/math/aliases.hpp
    include/math/box.hpp
    include/math/quaternion.hpp
    include/math/utils.hpp
    include/math/matrix.hpp
    include/math/vector.hpp
    include/math/gauss.hpp
    include/math/bezier.hpp
)

# ---------- wgpu_app ----------
add_library(wgpu_app STATIC
    include/wgpu_app/wgpu_app.hpp
    include/wgpu_app/file_utils.hpp
    include/wgpu_app/image_utils.hpp
    include/wgpu_app/obj_loader.hpp
    src/wgpu_app/wgpu_app.cpp
    src/wgpu_app/file_utils.cpp
    src/wgpu_app/image_utils.cpp
    src/wgpu_app/obj_loader.cpp
)
target_include_directories(wgpu_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wgpu_app
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(wgpu_app PUBLIC math SDL3::SDL3 wgpu-native)

# ---------- executable ----------
add_executable(planetary-system-webgpu src/main.cpp)
target_link_libraries(planetary-system-webgpu PRIVATE math wgpu_app)
target_compile_definitions(planetary-system-webgpu PRIVATE
    RESOURCES_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)
set_target_properties(planetary-system-webgpu PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/planetary-system-webgpu"
)
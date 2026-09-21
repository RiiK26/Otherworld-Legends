cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/config.json" VERSION_JSON_CONTENT)
string(JSON PARSED_VERSION GET "${VERSION_JSON_CONTENT}" "version")

project(
    Otherworld-Legends
    VERSION ${PARSED_VERSION}
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release")

add_compile_definitions(
    PROJECT_VERSION=\"${PROJECT_VERSION}\"
    PROJECT_ROOT=\"${CMAKE_BINARY_DIR}/release\"
    NDEBUG # Disables asserts, hiding ImGui file paths
)
add_compile_options(-fpermissive -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections)

file(GLOB MINHOOK_SRC "src/Modules/MinHook/*.c" "src/Modules/MinHook/hde/*.c")
file(GLOB SRC_FILES "src/Cores/dll.cpp" "src/Cores/Scanner.cpp" "src/Modules/Hooks/*.cpp" "src/Modules/Menu/*.cpp" "src/Features/*/*.cpp")

add_library(Otherworld-Legends MODULE
    ${SRC_FILES}
    ${MINHOOK_SRC}
    src/Modules/ImGui/imgui.cpp
    src/Modules/ImGui/imgui_draw.cpp
    src/Modules/ImGui/imgui_widgets.cpp
    src/Modules/ImGui/imgui_tables.cpp
    src/Modules/ImGui/imgui_impl_win32.cpp
    src/Modules/ImGui/imgui_impl_dx11.cpp
    src/Modules/ImGui/imgui_impl_dx12.cpp
)

# Build the injector separately
add_executable(injector WIN32 src/Cores/injector.cpp)
set_target_properties(injector PROPERTIES SUFFIX ".exe")
target_link_options(injector PRIVATE "-static" "-static-libgcc" "-static-libstdc++")

target_include_directories(Otherworld-Legends PRIVATE
    src/Modules/ImGui
    src/Modules/MinHook
    src/Modules/MinHook/include
    src
    src/Modules
    src/Features
)

set_target_properties(Otherworld-Legends PROPERTIES PREFIX "" SUFFIX ".dll")
target_link_options(Otherworld-Legends PRIVATE "-static" "-static-libgcc" "-static-libstdc++")
target_link_libraries(Otherworld-Legends PRIVATE d3d11 d3d12 d3dcompiler dxgi gdi32 dwmapi)

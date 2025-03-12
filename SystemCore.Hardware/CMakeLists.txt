cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SystemCore.Hardware")

# Add include directories for DirectX SDK
include_directories("C:/Dev/Include")

# Add library directories for DirectX SDK
link_directories("C:/Dev/Lib/x64")

add_executable (SystemCore.Hardware "src/main.cpp" "src/system_monitor.cpp" "src/overlay.cpp" "src/optimizer.cpp" "src/network.cpp" "include/system_monitor.h")

# Link DirectX libraries
target_link_libraries(SystemCore.Hardware d3d9 d3d11 dxgi wbemuuid d3d12)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SystemCore.Hardware PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

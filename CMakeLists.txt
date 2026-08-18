# CMakeList.txt : CMake project for CMakeApp, include source and define
# project specific logic here.

# Tell CMake to use vcpkg
set(VCPKG_ROOT "C:/Users/Andrew/source/repos/vcpkg" CACHE PATH "Vcpkg root directory")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
    CACHE FILEPATH "Vcpkg toolchain file")

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64" ${CMAKE_PREFIX_PATH})

cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MediaPopUp")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Add source to this project's executable.
add_executable (MediaPopUp WIN32 "src/main.cpp" "src/main.h" 
                        "src/controller.h" "src/controller.cpp" 
                        "src/mainwindow.h" "src/mainwindow.cpp" 
                        "src/logic.h" "src/logic.cpp"
                        "src/mediaplayerworker.h" "src/mediaplayerworker.cpp"
                        "src/mainwindow.qrc")

find_package(cppwinrt CONFIG REQUIRED)
target_link_libraries(MediaPopUp PRIVATE Microsoft::CppWinRT WindowsApp.lib)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(MediaPopUp PRIVATE spdlog::spdlog)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(MediaPopUp PRIVATE nlohmann_json::nlohmann_json)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
target_link_libraries(MediaPopUp PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MediaPopUp PROPERTY CXX_STANDARD 20)
endif()

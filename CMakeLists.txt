project (AppQt)
cmake_minimum_required(VERSION 3.6)

FILE(GLOB_RECURSE SOURCES src/*.cpp)
FILE(GLOB_RECURSE RESOURCES data/*.qrc)

include_directories(${CMAKE_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_SOURCE_DIR}/src/minus)

if (WIN32)
  set(CMAKE_CXX_FLAGS "-std=c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

if (WIN32)
  set(QT5_MINGW "c:/Devel/Tools/Qt5/5.15.1/mingw81_64")
  set(CMAKE_PREFIX_PATH "${QT5_MINGW}/lib/cmake/Qt5")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(default_build_type "Debug")
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_AUTOMOC ON) # must be before add_executable()
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-Wall -Wextra) # must be before add_executable()
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCES})

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

# for Linux Ubuntu 22.04: sudo apt install qt6-base-dev libgl-dev
set(QTLIBLIST Core Widgets Gui) # Gui Network Test WebSockets OpenGL ...
find_package(Qt6 REQUIRED ${QTLIBLIST})
foreach(qtlib ${QTLIBLIST})
  target_link_libraries(${PROJECT_NAME} Qt6::${qtlib})
endforeach()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${BINARY_OUTPUT_DIR})

if (WIN32)
  set(CMAKE_CXX_STANDARD_LIBRARIES "-limagehlp") # SymInitialize()
  set (dllpath "")
  foreach(dll stdc++-6 gcc_s_seh-1 winpthread-1)
    set(dllpath ${dllpath} ${QT5_MINGW}/bin/lib${dll}.dll)
  endforeach()
  foreach(qtlib ${QTLIBLIST})
    set(dllpath ${dllpath} ${QT5_MINGW}/bin/Qt5${qtlib}.dll)
  endforeach()
  file(COPY ${dllpath} DESTINATION ${CMAKE_BINARY_DIR})
  install(FILES ${dllpath} DESTINATION ${CMAKE_INSTALL_PREFIX})

  set(QTPLUGINDIR plugins/platforms)
  set(QTPLUGIN ${QT5_MINGW}/${QTPLUGINDIR}/qwindows.dll)
  file(COPY ${QTPLUGIN} DESTINATION ${CMAKE_BINARY_DIR}/${QTPLUGINDIR})
  install(FILES ${QTPLUGIN} DESTINATION ${CMAKE_INSTALL_PREFIX}/${QTPLUGINDIR})

endif()

cmake_minimum_required (VERSION 3.6)
project (Shmup)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)

file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.c
)

file(GLOB TOOL_FILES
    ${CMAKE_SOURCE_DIR}/tool/*.h
    ${CMAKE_SOURCE_DIR}/tool/*.cpp
    ${CMAKE_SOURCE_DIR}/tool/imgui-app/*.cpp
    ${CMAKE_SOURCE_DIR}/tool/imgui-app/*.h
    ${CMAKE_SOURCE_DIR}/src/moviePlayback.*

)

set(SDL2_LIBRARIES "SDL2" "SDL2main")
set(SDL2_INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL2-2.0.12/include)
set(SDL2_LIBDIR ${CMAKE_CURRENT_SOURCE_DIR}/dep/SDL2-2.0.12/lib/x86)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/tool
  ${CMAKE_CURRENT_SOURCE_DIR}/tool/imgui-app
  ${SDL2_INCLUDE_DIRS})
  
link_directories(
  ${SDL2_LIBDIR}
)
  
SET(EXE_NAME "Shmup")
SET(TOOL_NAME "Tool")

ADD_EXECUTABLE(${EXE_NAME} ${SRC_FILES})
ADD_EXECUTABLE(${TOOL_NAME} ${TOOL_FILES})
TARGET_LINK_LIBRARIES(${EXE_NAME} ${SDL2_LIBRARIES})
TARGET_LINK_LIBRARIES(${TOOL_NAME})

if(WIN32)
add_definitions(-DWIN32)
add_definitions(-DSOKOL_D3D11)
endif()
if(APPLE)
add_definitions(-DSOKOL_METAL)
target_compile_options(${TOOL_NAME} PRIVATE -x objective-c++)
    target_link_libraries(${TOOL_NAME} PUBLIC
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal")
endif()

set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(${EXE_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")

set_target_properties(${TOOL_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${TOOL_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${TOOL_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
set_target_properties(${TOOL_NAME} PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties(${TOOL_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")


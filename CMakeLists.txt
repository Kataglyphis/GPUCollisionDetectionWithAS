cmake_minimum_required(VERSION 3.14)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(CollisionDetectionWithAS VERSION 1.0
                       DESCRIPTION "Epic Graphics"
                       LANGUAGES CXX)

add_subdirectory(ExternalLib)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


add_compile_definitions(SHADER_SRC_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader")
add_compile_definitions(SHADER_SPV_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/spv")
add_compile_definitions(SHADER_LOG_DIR="${PROJECT_BINARY_DIR}/../Resources/Shader/log")

# we depend on vulkan 
find_package(Vulkan REQUIRED)






FILE(GLOB DESCRIPTORS_FILTER            "include/framework/descriptors/*.h"                         "Src/framework/descriptors/*.cpp")
FILE(GLOB MEMORY_FILTER                 "include/framework/memory/*.h"                              "Src/framework/memory/*.cpp")
FILE(GLOB MODEL_FILTER                  "include/framework/model/*.h"                               "Src/framework/model/*.cpp")
FILE(GLOB PIPELINE_FILTER               "include/framework/pipeline/*.h"                            "Src/framework/pipeline/*.cpp")
FILE(GLOB UTILITY_FILTER                "include/framework/utility/*.h"                             "Src/framework/utility/*.cpp")
FILE(GLOB PHYSICS_FILTER                "include/framework/physics/*.h"                             "Src/framework/physics/*.cpp")
FILE(GLOB FRAMEWORK_FILTER              "include/framework/*.h"                                     "Src/framework/*.cpp")

FILE(GLOB RENDERER_FILTER               "include/renderer/*.h"                                      "Src/renderer/*.cpp")
FILE(GLOB RENDER_STAGES_FILTER          "include/renderer/render_stages/*.h"                        "Src/renderer/render_stages/*.cpp")
FILE(GLOB PARTICLE_FILTER               "include/particle/*.h"                                      "Src/particle/*.cpp")
FILE(GLOB VECTOR_FIELD_FILTER           "include/particle/vector_field/*.h"                         "Src/particle/vector_field/*.cpp")
FILE(GLOB MOVEMENT_STRATEGY_FILTER      "include/particle/vector_field/movement_strategies/*.h"     "Src/particle/vector_field/movement_strategies/*.cpp")
FILE(GLOB APP_FILTER                    "include/*.h"                                               "Src/*.cpp")

FILE(GLOB GUI_FILTER            "ExternalLib/IMGUI/*.h" 
                                "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"
                                "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h"
                                "ExternalLib/IMGUI/*.cpp"
                                "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
                                "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp"
                                )

source_group("framework/descriptors"                            FILES ${DESCRIPTORS_FILTER})
source_group("framework/memory"                                 FILES ${MEMORY_FILTER})
source_group("framework/model"                                  FILES ${MODEL_FILTER})
source_group("framework/pipeline"                               FILES ${PIPELINE_FILTER})
source_group("framework/utility"                                FILES ${UTILITY_FILTER})
source_group("framework/physics"                                FILES ${PHYSICS_FILTER})
source_group("framework"                                        FILES ${FRAMEWORK_FILTER})

source_group("app/particle"                                     FILES ${PARTICLE_FILTER})
source_group("app/particle/vector_field"                        FILES ${VECTOR_FIELD_FILTER})
source_group("app/particle/vector_field/movement_strategies"    FILES ${MOVEMENT_STRATEGY_FILTER})
source_group("app/renderer"                                     FILES ${RENDERER_FILTER})
source_group("app/renderer/render_stages"                       FILES ${RENDER_STAGES_FILTER})
source_group("app"                                              FILES ${APP_FILTER})

source_group("gui"                                              FILES ${GUI_FILTER})





#set all your sources accordingly
file(GLOB_RECURSE SOURCES 
            "Src/*.cpp" 
            "Src/*.c"
            "ExternalLib/IMGUI/imgui.cpp"
            "ExternalLib/IMGUI/imgui_demo.cpp"
            "ExternalLib/IMGUI/imgui_draw.cpp"
            "ExternalLib/IMGUI/imgui_tables.cpp"
            "ExternalLib/IMGUI/imgui_widgets.cpp"
            "ExternalLib/IMGUI/backends/imgui_impl_glfw.cpp"
            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.cpp")

file(GLOB_RECURSE HEADERS 
            "include/framework/descriptors/*.h"
            "include/framework/memory/*.h"
            "include/framework/model/*.h"
            "include/framework/pipeline/*.h"
            "include/framework/utility/*.h"
            "include/framework/physics/*.h"
            "include/framework/*.h"
            "include/renderer/*.h"
            "include/particle/*.h"
            "include/*.h"
            )

file(GLOB GUI_HEADERS 
            "ExternalLib/IMGUI/*.h"
            "ExternalLib/IMGUI/backends/imgui_impl_glfw.h"
            "ExternalLib/IMGUI/backends/imgui_impl_vulkan.h")

# add filters to the solution explorer for the shaders :))
file(GLOB SHADER_FILTER   
         ${PROJECT_BINARY_DIR}/../Resources/Shader/*.vert
         ${PROJECT_BINARY_DIR}/../Resources/Shader/*.frag
         ${PROJECT_BINARY_DIR}/../Resources/Shader/*.glsl
         ${PROJECT_BINARY_DIR}/../Resources/Shader/*.comp
	     ${PROJECT_BINARY_DIR}/../Resources/Shader/*.geom
	     ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rgen
	     ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rchit
	     ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rahit
	     ${PROJECT_BINARY_DIR}/../Resources/Shader/*.rmiss
         )

#source_group("Header" FILES ${HEADERS})
source_group("app/shaders" FILES ${SHADER_FILTER})

# add the executable
#add_executable(${PROJECT_NAME} ${SOURCES} ${SHADER_FILTER} ${HEADERS})


add_executable(${PROJECT_NAME}
${SHADER_FILTER}
${DESCRIPTORS_FILTER}
${MEMORY_FILTER}
${MODEL_FILTER}
${PIPELINE_FILTER}
${UTILITY_FILTER}
${FRAMEWORK_FILTER}
${RENDERER_FILTER}
${PARTICLE_FILTER}
${VECTOR_FIELD_FILTER}
${MOVEMENT_STRATEGY_FILTER}
${APP_FILTER}
${RENDER_STAGES_FILTER}
${GUI_FILTER}
${PHYSICS_FILTER}
)

if(UNIX)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC   ./include/framework/descriptors
                                                    ./include/framework/memory
                                                    ./include/framework/model
                                                    ./include/framework/pipeline
                                                    ./include/framework/utility
                                                    ./include/framework/physics
                                                    ./include/framework
                                                    ./include/renderer
                                                    ./include/renderer/render_stages
                                                    ./include/particle
                                                    ./include/particle/vector_field
                                                    ./include/particle/vector_field/movement_strategies
                                                    ./include/ )

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

# file(COPY Resources/Shader DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resources/Models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resources/Textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add_custom_command(
#   TARGET ${PROJECT_NAME} PRE_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#           ${CMAKE_SOURCE_DIR}/Resources/Shaders/*
#           ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)

# set all glfw dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ExternalLib/GLFW/glfw-3.3.4)

target_link_libraries(${PROJECT_NAME} glfw)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${GraphicsEngine_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GraphicsEngine_VERSION_MINOR}")

set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY 
                       ${PROJECT_BINARY_DIR})

set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                       ${PROJECT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan imgui stb glm tinyobjloader vma ktx csvwriter)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS}
                                      Threads::Threads)


if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
else()
endif()

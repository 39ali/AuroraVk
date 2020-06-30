cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

project(AURORAVK)

file (GLOB_RECURSE sources "src/*.cpp")

set(SOURCE   ${sources} )

add_executable(AURORAVK ${SOURCE})


if(APPLE)
set(_GLFW_COCOA TRUE)
add_definitions(-D_GLFW_COCOA)
file(GLOB VULK_DLYB "$ENV{VULKAN_SDK}/lib/libvulkan.*.dylib")
target_link_libraries(AURORAVK
"-framework Cocoa"
"-framework IOKit"
"-framework CoreVideo"
${VULK_DLYB}
)

elseif(WIN32)
set(_GLFW_WIN32 TRUE)
add_definitions(-D_GLFW_WIN32)
message("$ENV{VULKAN_SDK}")
target_link_libraries(AURORAVK 
$ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
endif()



#include glfw
add_subdirectory(vendor/glfw/src)


target_include_directories(AURORAVK PRIVATE ${PROJECT_SOURCE_DIR}/src 
 $ENV{VULKAN_SDK}/include)

#include glm
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/vendor/glm)


 target_link_libraries(AURORAVK
 glfw
)

 

project (glfw)




set(common_HEADERS internal.h mappings.h
                   "glfw_config.h.in"
                   "../include/GLFW/glfw3.h"
                   "../include/GLFW/glfw3native.h")
set(common_SOURCES context.c init.c input.c monitor.c vulkan.c window.c )

if (_GLFW_COCOA)
    set(glfw_HEADERS ${common_HEADERS} cocoa_platform.h cocoa_joystick.h cocoa_joystick.m 
                     posix_thread.h nsgl_context.h egl_context.h osmesa_context.h )
    set(glfw_SOURCES ${common_SOURCES} cocoa_init.m cocoa_joystick.m
                     cocoa_monitor.m cocoa_window.m cocoa_time.c posix_thread.c
                     nsgl_context.m egl_context.c osmesa_context.c )
elseif (_GLFW_WIN32)
    set(glfw_HEADERS ${common_HEADERS} win32_platform.h win32_joystick.h
                     wgl_context.h egl_context.h osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} win32_init.c win32_joystick.c
                     win32_monitor.c win32_time.c win32_thread.c win32_window.c
                     wgl_context.c egl_context.c osmesa_context.c)

endif()



add_library(${PROJECT_NAME} ${glfw_SOURCES})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/../include
)
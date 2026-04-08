project(opengl_renderer VERSION 1.0.0)

# Include directories
include_directories(include)
# If using GLM as header-only, add its path here too
# include_directories(deps/glm) 

find_package(OpenGL REQUIRED)

# Identify Source Files

set(SOURCES 
    main.cpp 
    src/glad.c
    src/Shader.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)


# Link Libraries
# OpenGL is provided by the OS; glfw is the windowing library
target_link_libraries(${PROJECT_NAME} WaveLogic glfw ${OPENGL_LIBRARIES})

set(CORE_SHADER_DIR "${CMAKE_SOURCE_DIR}/core/shaders")
set(EXAMPLE_SHADER_DIR "${CMAKE_SOURCE_DIR}/examples/opengl_renderer/shaders")

# Define example shaders
set(EXAMPLE_SHADERS 
    "${CORE_SHADER_DIR}/shader.vs"
    "${CORE_SHADER_DIR}/shader.fs"
    "${EXAMPLE_SHADER_DIR}/sphere.vs"
    "${EXAMPLE_SHADER_DIR}/sphere.fs"
)

# Copy shaders to the output directory after building
foreach(SHADER_FILE ${EXAMPLE_SHADERS})
    add_custom_command(
        TARGET opengl_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER_FILE}
        $<TARGET_FILE_DIR:opengl_renderer>
    )
endforeach()

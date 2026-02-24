cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures configuration fails if compiler lacks C++17 support
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(add_shader_header TARGET_NAME INPUT_SHADER OUTPUT_HEADER VARIABLE_NAME)
    set(TEMPLATE_FILE "${PROJECT_SOURCE_DIR}/template/include_str.h")
    set(GENERATOR_SCRIPT "${PROJECT_SOURCE_DIR}/template/generate.cmake")

    add_custom_command(
        OUTPUT "${OUTPUT_HEADER}"
        COMMAND ${CMAKE_COMMAND}
            -D INPUT_FILE=${INPUT_SHADER}
            -D TEMPLATE_FILE=${TEMPLATE_FILE}
            -D OUTPUT_FILE=${OUTPUT_HEADER}
            -D STRING_NAME=${VARIABLE_NAME}
            -P ${GENERATOR_SCRIPT}
        DEPENDS ${INPUT_SHADER} ${TEMPLATE_FILE} ${GENERATOR_SCRIPT}
        COMMENT "Generating header ${OUTPUT_HEADER} from ${INPUT_SHADER}"
        VERBATIM
    )
    add_custom_target(${TARGET_NAME}_gen DEPENDS "${OUTPUT_HEADER}")

    # 3. Create the interface library
    add_library(${TARGET_NAME} INTERFACE)
    
    # Ensure the generation happens BEFORE anything linking to this library builds
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_gen)

    # Export the include directory and the source file
    target_sources(${TARGET_NAME} INTERFACE "${OUTPUT_HEADER}")
    # Use the directory of the OUTPUT_HEADER, not just CMAKE_CURRENT_BINARY_DIR
    get_filename_component(HEADER_DIR "${OUTPUT_HEADER}" DIRECTORY)
    target_include_directories(${TARGET_NAME} INTERFACE "${HEADER_DIR}")
endfunction()

add_subdirectory(external)
add_subdirectory(src)


add_executable(game src/main.cpp)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX) # Level 4 warnings, treat as errors
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(SDL3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm CONFIG REQUIRED)
target_include_directories(game PRIVATE external/include/)
target_link_libraries(game PRIVATE SDL3::SDL3 PRIVATE glm::glm assimp::assimp external src)


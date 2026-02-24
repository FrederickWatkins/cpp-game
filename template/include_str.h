#pragma once

inline const char * @STRING_NAME@ =
    R"glsl(#version 330 core
@DEFINES@)glsl"
    R"glsl(@STRING_CONTENT@)glsl";
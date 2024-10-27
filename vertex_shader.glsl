#version 120

// each vertex has its own set of attribute data
// attributes are used to pass data from the application (C++ code) to the vertex shader
attribute vec2 aPos;       // Position of each vertex
attribute vec2 aTexCoord;  // Texture coordinates of each vertex

// each vertex has its own varying value
// varying is used to pass data from the vertex shader to the fragment shader
// when passed to the fragment shader, OpenGL will interpolate these coordinates across the entire surface of the shape
varying vec2 TexCoord;     // Pass this to the fragment shader

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0); // Position on the screen
    TexCoord = aTexCoord;               // Pass texture coordinates to the fragment shader
}
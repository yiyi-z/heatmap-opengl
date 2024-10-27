#version 120

// uniform stays constant, don't change between fragments
// it's used to pass non-changing data from C++ to shaders
uniform sampler2D heatmapTexture; // The texture containing the scalar field
varying vec2 TexCoord;            // Texture coordinates from the vertex shader

// Convert scalar value to color (heatmap: blue to red)
vec4 scalarToColor(float value) {
    return vec4(value, 0.0, 1.0 - value, 1.0); // Blue to red gradient
}

void main() {
    float scalarValue = texture2D(heatmapTexture, TexCoord).r; // Sample scalar value from texture
    gl_FragColor = scalarToColor(scalarValue); // Map scalar to color
}
/* 
    In this program, we write vertex and fragment shaders in GLSL. 
    The C++ program reads these files, creates the shaders, and links them into a shader program.
    
    The vertex shader expects vertex data, using the `aPos` and `aTexCoord` attributes from the C++ program. 
    We generate vertices and indices in C++ and store them on the GPU using buffers. By binding these buffers 
    to specific targets, OpenGL knows where to look for the data when rendering.

    During each render loop iteration, we tell OpenGL how to interpret the buffer data for the attributes, 
    allowing it to form the shape. After rasterizing the shape, the fragment shader colors each pixel. 
    It interpolates texture coordinates passed from the vertex shader to get the texture coordinate 
    for each pixel, not just the vertices.

    Textures are created in C++, uploaded to the GPU, and bound to targets so OpenGL can apply texture 
    operations. Each render loop ensures the fragment shader uses the `heatmapTexture` uniform stored in the GPU.
*/

#include <GL/glew.h>    // GLEW library manages OpenGL extensions
#include <GLFW/glfw3.h> // GLFW library helps us make windows
#include <string>
#include <fstream>      // For reading files, help us to read our shaders from separate files
#include <sstream>      // For working with file streams, help us to read our shaders from separate files
#include <iostream>     // For standard input/output
#include <cmath>


// Function to read shader code from a file
std::string readShaderFile (const char* filePath) {
    std::ifstream shaderFile;   // Declare an input file stream
    shaderFile.open(filePath);   // Open the file using the filePath

    // If we couldn't open the shaderFile, output the error to standard error stream 
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
    }

    // Takes the entire content of the shaderFile and streams it into the string stream object.
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf(); 
    shaderFile.close(); 

    // Convert stream content to string and return it
    return shaderStream.str();  
}


// Function to compile a shader from a file
// GLuint is a type defined in OpenGL. It is used to represent various OpenGL objects, such as shaders, textures, buffers, and programs. 
// GL_VERTEX_SHADER or GL_FRAGMENT_SHADER is specified by the type parameter.
GLuint compileShaderFromFile(GLenum type, const char* filePath) {
    std::string shaderCode = readShaderFile(filePath);  // Read the shader code from the file as a string
    const char* shaderSource = shaderCode.c_str();      // Covert the string to C-style string

    GLuint shader = glCreateShader(type);                // Tells OpenGL to create a new shader with certain type (vertex or fragment)
    glShaderSource(shader, 1, &shaderSource, NULL);      // Pass the actual shader code to OpenGL, so it knows what code to compile
    glCompileShader(shader);                             // Compile the shader

    // Check whether the shader is comiles correctly.
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog); // Get error message from OpenGL
        std::cerr << "Error compiling shader from file: " << filePath << "\n" << infoLog << std::endl;
    }
    return shader;  // return the compiled shader obejct
}


// Function to create a shader program using vertex and fragment shaders from files.
// A shader program takes both the vertex shader and the fragment shader and links them
// together into one program that the GPU can use.
GLuint createShaderProgram() {
    GLuint vertexShader = compileShaderFromFile(GL_VERTEX_SHADER, "vertex_shader.glsl");
    GLuint fragmentShader = compileShaderFromFile(GL_FRAGMENT_SHADER, "fragment_shader.glsl");

    GLuint shaderProgram = glCreateProgram();       // Create a new shader program
    glAttachShader(shaderProgram, vertexShader);    // Attach the vertex shader
    glAttachShader(shaderProgram, fragmentShader);  // Attach the fragment shader
    glLinkProgram(shaderProgram);                   // Link both shaders into one program

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Check if linking was successful
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); // If linking failed, get error message
        std::cerr << "Error linking program: " << infoLog << std::endl;
    }

    // We don't need the individual shaders anymore
    glDeleteShader(vertexShader); 
    glDeleteShader(fragmentShader);

    return shaderProgram; // Return the linked shader program
}


// Function to generate a 2D scalar field as a texture (our heatmap)
// We will create sample texture as shown in the python code example (effect of rings radiating from the center)
void generateHeatmapTexture(GLuint* texture, int width, int height) {
    // 1D array of floats to hold our scalars
    float* data = new float[width * height];

    // adjust frequency
    float scale = 30.0f;
    // Center of the texture
    float centerX = 0.5f;
    float centerY = 0.5f;
    // Loop over each pixel and calculate the scalar value using sin(distance from center)
    // to create the effect of rings radiating from the center
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Normalize x and y to the range [0, 1]
            float xNorm = (float)x / (float)width;
            float yNorm = (float)y / (float)height;
            // Calculate the distance of this point from the center of the texture
            float dist = sqrt(pow(xNorm - centerX, 2.0f) + pow(yNorm - centerY, 2.0f));
            // Apply the sine function to the distance
            float value = sin(scale * dist);
            // Normalize the sine result from range [-1, 1] to the range [0, 1]
            value = (value + 1.0f) / 2.0f;
            // Store the value in the texture data
            data[y * width + x] = value;
        }
    }

    // Generate a new texture id and stores it in *texture
    glGenTextures(1, texture);
    // binds the generated texture ID to the 2D texture target
    glBindTexture(GL_TEXTURE_2D, *texture);
    // Upload the data to the GPU 
    // as a single-channel texture (GL_RED) since we have scaler data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, data);

    // Define how the texture is applied when the texture coordinates fall outside the range [0, 1]
    // S (horizontal), T (vertical)
    // GL_CLAMP_TO_EDGE means we use edge texel color repeatedly
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Define how the texture is sampled when rendered at different resolutions or sizes
    // GL_LINEAR means we do bilinear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clean up dynamically allocated memory
    delete[] data;
}


int main() {
    // Initialize the GLFW system, which is responsible for creating the window and handling user input.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // Want to use OpenGL version 2.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Create a window where our OpenGL graphics will be displayed
    GLFWwindow* window = glfwCreateWindow(600, 600, "OpenGL Heatmap", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW, which gives us access to all the OpenGL functions we need
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Build and compile shader program
    GLuint shaderProgram = createShaderProgram();

    // Defining the shape to render
    // Set up vertex data and buffers and configure vertex attributes
    float vertices[] = {
        // Positions    // Texture Coords
        -1.0f,  1.0f,    0.0f, 1.0f,  // Top-left corner
        -1.0f, -1.0f,    0.0f, 0.0f,  // Bottom-left corner
         1.0f, -1.0f,    1.0f, 0.0f,  // Bottom-right corner
         1.0f,  1.0f,    1.0f, 1.0f   // Top-right corner
    };
    unsigned int indices[] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };

    // Generate and bind VBO and EBO
    // Vertex Buffer Object (VBO) sotres vertex data on the GPU
    // It's much faster to sotre data on the GPU than to send it from CPU every frame
    // Element Buffer Objects (EBO), similar to VBO, but for indeces
    // Generate Buffers
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // There are different buffer targets in OpenGL, each representing a specific use or category of data.
    // Targets help organize and optimize how data is stored and used on the GPU.
    // GL_ARRAY_BUFFER: Used for vertex attributes
    // GL_ELEMENT_ARRAY_BUFFER: Used for index data
    // “From now on, whenever I set up vertex attributes (like glVertexAttribPointer), the data will come from this buffer (VBO).”
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate a 256x256 scalar field
    GLuint heatmapTexture;
    generateHeatmapTexture(&heatmapTexture, 256, 256); 

    // Get attribute locations in the shader
    // For us to be able to refer to and link these per-vertex attributes
    // in the vertex shader, so we can pass data from c++ code to vertex shader
    GLuint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    GLuint texAttrib = glGetAttribLocation(shaderProgram, "aTexCoord");

    // get location of the uniform variable heatmapTexture from the shader program
    GLint uniformLocation = glGetUniformLocation(shaderProgram, "heatmapTexture");

    // Use the shader program
    glUseProgram(shaderProgram);

    // texture units allow you to use multiple textures at the same time
    // tell the shader to use texture unit 0 for "heatmapTexture"
    glUniform1i(uniformLocation, 0);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Activating a the texture unit, ensure that the operations we're going to perform
        // will affect the currently active unit
        glActiveTexture(GL_TEXTURE0);
        // bind the heatmap texture to the active texture unit 
        glBindTexture(GL_TEXTURE_2D, heatmapTexture);

        // Enable vertex attribute arrays
        // Tells OpenGL that we will provide data for this attribute
        glEnableVertexAttribArray(posAttrib);
        // tells OpenGL how to interpret the vertex data stored in the currently bound GL_ARRAY_BUFFER (VBO)
        // posAttrib: Which attribute in the shader to map this data to.
        // 2: The number of components per vertex (e.g., x and y for 2D positions)
        // GL_FLOAT: The data type
        // 4 * sizeof(float): The stride, which tells OpenGL how far apart consecutive vertices are in the buffer
        // (void*)0: no offset
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        // similarly for texAttrib
        glEnableVertexAttribArray(texAttrib);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        // Tell OpenGL to draw the actual shape (2 triangles, a quad)
        // 1. the primitive type to draw is GL_TRIANGLES
        // 2. number of indices to read from the index buffer: 6
        // 3. data type of the indices is GL_UNSIGNED_INT
        // 4. no offset
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // After finish using them for rendering
        // tell OpenGL not to expect or use data for these attributes
        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(texAttrib);

        // rendering happens in a double-buffered environment
        // the front buffer is the currently displayed buffer
        // the back buffer is where the next frame is draw
        // swap them shows the newly rendered frame
        glfwSwapBuffers(window);
        // checking if any user inputs or system events have occurred, processes them
        glfwPollEvents();
    }

    // Clean up resources
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &heatmapTexture);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
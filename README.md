# heatmap-opengl

This is a submission for Navier AI, demonstrating the creation of a heatmap using C++ and OpenGL.

## Requirements

To build and run the project, you need to install the following dependencies:
- **C++11 or higher**
- **GLFW**
- **GLAD**

## Build Instructions

To compile the program, use the following command:

```bash
g++ main.cpp -o heatmap -I/path/to/glad/include -I/path/to/glfw/include -L/path/to/glfw/lib -lglfw -ldl -framework OpenGL -std=c++11

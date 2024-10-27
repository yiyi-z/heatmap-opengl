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
```
**Please replace /path/to/glad and /path/to/glfw with the actual paths where you have installed GLAD and GLFW on your system.**

## Sample Window Output

![Heatmap Window Output](https://github.com/user-attachments/assets/cd02dc9e-ddaa-4c25-bc0a-20fad01fbffc)

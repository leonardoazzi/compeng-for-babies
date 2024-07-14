#ifndef _WINDOW_H
#define _WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include "callback.h"

void initializeGLFW();
void configureGLFW();
void createWindow(GLFWwindow* window);
void setCallbacks(GLFWwindow* window);
void printGPUinfo();

#endif // _WINDOW_H
#pragma once

#include "Camera.h"
#include "Cube.h"
#include "AnimationClip.h"
#include "Shader.h"
#include "core.h"


class Window {
public:
    // Window Properties
    static int width;
    static int height;
    static const char* windowTitle;

    // Objects to render
    static Cube* cube;

    //Objects to render
    static Skeleton* skeleton;

    //Objects to render
    static Skin* skin;

    //Object that control skeleton
    static AnimationClip* animation;

    // Shader Program
    static GLuint shaderProgram;

    // Act as Constructors and desctructors
    static bool initializeProgram();
    static bool initializeObjects();
    static bool initializeObjects(const char* file);
    static bool initializeObjects(const char* file1, const char* file2);
    static bool initializeObjects(const char* file1, const char* file2, const char* file3);
    static void cleanUp();

    // for the Window
    static GLFWwindow* createWindow(int width, int height);
    static void resizeCallback(GLFWwindow* window, int width, int height);

    // update and draw functions
    static void idleCallback();
    static void displayCallback(GLFWwindow*);

    // helper to reset the camera
    static void resetCamera();

    // callbacks - for interaction
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_callback(GLFWwindow* window, double currX, double currY);
};
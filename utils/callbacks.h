#pragma once 
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>

#include "ospcommon/math/vec.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ArcballCamera.h"

struct App
{
    ospcommon::math::vec2f preMousePos = ospcommon::math::vec2f(-1);
    bool isCameraChanged = false;
    ospcommon::math::vec2i fbSize;
    ArcballCamera camera; 
    bool isTimeStepChanged = false;
    bool isTransferFcnChanged = false;

    App(ospcommon::math::vec2i imgSize, ArcballCamera &camera) 
        : fbSize(imgSize), isCameraChanged(false), camera(camera)
    {}
};

void cursorPosCallback(GLFWwindow *window, double x, double y);
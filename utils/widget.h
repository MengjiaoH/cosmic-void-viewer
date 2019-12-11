#pragma once 

#include <iostream>
#include <mutex>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


// class Widget{
//     int preTimeStep = 1;
//     int currentTimeStep = 1;
//     int beginTimeStep = 1;
//     int endTimeStep = 1;
//     bool timeStepChanged = false;

//     // bool doUpdate{false}; // no initial update
//     // std::shared_ptr<tfn::tfn_widget::TransferFunctionWidget> widget;
//     std::mutex lock;

//     public:
//         Widget(int begin, int end);
//         void draw();
//         bool changed();
//         int getTimeStep();   
// };

class Widget{
    float preTimeStep = 1;
    float currentTimeStep = 1;
    float beginTimeStep = 1;
    float endTimeStep = 1;
    bool timeStepChanged = false;

    // bool doUpdate{false}; // no initial update
    // std::shared_ptr<tfn::tfn_widget::TransferFunctionWidget> widget;
    std::mutex lock;

    public:
        Widget(float begin, float end);
        void draw();
        bool changed();
        float getTimeStep();   
};
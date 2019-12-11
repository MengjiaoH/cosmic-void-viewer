#include "widget.h"

Widget::Widget(float begin, float end)
    :beginTimeStep{begin}, endTimeStep(end)
{
    currentTimeStep = begin;
}

void Widget::draw()
{
    ImGui::SliderFloat("Time step", &currentTimeStep, beginTimeStep, endTimeStep); 
    if(currentTimeStep != preTimeStep){
        timeStepChanged = true;
    }else{
        // std::cout << "current time step " << currentTimeStep << " and pre time step " << preTimeStep << std::endl; 
        timeStepChanged = false;
    }

    if(timeStepChanged && lock.try_lock()){
        // std::cout << "current time step " << currentTimeStep << " and pre time step " << preTimeStep << std::endl; 
        preTimeStep = currentTimeStep;
        lock.unlock();
    }
}

bool Widget::changed(){
    return timeStepChanged;
}

float Widget::getTimeStep(){
    return currentTimeStep;
}


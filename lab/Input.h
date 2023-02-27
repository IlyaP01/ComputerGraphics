#pragma once

#include "Camera.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <memory>

class Input
{
public:
     Input(HINSTANCE hinstance, HWND hwnd, std::shared_ptr<Camera> pCamera);
     void Process();
     ~Input();
private:
     std::shared_ptr<Camera> pCamera;
     IDirectInput8* directInput = nullptr;
     IDirectInputDevice8* mouse = nullptr;

     DIMOUSESTATE mouseState = {};
};


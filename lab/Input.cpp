#include "Input.h"

Input::Input(HINSTANCE hinstance, HWND hwnd, std::shared_ptr<Camera> pCamera) : pCamera(pCamera)
{
     DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
     directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
     mouse->SetDataFormat(&c_dfDIMouse);
     mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
     mouse->Acquire();
}

void Input::Process()
{
     DirectX::XMFLOAT3 move = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
     HRESULT result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
     if (FAILED(result)) 
     {
          if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
          {
               mouse->Acquire();
          }
     }
     else 
     {
          if (mouseState.rgbButtons[0] || mouseState.rgbButtons[1] || mouseState.rgbButtons[2] & 0x80)
          {
               move = DirectX::XMFLOAT3((float)mouseState.lX, (float)mouseState.lY, (float)mouseState.lZ);
          }
     }

     pCamera->MoveCamera(move.x / 100.0f, move.y / 100.0f, -move.z / 100.0f);
}

Input::~Input()
{
     if (mouse) 
     {
          mouse->Unacquire();
          mouse->Release();
          mouse = NULL;
     }

     if (directInput) 
     {
          directInput->Release();
          directInput = NULL;
     }
}

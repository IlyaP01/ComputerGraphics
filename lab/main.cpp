#ifndef UNICODE
#define UNICODE
#endif 

#include "Renderer.h"

#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
     // Register the window class.
     const wchar_t CLASS_NAME[] = L"Window Class";

     WNDCLASS wc = { };

     wc.lpfnWndProc = WindowProc;
     wc.hInstance = hInstance;
     wc.lpszClassName = CLASS_NAME;

     RegisterClass(&wc);

     // Create the window.

     HWND hwnd = CreateWindowEx(
          0,                              // Optional window styles.
          CLASS_NAME,                     // Window class
          L"lab1",                        // Window text
          WS_OVERLAPPEDWINDOW,            // Window style

          // Size and position
          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

          NULL,       // Parent window    
          NULL,       // Menu
          hInstance,  // Instance handle
          NULL        // Additional application data
     );

     if (hwnd == NULL)
     {
          return 0;
     }

     ShowWindow(hwnd, nCmdShow);

     MSG msg = { };
     HACCEL hAccelTable = LoadAccelerators(hInstance, L"");
     bool exit = false;
     while (!exit)
     {
          if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
          {
               if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
               {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
               }
               if (WM_QUIT == msg.message)
                    exit = true;
          }
          Renderer::GetInstance().Render();
     }

     return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     switch (uMsg)
     {
          case WM_CREATE:
               if (!Renderer::GetInstance().Init(hwnd))
               {
                    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
               }
               break;
          case WM_DESTROY:
               PostQuitMessage(0);
               return 0;

          case WM_PAINT:
          {
               PAINTSTRUCT ps;
               HDC hdc = BeginPaint(hwnd, &ps);
               EndPaint(hwnd, &ps);
          }
          case WM_SIZE:
          {
               RECT rc;
               GetClientRect(hwnd, &rc);
               Renderer::GetInstance().Resize(rc.right - rc.left, rc.bottom - rc.top);
          }
          return 0;
     }
     return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

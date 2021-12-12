// DelaunayDemo.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "D3D11Renderer.h"
#include "Demo.h"

// --------------------------------------------------------------

// Global Variables:
HINSTANCE gInstance;                                // current instance
HWND gWindow;

// --------------------------------------------------------------

WCHAR const* szTitle = L"QuickStartApp";
WCHAR const* szWindowClass = L"QuickStart";

// --------------------------------------------------------------

// Forward declarations of functions included in this code module:
ATOM                register_class(HINSTANCE hInstance);
BOOL                init_instance(HINSTANCE, int);
LRESULT CALLBACK    wnd_proc(HWND, UINT, WPARAM, LPARAM);

// --------------------------------------------------------------

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    register_class(hInstance);

    // Perform application initialization:
    if (!init_instance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    NRenderer::initialize(gWindow);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {   
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        NDemo::simulate();
        NDemo::render();
    }

    NRenderer::shutdown();

    return (int) msg.wParam;
}

// --------------------------------------------------------------

ATOM register_class(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = wnd_proc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = nullptr;

    return RegisterClassExW(&wcex);
}

// --------------------------------------------------------------

BOOL init_instance(HINSTANCE hInstance, int nCmdShow)
{
   gInstance = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   gWindow = hWnd;
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// --------------------------------------------------------------

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// --------------------------------------------------------------
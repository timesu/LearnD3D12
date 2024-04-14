#include "SingleTriangle.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_PAINT:
        //OnRender();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
    // Handle any messages the switch statement didn't.
    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool SingleTriangle::InitMainWindow()
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = mhAppInst;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"LearnSingleTriangle";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, 1280, 720 };

    hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"02.LearnSingleTriangle",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        mhAppInst,
        nullptr);

    ShowWindow(hwnd, SW_SHOW);
}

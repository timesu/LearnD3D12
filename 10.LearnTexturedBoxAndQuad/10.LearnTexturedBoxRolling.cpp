//The Linker shoulde be Win32 Type
//Check project configuration. Linker -> System -> SubSystem should be Windows ¨C Michael Nastenko May 3, 2016 at 3:25
//https://stackoverflow.com/questions/33400777/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-int-cde

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>
//#include "d3dx12.h"
#include "TexturedBoxRolling.h"



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

    TexturedBoxRolling texturedboxrolling(hInstance);
    texturedboxrolling.InitMainWindow();
    texturedboxrolling.LoadPipeLine();
    texturedboxrolling.LoadAssets();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


}

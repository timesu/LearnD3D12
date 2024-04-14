# LearnD3D12
 
Document Draft version 0.01

## Enivronment setting probelm FAQ list

1. 
Error: Error LNK2019 unresolved external symbol _main

Reason : Should create A winApp32 type project instead of Console or Empty 

Reference: https://stackoverflow.com/questions/33400777/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-int-cde

2. 
Error : Unresolved external symbol
Reason :Link the lib mannually to compile like  #pragma comment(lib, "dxgi.lib")

3. 
Error:error X3501: 'main': entrypoint not found

Reason: The visual studio 2022 intergraded a HLSL compiler.
        Exclude the shader file from the project to avoid the HLSL compiler compile the file,
        since we compile the shader with D3DCompile function by ourself.
Resource : https://stackoverflow.com/a/31088031/24316308

## Reference:
(English)Micsoft D3D12 sample : https://github.com/microsoft/DirectX-Graphics-Samples
(English)Introduction to Game Programming with D3D12 : https://github.com/d3dcoder/d3d12book
(Japanese)D3D's nest : https://sites.google.com/site/monshonosuana/directx%E3%81%AE%E8%A9%B1

## Introduction
01.LearnWindow : A simplefied version of D3D sample HelloWindow
02.LearnSingleTriangle : A simplefied version of D3D sample HelloTriangle.
                         Spliting LoadPipeLine and LoadAssets into more stage that just like what d3d12book did.

                         //what dxgi is : https://www.zhihu.com/question/36501678(chinese)
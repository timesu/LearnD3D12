#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <vector>
//#include "d3dx12.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class SingleTriangle
{
public:
	SingleTriangle(HINSTANCE hInstance);

public:
	static SingleTriangle* GetApp();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool InitMainWindow();

	void LoadPipeLine();
	void CreateDXGI();
	void CreateFence();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateRTV();

	void LoadAssets();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();

	void OnRender();
	void ResetCommandList();
	void SetCommandList();
	void DrawCommandList();
	void ExecuteCommandList();
	void WaitForPreviousFrame();



protected:


	static SingleTriangle* mApp;
	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr;

	//CD3DX12_VIEWPORT m_viewport(0.0f, 0.0f, 1280.0f, 720.0f, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);

	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-viewport
	//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_viewport
	//D3D12_VIEWPORT viewport;

	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/cd3dx12-rect
	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/d3d12-rect
	//CD3DX12_RECT m_scissorRect(0, 0, 1280, 720);
	//D3D12_RECT scissorRect;


	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	static const UINT FrameCount = 2;

	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize = 0;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects.
	UINT m_frameIndex = 0;;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

};
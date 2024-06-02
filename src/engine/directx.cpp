#include "directx.h"

#include "window.h"

#include <wrl/client.h>
using namespace Microsoft::WRL;

#include "../include/directx/d3dx12.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>

#include <dxgi.h>
#include <dxgi1_4.h>
#pragma comment(lib, "DXGI.lib")

#include "directxhelper.h"

namespace engine
{
	namespace directx
	{
		constexpr UINT FrameCount = 2;

		// pipeline components
		CD3DX12_VIEWPORT g_viewport;
		CD3DX12_RECT g_scissorRect;
		ComPtr<IDXGISwapChain3> g_swapChain;
		ComPtr<ID3D12Device> g_device;
		ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
		ComPtr<ID3D12CommandAllocator> g_commandAllocator;
		ComPtr<ID3D12CommandQueue> g_commandQueue;
		ComPtr<ID3D12RootSignature> g_rootSignature;
		ComPtr<ID3D12DescriptorHeap> g_rtvHeap;
		ComPtr<ID3D12PipelineState> g_pipelineState;
		ComPtr<ID3D12GraphicsCommandList> g_commandList;
		UINT g_rtvDescriptorSize;

		// App resources.
		ComPtr<ID3D12Resource> g_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

		// Synchronization objects.
		UINT g_frameIndex;
		HANDLE g_fenceEvent;
		ComPtr<ID3D12Fence> g_fence;
		UINT64 g_fenceValue;

		void Init()
		{
			InitializePipeline();
			InitializeAssets();
		}

		void InitializePipeline()
		{
			UINT dxgiFactoryFlags = 0;

			// init debug layer
#ifdef _DEBUG
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
#endif

			// create factory
			ComPtr<IDXGIFactory4> factory;
			DX::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

			// create device
			DX::ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device)));

			// command queue desc
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			//create command queue
			DX::ThrowIfFailed(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue)));

			// swapchain desc
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.BufferCount = FrameCount;
			swapChainDesc.Width = window::GetWindowRect().w;
			swapChainDesc.Height = window::GetWindowRect().h;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.SampleDesc.Count = 1;

			// create swapchain
			ComPtr<IDXGISwapChain1> swapChain;
			DX::ThrowIfFailed(factory->CreateSwapChainForHwnd(
				g_commandQueue.Get(),
				window::GetHwnd(),
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain
				));
			DX::ThrowIfFailed(swapChain.As(&g_swapChain));

			// get initial frameindex
			g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

			// create descriptor heaps
			{
				D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				rtvHeapDesc.NumDescriptors = FrameCount;
				rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				DX::ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvHeap)));

				g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}

			// create frame resource
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

				// create a rtv for each frame
				for (UINT n = 0; n < FrameCount; n++)
				{
					DX::ThrowIfFailed(g_swapChain->GetBuffer(n, IID_PPV_ARGS(&g_renderTargets[n])));
					g_device->CreateRenderTargetView(g_renderTargets[n].Get(), nullptr, rtvHandle);
					rtvHandle.Offset(1, g_rtvDescriptorSize);
				}
			}

			// create command allocator
			DX::ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));

			return;
		}

		void InitializeAssets()
		{

		}

		void Update()
		{

		}

		void Render()
		{
			PopulateCommandList();

			ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
			g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			DX::ThrowIfFailed(g_swapChain->Present(1, 0));

			WaitForPreviousFrame();
		}

		void Destroy()
		{
			WaitForPreviousFrame();

			CloseHandle(g_fenceEvent);
		}

		void PopulateCommandList()
		{
			// Command list allocators can only be reset when the associated 
			// command lists have finished execution on the GPU; apps should use 
			// fences to determine GPU execution progress.
			DX::ThrowIfFailed(g_commandAllocator->Reset());

			// However, when ExecuteCommandList() is called on a particular command 
			// list, that command list can then be reset at any time and must be before 
			// re-recording.
			DX::ThrowIfFailed(g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get()));

			// Set necessary state.
			g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());
			g_commandList->RSSetViewports(1, &g_viewport);
			g_commandList->RSSetScissorRects(1, &g_scissorRect);

			// Indicate that the back buffer will be used as a render target.
			g_commandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					g_renderTargets[g_frameIndex].Get(), 
					D3D12_RESOURCE_STATE_PRESENT, 
					D3D12_RESOURCE_STATE_RENDER_TARGET
				));

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);
			g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

			// Record commands.
			const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
			g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
			g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
			g_commandList->DrawInstanced(3, 1, 0, 0);
			//g_commandList->IASetIndexBuffer();
			//g_commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

			// Indicate that the back buffer will now be used to present.
			g_commandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					g_renderTargets[g_frameIndex].Get(), 
					D3D12_RESOURCE_STATE_RENDER_TARGET, 
					D3D12_RESOURCE_STATE_PRESENT
				));

			DX::ThrowIfFailed(g_commandList->Close());
		}

		void WaitForPreviousFrame()
		{
			// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
			// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
			// sample illustrates how to use fences for efficient resource usage and to
			// maximize GPU utilization.

			// Signal and increment the fence value.
			const UINT64 fence = g_fenceValue;
			DX::ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), fence));
			g_fenceValue++;

			if (g_fence->GetCompletedValue() < fence)
			{
				DX::ThrowIfFailed(g_fence->SetEventOnCompletion(fence, g_fenceEvent));
				WaitForSingleObject(g_fenceEvent, INFINITE);
			}

			g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
		}
	}
}
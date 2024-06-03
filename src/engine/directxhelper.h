#pragma once
#include <exception>
#include <winerror.h>

#include <DirectXMath.h>


#define EXPAND(x) x
#define SHADERPATH(shadername) SOLUTIONDIR ## "src\\engine\\shaders\\"###shadername##".hlsl"
#define SHADERPATHW(shadername) SOLUTIONDIRW "src\\engine\\shaders\\"###shadername##".hlsl"

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {

        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}
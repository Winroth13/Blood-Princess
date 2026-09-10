#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

struct ID3D11VertexBuffer;
struct ID3D11IndexBuffer;

struct GeometryCommand 
{	
    ID3D11VertexBuffer *vertexBuffer;
    ID3D11IndexBuffer *indexBuffer;
    uint32_t startIndex;
    uint32_t indexCount;
};

struct PointLightCommand 
{

};

struct SkyboxCommand 
{
	
};
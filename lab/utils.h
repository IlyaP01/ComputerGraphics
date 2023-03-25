#pragma once

#include <d3d11.h>
#include <windows.h>

#define SAFE_RELEASE(DXResource) do { if ((DXResource) != NULL) { (DXResource)->Release(); } } while (false);

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

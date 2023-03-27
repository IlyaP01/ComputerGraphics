#include "utils.h"
#include "D3DInclude.h"
#include <d3dcompiler.h>

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
     DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
     dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

     D3DInclude includeObject;
     ID3DBlob* pErrorBlob = NULL;
     auto hr = D3DCompileFromFile(
          szFileName,
          NULL,
          &includeObject,
          szEntryPoint,
          szShaderModel,
          dwShaderFlags,
          0,
          ppBlobOut,
          &pErrorBlob);
     if (FAILED(hr))
     {
          if (pErrorBlob)
          {
               OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
               pErrorBlob->Release();
          }
          return hr;
     }

     SAFE_RELEASE(pErrorBlob);
     return S_OK;
}
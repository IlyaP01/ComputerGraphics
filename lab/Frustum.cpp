#include "Frustum.h"

// Function to build frustum
void Frustum::ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix) 
{
     // Convert the projection matrix into a 4x4 float type.
     XMFLOAT4X4 pMatrix;
     XMStoreFloat4x4(&pMatrix, projectionMatrix);

     // Calculate the minimum Z distance in the frustum.
     float zMinimum = -pMatrix._43 / pMatrix._33;
     float r = screenDepth / (screenDepth - zMinimum);

     // Load the updated values back into the projection matrix.
     pMatrix._33 = r;
     pMatrix._43 = -r * zMinimum;
     projectionMatrix = XMLoadFloat4x4(&pMatrix);

     // Create the frustum matrix from the view matrix and updated projection matrix.
     XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

     // Convert the final matrix into a 4x4 float type.
     XMFLOAT4X4 matrix;
     XMStoreFloat4x4(&matrix, finalMatrix);

     // Calculate near plane of frustum.
     planes[0][0] = matrix._14 + matrix._13;
     planes[0][1] = matrix._24 + matrix._23;
     planes[0][2] = matrix._34 + matrix._33;
     planes[0][3] = matrix._44 + matrix._43;

     // Normalize the near plane.
     float length = sqrtf((planes[0][0] * planes[0][0]) + (planes[0][1] * planes[0][1]) + (planes[0][2] * planes[0][2]));
     planes[0][0] /= length;
     planes[0][1] /= length;
     planes[0][2] /= length;
     planes[0][3] /= length;

     // Calculate far plane of frustum.
     planes[1][0] = matrix._14 - matrix._13;
     planes[1][1] = matrix._24 - matrix._23;
     planes[1][2] = matrix._34 - matrix._33;
     planes[1][3] = matrix._44 - matrix._43;

     // Normalize the far plane.
     length = sqrtf((planes[1][0] * planes[1][0]) + (planes[1][1] * planes[1][1]) + (planes[1][2] * planes[1][2]));
     planes[1][0] /= length;
     planes[1][1] /= length;
     planes[1][2] /= length;
     planes[1][3] /= length;

     // Calculate left plane of frustum.
     planes[2][0] = matrix._14 + matrix._11;
     planes[2][1] = matrix._24 + matrix._21;
     planes[2][2] = matrix._34 + matrix._31;
     planes[2][3] = matrix._44 + matrix._41;

     // Normalize the left plane.
     length = sqrtf((planes[2][0] * planes[2][0]) + (planes[2][1] * planes[2][1]) + (planes[2][2] * planes[2][2]));
     planes[2][0] /= length;
     planes[2][1] /= length;
     planes[2][2] /= length;
     planes[2][3] /= length;

     // Calculate right plane of frustum.
     planes[3][0] = matrix._14 - matrix._11;
     planes[3][1] = matrix._24 - matrix._21;
     planes[3][2] = matrix._34 - matrix._31;
     planes[3][3] = matrix._44 - matrix._41;

     // Normalize the right plane.
     length = sqrtf((planes[3][0] * planes[3][0]) + (planes[3][1] * planes[3][1]) + (planes[3][2] * planes[3][2]));
     planes[3][0] /= length;
     planes[3][1] /= length;
     planes[3][2] /= length;
     planes[3][3] /= length;

     // Calculate top plane of frustum.
     planes[4][0] = matrix._14 - matrix._12;
     planes[4][1] = matrix._24 - matrix._22;
     planes[4][2] = matrix._34 - matrix._32;
     planes[4][3] = matrix._44 - matrix._42;

     // Normalize the top plane.
     length = sqrtf((planes[4][0] * planes[4][0]) + (planes[4][1] * planes[4][1]) + (planes[4][2] * planes[4][2]));
     planes[4][0] /= length;
     planes[4][1] /= length;
     planes[4][2] /= length;
     planes[4][3] /= length;

     // Calculate bottom plane of frustum.
     planes[5][0] = matrix._14 + matrix._12;
     planes[5][1] = matrix._24 + matrix._22;
     planes[5][2] = matrix._34 + matrix._32;
     planes[5][3] = matrix._44 + matrix._42;

     // Normalize the bottom plane.
     length = sqrtf((planes[5][0] * planes[5][0]) + (planes[5][1] * planes[5][1]) + (planes[5][2] * planes[5][2]));
     planes[5][0] /= length;
     planes[5][1] /= length;
     planes[5][2] /= length;
     planes[5][3] /= length;
}

bool Frustum::CheckRectangle(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth) 
{
     // Check if any of the 6 planes of the rectangle are inside the view frustum.
     for (int i = 0; i < 6; i++) 
     {
          float dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * minHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * minHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * minDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * minHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * minHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * minWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          dotProduct = ((planes[i][0] * maxWidth) + (planes[i][1] * maxHeight) + (planes[i][2] * maxDepth) + (planes[i][3] * 1.0f));
          if (dotProduct >= 0.0f) 
          {
               continue;
          }

          return false;
     }

     return true;
}
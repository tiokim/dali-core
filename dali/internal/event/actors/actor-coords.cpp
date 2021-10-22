/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dali/internal/event/actors/actor-coords.h>
#include <dali/internal/event/common/projection.h>

namespace Dali::Internal
{
bool ConvertScreenToLocal(
  const Matrix&   viewMatrix,
  const Matrix&   projectionMatrix,
  const Matrix&   worldMatrix,
  const Vector3&  currentSize,
  const Viewport& viewport,
  float&          localX,
  float&          localY,
  float           screenX,
  float           screenY)
{
  // Get the ModelView matrix
  Matrix modelView;
  Matrix::Multiply(modelView, worldMatrix, viewMatrix);

  // Calculate the inverted ModelViewProjection matrix; this will be used for 2 unprojects
  Matrix invertedMvp(false /*don't init*/);
  Matrix::Multiply(invertedMvp, modelView, projectionMatrix);
  bool success = invertedMvp.Invert();

  // Convert to GL coordinates
  Vector4 screenPos(screenX - static_cast<float>(viewport.x), static_cast<float>(viewport.height) - screenY - static_cast<float>(viewport.y), 0.f, 1.f);

  Vector4 nearPos;
  if(success)
  {
    success = Unproject(screenPos, invertedMvp, static_cast<float>(viewport.width), static_cast<float>(viewport.height), nearPos);
  }

  Vector4 farPos;
  if(success)
  {
    screenPos.z = 1.0f;
    success     = Unproject(screenPos, invertedMvp, static_cast<float>(viewport.width), static_cast<float>(viewport.height), farPos);
  }

  if(success)
  {
    Vector4 local;
    if(XyPlaneIntersect(nearPos, farPos, local))
    {
      Vector3 size = currentSize;
      localX       = local.x + size.x * 0.5f;
      localY       = local.y + size.y * 0.5f;
    }
    else
    {
      success = false;
    }
  }

  return success;
}

bool ConvertScreenToLocalRenderTask(
  const RenderTask& renderTask,
  const Matrix&     worldMatrix,
  const Vector3&    currentSize,
  float&            localX,
  float&            localY,
  float             screenX,
  float             screenY)
{
  bool         success = false;
  CameraActor* camera  = renderTask.GetCameraActor();
  if(camera)
  {
    Viewport viewport;
    renderTask.GetViewport(viewport);

    // need to translate coordinates to render tasks coordinate space
    Vector2 converted(screenX, screenY);
    if(renderTask.TranslateCoordinates(converted))
    {
      success = ConvertScreenToLocal(camera->GetViewMatrix(), camera->GetProjectionMatrix(), worldMatrix, currentSize, viewport, localX, localY, converted.x, converted.y);
    }
  }
  return success;
}

bool ConvertScreenToLocalRenderTaskList(
  const RenderTaskList& renderTaskList,
  const Matrix&         worldMatrix,
  const Vector3&        currentSize,
  float&                localX,
  float&                localY,
  float                 screenX,
  float                 screenY)
{
  // do a reverse traversal of all lists (as the default onscreen one is typically the last one)
  uint32_t taskCount = renderTaskList.GetTaskCount();
  for(uint32_t i = taskCount; i > 0; --i)
  {
    RenderTaskPtr task = renderTaskList.GetTask(i - 1);
    if(ConvertScreenToLocalRenderTask(*task, worldMatrix, currentSize, localX, localY, screenX, screenY))
    {
      // found a task where this conversion was ok so return
      return true;
    }
  }
  return false;
};

} // namespace Dali::Internal
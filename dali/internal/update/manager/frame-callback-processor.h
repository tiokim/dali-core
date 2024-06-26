#ifndef DALI_INTERNAL_SCENE_GRAPH_FRAME_CALLBACK_PROCESSOR_H
#define DALI_INTERNAL_SCENE_GRAPH_FRAME_CALLBACK_PROCESSOR_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
 *
 */

// EXTERNAL INCLUDES
#include <memory>
#if defined(LOW_SPEC_MEMORY_MANAGEMENT_ENABLED)
#include <dali/devel-api/common/map-wrapper.h>
#else
#include <unordered_map>
#endif

// INTERNAL INCLUDES
#include <dali/internal/common/buffer-index.h>
#include <dali/internal/common/owner-pointer.h>
#include <dali/internal/update/manager/scene-graph-frame-callback.h>
#include <dali/internal/update/manager/scene-graph-traveler.h>
#include <dali/internal/update/manager/update-proxy-impl.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{
class FrameCallbackInterface;

namespace Internal
{
namespace SceneGraph
{
class Node;
class TransformManager;
class UpdateManager;

/**
 * This class processes all the registered frame-callbacks.
 */
class FrameCallbackProcessor
{
public:
  /**
   * Construct a new FrameCallbackProcessor.
   * @param[in]  updateManager     A reference to the UpdateManager
   * @param[in]  transformManager  A reference to the TransformManager
   */
  FrameCallbackProcessor(UpdateManager& updateManager, TransformManager& transformManager);

  /**
   * Non-virtual Destructor.
   */
  ~FrameCallbackProcessor();

  // Movable but not copyable

  FrameCallbackProcessor(const FrameCallbackProcessor&) = delete;            ///< Deleted copy constructor.
  FrameCallbackProcessor(FrameCallbackProcessor&&)      = default;           ///< Default move constructor.
  FrameCallbackProcessor& operator=(const FrameCallbackProcessor&) = delete; ///< Deleted copy assignment operator.
  FrameCallbackProcessor& operator=(FrameCallbackProcessor&&) = delete;      ///< Deleted move assignment operator.

  /**
   * Adds an implementation of the FrameCallbackInterface.
   * @param[in]  frameCallback  An OwnerPointer to the SceneGraph FrameCallback object
   * @param[in]  rootNode       A pointer to the root node to apply the FrameCallback to. Or nullptr if given frame callback use globally.
   */
  void AddFrameCallback(OwnerPointer<FrameCallback>& frameCallback, const Node* rootNode);

  /**
   * Removes the specified implementation of FrameCallbackInterface.
   * @param[in]  frameCallback  A pointer to the implementation of the FrameCallbackInterface to remove.
   */
  void RemoveFrameCallback(FrameCallbackInterface* frameCallback);

  /**
   * Notify the specified implementation of FrameCallbackInterface.
   * @param[in]  frameCallback  A pointer to the implementation of the FrameCallbackInterface to notify.
   * @param[in] syncPoint The unique sync point to notify with
   */
  void NotifyFrameCallback(FrameCallbackInterface* frameCallback, Dali::UpdateProxy::NotifySyncPoint syncPoint);

  /**
   * Called on Update by the UpdateManager.
   * @param[in]  bufferIndex     The bufferIndex to use
   * @param[in]  elapsedSeconds  Time elapsed time since the last frame (in seconds)
   * @return Whether we should keep rendering.
   */
  bool Update(BufferIndex bufferIndex, float elapsedSeconds);

  /**
   * Called by the UpdateManager when the node hierarchy changes.
   */
  void NodeHierarchyChanged()
  {
    mNodeHierarchyChanged = true;
  }

private:
  SceneGraphTravelerPtr GetSceneGraphTraveler(Node* rootNode);

private:
  std::vector<OwnerPointer<FrameCallback> > mFrameCallbacks; ///< A container of all the frame-callbacks & accompanying update-proxies.

  UpdateManager& mUpdateManager;

  TransformManager& mTransformManager;

#if defined(LOW_SPEC_MEMORY_MANAGEMENT_ENABLED)
  using TravelerContainer = std::map<const Node*, SceneGraphTravelerPtr>;
#else
  using TravelerContainer = std::unordered_map<const Node*, SceneGraphTravelerPtr>;
#endif
  TravelerContainer mRootNodeTravelerMap;

  SceneGraphTravelerInterfacePtr mGlobalTraveler;

  bool mNodeHierarchyChanged; ///< Set to true if the node hierarchy changes
};

} // namespace SceneGraph

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SCENE_GRAPH_FRAME_CALLBACK_PROCESSOR_H

#ifndef DALI_INTERNAL_ANIMATION_PLAYLIST_H
#define DALI_INTERNAL_ANIMATION_PLAYLIST_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/common/set-wrapper.h>
#include <dali/integration-api/ordered-set.h>
#include <dali/internal/common/message.h>
#include <dali/internal/event/common/complete-notification-interface.h>
#include <dali/internal/event/common/scene-graph-notifier-interface-mapper.h>
#include <dali/public-api/animation/animation.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{
namespace Internal
{
namespace SceneGraph
{
class Animation;
}

class Animation;

/**
 * AnimationPlaylist provides notifications to applications when animations are finished.
 * It reference-counts playing animations, to allow "fire and forget" behaviour.
 */
class AnimationPlaylist : public CompleteNotificationInterface, public SceneGraphNotifierInterfaceMapper<Animation>
{
public:
  /**
   * Create an AnimationPlaylist.
   * @return A newly allocated animation playlist.
   */
  static AnimationPlaylist* New();

  /**
   * Virtual destructor.
   */
  ~AnimationPlaylist() override;

  /**
   * Called when an animation is constructed.
   */
  void AnimationCreated(Animation& animation);

  /**
   * Called when an animation is destroyed.
   */
  void AnimationDestroyed(Animation& animation);

  /**
   * Called when an animation is playing.
   * @post The animation will be referenced by AnimationPlaylist, until the "Finished" signal is emitted.
   */
  void OnPlay(Animation& animation);

  /**
   * Called when an animation is cleared.
   * @param[in] animation The animation that is cleared.
   * @param[in] ignoreRequired Whether to ignore the notify for current event loop, or not.
   * @post The animation will no longer be referenced by AnimationPlaylist.
   */
  void OnClear(Animation& animation, bool ignoreRequired);

  /**
   * Notify from core that current event loop finisehd.
   * It will clear all ignored animations at OnClear.
   */
  void EventLoopFinished();

  /**
   * @brief Notify that an animation has reached a progress marker
   * @param[in] notifyId scene graph animation's notifyId that has reached progress
   */
  void NotifyProgressReached(NotifierInterface::NotifyId notifyId);

  /**
   * @brief Retrive the number of Animations.
   *
   * @return The number of Animations.
   */
  uint32_t GetAnimationCount();

  /**
   * @brief Retrieve an Animation by index.
   *
   * @param[in] index The index of the Animation to retrieve
   * @return The Dali::Animation for the given index or empty handle
   */
  Dali::Animation GetAnimationAt(uint32_t index);

private:
  /**
   * Create an AnimationPlaylist.
   */
  AnimationPlaylist();

  // Undefined
  AnimationPlaylist(const AnimationPlaylist&);

  // Undefined
  AnimationPlaylist& operator=(const AnimationPlaylist& rhs);

private: // from CompleteNotificationInterface
  /**
   * @copydoc CompleteNotificationInterface::NotifyCompleted()
   */
  void NotifyCompleted(CompleteNotificationInterface::ParameterList notifierList) override;

private:
  Integration::OrderedSet<Animation, false> mAnimations;        ///< All existing animations (not owned)
  std::set<Dali::Animation>                 mPlaylist;          ///< The currently playing animations (owned through handle).
  std::set<NotifierInterface::NotifyId>     mIgnoredAnimations; ///< The currently cleard animations. We should not send notification at NotifyCompleted.
};

/**
 * Called when an animation reaches a progress marker
 *
 * Note animationPlaylist is of type CompleteNotificationInterface because of updateManager only knowing about the interface not actual playlist
 */
inline MessageBase* NotifyProgressReachedMessage(CompleteNotificationInterface& animationPlaylist, NotifierInterface::NotifyId notifyId)
{
  return new MessageValue1<AnimationPlaylist, NotifierInterface::NotifyId>(static_cast<AnimationPlaylist*>(&animationPlaylist), &AnimationPlaylist::NotifyProgressReached, notifyId);
}

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ANIMATION_PLAYLIST_H

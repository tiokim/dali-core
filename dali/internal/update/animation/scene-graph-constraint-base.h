#ifndef DALI_INTERNAL_SCENE_GRAPH_CONSTRAINT_BASE_H
#define DALI_INTERNAL_SCENE_GRAPH_CONSTRAINT_BASE_H

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
#include <dali/internal/common/message.h>
#include <dali/internal/event/common/event-thread-services.h>
#include <dali/internal/update/animation/scene-graph-constraint-declarations.h>
#include <dali/internal/update/common/animatable-property.h>
#include <dali/internal/update/common/property-owner.h>
#include <dali/internal/update/common/scene-graph-buffers.h>
#include <dali/public-api/animation/constraint.h>
#include <dali/public-api/common/dali-common.h>

namespace Dali
{
namespace Internal
{
// value types used by messages
template<>
struct ParameterType<Dali::Constraint::RemoveAction>
: public BasicType<Dali::Constraint::RemoveAction>
{
};

namespace SceneGraph
{
using PropertyOwnerContainer = Dali::Vector<PropertyOwner*>;
using PropertyOwnerIter      = PropertyOwnerContainer::Iterator;

/**
 * An abstract base class for Constraints.
 * This can be used to constrain a property of a scene-object, after animations have been applied.
 */
class ConstraintBase : public PropertyOwner::Observer
{
public:
  /**
   * Observer to determine when the constraint is no longer present
   */
  class LifecycleObserver
  {
  public:
    /**
     * Called shortly before the constraint is destroyed.
     */
    virtual void ObjectDestroyed() = 0;

  protected:
    /**
     * Virtual destructor, no deletion through this interface
     */
    virtual ~LifecycleObserver() = default;
  };

public:
  using RemoveAction = Dali::Constraint::RemoveAction;

  /**
   * Constructor
   * @param ownerContainer the properties to constraint
   * @oparam removeAction perform when removed
   */
  ConstraintBase(PropertyOwnerContainer& ownerContainer, RemoveAction removeAction);

  /**
   * Virtual destructor.
   */
  ~ConstraintBase() override;

  /**
   * Property resetter observes the lifecycle of this object
   */
  void AddLifecycleObserver(LifecycleObserver& observer)
  {
    mLifecycleObserver = &observer;
  }

  /**
   * Property resetter observers the lifecycle of this object
   */
  void RemoveLifecycleObserver(LifecycleObserver& observer)
  {
    mLifecycleObserver = nullptr;
  }

  /**
   * Initialize the constraint.
   * This should by called by a scene-object, when the constraint is connected.
   */
  void OnConnect()
  {
    StartObservation();

    mDisconnected = false;
  }

  /**
   * @copydoc Dali::Constraint::SetRemoveAction()
   */
  void SetRemoveAction(RemoveAction action)
  {
    mRemoveAction = action;
  }

  /**
   * @copydoc Dali::Constraint::GetRemoveAction()
   */
  RemoveAction GetRemoveAction() const
  {
    return mRemoveAction;
  }

  /**
   * Constrain the associated scene object.
   * @param[in] updateBufferIndex The current update buffer index.
   */
  virtual void Apply(BufferIndex updateBufferIndex) = 0;

  /**
   * Helper for internal test cases; only available for debug builds.
   * @return The current number of Constraint instances in existence.
   */
  static uint32_t GetCurrentInstanceCount();

  /**
   * Helper for internal test cases; only available for debug builds.
   * @return The total number of Constraint instances created during the Dali core lifetime.
   */
  static uint32_t GetTotalInstanceCount();

private:
  /**
   * Helper to start observing property owners
   */
  void StartObservation()
  {
    const PropertyOwnerIter end = mObservedOwners.End();
    for(PropertyOwnerIter iter = mObservedOwners.Begin(); end != iter; ++iter)
    {
      (*iter)->AddObserver(*this);
    }
  }

  /**
   * Helper to stop observing property owners
   */
  void StopObservation()
  {
    const PropertyOwnerIter end = mObservedOwners.End();
    for(PropertyOwnerIter iter = mObservedOwners.Begin(); end != iter; ++iter)
    {
      (*iter)->RemoveObserver(*this);
    }

    mObservedOwners.Clear();
  }

  /**
   * @copydoc PropertyOwner::Observer::PropertyOwnerConnected()
   */
  void PropertyOwnerConnected(PropertyOwner& owner) override
  {
  }

  /**
   * @copydoc PropertyOwner::Observer::PropertyOwnerDisconnected()
   */
  NotifyReturnType PropertyOwnerDisconnected(BufferIndex bufferIndex, PropertyOwner& owner) override
  {
    if(!mDisconnected)
    {
      // Call PropertyOwnerDestroyed(), for reduce duplicated code.
      ConstraintBase::PropertyOwnerDestroyed(owner);

      // Let we make owner Stop observing this.
      return NotifyReturnType::STOP_OBSERVING;
    }

    return NotifyReturnType::KEEP_OBSERVING;
  }

  /**
   * @copydoc PropertyOwner::Observer::PropertyOwnerDestroyed()
   */
  void PropertyOwnerDestroyed(PropertyOwner& owner) override
  {
    if(!mDisconnected)
    {
      // Discard pointer to destroyed property owner. Otherwise StopObservation() would crash when trying to remove
      //the constraint from the destroyed PropertyOwner's observers list
      PropertyOwnerIter iter = std::find(mObservedOwners.Begin(), mObservedOwners.End(), &owner);
      if(mObservedOwners.End() != iter)
      {
        mObservedOwners.Erase(iter);
      }

      // Stop observing the rest of property owners
      StopObservation();

      // Notification for derived class
      OnDisconnect();

      mDisconnected = true;
    }
  }

  /**
   * Notify the derived class to disconnect from property owners
   */
  virtual void OnDisconnect() = 0;

protected:
  RemoveAction mRemoveAction;

  bool mFirstApply : 1;
  bool mDisconnected : 1;

  PropertyOwnerContainer mObservedOwners; ///< A set of pointers to each observed object. Not owned.

private:
  LifecycleObserver* mLifecycleObserver; ///< Resetter observers this object

#ifdef DEBUG_ENABLED
  static uint32_t mCurrentInstanceCount; ///< The current number of Constraint instances in existence.
  static uint32_t mTotalInstanceCount;   ///< The total number of Constraint instances created during the Dali core lifetime.
#endif
};

// Messages for ConstraintBase

inline void SetRemoveActionMessage(EventThreadServices& eventThreadServices, const ConstraintBase& constraint, Dali::Constraint::RemoveAction removeAction)
{
  using LocalType = MessageValue1<ConstraintBase, Dali::Constraint::RemoveAction>;

  // Reserve some memory inside the message queue
  uint32_t* slot = eventThreadServices.ReserveMessageSlot(sizeof(LocalType));

  // Construct message in the message queue memory; note that delete should not be called on the return value
  new(slot) LocalType(&constraint, &ConstraintBase::SetRemoveAction, removeAction);
}

} // namespace SceneGraph

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SCENE_GRAPH_CONSTRAINT_BASE_H

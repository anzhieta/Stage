#pragma once

#ifndef STATICGEOMETRYCOMPONENT_H
#define STATICGEOMETRYCOMPONENT_H

#define STATICGEOMETRYCOMPONENT_ID 5

#include "PhysicsComponent.h"

namespace stage{
	class StaticGeometryComponent : public Component{
	public:

		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, float radius, Theron::Address transform, 
			Theron::Address collisionEventChannel) : Component(fw, owner), transform(transform), collisionEventChannel(collisionEventChannel){
			RegisterHandler(this, &StaticGeometryComponent::finishSphereSetup);
			uint64_t id = setup();
			tracker.setVariable<float>(id, 0, radius);
			Send(Transform::GetPosition(id), transform);
		}
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, glm::vec3 size, Theron::Address transform,
			Theron::Address collisionEventChannel) : Component(fw, owner), transform(transform), collisionEventChannel(collisionEventChannel){
			RegisterHandler(this, &StaticGeometryComponent::finishAABBSetup);
			uint64_t id = setup();
			tracker.setVariable<glm::vec3>(id, 0, size);
			Send(Transform::GetPosition(id), transform);
		}
				

		int id(){
			return STATICGEOMETRYCOMPONENT_ID;
		}

		~StaticGeometryComponent(){
			delete collider;
		}
	private:
		stage_common::Collider* collider;
		Theron::Address transform;
		Theron::Address collisionEventChannel;
		bool init = false;

		void update(const Update &up, Theron::Address from){
			if (!init){
				Send(AllDone(up.id), from);
				return;
			}
			uint64_t id = tracker.getNextID();
			tracker.addContext(up.id, id, from);
			Send(Transform::GetPosition(id), transform);
		}

		void finishUpdate(const Transform::Position& msg, Theron::Address from){
			collider->center = msg.position;
			tracker.decrement(msg.id);
		}

		void collisionCheck(const PhysicsComponent::CollisionCheck& msg, Theron::Address from){
			if (!init){
				Send(AllDone(msg.id), from);
				return;
			}
			if (!collider->checkCollision(msg.coll)) Send(AllDone(msg.id), from);
			else {
				uint64_t id = tracker.getNextID();
				tracker.addContext(msg.id, id, from);
				Send(PhysicsComponent::StaticCollision(id, *collider), msg.originator);
			}
		}

		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &StaticGeometryComponent::finishSphereSetup);
			RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
			collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
			init = true;
			tracker.decrement(msg.id);
		}
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &StaticGeometryComponent::finishAABBSetup);
			RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
			collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
			init = true;
			tracker.decrement(msg.id);
		}

		uint64_t setup(){
			RegisterHandler(this, &StaticGeometryComponent::collisionCheck);
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(0, id, Theron::Address::Null());
			context.finalize = [](){};
			context.error = context.finalize;
			
			Send(EventChannel<PhysicsComponent::CollisionCheck>::RegisterRecipient(this->GetAddress()), collisionEventChannel);
			return id;
		}

	};
}

#endif
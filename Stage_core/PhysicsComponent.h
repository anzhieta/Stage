#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#define PHYSICSCOMPONENT_ID 4

#include "stdafx.h"
#include <Component.h>
#include <Collisions.h>
#include <Collider.h>
#include <SphereCollider.h>
#include <AABBCollider.h>
#include <Transform.h>
#include <EventChannel.h>
#include <LogActor.h>
#include <unordered_set>



namespace stage{
	class PhysicsComponent : public Component{
	public:
		struct CollisionCheck : public Event{
			const stage_common::Collider& coll;
			const Theron::Address originator;
			float elapsedMS;
			CollisionCheck(uint64_t id, stage_common::Collider& coll, Theron::Address originator, float elapsedMS)
				: Event(id), coll(coll), originator(originator), elapsedMS(elapsedMS){}
		};

		struct CollisionDetected : public Event{
			const stage_common::Collider& coll;
			glm::vec3 otherVelocity;
			float otherMass;
			CollisionDetected(uint64_t id, stage_common::Collider& coll, glm::vec3 otherVelocity, float otherMass)
				: Event(id), coll(coll), otherVelocity(otherVelocity), otherMass(otherMass){}
		};

		struct StaticCollision : public Event{
			const stage_common::Collider& coll;
			StaticCollision(uint64_t id, stage_common::Collider& coll) : Event(id), coll(coll){}
		};

		struct FinishCollision : public Event{
			glm::vec3 velocityAdjustment;
			FinishCollision(uint64_t id, glm::vec3 velAdj) : Event(id), velocityAdjustment(velAdj){}
		};

		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform, 
			float radius, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), transform(transform), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){

			RegisterHandler(this, &PhysicsComponent::finishSphereSetup);
			uint64_t id = setup();
			tracker.setVariable<float>(id, 0, radius);
			Send(Transform::GetPosition(id), transform);
		}
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform, 
			glm::vec3 size, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), transform(transform), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){

			RegisterHandler(this, &PhysicsComponent::finishAABBSetup);
			uint64_t id = setup();
			tracker.setVariable<glm::vec3>(id, 0, size);
			Send(Transform::GetPosition(id), transform);
		}

		~PhysicsComponent(){
			delete collider;
			delete tempCollider;
		}

		virtual int id(){ return PHYSICSCOMPONENT_ID; }
	private:
		EventChannel<CollisionCheck>& collisionEventChannel;

		stage_common::Collider* collider;
		stage_common::Collider* tempCollider;
		Theron::Address transform;
		glm::vec3 velocity;
		glm::vec3 oldPos;
		float mass;
		bool updatedThisFrame = false;
		bool init = false;
		std::unordered_set<int> collidedThisFrame;

		uint64_t setup(){
			RegisterHandler(this, &PhysicsComponent::collisionCheck);
			RegisterHandler(this, &PhysicsComponent::collisionDetected);
			RegisterHandler(this, &PhysicsComponent::staticCollision);
			RegisterHandler(this, &PhysicsComponent::finishCollision);
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(0, id, Theron::Address::Null());
			context.finalize = [](){};
			context.error = context.finalize;
			
			Send(EventChannel<CollisionCheck>::RegisterRecipient(this->GetAddress()), collisionEventChannel.GetAddress());
			return id;
		}

		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &PhysicsComponent::finishSphereSetup);
			RegisterHandler(this, &PhysicsComponent::finishUpdate);
			collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
			oldPos = msg.position;
			init = true;
			tracker.decrement(msg.id);
		}
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &PhysicsComponent::finishAABBSetup);
			RegisterHandler(this, &PhysicsComponent::finishUpdate);
			collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
			oldPos = msg.position;
			init = true;
			tracker.decrement(msg.id);
		}

		void update(const Update &up, Theron::Address from){
			if (!init){
				Send(AllDone(up.id), from);
				return;
			}
			if (!updatedThisFrame) updatePosition(up.elapsedMS);
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(up.id, id, from, 0);
			context.finalize = [this, &context](){
				uint64_t id = tracker.getNextID();
				tracker.addContext(context.getOriginalID(), id, context.getOriginalSender());
				Send(Transform::GetPosition(id), transform);
			};
			CollisionCheck newmsg(id, *tempCollider, this->GetAddress(), up.elapsedMS);
			const std::list<Theron::Address>& recipients = collisionEventChannel.getRecipients();
			bool sent = false;
			for (std::list<Theron::Address>::const_iterator i = recipients.cbegin(); i != recipients.cend(); i++){
				if ((*i) != this->GetAddress()){
					tracker.trackedSend<CollisionCheck>(up.id, newmsg, *i, from);
					sent = true;
				}
			}
			if (!sent){
				Send(AllDone(up.id), from);
			}
		}

		void collisionCheck(const CollisionCheck& msg, Theron::Address from){
			if (!init){
				Send(AllDone(msg.id), from);
				return;
			}
			if (!updatedThisFrame) updatePosition(msg.elapsedMS);
			if (collidedThisFrame.count(msg.originator.AsInteger()) || !tempCollider->checkCollision(msg.coll)) Send(AllDone(msg.id), from);
			else {
				if (this->GetAddress().AsInteger() < msg.originator.AsInteger()) collidedThisFrame.insert(msg.originator.AsInteger());
				uint64_t id = tracker.getNextID();
				tracker.addContext(msg.id, id, from);
				Send(CollisionDetected(id, *tempCollider, velocity, mass), msg.originator);
			}
		}

		void collisionDetected(const CollisionDetected& msg, Theron::Address from){
			if (collidedThisFrame.count(from.AsInteger())) Send(AllDone(msg.id), from);
			else {
				if (this->GetAddress().AsInteger() < from.AsInteger()) collidedThisFrame.insert(from.AsInteger());
				glm::vec3 otherNewV = msg.otherVelocity;
				stage_common::Collisions::collisionVelocityChange(velocity, mass, otherNewV, msg.otherMass);
				stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
				Send(FinishCollision(msg.id, otherNewV - msg.otherVelocity), from);
			}
		}

		void staticCollision(const StaticCollision& msg, Theron::Address from){
			velocity = stage_common::Collisions::reflect(velocity, msg.coll.getCollisionNormal(*tempCollider, velocity));
			stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
			Send(AllDone(msg.id), from);
		}

		void finishCollision(const FinishCollision& msg, Theron::Address from){
			if (!tracker.contains(msg.id)){
				LOGERR("Warning: Unknown collision finish event received from " + std::to_string(from.AsInteger()));
				return;
			}
			velocity += msg.velocityAdjustment;
			tracker.decrement(msg.id);
		}


		void finishUpdate(const Transform::Position& msg, Theron::Address from){
			glm::vec3 translation = collider->center - oldPos;
			collider->center = oldPos = msg.position + translation;
			Send(Transform::Translate(msg.id, translation), transform);
		}
		
		void updatePosition(float elapsedMS){
			collider->center = oldPos + (velocity * elapsedMS);
			updatedThisFrame = true;
			delete tempCollider;
			tempCollider = collider->copy();
		}
		void render(const Render &rend, Theron::Address from){
			updatedThisFrame = false;
			collidedThisFrame.clear();
			Send(AllDone(rend.id), from);
		}
	};
}

#endif
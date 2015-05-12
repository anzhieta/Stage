#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#define MODEL_ID 3

#include "stdafx.h"
#include <Model.h>
#include <Component.h>
#include <Transform.h>
#include "GraphicsControlActor.h"
#include <iostream>

namespace stage{
	class ModelComponent : public Component{
	public:
		ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner) : Component(fw, owner), mod(mod){
			Send(GameObject::GetComponent(TRANSFORM_ID, this->GetAddress()), owner);
			RegisterHandler(this, &ModelComponent::initialize);
			RegisterHandler(this, &ModelComponent::completeRender);
		}

		virtual int id(){ return MODEL_ID; }
	private:
		bool init = false;
		stage_common::Model* mod;
		Theron::Address transform;
		void initialize(const Component::ComponentID& msg, Theron::Address sender){
			if (!msg.id == TRANSFORM_ID) return;
			transform = sender;
			DeregisterHandler(this, &ModelComponent::initialize);
			init = true;
		}
		void render(const Render& msg, Theron::Address sender){
			if (!init) return;
			uint64_t id = tracker.getNextID();
			tracker.addContext(msg.id, id, sender);
			Send(Transform::GetMatrix(id), transform);
		}
		void completeRender(const Transform::Matrix& msg, Theron::Address sender){
			if (!tracker.contains(msg.id)) return;
			Send(GraphicsControlActor::Queue(msg.id, mod, msg.matrix), GraphicsControlActor::getGlobalController());
		}
		
	};
}

#endif
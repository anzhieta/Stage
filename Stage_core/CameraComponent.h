#ifndef CAMERACOMPONENT_H
#define CAMERACOMPONENT_H

#define CAMERA_ID 2

#include "stdafx.h"
#include <Component.h>
#include <Camera.h>
#include <Transform.h>
#include <CoreEvents.h>
#include <iostream>
#include "GraphicsControlActor.h"

namespace stage{
	class CameraComponent : public Component{
	public:
		struct DrawCamera : public Event{
			DrawCamera(uint64_t id) : Event(id){}
		};
		struct SetViewMatrix : public Event{
			glm::mat4& view;
			SetViewMatrix(uint64_t id, glm::mat4& view) : Event(id), view(view){}
		};
		struct SetProjectionMatrix : public Event{
			glm::mat4& projection;
			SetProjectionMatrix(uint64_t id, glm::mat4& projection) : Event(id), projection(projection){}
		};
		CameraComponent(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner){
			RegisterHandler(this, &CameraComponent::initialize);
			RegisterHandler(this, &CameraComponent::completeRender);
			RegisterHandler(this, &CameraComponent::drawCamera);
			RegisterHandler(this, &CameraComponent::setViewMatrix);
			RegisterHandler(this, &CameraComponent::setProjectionMatrix);
			Send(GameObject::GetComponent(TRANSFORM_ID, this->GetAddress()), owner);
		}
		stage_common::Camera* getRawCamera(){ return &cam; }
	private:
		bool init = false;
		stage_common::Camera cam;
		Theron::Address transform;
		int id(){ return CAMERA_ID; }
		void initialize(const Component::ComponentID &msg, Theron::Address sender){
			if (msg.id != TRANSFORM_ID) return;
			DeregisterHandler(this, &CameraComponent::initialize);
			transform = sender;
			init = true;
		}
		virtual void render(const Render& msg, Theron::Address sender){
			if (!init){
				Send(AllDone(msg.id), sender);
				return;
			}
			uint64_t id = tracker.getNextID();
			tracker.addContext(msg.id, id, sender);
			Send(Transform::GetMatrix(id), transform);
		}
		virtual void completeRender(const Transform::Matrix& msg, Theron::Address sender){
			if (!tracker.contains(msg.id)) return;
			cam.setViewMatrix(msg.matrix);
			tracker.decrement(msg.id);
		}
		virtual void drawCamera(const DrawCamera& msg, Theron::Address sender){
			std::cout << "cam\n";
			uint64_t id = tracker.getNextID();
			tracker.addContext(msg.id, id, sender);
			Send(GraphicsControlActor::Draw(id, cam), GraphicsControlActor::getGlobalController());
		}
		virtual void setViewMatrix(const SetViewMatrix& msg, Theron::Address sender){
			cam.setViewMatrix(msg.view);
			Send(AllDone(msg.id), sender);
		}
		virtual void setProjectionMatrix(const SetProjectionMatrix& msg, Theron::Address sender){
			cam.setProjectionMatrix(msg.projection);
			Send(AllDone(msg.id), sender);
		}
	};
}

#endif
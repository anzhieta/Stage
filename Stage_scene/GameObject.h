#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "stdafx.h"
#include <Theron\Theron.h>
#include <string>
#include <list>
#include <CoreEvents.h>
#include <iostream>
#include <ContextTracker.h>

namespace stage{
	
	class Component;

	class GameObject : public Theron::Actor{
	public:
		struct AddComponent : Event {
			Component* component;
			AddComponent(Component* component, uint64_t id) : component(component), Event(id){}
		};
		struct GetComponent : Event{
			int compID;
			Theron::Address requester;
			GetComponent(uint64_t id, int compID, Theron::Address req) : Event(id), compID(compID), requester(req){}
		};
		struct ComponentFound : Event{
			Theron::Address component;
			ComponentFound(uint64_t id, Theron::Address component) : Event(id), component(component){}
		};

		GameObject(Theron::Framework &fw) : Theron::Actor(fw), tracker(fw, this->GetAddress()) {
			RegisterHandler(this, &GameObject::update);
			RegisterHandler(this, &GameObject::render);
			RegisterHandler(this, &GameObject::addComponent);
			RegisterHandler(this, &GameObject::getComponent);
			RegisterHandler(this, &GameObject::allDone);
			RegisterHandler(this, &GameObject::error);
			RegisterHandler(this, &GameObject::componentFound);
		}
		~GameObject();
	private:
		//int id;
		std::list<Component*> components;
		ContextTracker tracker;
		void update(const Update &msg, Theron::Address from);
		void render(const Render &msg, Theron::Address from);
		void addComponent(const AddComponent &msg, Theron::Address from);
		void getComponent(const GetComponent &msg, Theron::Address from);
		void componentFound(const ComponentFound &msg, Theron::Address from);
		void allDone(const AllDone &msg, Theron::Address from);
		void error(const Error &msg, Theron::Address from);
		
	};
}

#endif
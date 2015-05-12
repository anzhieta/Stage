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
		struct GetComponent{
			int id;
			Theron::Address requester;
			GetComponent(int id, Theron::Address req) : id(id), requester(req){}
		};

		GameObject(Theron::Framework &fw) : Theron::Actor(fw), tracker(fw, this->GetAddress()) {
			RegisterHandler(this, &GameObject::update);
			RegisterHandler(this, &GameObject::render);
			RegisterHandler(this, &GameObject::addComponent);
			RegisterHandler(this, &GameObject::getComponent);
			RegisterHandler(this, &GameObject::allDone);
			RegisterHandler(this, &GameObject::error);
		}
	private:
		//int id;
		std::list<Component*> components;
		ContextTracker tracker;
		void update(const Update &msg, Theron::Address from);
		void render(const Render &msg, Theron::Address from);
		void addComponent(const AddComponent &msg, Theron::Address from);
		void getComponent(const GetComponent &msg, Theron::Address from);
		void allDone(const AllDone &msg, Theron::Address from);
		void error(const Error &msg, Theron::Address from);
	};
}

#endif
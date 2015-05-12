#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "stdafx.h"
#include "GameObject.h"
#include <CoreEvents.h>
#include <ContextTracker.h>
#include <iostream>

namespace stage{
	
	class Component : public Theron::Actor{
	public:
		struct ComponentID{
			unsigned int id;
			ComponentID(int id): id(id){}
		};
		struct GetComponentID{
		};
		Component(Theron::Framework &fw, Theron::Address owner) : Theron::Actor(fw), owner(owner), tracker(fw, this->GetAddress()){
			RegisterHandler(this, &Component::update);
			RegisterHandler(this, &Component::render);
			RegisterHandler(this, &Component::isType);
			RegisterHandler(this, &Component::allDone);
			RegisterHandler(this, &Component::error);
			Send(GameObject::AddComponent(this, tracker.getNextID()), owner);
			RegisterHandler(this, &Component::getId);
			std::cout << "address " << this->GetAddress().AsInteger() << "\n";
		}
	protected:
		Theron::Address owner;
		ContextTracker tracker;
		virtual void update(const Update &up, Theron::Address from){
			Send(AllDone(up.id), from);
		}
		virtual void render(const Render &rend, Theron::Address from){
			Send(AllDone(rend.id), from);
		}

		virtual void isType(const GameObject::GetComponent &msg, Theron::Address from){
			if (msg.id == id())
			Send(ComponentID(id()), msg.requester);
		}
		virtual void getId(const GetComponentID &msg, Theron::Address from){
			Send(ComponentID(id()), from);
		}
		virtual void allDone(const AllDone& msg, Theron::Address from){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
		virtual void error(const Error& msg, Theron::Address from){
			if (tracker.contains(msg.id)){
				tracker.getContext(msg.id).error();
				tracker.remove(msg.id);
			}
		}
		virtual int id() = 0;
	};
}

#endif
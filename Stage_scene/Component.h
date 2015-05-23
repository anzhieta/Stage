#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "stdafx.h"
#include "GameObject.h"
#include <CoreEvents.h>
#include <ContextTracker.h>
#include <iostream>

namespace stage{
	
	/** Abstrakti pelimoottorikomponenttien yliluokka.
	Kaikki pelimoottoreihin liitettävät komponentit perivät tämän luokan.
	*/
	class Component : public Theron::Actor{
	public:
		struct ComponentID : Event{
			unsigned int compID;
			ComponentID(uint64_t id, int compID) : Event(id), compID(compID){}
		};
		struct GetComponentID : Event{
			GetComponentID(uint64_t id) : Event(id){}
		};

		/** Luo uuden pelimoottorikomponentin.
		HUOM: luo komponenttiolio aina new:llä äläkä tuhoa sitä itse.
		Komponenttiolio tuhotaan aina automaattisesti, kun sen omistava peliolio tuhotaan.
		*/
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
			if (msg.compID == id()){
				Send(GameObject::ComponentFound(msg.id, this->GetAddress()), from);
			}
			else Send(AllDone(msg.id), from);
		}
		virtual void getId(const GetComponentID &msg, Theron::Address from){
			Send(ComponentID(msg.id, id()), from);
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
#include "stdafx.h"
#include "GameObject.h"
#include "Component.h"
#include <iostream>

using namespace stage;
typedef std::list<Component*>::iterator comp_iterator;


void GameObject::addComponent(const AddComponent &msg, Theron::Address from){
	components.push_back(msg.component);
}
void GameObject::getComponent(const GetComponent &msg, Theron::Address from){
	for (comp_iterator i = components.begin(); i != components.end(); i++){
		Send(msg, (*i)->GetAddress());
	}
}
void GameObject::update(const Update &msg, Theron::Address from){
	if (components.size() == 0){
		Send(AllDone(msg.id), from);
	}
	else {
		uint64_t id = tracker.getNextID();
		for (comp_iterator i = components.begin();i != components.end(); i++){
			tracker.trackedSend<Update>(msg.id, Update(msg.elapsedMS, id), (*i)->GetAddress(), from);
		}
	}
	
}
void GameObject::render(const Render &msg, Theron::Address from){
	if (components.size() == 0){
		Send(AllDone(msg.id), from);
	}
	else {
		uint64_t id = tracker.getNextID();
		for (comp_iterator i = components.begin(); i != components.end(); i++){
			tracker.trackedSend<Render>(msg.id, Render(id), (*i)->GetAddress(), from);
		}
	}
}

void GameObject::allDone(const AllDone &msg, Theron::Address from){
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}

void GameObject::error(const Error &msg, Theron::Address from){
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}
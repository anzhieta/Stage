#include "stdafx.h"
#include "GameObject.h"
#include "Component.h"
#include <boost\any.hpp>
#include <iostream>
#include <LogActor.h>

using namespace stage;
typedef std::list<Component*>::iterator comp_iterator;

GameObject::GameObject(Theron::Framework &fw) : Theron::Actor(fw), tracker(fw, Destination(this->GetAddress(), INVALID_COMPONENT_ID)) {
	RegisterHandler(this, &GameObject::update);
	RegisterHandler(this, &GameObject::render);
	RegisterHandler(this, &GameObject::addComponent);
	//RegisterHandler(this, &GameObject::getComponent);
	RegisterHandler(this, &GameObject::allDone);
	RegisterHandler(this, &GameObject::error);
	//RegisterHandler(this, &GameObject::componentFound);
}

GameObject::~GameObject(){
	for (comp_iterator i = components.begin(); i != components.end(); i++){
		delete *i;
	}
}

void GameObject::addComponent(const AddComponent &msg, Theron::Address from){
	if (idmap.find(msg.component->id()) != idmap.end()){
		LOGERR("Could not add component " + msg.component->name() + "; game object already contains component with id " +
			std::to_string(msg.component->id()));
		Send(Error(msg.id, INVALID_COMPONENT_ID, msg.notifyDestination.component), msg.notifyDestination.address);
	}
	components.push_back(msg.component);
	idmap.emplace(msg.component->id(), msg.component);
	msg.component->initialize(this);
	Send(AllDone(msg.id, INVALID_COMPONENT_ID, msg.notifyDestination.component), msg.notifyDestination.address);
}

void GameObject::update(const Update &msg, Theron::Address from){
	if (components.size() == 0){
		Send(AllDone(msg.id, INVALID_COMPONENT_ID, INVALID_COMPONENT_ID), from);
	}
	else {
		uint64_t id = tracker.getNextID();
		tracker.addContext(msg.id, id, Destination(from, INVALID_COMPONENT_ID), components.size());
		for (comp_iterator i = components.begin();i != components.end(); i++){
			(*i)->update(msg.elapsedMS, id);
		}
	}
	
}
void GameObject::render(const Render &msg, Theron::Address from){
	if (components.size() == 0){
		Send(AllDone(msg.id, INVALID_COMPONENT_ID, INVALID_COMPONENT_ID), from);
	}
	else {
		uint64_t id = tracker.getNextID();
		tracker.addContext(msg.id, id, Destination(from, INVALID_COMPONENT_ID), components.size());
		for (comp_iterator i = components.begin(); i != components.end(); i++){
			(*i)->render(id);
		}
	}
}

void GameObject::allDone(const AllDone &msg, Theron::Address from){
	if (msg.receiverComponent == INVALID_COMPONENT_ID && tracker.contains(msg.id)) tracker.decrement(msg.id);
}

void GameObject::allDone(uint64_t id){
	if (tracker.contains(id)) tracker.decrement(id);
}

void GameObject::error(const Error &msg, Theron::Address from){
	LOGERR(std::string("Warning: object ") + from.AsString() + " reported error during processing");
	if (msg.receiverComponent == INVALID_COMPONENT_ID && tracker.contains(msg.id)) tracker.decrement(msg.id);
}

void GameObject::error(uint64_t id, const std::string& compname){
	LOGERR(std::string("Warning: component ") + compname + " reported error during processing");
	if (tracker.contains(id)) tracker.decrement(id);
}

Component* GameObject::getComponent(int id){
	for (comp_iterator i = components.begin(); i != components.end(); i++){
		if ((*i)->id() == id) return *i;
	}
	LOGERR("Error: game object does not contain component with ID " + id);
	abort();
}
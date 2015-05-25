#include "stdafx.h"
#include "GameObject.h"
#include "Component.h"
#include <boost\any.hpp>
#include <iostream>
#include <LogActor.h>

using namespace stage;
typedef std::list<Component*>::iterator comp_iterator;

GameObject::GameObject(Theron::Framework &fw) : Theron::Actor(fw), tracker(fw, this->GetAddress()) {
	RegisterHandler(this, &GameObject::update);
	RegisterHandler(this, &GameObject::render);
	RegisterHandler(this, &GameObject::addComponent);
	RegisterHandler(this, &GameObject::getComponent);
	RegisterHandler(this, &GameObject::allDone);
	RegisterHandler(this, &GameObject::error);
	RegisterHandler(this, &GameObject::componentFound);
}

GameObject::~GameObject(){
	for (comp_iterator i = components.begin(); i != components.end(); i++){
		delete *i;
	}
}

void GameObject::addComponent(const AddComponent &msg, Theron::Address from){
	components.push_back(msg.component);
}
void GameObject::getComponent(const GetComponent &msg, Theron::Address from){
	uint64_t id = tracker.getNextID();
	GetComponent newMsg(id, msg.compID);
	for (comp_iterator i = components.begin(); i != components.end(); i++){
		tracker.trackedSend<GetComponent>(msg.id, newMsg, (*i)->GetAddress(), from);
	}
	tracker.setVariable<bool>(id, 0, false);
	EventContext& ev = tracker.getContext(id);
	ev.finalize = [this, &ev](){
		bool result = boost::any_cast<bool>(ev.getVar(0));
		if (!result){
			this->GetFramework().Send(Error(ev.getOriginalID()), this->GetAddress(), ev.getOriginalSender());
		}
	};
}

void GameObject::componentFound(const ComponentFound &msg, Theron::Address from){
	if (tracker.contains(msg.id)){
		Send(ComponentFound(tracker.getContext(msg.id).getOriginalID(), msg.component), tracker.getContext(msg.id).getOriginalSender());
		tracker.setVariable<bool>(msg.id, 0, true);
		tracker.decrement(msg.id);
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
	LOGERR(std::string("Warning: component ") + from.AsString() + " reported error during processing");
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}
#include "stdafx.h"
#include "Component.h"

using namespace stage;

Component::Component(Theron::Framework &fw, Theron::Address owner) : Theron::Actor(fw), owner(owner), tracker(fw, this->GetAddress()){
	RegisterHandler(this, &Component::update);
	RegisterHandler(this, &Component::render);
	RegisterHandler(this, &Component::isType);
	RegisterHandler(this, &Component::allDone);
	RegisterHandler(this, &Component::error);
	Send(GameObject::AddComponent(this, tracker.getNextID()), owner);
	RegisterHandler(this, &Component::getId);
}
void Component::update(const Update &up, Theron::Address from){
	Send(AllDone(up.id), from);
}
void Component::render(const Render &rend, Theron::Address from){
	Send(AllDone(rend.id), from);
}
void Component::isType(const GameObject::GetComponent &msg, Theron::Address from){
	if (msg.compID == id()){
		Send(GameObject::ComponentFound(msg.id, this->GetAddress()), from);
	}
	else Send(AllDone(msg.id), from);
}
void Component::getId(const GetComponentID &msg, Theron::Address from){
	Send(ComponentID(msg.id, id()), from);
}
void Component::allDone(const AllDone& msg, Theron::Address from){
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}
void Component::error(const Error& msg, Theron::Address from){
	if (tracker.contains(msg.id)){
		tracker.getContext(msg.id).error();
		tracker.remove(msg.id);
	}
}
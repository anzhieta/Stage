#include "stdafx.h"
#include "Component.h"

using namespace stage;

Component::Component(Theron::Framework &fw, Theron::Address owner) : tracker(fw, Destination(owner, INVALID_COMPONENT_ID)){	
}

void Component::initialize(GameObject* owner){
	this->owner = owner;
	RegisterHandler<Component, AllDone, &Component::allDone>();
	RegisterHandler<Component, Error, &Component::error>();
}

void Component::update(float elapsedMS, uint64_t id){
	owner->allDone(id);
}

void Component::render(uint64_t id){
	owner->allDone(id);
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

void Component::finishPhase(uint64_t id){
	owner->allDone(id);
}

void Component::abortPhase(uint64_t id){
	owner->error(id, name());
}

uint64_t Component::createActorContext(uint64_t oldid, Destination origSender, int responseCount){
	uint64_t newid = tracker.getNextID();
	EventContext& context = tracker.addContext(oldid, newid, origSender, responseCount);
	context.finalize = [this, &context](){
		Send(AllDone(context.getOriginalID(), id(), context.getOriginalSender().component), context.getOriginalSender().address);
	};
	context.error = [this, &context](){
		Send(Error(context.getOriginalID(), id(), context.getOriginalSender().component), context.getOriginalSender().address);
	};
	return newid;
}

uint64_t Component::createStandardContext(uint64_t oldid, int responseCount){
	uint64_t newid = tracker.getNextID();
	EventContext& context = tracker.addContext(oldid, newid, Destination(Theron::Address::Null(), INVALID_COMPONENT_ID), responseCount);
	context.finalize = [this, &context](){
		this->finishPhase(context.getOriginalID());
	};
	context.error = [this, &context](){
		this->abortPhase(context.getOriginalID());
	};
	return newid;
}
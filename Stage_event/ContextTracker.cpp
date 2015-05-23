#include "stdafx.h"
#include "ContextTracker.h"

using namespace stage;

EventContext& ContextTracker::addContext(uint64_t oldID, uint64_t newID, Theron::Address originalSender, int responseCount){
	pending[newID] = EventContext(oldID, originalSender, responseCount);
	EventContext& context = pending[newID];
	//Asetetaan oletuskäsittelijä kontekstin loppuunsuorittamiselle: palautetaan AllDone alkuperäiselle lähettäjälle
	context.finalize = [this, oldID, context](){
		this->fw.Send(AllDone(oldID), this->owner, context.getOriginalSender());
	};
	context.error = context.finalize;
	return context;
}

EventContext& ContextTracker::getContext(uint64_t id){
	return pending[id];
}

void ContextTracker::increment(uint64_t id){
	EventContext& context = pending[id];
	context.responseCount++;
}

void ContextTracker::setResponseCount(uint64_t id, unsigned int count){
	EventContext& context = pending[id];
	context.responseCount = count;
}

void ContextTracker::decrement(uint64_t id){
	EventContext& context = pending[id];
	context.responseCount--;
	if (context.responseCount < 1){
		context.finalize();
		pending.erase(id);
	}
}

void ContextTracker::remove(uint64_t id){
	pending.erase(id);
}

uint64_t ContextTracker::getNextID(){
	lastID++;
	return Event::generateID(owner, lastID);
}

bool ContextTracker::contains(uint64_t id){
	if (pending.count(id)) return true;
	return false;
}
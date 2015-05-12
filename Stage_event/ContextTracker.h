#pragma once

#ifndef CONTEXTTRACKER_H
#define CONTEXTTRACKER_H

#include "stdafx.h"
#include "EventContext.h"
#include "CoreEvents.h"
#include <unordered_map>

namespace stage{
	class ContextTracker{
	public:
		ContextTracker(Theron::Framework& fw, Theron::Address owner) : fw(fw), owner(owner), pending(){
		}
		EventContext& addContext(uint64_t oldID, uint64_t newID, Theron::Address tracker, int responseCount = 1){
			pending[newID] = EventContext(oldID, tracker, responseCount);
			EventContext& context = pending[newID];
			context.finalize = [this, oldID, context](){
				this->fw.Send(AllDone(oldID), this->owner, context.getOriginalSender());
			};
			context.error = context.finalize;
			return context;
		}
		EventContext& getContext(uint64_t id){
			return pending[id];
		}
		void increment(uint64_t id){
			EventContext& context = pending[id];
			context.responseCount++;
		}
		void setResponseCount(uint64_t id, unsigned int count){
			EventContext& context = pending[id];
			context.responseCount = count;
		}
		void decrement(uint64_t id){
			EventContext& context = pending[id];
			context.responseCount--;
			if (context.responseCount < 1){
				context.finalize();
				pending.erase(id);
			}
		}
		void remove(uint64_t id){
			pending.erase(id);
		}
		template <class MessageType>
		void trackedSend(uint64_t evID, const MessageType& ev, Theron::Address recipient, Theron::Address tracker){
			if (!contains(ev.id)){
				addContext(evID, ev.id, tracker, 0);
			}
			pending[ev.id].responseCount++;
			fw.Send(ev, owner, recipient);
		}
		uint64_t getNextID(){
			nextId++;
			return Event::generateID(owner, nextId);
		}
		bool contains(uint64_t id){
			if (pending.count(id)) return true;
			return false;
		}
	private:
		std::unordered_map<uint64_t, EventContext> pending;
		uint32_t nextId = 0;
		Theron::Framework& fw;
		Theron::Address owner;
	};
}

#endif
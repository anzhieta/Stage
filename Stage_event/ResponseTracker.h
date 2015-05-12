#ifndef RESPONSETRACKER_H
#define RESPONSETRACKER_H

#include "stdafx.h"
#include <unordered_map>
#include <utility>
#include "CoreEvents.h"
#include <iostream>

namespace stage{
	
	
	class ResponseTracker{
	public:
		ResponseTracker(Theron::Framework& fw, Theron::Address owner): fw(fw), owner(owner){
		}
		template <class MessageType>
		void trackedSend(const MessageType& ev, Theron::Address recipient, Theron::Address tracker){
			//std::cout << recipient.AsInteger();
			pending[ev.id] = std::pair<Theron::Address, int>(tracker, pending[ev.id].second + 1);
			fw.Send(ev, owner, recipient);
		}
		uint32_t getNextID(){
			nextId++;
			return nextId;
		}
		void track(const AllDone& msg, const Theron::Address from){
			if (!pending.count(msg.id)) return;
			int remaining = pending[msg.id].second;
			remaining--;
			if (remaining < 1){
				fw.Send(msg, owner, pending[msg.id].first);
				pending.erase(msg.id);
			}
			else {
				pending[msg.id] = std::pair<Theron::Address, int>(pending[msg.id].first, remaining);
			}
		}
	protected:
		std::unordered_map<uint64_t, std::pair<Theron::Address, int>> pending;
		uint32_t nextId = 0;
		Theron::Framework& fw;
		Theron::Address owner;
	};
}


#endif
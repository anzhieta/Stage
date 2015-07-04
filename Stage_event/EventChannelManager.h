#ifndef EVENTCHANNELMANAGER_H
#define EVENTCHANNELMANAGER_H

#include "stdafx.h"
#include "ContextTracker.h"
#include "CoreEvents.h"

namespace stage{
	class EventChannelManager : public Theron::Actor{
	public:
		struct ChannelMaintenance: public Event{
			ChannelMaintenance(uint64_t id): Event(id){}
		};

		EventChannelManager(Theron::Framework& fw): Theron::Actor(fw), tracker(fw, this->GetAddress()){
			RegisterHandler(this, &EventChannelManager::channelMaintenance);
			RegisterHandler(this, &EventChannelManager::allDone);
		}

		void addChannel(Theron::Address channel){
			channels.push_back(channel);
		}
	private:
		std::list<Theron::Address> channels;
		ContextTracker tracker;

		void channelMaintenance(const ChannelMaintenance& msg, Theron::Address sender){
			if (channels.size() == 0) Send(AllDone(msg.id), sender);
			ChannelMaintenance newmsg(tracker.getNextID());
			for (std::list<Theron::Address>::const_iterator it = channels.begin(); it != channels.end(); it++){
				tracker.trackedSend(msg.id, newmsg, *it, sender);
			}
		}

		void allDone(const AllDone& msg, Theron::Address sender){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
	};
}

#endif
#pragma once
#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include "stdafx.h"
#include <list>
#include "CoreEvents.h"
#include <iostream>
#include "LogActor.h"

namespace stage {

	//TODO threadsafe register/deregister
		
	template <class MessageType>
	class EventChannel : public Theron::Actor{
	public:
		struct RegisterRecipient {
			RegisterRecipient(Theron::Address &rec) : recipient(rec){}
			Theron::Address recipient;
		};
		struct DeregisterRecipient {
			DeregisterRecipient(Theron::Address &rec) : recipient(rec){}
			Theron::Address recipient;
		};
		EventChannel(Theron::Framework& fw) : Theron::Actor(fw), fw(fw), recipients(), tracker(fw, this->GetAddress()){
			RegisterHandler(this, &EventChannel<MessageType>::forward<MessageType>);
			RegisterHandler(this, &EventChannel<MessageType>::registerRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::deregisterRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::allDone);
			RegisterHandler(this, &EventChannel<MessageType>::error);
		}
		const std::list<Theron::Address>& getRecipients(){
			return recipients;
		}
		
	private:
		std::list<Theron::Address> recipients;
		ContextTracker tracker;
		Theron::Framework& fw;

		template <class MessageType>
		void forward(const MessageType &msg, const Theron::Address from){
			int sent = 0;
			for (std::list<Theron::Address>::const_iterator it = recipients.begin(); it != recipients.end(); it++){
				if (*it != from){
					tracker.trackedSend(msg.id, msg, *it, from);
					sent++;
				}
			}
			if (sent == 0){
				fw.Send(AllDone(msg.id), this->GetAddress(), from);
			}
		}
		void registerRecipient(const RegisterRecipient& msg, const Theron::Address from){
			recipients.push_back(msg.recipient);
		}
		void deregisterRecipient(const DeregisterRecipient& msg, const Theron::Address from){
			for (std::list<Theron::Address>::const_iterator it = recipients.begin(); it != recipients.end(); it++){
				if (*it == msg.recipient) recipients.erase(it);
			}
		}
		void allDone(const AllDone& msg, const Theron::Address from){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}

		void error(const Error &msg, Theron::Address from){
			LOGERR(std::string("Warning: component ") + from.AsString() + " reported error during processing");
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
	};
}

#endif
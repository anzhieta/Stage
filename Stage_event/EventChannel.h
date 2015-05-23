#pragma once
#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include "stdafx.h"
#include <list>
#include "CoreEvents.h"
#include <iostream>

namespace stage {

	/** --DEPRECATED--
	TODO: kontekstimalliin sopiva versio
	*/

#define REGISTER_CHANNEL(TYPE) \
	RegisterHandler(this, &EventChannel<TYPE>::registerRecipient);\
	RegisterHandler(this, &EventChannel<TYPE>::deregisterRecipient);\
	RegisterHandler(this, &EventChannel<TYPE>::forward);\
	RegisterHandler(this, &EventChannel<TYPE>::track);
	
	template <class MessageType>
	class EventChannel : public ResponseTracker{
	public:
		struct RegisterRecipient {
			RegisterRecipient(Theron::Address &rec) : recipient(rec){}
			Theron::Address recipient;
		};
		struct DeregisterRecipient {
			DeregisterRecipient(Theron::Address &rec) : recipient(rec){}
			Theron::Address recipient;
		};
		EventChannel(Theron::Framework& fw, Theron::Address owner) : ResponseTracker(fw, owner), recipients(){}
		/*EventChannel(Theron::Framework &fw) : ResponseTracker(fw), recipients(){
			RegisterHandler(this, &EventChannel<MessageType>::registerRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::deregisterRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::forward);
		}*/
		void registerRecipient(Theron::Address recipient){
			recipients.push_back(recipient);
		}
		void deregisterRecipient(Theron::Address recipient){
			for (std::list<Theron::Address>::const_iterator it = recipients.begin(); it != recipients.end(); it++){
				if (*it == recipient) recipients.erase(it);
			}
		}
		template <class MessageType>
		void forward(const MessageType &msg, const Theron::Address from){
			std::cout << "channel";
			if (recipients.size() == 0) fw.Send(AllDone(msg.id), owner, from);
			for (std::list<Theron::Address>::const_iterator it = recipients.begin(); it != recipients.end(); it++){
				trackedSend(msg, *it, from);
			}
		}
	private:
		std::list<Theron::Address> recipients;

		
	};
}

#endif
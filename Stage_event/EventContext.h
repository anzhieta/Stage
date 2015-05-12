#ifndef EVENTCONTEXT_H
#define EVENTCONTEXT_H

#include "stdafx.h"
#include <functional>
#include <iostream>

namespace stage{
	class EventContext{
	public:
		unsigned int responseCount;
		std::function<void()> finalize;
		std::function<void()> error;

		EventContext(uint64_t id, Theron::Address sender, unsigned int responseCount): originalID(id), originalSender(sender), 
			responseCount(responseCount){

		}
		EventContext() : originalID(0), originalSender(0), responseCount(0){
			finalize = [](){abort(); };
			error = finalize;
		}
		uint64_t getOriginalID() const { return originalID; }
		Theron::Address getOriginalSender() const { return originalSender; }
	private:
		uint64_t originalID;
		Theron::Address originalSender;
	};

}

#endif
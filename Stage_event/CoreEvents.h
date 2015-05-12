#ifndef COREEVENTS_H
#define COREEVENTS_H

#include "stdafx.h"

namespace stage {
	struct Event {
		Event(Theron::Address originator, uint32_t msgID){
			id = generateID(originator, msgID);
		}
		Event(uint64_t msgID){
			id = msgID;
		}
		uint64_t id;
		static uint64_t generateID(Theron::Address originator, uint32_t msgID){
			uint64_t id = originator.AsInteger();
			id = id << 32;
			id = id | msgID;
			return id;
		}
	};
	struct Update : Event {
		const int elapsedMS;
		Update(int ms, uint64_t msgID) : elapsedMS(ms), Event(msgID){}
		Update(int ms, Theron::Address originator, uint32_t msgID): elapsedMS(ms), Event(originator, msgID){}
	};
	struct Render : Event {
		Render(uint64_t msgID) : Event(msgID){}
		Render(Theron::Address originator, uint32_t msgID) : Event(originator, msgID){}
	};
	struct KillPill : Event {
	};
	struct AllDone : Event {
		AllDone(uint64_t msgID) : Event(msgID){}
	};
	struct Error : Event {
		Error(uint64_t msgID) : Event(msgID){}
	};
}

#endif
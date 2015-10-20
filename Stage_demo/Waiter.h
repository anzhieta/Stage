#ifndef WAITER_H
#define WAITER_H

#define WAITER_ID 7

#include "stdafx.h"
#include <Component.h>
#include <chrono>

typedef std::chrono::high_resolution_clock hrclock;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_time;

namespace stage{
	class Waiter : public Component{
	public:
		Waiter(Theron::Framework& fw, Theron::Address owner, double waitMicros) :
			Component(fw, owner), waitMicros(waitMicros){}

		virtual void update(const Update &up, Theron::Address from){
			clock_time start = hrclock::now();
			std::chrono::duration<double, std::micro> duration = start - start;
			while (duration.count() < waitMicros){
				clock_time now = hrclock::now();
				duration = now - start;
			}
			Send(AllDone(up.id), from);
		}

		virtual int id(){ return WAITER_ID; }
	private:
		double waitMicros;
	};

}

#endif
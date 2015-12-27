#ifndef WAITER_H
#define WAITER_H

#define WAITER_ID 7

#include "stdafx.h"
#include <Component.h>

namespace stage{
	class Waiter : public Component{
	public:
		Waiter(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID, int limit) :
			Component(fw, owner), limit(limit){
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		virtual void update(float elapsedMS, uint64_t id){
			for (int i = 3; i <= limit; i++){
				for (int j = 2; j < i; j++){
					if (i % j == 0) break;
				}
			}
			finishPhase(id);
		}

		virtual int id(){ return WAITER_ID; }
		virtual std::string name(){ return "Waiter"; }
	private:
		int limit;
	};

}

#endif
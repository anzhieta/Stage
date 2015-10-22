#ifndef WAITER_H
#define WAITER_H

#define WAITER_ID 7

#include "stdafx.h"
#include <Component.h>

namespace stage{
	class Waiter : public Component{
	public:
		Waiter(Theron::Framework& fw, Theron::Address owner, int limit) :
			Component(fw, owner), limit(limit){}

		virtual void update(const Update &up, Theron::Address from){
			for (int i = 3; i <= limit; i++){
				for (int j = 2; j < i; j++){
					if (i % j == 0) break;
				}
			}
			Send(AllDone(up.id), from);
		}

		virtual int id(){ return WAITER_ID; }
	private:
		int limit;
	};

}

#endif
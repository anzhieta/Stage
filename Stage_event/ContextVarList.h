#ifndef CONTEXTVARLIST_H
#define CONTEXTVARLIST_H

#include "stdafx.h"
#include <boost\any.hpp>

namespace stage{
	struct ContextVar{
		boost::any content;
		ContextVar* next = nullptr;

		ContextVar(boost::any content) : content(content){}

		~ContextVar(){
			if (next != nullptr) delete next;
		}

		boost::any get(int depth){
			if (depth < 1) return content;
			return next->get(depth - 1);
		}

		void set(int depth, boost::any content){
			if (depth < 1) this->content = content;
			else {
				if (next == nullptr){
					boost::any a;
					next = new ContextVar(a);
				}
				next->set(depth - 1, content);
			}
		}
	};
}

#endif
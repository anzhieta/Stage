#include "stdafx.h"
#include "GraphicsControlActor.h"

using namespace stage;

//Alkuarvo globaalin singletonin osoitteelle
Theron::Address GraphicsControlActor::globalController = Theron::Address::Null();

GraphicsControlActor::GraphicsControlActor(Theron::Framework& fw, std::string windowname, int x, int y) :
Theron::Actor(fw), gc(windowname, x, y) {
	if (globalController == Theron::Address::Null()){
		globalController = this->GetAddress();
	}
	else {
		//Sallitaan vain yksi grafiikkamoottoriolio
		std::cout << "Global GraphicsControlActor already set\n";
		abort();
	}
	RegisterHandler(this, &GraphicsControlActor::queue);
}
GraphicsControlActor::~GraphicsControlActor(){
	if (globalController == this->GetAddress()) globalController = Theron::Address::Null();
}
void GraphicsControlActor::queue(const Queue& msg, Theron::Address sender){
	gc.queue(msg.model, msg.position);
	Send(AllDone(msg.id), sender);
}
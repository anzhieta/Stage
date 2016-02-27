#include "stdafx.h"
#include "ModelComponent.h"
#include <LogActor.h>

using namespace stage;

//---Kontekstiyksikkö alkaa--
ModelComponent::ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner) : Component(fw, owner), mod(mod){
	RegisterHandler(this, &ModelComponent::initialize);
	//Haetaan isäntäolion sijaintikomponentti, jotta malli voidaan piirtää oikeaan paikkaan
	uint64_t msgid = tracker.getNextID();
	EventContext& ev = tracker.addContext(0, msgid, Theron::Address::Null());
	ev.finalize = [](){};
	ev.error = [this](){
		LOGMSG("Error: Attempted to initialize model component, but owner does not have a transform");
	};
	Send(GameObject::GetComponent(msgid, TRANSFORM_ID), owner);
	RegisterHandler(this, &ModelComponent::completeRender);
}
void ModelComponent::initialize(const GameObject::ComponentFound& msg, Theron::Address sender){
	if (!tracker.contains(msg.id)) return;
	tracker.decrement(msg.id);
	transform = msg.component;
	DeregisterHandler(this, &ModelComponent::initialize);
	init = true;
}
//---Kontekstiyksikkö päättyy--

//---Kontekstiyksikkö alkaa--
void ModelComponent::render(const Render& msg, Theron::Address sender){
	if (!init) return;
	uint64_t id = tracker.getNextID();
	tracker.addContext(msg.id, id, sender);
	//Haetaan isäntäolion nykyinen sijainti, jotta malli voidaan piirtää oikeaan paikkaan
	Send(Transform::GetMatrix(id), transform);
}
void ModelComponent::completeRender(const Transform::Matrix& msg, Theron::Address sender){
	if (!tracker.contains(msg.id)) return;
	Send(GraphicsControlActor::Queue(msg.id, mod, msg.matrix), GraphicsControlActor::getGlobalController());
}
//---Kontekstiyksikkö päättyy--
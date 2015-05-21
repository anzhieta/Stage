#include "stdafx.h"
#include "ModelComponent.h"

using namespace stage;

//---Kontekstiyksikkö alkaa--
ModelComponent::ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner) : Component(fw, owner), mod(mod){
	RegisterHandler(this, &ModelComponent::initialize);
	//Haetaan isäntäolion sijaintikomponentti, jotta malli voidaan piirtää oikeaan paikkaan
	Send(GameObject::GetComponent(TRANSFORM_ID, this->GetAddress()), owner);
	RegisterHandler(this, &ModelComponent::completeRender);
}

void ModelComponent::initialize(const Component::ComponentID& msg, Theron::Address sender){
	if (!msg.id == TRANSFORM_ID) return;
	transform = sender;
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
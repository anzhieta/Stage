#include "stdafx.h"
#include "CameraComponent.h"
#include <LogActor.h>

using namespace stage;

//---Kontekstiyksikkö alkaa--
CameraComponent::CameraComponent(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner){
	RegisterHandler(this, &CameraComponent::initialize);
	//Haetaan isäntäolion sijaintikomponentti, jotta kameran sijainti voidaan päivittää ruudun piirron yhteydessä
	uint64_t msgid = tracker.getNextID();
	EventContext& ev = tracker.addContext(0, msgid, Theron::Address::Null());
	ev.finalize = [](){};
	ev.error = [this](){
		LOGMSG("Error: Attempted to initialize camera component, but owner does not have a transform");
	};
	Send(GameObject::GetComponent(msgid, TRANSFORM_ID), owner);
	RegisterHandler(this, &CameraComponent::completeRender);
	RegisterHandler(this, &CameraComponent::setViewMatrix);
	RegisterHandler(this, &CameraComponent::setProjectionMatrix);
	
}

void CameraComponent::initialize(const GameObject::ComponentFound &msg, Theron::Address sender){
	if (!tracker.contains(msg.id)) return;
	tracker.decrement(msg.id);
	DeregisterHandler(this, &CameraComponent::initialize);
	transform = msg.component;
	init = true;
}
//---Kontekstiyksikkö päättyy--

//---Kontekstiyksikkö alkaa--
void CameraComponent::render(const Render& msg, Theron::Address sender){
	if (!init){
		//sijaintikomponenttia ei vielä saatu, joten kameran sijaintia ei voida päivittää
		Send(AllDone(msg.id), sender);
		return;
	}
	uint64_t id = tracker.getNextID();
	tracker.addContext(msg.id, id, sender);
	//Haetaan sijaintikomponentilta kameran sijainti tämän ruudunpäivityksen aikana
	Send(Transform::GetMatrix(id), transform);
}

void CameraComponent::completeRender(const Transform::Matrix& msg, Theron::Address sender){
	if (!tracker.contains(msg.id)) return;
	//Asetetaan kameralle uusi sijainti
	cam.setViewMatrix(msg.matrix);
	tracker.decrement(msg.id);
}
//---Kontekstiyksikkö päättyy--

void CameraComponent::setViewMatrix(const SetViewMatrix& msg, Theron::Address sender){
	cam.setViewMatrix(msg.view);
	Send(AllDone(msg.id), sender);
}

void CameraComponent::setProjectionMatrix(const SetProjectionMatrix& msg, Theron::Address sender){
	cam.setProjectionMatrix(msg.projection);
	Send(AllDone(msg.id), sender);
}
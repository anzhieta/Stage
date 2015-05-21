#include "stdafx.h"
#include "CameraComponent.h"

using namespace stage;

//---Kontekstiyksikkö alkaa--
CameraComponent::CameraComponent(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner){
	RegisterHandler(this, &CameraComponent::initialize);
	//Haetaan isäntäolion sijaintikomponentti, jotta kameran sijainti voidaan päivittää ruudun piirron yhteydessä
	Send(GameObject::GetComponent(TRANSFORM_ID, this->GetAddress()), owner);
	RegisterHandler(this, &CameraComponent::completeRender);
	RegisterHandler(this, &CameraComponent::setViewMatrix);
	RegisterHandler(this, &CameraComponent::setProjectionMatrix);
	
}

void CameraComponent::initialize(const Component::ComponentID &msg, Theron::Address sender){
	if (msg.id != TRANSFORM_ID) return;
	DeregisterHandler(this, &CameraComponent::initialize);
	transform = sender;
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
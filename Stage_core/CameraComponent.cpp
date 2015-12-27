#include "stdafx.h"
#include "CameraComponent.h"
#include <LogActor.h>

using namespace stage;

//---Kontekstiyksikkö alkaa--
CameraComponent::CameraComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID) : Component(fw, owner){	
	registerSelf(fw, owner, notifyDest, notifyID);
}

void CameraComponent::initialize(GameObject* owner){
	Component::initialize(owner);
	transform = (Transform*)owner->getComponent(TRANSFORM_ID);
	RegisterHandler<CameraComponent, SetViewMatrix, &CameraComponent::setViewMatrix>();
	RegisterHandler<CameraComponent, SetProjectionMatrix, &CameraComponent::setProjectionMatrix>();
}
//---Kontekstiyksikkö päättyy--

//---Kontekstiyksikkö alkaa--
void CameraComponent::render(uint64_t id){
	cam.setViewMatrix(transform->getMatrix());
	owner->allDone(id);
}

//---Kontekstiyksikkö päättyy--

void CameraComponent::setViewMatrix(const SetViewMatrix& msg, Theron::Address sender){
	cam.setViewMatrix(msg.view);
	Send(AllDone(msg.id, CAMERA_ID, msg.senderComponent), sender);
}

void CameraComponent::setProjectionMatrix(const SetProjectionMatrix& msg, Theron::Address sender){
	cam.setProjectionMatrix(msg.projection);
	Send(AllDone(msg.id, CAMERA_ID, msg.senderComponent), sender);
}
#include "stdafx.h"
#include "ModelComponent.h"
#include <LogActor.h>

using namespace stage;

//---Kontekstiyksikkö alkaa--
ModelComponent::ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner, Destination notifyDest, uint64_t notifyID
	) : Component(fw, owner), mod(mod){
	registerSelf(fw, owner, notifyDest, notifyID);
}

void ModelComponent::initialize(GameObject* owner){
	Component::initialize(owner);
	transform = (Transform*)owner->getComponent(TRANSFORM_ID);
}
//---Kontekstiyksikkö päättyy--

//---Kontekstiyksikkö alkaa--
void ModelComponent::render(uint64_t oldid){
	uint64_t id = createStandardContext(oldid);	
	Send(GraphicsControlActor::Queue(id, mod, transform->getMatrix(), MODEL_ID), GraphicsControlActor::getGlobalController());
}

//---Kontekstiyksikkö päättyy--
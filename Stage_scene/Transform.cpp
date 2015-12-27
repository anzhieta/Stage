#include "stdafx.h"
#include "Transform.h"

using namespace stage;

Transform::Transform(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID, glm::mat4& tr) : Component(fw, owner), transform(tr){
	registerSelf(fw, owner, notifyDest, notifyID);
}

void Transform::initialize(GameObject* owner){
	Component::initialize(owner);
	RegisterHandler<Transform, GetMatrix, &Transform::getMatrix>();
	RegisterHandler<Transform, SetMatrix, &Transform::setMatrix>();
	RegisterHandler<Transform, Translate, &Transform::translate>();
	RegisterHandler<Transform, GetPosition, &Transform::getPosition>();
}

void Transform::getMatrix(const GetMatrix& msg, Theron::Address sender){
	Send(Matrix(msg.id, transform, msg.senderComponent), sender);
}

void Transform::getPosition(const GetPosition& msg, Theron::Address sender){
	glm::vec3 position = glm::vec3(transform[3]);
	Send(Position(msg.id, position, msg.senderComponent), sender);
}

void Transform::setMatrix(const SetMatrix& msg, Theron::Address sender){
	transform = msg.matrix;
	Send(AllDone(msg.id, id(), msg.senderComponent), sender);
}

void Transform::translate(const Translate& msg, Theron::Address sender){
	transform = glm::translate(transform, msg.vector);
	Send(AllDone(msg.id, id(), msg.senderComponent), sender);
}
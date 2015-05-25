#include "stdafx.h"
#include "Transform.h"

using namespace stage;

Transform::Transform(Theron::Framework& fw, Theron::Address owner, glm::mat4& tr) : Component(fw, owner), transform(tr){
	RegisterHandler(this, &Transform::getMatrix);
	RegisterHandler(this, &Transform::setMatrix);
	RegisterHandler(this, &Transform::translate);
}

void Transform::getMatrix(const GetMatrix& msg, Theron::Address sender){
	Send(Matrix(msg.id, transform), sender);
}

void Transform::setMatrix(const SetMatrix& msg, Theron::Address sender){
	transform = msg.matrix;
	Send(AllDone(msg.id), sender);
}

void Transform::translate(const Translate& msg, Theron::Address sender){
	transform = glm::translate(transform, msg.vector);
	Send(AllDone(msg.id), sender);
}
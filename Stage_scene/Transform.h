#pragma once

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define TRANSFORM_ID 1

#include "stdafx.h"
#include "Component.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <CoreEvents.h>

namespace stage{
	class Transform : public Component {
	public:
		struct Matrix : public Event {
			glm::mat4& matrix;
			Matrix(glm::mat4& mt, Theron::Address originator, uint32_t msgID) : matrix(mt), Event(originator, msgID){}
			Matrix(uint64_t msgID, glm::mat4& mt) : matrix(mt), Event(msgID){}
		};
		struct GetMatrix : public Event {
			GetMatrix(Theron::Address originator, uint32_t msgID) : Event(originator, msgID){}
			GetMatrix(uint64_t msgID) : Event(msgID){}
		};
		struct SetMatrix : public Event {
			glm::mat4& matrix;
			SetMatrix(Theron::Address originator, uint32_t msgID, glm::mat4& mt) : matrix(mt), Event(originator, msgID){}
			SetMatrix(uint64_t msgID, glm::mat4& mt) : matrix(mt), Event(msgID){}
		};
		struct Translate : public Event {
			glm::vec3 vector;
			Translate(Theron::Address originator, uint32_t msgID, glm::vec3 vec) : vector(vec), Event(originator, msgID){}
			Translate(uint64_t msgID, glm::vec3 vec) : vector(vec), Event(msgID){}
		};

		Transform(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner){
			transform = glm::mat4();
			initialize();
		}
		Transform(Theron::Framework& fw, Theron::Address owner, glm::mat4& tr) : Component(fw, owner), transform(tr){
			initialize();
		}
	private:
		virtual int id(){ return TRANSFORM_ID; }
		glm::mat4 transform;
		void getMatrix(const GetMatrix& msg, Theron::Address sender){
			Send(Matrix(msg.id, transform), sender);
		}
		void setMatrix(const SetMatrix& msg, Theron::Address sender){
			transform = msg.matrix;
			Send(AllDone(msg.id), sender);
		}
		void translate(const Translate& msg, Theron::Address sender){
			transform = glm::translate(transform, msg.vector);
			Send(AllDone(msg.id), sender);
		}
		void initialize(){
			RegisterHandler(this, &Transform::getMatrix);
			RegisterHandler(this, &Transform::setMatrix);
			RegisterHandler(this, &Transform::translate);
		}
	};
}

#endif
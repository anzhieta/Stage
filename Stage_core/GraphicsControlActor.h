#ifndef GRAPHICSCONTROLACTOR_H
#define GRAPHICSCONTROLACTOR_H

#include "stdafx.h"
#include <GraphicsController.h>
#include <CoreEvents.h>
#include <Model.h>
#include <Camera.h>
#include <glm\glm.hpp>
#include <iostream>


namespace stage {
	class GraphicsControlActor : public Theron::Actor{
	public:
		struct Queue : public Event{
			stage_common::Model* model;
			const glm::mat4& position;
			Queue(Theron::Address originator, uint32_t msgID, stage_common::Model* mod, glm::mat4& pos):
				Event(originator, msgID), model(mod), position(pos){}
			Queue(uint64_t msgID, stage_common::Model* mod, const glm::mat4& pos) :
				Event(msgID), model(mod), position(pos){}
		};
		struct Draw : public Event{
			const stage_common::Camera& cam;
			Draw(Theron::Address originator, uint32_t msgID, stage_common::Camera& cam) :
				Event(originator, msgID), cam(cam){}
			Draw(uint64_t msgID, stage_common::Camera& cam) :
				Event(msgID), cam(cam){}
		};

		GraphicsControlActor(Theron::Framework& fw, std::string windowname, int x, int y) :
			Theron::Actor(fw), gc(windowname, x, y) {
			if (globalController == Theron::Address::Null()){
				globalController = this->GetAddress();
			}
			else {
				std::cout << "gc already set\n";
				return;
			}
			RegisterHandler(this, &GraphicsControlActor::queue);
			RegisterHandler(this, &GraphicsControlActor::draw);
			
		}
		stage_common::GraphicsController* getRawController() {
			return &gc;
		}
		static Theron::Address getGlobalController(){ return globalController; }
	private:
		static Theron::Address globalController;
		stage_common::GraphicsController gc;
		void queue(const Queue& msg, Theron::Address sender){
			gc.queue(msg.model, msg.position);
			Send(AllDone(msg.id), sender);
		}
		void draw(const Draw& msg, Theron::Address sender){
			std::cout << "drawing\n";
			gc.draw(msg.cam);
			//std::cout << "drew\n";
			Send(AllDone(msg.id), sender);
		}
	};
}

#endif
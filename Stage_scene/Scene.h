#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "GameObject.h"
#include <EventChannel.h>
#include <CoreEvents.h>
#include <iostream>

namespace stage {
	class Scene : public Theron::Actor{
		friend class SceneManager;
	public:
		struct CreateObject{
			CreateObject(Theron::Address &req) : requester(req){}
			Theron::Address requester;
		};
		struct NewObject{
			NewObject(Theron::Address &obj) : object(obj){}
			NewObject(){}
			Theron::Address object;
		};
		~Scene();
	private:
		Scene(Theron::Framework &fw, int id) : Theron::Actor(fw), id(id), tracker(fw, this->GetAddress()){
			RegisterHandler(this, &Scene::update);
			RegisterHandler(this, &Scene::render);
			RegisterHandler(this, &Scene::allDone);
			RegisterHandler(this, &Scene::error);
			RegisterHandler(this, &Scene::createObject);
		}
		int id;
		std::list<GameObject*> objects;
		ContextTracker tracker;
		void createObject(const CreateObject &msg, Theron::Address sender);
		void update(const Update &msg, Theron::Address sender){
			uint64_t id = tracker.getNextID();
			for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); it++){
				tracker.trackedSend<Update>(msg.id, Update(msg.elapsedMS, id), (*it)->GetAddress(), sender);
			}
		}
		void render(const Render &msg, Theron::Address sender){
			uint64_t id = tracker.getNextID();
			for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); it++){
				tracker.trackedSend<Render>(msg.id, Render(id), (*it)->GetAddress(), sender);
			}
		}
		void allDone(const AllDone &msg, Theron::Address sender){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
		void error(const Error &msg, Theron::Address sender){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
		
	};
}

#endif
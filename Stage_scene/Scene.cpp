#include "stdafx.h"
#include "Scene.h"
#include <iostream>
#include <LogActor.h>

using namespace stage;

Scene::Scene(Theron::Framework &fw) : Theron::Actor(fw), tracker(fw, this->GetAddress()){
	RegisterHandler(this, &Scene::update);
	RegisterHandler(this, &Scene::render);
	RegisterHandler(this, &Scene::allDone);
	RegisterHandler(this, &Scene::error);
	RegisterHandler(this, &Scene::createObject);
}
void Scene::createObject(const Scene::CreateObject &msg, Theron::Address sender){
	GameObject* newObject = new GameObject(this->GetFramework());
	objects.push_back(newObject);
	Send(NewObject(msg.id, newObject->GetAddress()), sender);
}
Scene::~Scene(){
	for (std::list<GameObject*>::iterator i = objects.begin(); i != objects.end(); i++){
		delete *i;
	}
}
void Scene::update(const Update &msg, Theron::Address sender){
	uint64_t id = tracker.getNextID();
	for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); it++){
		tracker.trackedSend<Update>(msg.id, Update(msg.elapsedMS, id), (*it)->GetAddress(), sender);
	}
}
void Scene::render(const Render &msg, Theron::Address sender){
	uint64_t id = tracker.getNextID();
	for (std::list<GameObject*>::iterator it = objects.begin(); it != objects.end(); it++){
		tracker.trackedSend<Render>(msg.id, Render(id), (*it)->GetAddress(), sender);
	}
}
void Scene::allDone(const AllDone &msg, Theron::Address sender){
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}
void Scene::error(const Error &msg, Theron::Address sender){
	LOGERR(std::string("Warning: game object ") + sender.AsString() + " reported error during processing");
	if (tracker.contains(msg.id)) tracker.decrement(msg.id);
}
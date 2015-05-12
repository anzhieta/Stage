#include "stdafx.h"
#include "Scene.h"
#include <iostream>

using namespace stage;

void Scene::createObject(const Scene::CreateObject &msg, Theron::Address sender){
	GameObject* newObject = new GameObject(this->GetFramework());
	//std::cout << newObject.GetAddress().AsInteger();
	//Send(Render(GetAddress(), 1), newObject.GetAddress());
	objects.push_back(newObject);
	//up.registerRecipient(newObject->GetAddress());
	//rend.registerRecipient(newObject->GetAddress());
	Send(NewObject(newObject->GetAddress()), msg.requester);
}
#include "stdafx.h"
#include "SceneManager.h"
#include <iostream>

using namespace stage;

SceneManager::~SceneManager(){
}

Theron::Address SceneManager::createScene(){
	Scene* scene = new Scene(getFramework(), scenes.size());
	scenes.push_back(scene);
	return scene->GetAddress();
}

void SceneManager::setActiveScene(unsigned int scene){
	if (scene >= scenes.size()){
		//TODO error logging
		return;
	}
	else{
		activeScene = scenes[scene]->GetAddress();
	}
}

void SceneManager::setActiveCamera(Theron::Address cam){
	activeCamera = cam;
}

Theron::Address SceneManager::globalManager = Theron::Address::Null();
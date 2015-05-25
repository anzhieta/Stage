#include "stdafx.h"
#include "SceneManager.h"
#include <iostream>
#include <LogActor.h>

using namespace stage;

SceneManager::SceneManager(){
	receiver.RegisterHandler(&doneCatcher, &Theron::Catcher<AllDone>::Push);
	receiver.RegisterHandler(&abortCatcher, &Theron::Catcher<Abort>::Push);
	receiver.RegisterHandler(&setSceneCatcher, &Theron::Catcher<SetActiveScene>::Push);
	receiver.RegisterHandler(&createSceneCatcher, &Theron::Catcher<CreateScene>::Push);
}

SceneManager::~SceneManager(){
}

Theron::Address SceneManager::createScene(){
	Scene* scene = new Scene(getFramework());
	scenes.push_back(scene);
	return scene->GetAddress();
}

bool SceneManager::setActiveScene(unsigned int scene){
	if (scene >= scenes.size()){
		getFramework().Send(LogActor::LogError("Error: Attempted to activate a scene that does not exist"),
			receiver.GetAddress(), LogActor::getGlobalLogger());
		return false;
	}
	else{
		activeScene = scenes[scene]->GetAddress();
		return true;
	}
}

/*void SceneManager::setActiveCamera(Theron::Address cam){
	activeCamera = cam;
}*/

Theron::Address SceneManager::globalManager = Theron::Address::Null();
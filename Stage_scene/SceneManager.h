#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "stdafx.h"
#include <vector>

#include "Scene.h"

namespace stage{
	class SceneManager{
	public:
		struct Abort{};
		struct SetActiveScene{
			Theron::Address scene;
			SetActiveScene(Theron::Address scene) :scene(scene){}
		};
		struct SetActiveCamera{
			Theron::Address cam;
			SetActiveCamera(Theron::Address cam) :cam(cam){}
		};

		SceneManager(){
			receiver.RegisterHandler(&doneCatcher, &Theron::Catcher<AllDone>::Push);
			receiver.RegisterHandler(&abortCatcher, &Theron::Catcher<Abort>::Push);
			receiver.RegisterHandler(&sceneCatcher, &Theron::Catcher<SetActiveScene>::Push);
			receiver.RegisterHandler(&camCatcher, &Theron::Catcher<SetActiveCamera>::Push);
		}

		static Theron::Address getGlobalManager(){ return globalManager; }
		virtual void setActiveScene(unsigned int scene);
		virtual void setActiveCamera(Theron::Address cam);
		virtual Theron::Address createScene();

		virtual Theron::Framework& getFramework() = 0;

	protected:
		std::vector<Scene*> scenes;
		static Theron::Address globalManager;
		Theron::Address activeScene;
		Theron::Address activeCamera;

		Theron::Receiver receiver;
		Theron::Catcher<AllDone> doneCatcher;
		Theron::Catcher<Abort> abortCatcher;
		Theron::Catcher<SetActiveScene> sceneCatcher;
		Theron::Catcher<SetActiveCamera> camCatcher;
	};

}

#endif
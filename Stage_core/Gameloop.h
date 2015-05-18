#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "stdafx.h"
#include <Theron\Framework.h>
#include <SceneManager.h>
#include "GraphicsControlActor.h"
#include <LogActor.h>

namespace stage {

	class Gameloop : public SceneManager{
	public:
		Gameloop(std::string& windowName, int xres, int yres){
			if (SceneManager::globalManager != Theron::Address::Null()){
				std::abort();
			}
			gc = new GraphicsControlActor(fw, windowName, xres, yres);
			logger = new LogActor(fw);
			SceneManager::globalManager = receiver.GetAddress();
		}
		~Gameloop(){
			if (globalManager == receiver.GetAddress()) globalManager = Theron::Address::Null();
			for (std::vector<Scene*>::iterator i = scenes.begin(); i != scenes.end(); i++){
				delete *i;
			}
			delete gc;
			delete logger;
		}
		float getTimescale();
		void setTimescale(float ts);
		Theron::Framework& getFramework(){ return fw; }
		void setActiveCamera(stage_common::Camera* cam){
			activeCam = cam;
		}

		void start();
		void stop();
	private:
		Theron::Framework fw;
		GraphicsControlActor* gc;
		LogActor* logger;
		stage_common::Camera* activeCam;
		float timescale = 1;
		bool abort = false;

		void loop();
		void shutdown();
	};
}

#endif
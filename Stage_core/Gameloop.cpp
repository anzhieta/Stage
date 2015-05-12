#pragma once

#include "stdafx.h"
#include <Theron\Theron.h>
#include <iostream>
#include <string>
#include "Gameloop.h"
#include "GameObject.h"
#include <CoreEvents.h>
#include "CameraComponent.h"

using namespace stage;

namespace stage {

	float Gameloop::getTimescale() {
			return timescale;
	}
	void Gameloop::setTimescale(float ts) {
		timescale = ts;
	}
	void Gameloop::start() {
		if (Theron::Address::Null() == activeScene) return;
		loop();
	}
	void Gameloop::stop(){
		abort = true;
	}
	void Gameloop::loop() {
		Theron::Address sender;
		AllDone ad(0);
		Theron::Address recAddress = receiver.GetAddress();
		while (!abort) {
			//Update phase
			std::cout << "update phase\n";
			fw.Send(Update(1, receiver.GetAddress(), 1), recAddress, activeScene);
			while (doneCatcher.Empty()){
				receiver.Wait();
			}
			doneCatcher.Pop(ad, sender);

			//Delete phase

			//Render phase
			std::cout << "render phase\n";
			fw.Send(Render(receiver.GetAddress(), 1), recAddress, activeScene);
			while (doneCatcher.Empty()){
				receiver.Wait();
			}
			doneCatcher.Pop(ad, sender);
			std::cout << "render finish\n";
			gc->getRawController()->draw(*activeCam);
			/*fw.Send(CameraComponent::DrawCamera(1), recAddress, activeCamera);
			while (doneCatcher.Empty()){
				receiver.Wait();
			}
			doneCatcher.Pop(ad, sender);*/

			//Maintenance phase
			std::cout << "maintenance phase\n";

			if (!abortCatcher.Empty()) abort = true;
		}
		shutdown();
	}
	void Gameloop::shutdown(){
		std::cout << "shutting down";
	}

	/*void Gameloop::setActiveScene(Theron::Address scene){
		activeScene = scene;
	}

	Theron::Address Gameloop::createScene(){
		activeScene = scene;
	}*/
};
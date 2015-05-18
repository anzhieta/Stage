#pragma once

#include "stdafx.h"
#include <Theron\Theron.h>
#include <iostream>
#include <string>
#include "Gameloop.h"
#include "GameObject.h"
#include <CoreEvents.h>
#include "CameraComponent.h"
#include <Timer.h>


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
		stage_common::Timer upTimer;
		stage_common::Timer rendTimer;
		stage_common::Timer maintTimer;
		stage_common::Timer loopTimer;
		while (!abort) {
			loopTimer.start();

			//Update phase
			upTimer.start();
			//std::cout << "update phase\n";
			fw.Send(Update((float)loopTimer.lastTickTime(), receiver.GetAddress(), 1), recAddress, activeScene);
			while (doneCatcher.Empty()){
				receiver.Wait();
			}
			doneCatcher.Pop(ad, sender);
			upTimer.stop();

			//Delete phase

			//Render phase
			rendTimer.start();
			//std::cout << "render phase\n";
			fw.Send(Render(receiver.GetAddress(), 1), recAddress, activeScene);
			while (doneCatcher.Empty()){
				receiver.Wait();
			}
			doneCatcher.Pop(ad, sender);
			//std::cout << "render finish\n";
			gc->getRawController()->draw(*activeCam);
			rendTimer.stop();

			//Maintenance phase
			maintTimer.start();
			//std::cout << "maintenance phase\n";

			if (!abortCatcher.Empty()) abort = true;
			if (gc->shouldClose()) abort = true;
			maintTimer.stop();
			loopTimer.stop();
		}
		fw.Send(LogActor::LogMessage("Total runtime " + std::to_string(loopTimer.totalTime())), recAddress, logger->GetAddress());
		//std::cout << "Total runtime: " << loopTimer.totalTime() << std::endl;
		fw.Send(LogActor::LogMessage("Total frames: " + std::to_string(loopTimer.totalTicks())), recAddress, logger->GetAddress());
		fw.Send(LogActor::LogMessage("Average loop time: " + std::to_string(loopTimer.averageTime())), recAddress, logger->GetAddress());
		fw.Send(LogActor::LogMessage("Average update time: " + std::to_string(upTimer.averageTime())), recAddress, logger->GetAddress());
		fw.Send(LogActor::LogMessage("Average render time: " + std::to_string(rendTimer.averageTime())), recAddress, logger->GetAddress());
		fw.Send(LogActor::LogMessage("Average maintenance time: " + std::to_string(maintTimer.averageTime())), recAddress, logger->GetAddress());

		shutdown();
	}
	void Gameloop::shutdown(){
		fw.Send(LogActor::LogMessage("Shutting down"), receiver.GetAddress(), logger->GetAddress());
		Theron::Catcher<LogActor::Terminate> terminateCatcher;
		receiver.RegisterHandler(&terminateCatcher, &Theron::Catcher<LogActor::Terminate>::Push);
		fw.Send(LogActor::Terminate(), receiver.GetAddress(), logger->GetAddress());
		while (terminateCatcher.Empty()){
			receiver.Wait();
		}
	}

	/*void Gameloop::setActiveScene(Theron::Address scene){
		activeScene = scene;
	}

	Theron::Address Gameloop::createScene(){
		activeScene = scene;
	}*/
};
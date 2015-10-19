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
#include <Input.h>


using namespace stage;

Gameloop::Gameloop(std::string& windowName, int xres, int yres, uint32_t threadcount): 
	fw(Theron::Framework::Parameters(threadcount)), eventChannelManager(fw){
	if (SceneManager::globalManager != Theron::Address::Null()){
		//Sallitaan vain yksi Gameloop
		std::abort();
	}
	gc = new GraphicsControlActor(fw, windowName, xres, yres);
	logger = new LogActor(fw);
	SceneManager::globalManager = receiver.GetAddress();
}

Gameloop::~Gameloop(){
	if (globalManager == receiver.GetAddress()) globalManager = Theron::Address::Null();
	//Poistetaan pelialueet
	for (std::vector<Scene*>::iterator i = scenes.begin(); i != scenes.end(); i++){
		delete *i;
	}
	//Poistetaan grafiikkamoottori
	delete gc;
	//Poistetaan lokipalvelu
	delete logger;
}

float Gameloop::getTimescale() {
		return timescale;
}
void Gameloop::setTimescale(float ts) {
	timescale = ts;
}

void Gameloop::setActiveCamera(stage_common::Camera* cam){
	activeCam = cam;
}

void Gameloop::start() {
	if (Theron::Address::Null() == activeScene){
		fw.Send(LogActor::LogError("Failed to start game engine: active scene not set"), receiver.GetAddress(), logger->GetAddress());
		return;
	}
	if (activeCam == nullptr){
		fw.Send(LogActor::LogError("Failed to start game engine: active camera not set"), receiver.GetAddress(), logger->GetAddress());
		return;
	}
	loop();
}
void Gameloop::stop(){
	abort = true;
}
void Gameloop::loop() {
	Theron::Address sender;

	//Placeholder-viestit catchereitä varten
	AllDone adMSG(0);
	SetActiveScene sasMSG(0,0);
	CreateScene csMSG(0);

	Theron::Address recAddress = receiver.GetAddress();
	stage_common::Timer upTimer;	//Päivitysajastin
	stage_common::Timer rendTimer;	//Piirtoajastin
	stage_common::Timer maintTimer;	//Ylläpitoajastin
	stage_common::Timer loopTimer;	//Koko pelisilmukan ajastin

	//Pelisilmukka
	while (!abort) {
		loopTimer.start();

		//Päivitysvaihe
		upTimer.start();
		uint64_t id = Event::generateID(recAddress, msgid++);
		fw.Send(Update((float)loopTimer.lastTickTime() * timescale, id), recAddress, activeScene);
		while (doneCatcher.Empty()){
			//Odotetaan, kunnes saadaan viesti
			receiver.Wait();

			//Luodaan uudet pelialueet
			while (!createSceneCatcher.Empty()){
				createSceneCatcher.Pop(csMSG, sender);
				Theron::Address newScene = createScene();
				fw.Send(NewScene(csMSG.id, scenes.size() - 1, newScene), recAddress, sender);
			}

			//Vaihdetaan pelialuetta
			while (!setSceneCatcher.Empty()){
				setSceneCatcher.Pop(sasMSG, sender);
				if (setActiveScene(sasMSG.scene)){
					fw.Send(AllDone(sasMSG.id), recAddress, sender);
				}
				else fw.Send(Error(sasMSG.id), recAddress, sender);
			}
		}
		doneCatcher.Pop(adMSG, sender);
		upTimer.stop();

		//Poistovaihe
		//TODO: peliolioiden poistaminen

		

		//Piirtovaihe
		rendTimer.start();
		//Haetaan piirrettävät mallit
		id = Event::generateID(recAddress, msgid++);
		fw.Send(Render(id), recAddress, activeScene);
		while (doneCatcher.Empty()){
			receiver.Wait();
		}
		doneCatcher.Pop(adMSG, sender);
		//Piirretään kuva ruudulle (tehtävä pääsäikeessä, koska OpenGL-kontekstit ovat säiekohtaisia)
		gc->getRawController()->draw(*activeCam);
		rendTimer.stop();

		//Ylläpitovaihe
		maintTimer.start();
		stage_common::Input::getSingleton().update(false);

		//Huolletaan tapahtumakanavat
		id = Event::generateID(recAddress, msgid++);
		fw.Send(EventChannelManager::ChannelMaintenance(id), recAddress, eventChannelManager.GetAddress());
		while (doneCatcher.Empty()){
			receiver.Wait();
		}
		doneCatcher.Pop(adMSG, sender);

		//Tarkistetaan, pitääkö ohjelma sulkea
		if (!abortCatcher.Empty()) stop();
		if (gc->shouldClose()) stop();
		maintTimer.stop();
		loopTimer.stop();
	}
	//Kirjoitetaan lokiin suorituskykytiedot
	fw.Send(LogActor::LogMessage("Total runtime " + std::to_string(loopTimer.totalTime())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Total frames: " + std::to_string(loopTimer.totalTicks())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Average loop time: " + std::to_string(loopTimer.averageTime())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Average fps: " + std::to_string(1000 / loopTimer.averageTime())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Average update time: " + std::to_string(upTimer.averageTime())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Average render time: " + std::to_string(rendTimer.averageTime())), recAddress, logger->GetAddress());
	fw.Send(LogActor::LogMessage("Average maintenance time: " + std::to_string(maintTimer.averageTime())), recAddress, logger->GetAddress());

	shutdown();
}
void Gameloop::shutdown(){
	//Annetaan lokiolion kirjoittaa kaikki vielä odottavat viestit, jotta se voi sulkeutua turvallisesti
	fw.Send(LogActor::LogMessage("Shutting down"), receiver.GetAddress(), logger->GetAddress());
	Theron::Catcher<LogActor::Terminate> terminateCatcher;
	receiver.RegisterHandler(&terminateCatcher, &Theron::Catcher<LogActor::Terminate>::Push);
	fw.Send(LogActor::Terminate(), receiver.GetAddress(), logger->GetAddress());
	while (terminateCatcher.Empty()){
		//Odotetaan, kunnes lokioliolla ei enää ole tehtävää
		receiver.Wait();
	}
}

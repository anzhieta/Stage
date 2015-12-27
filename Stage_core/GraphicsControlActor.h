#ifndef GRAPHICSCONTROLACTOR_H
#define GRAPHICSCONTROLACTOR_H

#include "stdafx.h"
#include <GraphicsController.h>
#include <CoreEvents.h>
#include <Model.h>
#include <Camera.h>
#include <glm\glm.hpp>
#include <iostream>


namespace stage {
	/** Grafiikkamoottoria hallinnoiva aktori-singleton. 
	Pitää huolen, että lista piirrettävistä 3D-malleista muodostetaan säieturvallisesti.
	Ottaa vastaan viestit:
	GraphicsControlActor::Queue (vastaa AllDone)
	*/
	class GraphicsControlActor : public Theron::Actor{
		friend class Gameloop;
	public:
		/** Viesti, joka lisää uuden 3D-mallin seuraavassa ruudunpäivityksessä piirrettävien mallien listalle
		*/
		struct Queue : public ComponentEvent{
			/** Piirrettävä 3D-malli
			*/
			stage_common::Model* model;
			/** Sijainti pelimaailmassa, johon malli piirretään
			*/
			const glm::mat4 position;
			Queue(uint64_t msgID, stage_common::Model* mod, glm::mat4 pos, int senderComponent) :
				ComponentEvent(msgID, senderComponent, INVALID_COMPONENT_ID), model(mod), position(pos){}
		};

		/** Luo uuden grafiikkamoottoria hallinnoivan aktorin ja avaa peli-ikkunan
		@param fw			Tätä aktoria hallinnoiva Theron::Framework
		@param windowname	Luotavan peli-ikkunan nimi
		@param x			Ikkunan vaakaresoluutio
		@param y			Ikkunan pystyresoluutio
		*/
		GraphicsControlActor(Theron::Framework& fw, std::string windowname, int x, int y);
		/** Tuhoaa grafiikkamoottoria hallinnoivan aktorin
		*/
		~GraphicsControlActor();
		/** Antaa globaalin GraphicsControlActor-singletonin osoitteen
		@returns	Globaalin grafiikkamoottoria hallinnoivan aktorin osoite
		*/
		static Theron::Address getGlobalController(){ return globalController; }
	private:
		/** Globaalin grafiikkamoottoria hallinnoivan aktorin osoite
		*/
		static Theron::Address globalController;
		/** Grafiikkamoottoriolio
		*/
		stage_common::GraphicsController gc;

		/** Antaa osoittimen varsinaiseen grafiikkamoottoriolioon.
		Tarvitaan kuvan piirtämistä varten, koska OpenGL-kontekstit ovat säiekohtaisia,
		joten 3D-mallit on piirrettävä siinä säikeessä, joka alun perin käynnisti grafiikkamoottorin.
		@returns	Osoitin grafiikkamoottoriolioon
		*/
		stage_common::GraphicsController* getRawController() { return &gc; }
		/** Kertoo, onko käyttäjä pyytänyt ohjelmaa pysähtymään.
		@returns	True, jos ohjelman suoritus tulisi pysäyttää
		*/
		bool shouldClose(){ return gc.shouldStop(); }

		/** Lisää uuden 3D-mallin seuraavassa ruudunpäivityksessä piirrettävien mallien listalle
		@param msg	Viesti, joka sisältää osoittimen piirrettävään malliin sekä sijainnin, johon malli piirretään
		*/
		void queue(const Queue& msg, Theron::Address sender);
	};
}

#endif
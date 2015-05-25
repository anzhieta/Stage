#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "GameObject.h"
#include <EventChannel.h>
#include <CoreEvents.h>
#include <iostream>

namespace stage {
	/** Luokka, joka mallintaa pelimaailman alueen.
	Ottaa vastaan viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	Scene::CreateObject (palauttaa Scene::NewObject)
	*/
	class Scene : public Theron::Actor{
		friend class SceneManager;
	public:
		/** Viesti, jolla pyydetään luomaan pelimaailmaan uusi peliolio
		*/
		struct CreateObject : Event{
			CreateObject(uint64_t id) : Event(id){}
		};

		/** Viesti, joka palauttaa luodun peliolion osoitteen
		*/
		struct NewObject : Event{
			NewObject(uint64_t id, Theron::Address obj) : Event(id), object(obj){}
			/** Uuden olion osoite
			*/
			Theron::Address object;
		};

		/** Tuhoaa pelialueen ja kaikki sen sisältämät pelioliot
		*/
		~Scene();
	private:
		
		/** Lista pelialueen sisältämistä peliolioista
		*/
		std::list<GameObject*> objects;
		/** Pelialueen lähettämien viestien kontekstilista
		*/
		ContextTracker tracker;

		/** Luo uuden pelialueen
		@param fw	Pelialuetta hallinnoiva Theron::Framework
		*/
		Scene(Theron::Framework &fw);
		
		/** Luo pelialueelle uuden peliolion
		@param msg		Peliolion luontia pyytävä viesti
		@param sender	Viestin lähettäjä
		*/
		void createObject(const CreateObject &msg, Theron::Address sender);

		/** Päivittää pelialueen tilan pyytämällä sen peliolioita päivittämään tilansa
		@param msg		Päivityspyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void update(const Update &msg, Theron::Address sender);

		/** Piirtää pelialueen ruudulle pyytämällä sen peliolioita piirtämään itsensä
		@param msg		Piirtopyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void render(const Render &msg, Theron::Address sender);

		/** Pitää kirjaa peliolioiden laskennan edistymisestä
		@param msg	Ilmoitus peliolion laskennan onnistumisesta
		@param from	Ilmoituksen lähettäjä
		*/
		void allDone(const AllDone &msg, Theron::Address sender);

		/** Käsittelee peliolioiden laskennassa tapahtuneet virheet
		@param msg	Ilmoitus peliolion laskennan epäonnistumisesta
		@param from	Ilmoituksen lähettäjä
		*/
		void error(const Error &msg, Theron::Address sender);
		
	};
}

#endif
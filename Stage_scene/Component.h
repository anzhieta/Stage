#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "stdafx.h"
#include "GameObject.h"
#include <CoreEvents.h>
#include <ContextTracker.h>
#include <iostream>

namespace stage{
	
	/** Abstrakti pelimoottorikomponenttien yliluokka.
	Kaikki pelimoottoreihin liitettävät komponentit perivät tämän luokan.
	Ottaa vastaan viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	Component::GetComponentID (palauttaa Component::ComponentID)
	GameObject::GetComponent (palauttaa GameObject::ComponentFound tai AllDone)
	*/
	class Component{
	public:
		/** Viesti, joka kertoo lähettäjänsä tyyppitunnuksen
		*/
		struct ComponentID : Event{
			/** Lähettäjän tyyppitunnus
			*/
			unsigned int compID;
			ComponentID(uint64_t id, int compID) : Event(id), compID(compID){}
		};

		/** Viesti, jolla pyydetään komponenttia lähettämään tyyppitunnuksensa
		*/
		struct GetComponentID : Event{
			GetComponentID(uint64_t id) : Event(id){}
		};

		/** Luo uuden pelimoottorikomponentin.
		HUOM: luo komponenttiolio aina new:llä äläkä tuhoa sitä itse.
		Komponenttiolio tuhotaan aina automaattisesti, kun sen omistava peliolio tuhotaan.
		@param fw		Komponenttia hallinnoiva Theron::Framework
		@param owner	Komponentin omistava peliolio
		*/
		Component(Theron::Framework &fw, Theron::Address owner);
	protected:
		/**	Komponentin omistava peliolio
		*/
		Theron::Address owner;

		/** Komponentin tapahtumakontekstien tilaa ylläpitävä olio
		*/
		ContextTracker tracker;

		/** Päivittää komponentin tilan.
		Jos tämä metodi ylikirjoitetaan aliluokassa, komponentti suorittaa laskentaa
		pelisilmukan päivitysvaiheessa.
		@param up	Päivityspyyntö
		@param from	Pyynnön lähettäjä
		*/
		virtual void update(const Update &up, Theron::Address from);

		/** Piirtää komponentin ruudulle.
		Jos tämä metodi ylikirjoitetaan aliluokassa, komponentti suorittaa laskentaa
		pelisilmukan piirtovaiheessa.
		@param up	Piirtopyyntö
		@param from	Pyynnön lähettäjä
		*/
		virtual void render(const Render &rend, Theron::Address from);

		/** Oletuskäsittelijä AllDone-viesteille (laskee viestiin liittyvän kontekstin
		odotettujen vastausten määrää yhdellä)
		Voidaan ylikirjoittaa aliluokassa.
		@param msg	AllDone-viesti
		@param from	Viestin lähettäjä
		*/
		virtual void allDone(const AllDone& msg, Theron::Address from);

		/** Oletuskäsittelijä Error-viesteille (kutsuu viestiin liittyvän kontekstin
		Error-metodia ja poistaa kontekstin)
		Voidaan ylikirjoittaa aliluokassa.
		@param msg	Error-viesti
		@param from	Viestin lähettäjä
		*/
		virtual void error(const Error& msg, Theron::Address from);

		/** Abstrakti metodi, joka palauttaa komponentin tyyppitunnuksen
		@returns	Komponentin tyypistä riippuva tunnusluku
		*/
		virtual int id() = 0;

		template <class ComponentType, class ValueType>
		void RegisterHandler(ComponentType* const comp, void (ComponentType::*handler)(const ValueType &message, const Theron::Address from)){
			owner->registerComponentHandler(comp, handler);
		}

	private:
		/** Kysyy komponentilta, onko se tiettyä tyyppiä
		@param msg	Etsityn tyypin tyyppitunnuksen sisältävä viesti
		@param from	Viestin lähettäjä
		Vastaa viestillä GameObject::ComponentFound jos tämä komponentti on etsittyä tyyppiä,
		muutoin viestillä AllDone
		*/
		void isType(const GameObject::GetComponent &msg, Theron::Address from);

		/** Kysyy komponentilta sen tyyppitunnusta
		@param msg	Pyyntöviesti
		@param from	Pyynnön lähettäjä
		*/
		void getId(const GetComponentID &msg, Theron::Address from);
	};
}

#endif
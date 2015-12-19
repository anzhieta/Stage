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
		friend class GameObject;
	public:
		/** Luo uuden pelimoottorikomponentin.
		HUOM: luo komponenttiolio aina new:llä äläkä tuhoa sitä itse.
		Komponenttiolio tuhotaan aina automaattisesti, kun sen omistava peliolio tuhotaan.
		@param fw		Komponenttia hallinnoiva Theron::Framework
		@param owner	Komponentin omistava peliolio
		*/
		Component(Theron::Framework &fw, Theron::Address owner);

		/** Abstrakti metodi, joka palauttaa komponentin tyyppitunnuksen
		@returns	Komponentin tyypistä riippuva tunnusluku
		*/
		virtual int id() = 0;

		virtual std::string name() = 0;

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
				
	protected:
		/**	Komponentin omistava peliolio
		*/
		GameObject* owner = nullptr;

		/** Komponentin tapahtumakontekstien tilaa ylläpitävä olio
		*/
		ContextTracker tracker;					

		template <class ComponentType, class EventType, void (ComponentType::* handler)(const EventType &message, const Theron::Address from)>
		void RegisterHandler(){
			owner->registerComponentHandler<ComponentType, EventType, handler>();
		}

		template <class EventType>
		void Send(const EventType& e, const Theron::Address& address){
			owner->Send(e, address);
		}

		void finishPhase(uint64_t id);
		void abortPhase(uint64_t id);
		uint64_t createContext(uint64_t oldid, Destination origSender, int responseCount = 1);
		virtual void initialize(GameObject* owner);
	private:
	};
}

#endif
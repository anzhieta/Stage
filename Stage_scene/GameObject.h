﻿#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "stdafx.h"
#include <Theron\Theron.h>
#include <string>
#include <list>
#include <CoreEvents.h>
#include <iostream>
#include <ContextTracker.h>

namespace stage{
	
	class Component;

	/** Luokka, joka mallintaa yksittäisen peliolion, joka kokoaa yhteen joukon toisiinsa liittyviä komponentteja
	Ottaa vastaan viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	GameObject::AddComponent (ei palauta vastausta)
	GameObject::GetComponent (palauttaa GameObject::ComponentFound tai Error)
	*/
	class GameObject : public Theron::Actor{
		friend class Component;
	public:
		/** Viesti, joka pyytää pelioliota liittämään itseensä komponentin
		Liitetty komponentti siirtyy peliolion hallintaan ja tuhotaan automaattisesti, kun peliolio tuhoutuu
		*/
		struct AddComponent : Event {
			/** Osoitin liitettävään komponenttiolioon
			*/
			Component* component;
			Destination notifyDestination;
			AddComponent(Component* component, uint64_t id, Destination notifyDestination) : 
				component(component), Event(id), notifyDestination(notifyDestination){}
		};
		
		/** Luo uuden tyhjän peliolion
		@param fw	Pelioliota hallinnoiva Theron::Framework
		*/
		GameObject(Theron::Framework &fw);

		/** Tuhoaa peliolion ja kaikki sen komponentit
		*/
		~GameObject();

		template <class ComponentType, class EventType, void(ComponentType::* handler)(const EventType &message, const Theron::Address from)>
		void registerComponentHandler(){
			RegisterHandler(this, &GameObject::handleComponentMessage<ComponentType, EventType, handler>);
		}

		template <class ComponentType, class EventType, void (ComponentType::* handler)(const EventType &message, const Theron::Address from)>
		void handleComponentMessage(const EventType& msg, Theron::Address from){
			for (std::list<Component*>::iterator i = components.begin(); i != components.end(); i++){
				if ((*i)->id() == msg.receiverComponent){
					ComponentType* c = (ComponentType*)(*i);
					(c->*handler)(msg, from);
				}
			}
		}
		void allDone(uint64_t id);
		void error(uint64_t id, const std::string& compname);
		Component* getComponent(int id);
	private:
		/** Peliolion kaikki komponentit sisältävä lista
		*/
		std::list<Component*> components;

		/** Peliolion viestikontekstilista
		*/
		ContextTracker tracker;

		/** Pyytää kaikkia peliolion komponentteja päivittämään tilansa
		@param msg	Päivityspyyntö
		@param from	Pyynnön lähettäjä
		*/
		void update(const Update &msg, Theron::Address from);

		/** Pyytää kaikkia peliolion komponentteja piirtämään itsensä ruudulle
		@param msg	Piirtopyyntö
		@param from	Pyynnön lähettäjä
		*/
		void render(const Render &msg, Theron::Address from);

		/** Liittää peliolioon uuden komponentin
		@param msg	Liitettävän komponentin määrittelevä viesti
		@param from	Komponentin lisäyspyynnön lähettäjä
		*/
		void addComponent(const AddComponent &msg, Theron::Address from);

		/** Hakee peliolioon liitetyt tiettyä tyyppiä olevat komponentit
		@param msg	Haettavan komponenttityypin määrittelevä viesti
		@param from	Hakupyynnön lähettäjä
		*/
//		void getComponent(const GetComponent &msg, Theron::Address from);

		/** Palauttaa getComponent-funktion löytämän komponentin osoitteen alkuperäiselle pyytäjälle
		@param msg	Komponentin löytymisestä kertova viesti
		@param from	Komponentin löytymisestä kertovan aktorin osoite
		*/
//		void componentFound(const ComponentFound &msg, Theron::Address from);

		/** Pitää kirjaa komponenttien laskennan edistymisestä
		@param msg	Ilmoitus komponentin laskennan onnistumisesta
		@param from	Ilmoituksen lähettäjä
		*/
		void allDone(const AllDone &msg, Theron::Address from);

		/** Käsittelee komponenttien laskennassa tapahtuneet virheet
		@param msg	Ilmoitus komponentin laskennan epäonnistumisesta
		@param from	Ilmoituksen lähettäjä
		*/
		void error(const Error &msg, Theron::Address from);
		
	};
}

#endif
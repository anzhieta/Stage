#pragma once

#ifndef CONTEXTTRACKER_H
#define CONTEXTTRACKER_H

#include "stdafx.h"
#include "EventContext.h"
#include "CoreEvents.h"
#include <unordered_map>

namespace stage{
	/** Luokka, joka ylläpitää usean viestin käsittelyn ajan säilyviä konteksteja.
	@see stage::EventContext
	*/
	class ContextTracker{
	public:
		/** Luo uuden kontekstilistan.
		@param fw		Se Theron::Framework, joka ylläpitää listan omistajaa
		@param owner	Listan omistajan Theron-osoite
		*/
		ContextTracker(Theron::Framework& fw, Theron::Address owner) : fw(fw), owner(owner), pending(){	}

		/** Luo uuden tapahtumakontekstin.
		@param oldID			Sen viestin tunnus, jonka seurauksena tämä konteksti luodaan
		@param newID			Tähän kontekstiin liittyvien viestien tunnus
		@param originalSender	Alkuperäisen viestin lähettäjä
		@param responseCount	Tähän viestiin odotettavien vastauksien määrä (oletus 1)
		@returns				Viite luotuun kontekstiolioon
		*/
		EventContext& addContext(uint64_t oldID, uint64_t newID, Theron::Address originalSender, int responseCount = 1);

		/** Hakee viitteen haluttuun kontekstiin
		HUOM: jos haluttua kontekstia ei ole, voi aiheuttaa virhetilanteen. 
		Tarkista tarvittaessa kontekstin olemassaolo contains()-metodilla.
		@param id	Haettavan kontekstin tunnus
		@returns	Viite haluttuun kontekstiin
		*/
		EventContext& getContext(uint64_t id);

		/** Lisää halutun kontekstin vastausten määrää yhdellä
		HUOM: jos haluttua kontekstia ei ole, voi aiheuttaa virhetilanteen.
		Tarkista tarvittaessa kontekstin olemassaolo contains()-metodilla.
		@param id	Sen kontekstin tunnus, jonka vastausten määrää kasvatetaan
		*/
		void increment(uint64_t id);

		/** Muuttaa halutun kontekstin vastausten määrää
		HUOM: jos haluttua kontekstia ei ole, voi aiheuttaa virhetilanteen.
		Tarkista tarvittaessa kontekstin olemassaolo contains()-metodilla.
		@param id		Sen kontekstin tunnus, jonka vastausten määrää muutetaan
		@param count	Uusi vastausmäärä
		*/
		void setResponseCount(uint64_t id, unsigned int count);

		/** Vähentää halutun kontekstin vastausten määrää yhdellä
		Jos tämä laskee vastausten määrän nollaan, kontekstin lopetusmetodi suoritetaan ja konteksti poistetaan listasta
		HUOM: jos haluttua kontekstia ei ole, voi aiheuttaa virhetilanteen.
		Tarkista tarvittaessa kontekstin olemassaolo contains()-metodilla.
		@param id	Sen kontekstin tunnus, jonka vastausten määrää lasketaan
		*/
		void decrement(uint64_t id);

		/** Poistaa halutun kontekstin kontekstilistasta
		@param id	Poistettavan kontekstin tunnus
		*/
		void remove(uint64_t id);

		template <class MessageType>
		/** Lähettää viestin, luo tarvittaessa sille uuden kontekstin ja kasvattaa olemassaolevan kontekstin
		odotettujen vastausten määrää yhdellä.
		@param oldID			Sen viestin tunnus, jonka seurauksena tämä konteksti luodaan
		@param ev				Lähetettävä viesti, oltava Event-luokan aliluokka
		@param recipient		Viestin vastaanottajan osoite
		@param originalSender	Alkuperäisen viestin lähettäjä
		@returns				Viite luotuun kontekstiolioon
		*/
		void trackedSend(uint64_t oldID, const MessageType& ev, Theron::Address recipient, Theron::Address originalSender){
			if (!contains(ev.id)){
				addContext(oldID, ev.id, originalSender, 0);
			}
			pending[ev.id].responseCount++;
			fw.Send(ev, owner, recipient);
		}

		template <typename T>
		void setVariable(uint64_t id, int index, T var){
			boost::any content = var;
			pending[id].setVar(index, content);
		}

		template <typename T>
		T getVariable(uint64_t id, int index){
			return boost::any_cast<T>(pending[id].getVar(index));
		}

		/** Generoi uuden viestitunnuksen
		@param returns	Viestin uniikisti määrittelevä tunnusarvo
		*/
		uint64_t getNextID();

		/** Tarkistaa, onko halutulla avainarvolla tallennettu kontekstilistaan kontekstia
		@param id	Tutkittava avainarvo
		@returns	True, jos arvolla on tallennettu listaan konteksti
		*/
		bool contains(uint64_t id);
	private:
		/** Tietorakenne, joka pitää kirjaa isäntäolion avoimista konteksteista
		*/
		std::unordered_map<uint64_t, EventContext> pending;

		/** Uniikkien viestitunnusten generointiin käytettävä laskuri
		*/
		uint32_t lastID = 0;
		
		/** Viite siihen Theron::Framework-olioin, joka hallinnoi tämän olion omistajaoliota
		*/
		Theron::Framework& fw;

		/** Tämän olion omistajaolion Theron-osoite
		*/
		Theron::Address owner;
	};
}

#endif
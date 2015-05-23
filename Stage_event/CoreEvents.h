#ifndef COREEVENTS_H
#define COREEVENTS_H

#include "stdafx.h"

namespace stage {
	/** Tietue, jonka kaikki pelimoottorin viestit perivät.
	Sisältää viestikohtaisen viestitunnuksen.
	*/
	struct Event {
		/** Luo uuden viestin.
		@param originator	Viestin luojan Theron-osoite
		@param msgID		32-bittinen tunnus, joka yksilöi viestin muiden saman lähettäjän viestien joukossa
		*/
		Event(Theron::Address originator, uint32_t msgID){
			id = generateID(originator, msgID);
		}

		/** Luo uuden viestin.
		@param msgID	64-bittinen tunnus, joka yksilöi viestin kaikkien pelimoottorin viestien joukossa
		*/
		Event(uint64_t msgID){
			id = msgID;
		}

		/** Viestin yksilöivä tunnusluku
		*/
		uint64_t id;

		/** Apufunktio, joka muodostaa osoitteesta ja oliokohtaisesti uniikista 32-bittisestä tunnuksesta
		pelimoottorin sisällä yksilöllisen 64-bittisen tunnuksen
		@param originator	Tunnuksen luojan osoite
		@param msgID		Tunnuksen luojan lähettämien viestien joukossa yksilöllinen tunnusluku
		@returns			64-bittinen luku, joka yksilöi viestin pelimoottorin sisällä
		*/
		static uint64_t generateID(Theron::Address originator, uint32_t msgID){
			uint64_t id = originator.AsInteger();
			id = id << 32;
			id = id | msgID;
			return id;
		}
	};

	/** Viesti, jolla pyydetään pelioliota tai komponenttia päivittämään sisäinen tilansa uutta
	ruudunpäivitystä vastaavaksi
	*/
	struct Update : Event {
		/** Edellisestä ruudunpäivityksestä kulunut aika millisekunteina
		*/
		float elapsedMS;
		Update(float ms, uint64_t msgID) : elapsedMS(ms), Event(msgID){}
		Update(float ms, Theron::Address originator, uint32_t msgID) : elapsedMS(ms), Event(originator, msgID){}
	};

	/** Viesti, jolla pyydetään pelioliota tai komponenttia piirtämään itsensä ruudulle
	*/
	struct Render : Event {
		Render(uint64_t msgID) : Event(msgID){}
		Render(Theron::Address originator, uint32_t msgID) : Event(originator, msgID){}
	};

	/** Viesti, joka ilmaisee pyydetyn aktorilaskennan päättyneen
	Viestin id-kenttä ilmaisee sen viestin, jonka käsittely on päättynyt
	*/
	struct AllDone : Event {
		AllDone(uint64_t msgID) : Event(msgID){}
	};

	/** Viesti, joka ilmaisee, että pyydetyssä aktorilaskennassa on kohdattu virhetilanne
	Viestin id-kenttä ilmaisee sen viestin, jonka käsittely on epäonnistunut
	*/
	struct Error : Event {
		Error(uint64_t msgID) : Event(msgID){}
	};
}

#endif
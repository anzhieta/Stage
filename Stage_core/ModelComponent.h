#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#define MODEL_ID 3

#include "stdafx.h"
#include <Model.h>
#include <Component.h>
#include <Transform.h>
#include "GraphicsControlActor.h"
#include <iostream>

namespace stage{
	/** Peliolioon liitettävä 3D-mallikomponentti.
	Kun olioon liitetään tämä komponentti, sen sijaintiin piirretään 3D-malli joka ruudunpäivityksellä.
	Jokaisella piirrettävällä pelioliolla tulee olla oma mallikomponentti, mutta usea mallinkomponentti
	voi käyttää samaa 3D-mallioliota.
	Ottaa vastaan viestit:
	Update (vastaa AllDone)
	Render (vastaa AllDone)
	*/
	class ModelComponent : public Component{
	public:
		/** Luo uuden mallikomponentin. Katso oikea käyttö yliluokasta.
		@see			stage::Component
		@param fw		Theron::Framework, jonka alaisuudessa tämä komponentti toimii
		@param mod		Osoitin siihen 3D-malliin, joka halutaan piirtää
		@param owner	Sen peliolion osoite, joka omistaa tämän komponentin
		*/
		ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner, Destination notifyDest, uint64_t notifyID);

		/** Hakee olion komponenttitunnuksen
		@returns	Tämän komponentin tunnus
		*/
		virtual int id(){ return MODEL_ID; }
		virtual std::string name() { return std::string("3D model"); }

		/** Suorittaa loppuun komponentin käynnistyksen
		@param msg		Sijaintiolion komponenttitunnuksen sisältävä viesti
		@param sender	Sijaintiolion osoite
		*/
		void initialize(GameObject* owner);
	private:

		/** Osoitin siihen 3D-malliin, joka piirretään, kun tälle oliolle lähetetään Render-viesti
		*/
		stage_common::Model* mod;

		/** Omistajaolion sijaintia ylläpitävän olion osoite
		*/
		Transform* transform;		

		/** Piirtää ruudulle tämän komponentin 3D-mallin
		@param msg		Renderöintipyyntö
		@param sender	Lähettäjän osoite
		*/
		void render(uint64_t id);
		
	};
}

#endif
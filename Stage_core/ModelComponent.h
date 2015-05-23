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
		ModelComponent(Theron::Framework& fw, stage_common::Model* mod, Theron::Address owner);

		/** Hakee olion komponenttitunnuksen
		@returns	Tämän komponentin tunnus
		*/
		virtual int id(){ return MODEL_ID; }
	private:
		/** Onko komponentti käynnistetty, eli voiko se suorittaa update- ja render-kutsuja
		*/
		bool init = false;

		/** Osoitin siihen 3D-malliin, joka piirretään, kun tälle oliolle lähetetään Render-viesti
		*/
		stage_common::Model* mod;

		/** Omistajaolion sijaintia ylläpitävän olion osoite
		*/
		Theron::Address transform;

		/** Suorittaa loppuun komponentin käynnistyksen
		@param msg		Sijaintiolion komponenttitunnuksen sisältävä viesti
		@param sender	Sijaintiolion osoite
		*/
		void initialize(const GameObject::ComponentFound& msg, Theron::Address sender);

		/** Piirtää ruudulle tämän komponentin 3D-mallin
		@param msg		Renderöintipyyntö
		@param sender	Lähettäjän osoite
		*/
		void render(const Render& msg, Theron::Address sender);

		/** Suorittaa loppuun tarvittavan laskennan mallin piirtoa varten
		@param msg		3D-mallin sijainti pelimaailmassa
		@param sender	Lähettäjän osoite
		*/
		void completeRender(const Transform::Matrix& msg, Theron::Address sender);
		
	};
}

#endif
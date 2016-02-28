#pragma once

#ifndef STATICGEOMETRYCOMPONENT_H
#define STATICGEOMETRYCOMPONENT_H

#define STATICGEOMETRYCOMPONENT_ID 5

#include "PhysicsComponent.h"

namespace stage{
	/** Luokka, joka esittää staattista pelimaailman tasogeometriaa, kuten maastoa tai seiniä.
	Käsittelee viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	PhysicsComponent::CollisionCheck (palauttaa AllDone)
	*/
	class StaticGeometryComponent : public Component{
	public:
		/** Luo uuden staattisen törmäyskomponentin pallotörmäyshahmolla
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					Pallotörmäyshahmon säde
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, float radius, Theron::Address transform,
			Theron::Address collisionEventChannel);
		/** Luo uuden staattisen törmäyskomponentin AABB-törmäyshahmolla (Axis-aligned bounding box)
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					AABB-törmäyshahmon koko
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, glm::vec3 size, Theron::Address transform,
			Theron::Address collisionEventChannel);
		/** Hakee staatisen törmäyshahmokomponentin komponenttitunnuksen
		@returns	Komponentin tunnusluku
		*/
		int id(){ return STATICGEOMETRYCOMPONENT_ID; }
		/** Tuhoaa staattisen törmäyshahmokomponentin*/
		~StaticGeometryComponent();
	private:
		/** Komponentin törmäyshahmo*/
		stage_common::Collider* collider;
		/** Komponentin isäntäolion sijaintia ylläpitävän olion Theron-osoite*/
		Theron::Address transform;
		/** Törmäysviestikanavan osoite*/
		Theron::Address collisionEventChannel;
		/** Onko tämän komponentin alustus suoritettu loppuun*/
		bool init = false;

		//--Kontekstiyksikkö alkaa--

		/** Päivittää komponentin tilan
		@param up		Tilanpäivityspyyntö
		@param sende	Pyynnön lähettäjä
		*/
		void update(const Update &up, Theron::Address from);
		/** Suorittaa tilanpäivityksen loppuun
		@param msg	Peliolion sijainnin sisältävä viesti
		@param from	Viestin lähettäjä
		*/
		void finishUpdate(const Transform::Position& msg, Theron::Address from);

		//--Kontekstiyksikkö päättyy--

		/** Tarkistaa onko fysiikkaolio törmännyt tähän olioon
		@param msg	Viesti, joka sisältää fysiikkaolion törmäyshahmon tiedot
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const PhysicsComponent::CollisionCheck& msg, Theron::Address from);
		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi pallo
		@param msg		Peliolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender);
		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi AABB (Axis-Aligned Bounding Box)
		@param msg		Peliolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender);
		/** Molemmille konstruktoreille yhteiset alustukset suorittava metodi
		@returns	Konteksti-ID, jota käytetään alustusviestien tunnistamiseen
		*/
		uint64_t setup();
	};
}
#endif
#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#define PHYSICSCOMPONENT_ID 4

#include "stdafx.h"
#include <Component.h>
#include <Collisions.h>
#include <Collider.h>
#include <SphereCollider.h>
#include <AABBCollider.h>
#include <Transform.h>
#include <EventChannel.h>
#include <LogActor.h>
#include <unordered_set>

namespace stage{
	/** Komponentti, joka tekee isäntäoliostaan fysiikkaolion, joka liikkuu pelimaailmassa vakionopeudella
	ja voi törmätä muihin fysiikkaolioihin sekä staattiseen tasogeometriaan
	Käsittelee viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	PhysicsComponent::CollisionCheck (palauttaa AllDone)
	PhysicsComponent::CollisionDetected (palauttaa AllDone jos törmäys on jo käsittelyssä tai FinishCollision)
	PhysicsComponent::StaticCollision (palauttaa AllDone)
	*/
	class PhysicsComponent : public Component{
	public:
		//---Viestit---

		/** Viesti, joka ilmoittaa lähettäneen fysiikkaolion sijainnin muuttuneen ja pyytää muita fysiikkaolioita
		tarkistamaan mahdolliset törmäykset	*/
		struct CollisionCheck : public Event{
			/** Lähettäjän törmäyshahmo*/
			const stage_common::Collider& coll;
			/** Alkuperäisen lähettäjän osoite*/
			const Theron::Address originator;
			/** Edellisestä ruudunpäivityksestä kulunut aika millisekunteina*/
			float elapsedMS;
			CollisionCheck(uint64_t id, stage_common::Collider& coll, Theron::Address originator, float elapsedMS)
				: Event(id), coll(coll), originator(originator), elapsedMS(elapsedMS){}
		};
		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt lähettäjään*/
		struct CollisionDetected : public Event{
			/** Lähettäjän törmäyshahmo	*/
			const stage_common::Collider& coll;
			/** Lähettäjän nopeus*/
			glm::vec3 otherVelocity;
			/** Lähettäjän massa*/
			float otherMass;
			CollisionDetected(uint64_t id, stage_common::Collider& coll, glm::vec3 otherVelocity, float otherMass)
				: Event(id), coll(coll), otherVelocity(otherVelocity), otherMass(otherMass){}
		};
		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt liikkumattomaan peliolioon*/
		struct StaticCollision : public Event{
			/** Lähettäjän törmäyshahmo*/
			const stage_common::Collider& coll;
			StaticCollision(uint64_t id, stage_common::Collider& coll) : Event(id), coll(coll){}
		};
		/** Viesti, joka ilmoittaa törmäyksen käsittelyn onnistuneen*/
		struct FinishCollision : public Event{
			/** Vastaanottajan nopeuteen tehtävä muutos*/
			glm::vec3 velocityAdjustment;
			FinishCollision(uint64_t id, glm::vec3 velAdj) : Event(id), velocityAdjustment(velAdj){}
		};

		//---Metodit---

		/** Luo uuden fysiikkakomponentin pallotörmäyshahmolla
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					Pallotörmäyshahmon säde
		@param initialV					Fysiikkaolion liikesuunta ja nopeus simulaation alussa
		@param mass						Fysiikkaolion massa
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform,
			float radius, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel);
		/** Luo uuden fysiikkakomponentin AABB-törmäyshahmolla
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param size						Törmäyshahmon koko
		@param initialV					Fysiikkaolion liikesuunta ja nopeus simulaation alussa
		@param mass						Fysiikkaolion massa
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform,
			glm::vec3 size, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel);
		/** Tuhoaa fysiikkakomponentin*/
		~PhysicsComponent();
		/** Hakee fysiikkakomponentin komponenttitunnuksen
		@returns	Komponentin tunnus
		*/
		virtual int id(){ return PHYSICSCOMPONENT_ID; }
	private:
		//---Kentät---
		/** Viite fysiikkamoottorin viestien käyttämään viestikanavaan*/
		EventChannel<CollisionCheck>& collisionEventChannel;
		/** Fysiikkaolion törmäyshahmo*/
		stage_common::Collider* collider;
		/** Kopio fysiikkaolion törmäyshahmosta.
		Käytetään törmäystentunnistuksessa varmistamaan, että törmäyshahmon muokkaaminen ei aiheuta rinnakkaisuusongelmia*/
		stage_common::Collider* tempCollider;
		/** Isäntäolion sijaintia ylläpitävän olion osoite*/
		Theron::Address transform;
		/** Fysiikkaolion liikesuunta ja nopeus*/
		glm::vec3 velocity;
		/** Fysiikkaolion sijainti ruudunpäivityksen alussa*/
		glm::vec3 oldPos;
		/** Fysiikkaolion massa*/
		float mass;
		/** Onko fysiikkaolion tila päivitetty nykyisen ruudunpäivityksen aikana*/
		bool updatedThisFrame = false;
		/** Onko komponentin alustus suoritettu loppuun*/
		bool init = false;
		/** Niiden fysiikkaolioiden joukko, joihin on törmätty tämän ruudunpäivityksen aikana.
		Käytetään estämään saman törmäyksen käsitteleminen kahdesti, jos kumpikin fysiikkaolio
		käsittelee sitä samanaikaisesti rinnakkain.*/
		std::unordered_set<int> collidedThisFrame;

		//---Metodit---

		/** Molemmille konstruktoreille yhteiset alustukset suorittava metodi
		@returns	Konteksti-ID, jota käytetään alustusviestien tunnistamiseen
		*/
		uint64_t setup();
		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi pallo
		@param msg		Fysiikkaolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender);
		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi AABB (Axis-Aligned Bounding Box)
		@param msg		Fysiikkaolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender);

		//--Kontekstiyksikkö alkaa--

		/** Päivitetään tila uutta ruudunpäivitystä vastaavaksi
		@param up	Päivityspyyntö
		@param from	Pyynnön lähettäjä
		*/
		void update(const Update &up, Theron::Address from);
		/** Suorittaa loppuun tilanpäivityksen päivittämällä muuttuneen sijainnin sijaintikomponenttiin
		@param msg	Isäntäolion nykyisen sijainnin ilmoittava viesti
		@param from	Viestin lähettäjä
		*/
		void finishUpdate(const Transform::Position& msg, Theron::Address from);

		//--Kontekstiyksikkö päättyy--

		//--Kontekstiyksikkö alkaa--

		/**Tarkistaa, onko tämä fysiikkolio törmännyt toiseen
		@param msg	Toisen fysiikkaolion törmäyshahmon sisältävä viesti
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const CollisionCheck& msg, Theron::Address from);
		/** Suorittaa loppuun törmäystapahtuman käsittelyn
		@param msg	Viesti, joka ilmoittaa törmäystapahtuman käsittelyn päättyneen
					ja sisältäää törmäyksen aiheuttaman muutoksen tämän kappaleen nopeuteen
		@param from	Viestin lähettäjä
		*/
		void finishCollision(const FinishCollision& msg, Theron::Address from);

		//--Kontekstiyksikkö päättyy--

		/** Käsittelee havaitun törmäyksen kahden fysiikkaolion välillä
		@param msg	Törmäyksestä kertova viesti
		@param from	Toinen törmännyt fysiikkaolio
		*/
		void collisionDetected(const CollisionDetected& msg, Theron::Address from);
		/** Käsittelee havaitun törmäyksen fysiikkaolion ja staattisen törmäyshahmon välillä
		@param msg	Törmäyksestä kertova viesti
		@param from	Staattinen törmäyshahmokomponentti, johon törmättiin
		*/
		void staticCollision(const StaticCollision& msg, Theron::Address from);
		/** Siirtää törmäyshahmoa fysiikkaolion nykyisen nopeuden mukaisesti
		@param elapsedMS	Edellisestä ruudunpäivityksestä kulunut aika millisekunteina
		*/
		void updatePosition(float elapsedMS);
		/** Suorittaa pelisilmukan piirtovaiheen laskennan
		@param rend	Piirtopyyntöviesti
		@param from	Viestin lähettäjä
		*/
		void render(const Render &rend, Theron::Address from);
	};
}
#endif
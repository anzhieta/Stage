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
			Theron::Address collisionEventChannel) : Component(fw, owner), transform(transform), collisionEventChannel(collisionEventChannel){
			//Rekisteröidään käsittelijä alustuksen suorittamiselle loppuun
			RegisterHandler(this, &StaticGeometryComponent::finishSphereSetup);
			//Konstruktoreille yhteiset alustukset
			uint64_t id = setup();
			//Talletetaan törmäyshahmon säde kontekstimuuttujaan
			tracker.setVariable<float>(id, 0, radius);
			Send(Transform::GetPosition(id), transform);
			//Suoritus jatkuu metodissa finishSphereSetup
		}

		/** Luo uuden staattisen törmäyskomponentin AABB-törmäyshahmolla (Axis-aligned bounding box)
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					AABB-törmäyshahmon koko
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, glm::vec3 size, Theron::Address transform,
			Theron::Address collisionEventChannel) : Component(fw, owner), transform(transform), collisionEventChannel(collisionEventChannel){
			//Rekisteröidään käsittelijä alustuksen suorittamiselle loppuun
			RegisterHandler(this, &StaticGeometryComponent::finishAABBSetup);
			//Konstruktoreille yhteiset alustukset
			uint64_t id = setup();
			//Talletetaan törmäyshahmon koko kontekstimuuttujaan
			tracker.setVariable<glm::vec3>(id, 0, size);
			Send(Transform::GetPosition(id), transform);
			//Suoritus jatkuu metodissa finishAABBSetup
		}
				
		/** Hakee staatisen törmäyskomponentin komponenttitunnuksen
		@returns	Komponentin tunnusluku
		*/
		int id(){
			return STATICGEOMETRYCOMPONENT_ID;
		}

		/** Tuhoaa staattisen törmäyskomponentin
		*/
		~StaticGeometryComponent(){
			delete collider;
		}
	private:
		/** Komponentin törmäyshahmo
		*/
		stage_common::Collider* collider;

		/** Komponentin isäntäolion sijaintia ylläpitävän olion Theron-osoite
		*/
		Theron::Address transform;

		/** Törmäysviestikanavan osoite
		*/
		Theron::Address collisionEventChannel;

		/** Onko tämän komponentin alustus suoritettu loppuun
		*/
		bool init = false;

		//--Kontekstiyksikkö alkaa--

		/** Päivittää komponentin tilan
		@param up		Tilanpäivityspyyntö
		@param sende	Pyynnön lähettäjä
		*/
		void update(const Update &up, Theron::Address from){
			if (!init){
				//Ei tehdä mitään, jos alustusta ei vielä ole suoritettu loppuun
				Send(AllDone(up.id), from);
				return;
			}
			//Haetaan uudestaan peliolion sijainti, sillä jokin muu komponentti on ehkä
			//muuttanut sitä
			uint64_t id = tracker.getNextID();
			tracker.addContext(up.id, id, from);
			Send(Transform::GetPosition(id), transform);
			//Suoritus jatkuu metodissa finishUpdate
		}

		/** Suorittaa tilanpäivityksen loppuun
		@param msg	Peliolion sijainnin sisältävä viesti
		@param from	Viestin lähettäjä
		*/
		void finishUpdate(const Transform::Position& msg, Theron::Address from){
			//Päivitetään törmäyshahmon sijainti
			collider->center = msg.position;
			//Poistetaan konteksti, jolloin vastausviesti Update-viestiin lähtee automaattisesti
			tracker.decrement(msg.id);
		}

		//--Kontekstiyksikkö päättyy--

		/** Tarkistaa onko fysiikkaolio törmännyt tähän olioon
		@param msg	Viesti, joka sisältää fysiikkaolion törmäyshahmon tiedot
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const PhysicsComponent::CollisionCheck& msg, Theron::Address from){
			if (!init){
				//Ei tehdä mitään, jos alustusta ei vielä ole suoritettu loppuun
				Send(AllDone(msg.id), from);
				return;
			}
			//Ei tehdä mitään, jos törmäystä ei ole tapahtunut
			if (!collider->checkCollision(msg.coll)) Send(AllDone(msg.id), from);
			else {
				//Luodaan konteksti törmäyksen käsittelyä varten
				uint64_t id = tracker.getNextID();
				tracker.addContext(msg.id, id, from);
				//Lähetetään ilmoitus törmäyksestä
				Send(PhysicsComponent::StaticCollision(id, *collider), msg.originator);
			}
		}

		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi pallo
		@param msg		Peliolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &StaticGeometryComponent::finishSphereSetup);
			RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
			//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla säteellä ja viestin sisältämällä sijainnilla
			collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
			init = true;
			tracker.decrement(msg.id);
		}
		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi AABB (Axis-Aligned Bounding Box)
		@param msg		Peliolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &StaticGeometryComponent::finishAABBSetup);
			RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
			//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla koolla ja viestin sisältämällä sijainnilla
			collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
			init = true;
			tracker.decrement(msg.id);
		}

		/** Molemmille konstruktoreille yhteiset alustukset suorittava metodi
		@returns	Konteksti-ID, jota käytetään alustusviestien tunnistamiseen
		*/
		uint64_t setup(){
			RegisterHandler(this, &StaticGeometryComponent::collisionCheck);
			//Luodaan uusi viestikonteksti alustuksen loppuunsuorittamista varten
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(0, id, Theron::Address::Null());
			context.finalize = [](){};
			context.error = context.finalize;
			//Rekisteröidään peliolio törmäystapahtumakanavan viestien vastaanottajaksi
			Send(EventChannel<PhysicsComponent::CollisionCheck>::RegisterRecipient(this->GetAddress()), collisionEventChannel);
			return id;
		}
	};
}

#endif
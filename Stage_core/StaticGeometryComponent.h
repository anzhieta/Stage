#pragma once

#ifndef STATICGEOMETRYCOMPONENT_H
#define STATICGEOMETRYCOMPONENT_H

#define STATICGEOMETRYCOMPONENT_ID 5

#include "PhysicsComponent.h"
#include <Collider.h>

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
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID, float radius,
			Theron::Address collisionEventChannel) : Component(fw, owner), collisionEventChannel(collisionEventChannel){

			collider = new stage_common::SphereCollider(radius, glm::vec3());
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		/** Luo uuden staattisen törmäyskomponentin AABB-törmäyshahmolla (Axis-aligned bounding box)
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					AABB-törmäyshahmon koko
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID, glm::vec3 size,
			Theron::Address collisionEventChannel) : Component(fw, owner), collisionEventChannel(collisionEventChannel){

			collider = new stage_common::AABBCollider(size, glm::vec3());
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		virtual void initialize(GameObject* owner){
			Component::initialize(owner);
			RegisterHandler<StaticGeometryComponent, PhysicsComponent::CollisionCheck, &StaticGeometryComponent::collisionCheck>();
			Send(EventChannel<PhysicsComponent::CollisionCheck>::RegisterRecipient(
				Destination(owner->GetAddress(), STATICGEOMETRYCOMPONENT_ID)), collisionEventChannel);
			transform = (Transform*)owner->getComponent(TRANSFORM_ID);
			collider->center = transform->getPosition();
		}
						
		/** Hakee staatisen törmäyskomponentin komponenttitunnuksen
		@returns	Komponentin tunnusluku
		*/
		int id(){
			return STATICGEOMETRYCOMPONENT_ID;
		}

		std::string name(){ return "Simple Static Level Geometry"; }

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
		Transform* transform;

		/** Törmäysviestikanavan osoite
		*/
		Theron::Address collisionEventChannel;
		
		//--Kontekstiyksikkö alkaa--

		/** Päivittää komponentin tilan
		@param up		Tilanpäivityspyyntö
		@param sende	Pyynnön lähettäjä
		*/
		void update(float elapsedMS, uint64_t id){
			//Haetaan uudestaan peliolion sijainti, sillä jokin muu komponentti on ehkä
			//muuttanut sitä
			collider->center = transform->getPosition();
			finishPhase(id);
		}

		//--Kontekstiyksikkö päättyy--

		/** Tarkistaa onko fysiikkaolio törmännyt tähän olioon
		@param msg	Viesti, joka sisältää fysiikkaolion törmäyshahmon tiedot
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const PhysicsComponent::CollisionCheck& msg, Theron::Address from){
			//Ei tehdä mitään, jos törmäystä ei ole tapahtunut
			if (!collider->checkCollision(msg.coll)){
				Send(AllDone(msg.id, STATICGEOMETRYCOMPONENT_ID, msg.senderComponent), from);
			}
			else {
				//Luodaan konteksti törmäyksen käsittelyä varten
				uint64_t id = createActorContext(msg.id, Destination(from, msg.senderComponent));
				//Lähetetään ilmoitus törmäyksestä
				Send(PhysicsComponent::StaticCollision(id, *collider, STATICGEOMETRYCOMPONENT_ID, msg.senderComponent), msg.originator);
			}
		}

	};
}

#endif
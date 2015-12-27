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
		/** Viesti, joka ilmoittaa lähettäneen fysiikkaolion sijainnin muuttuneen ja pyytää muita fysiikkaolioita
		tarkistamaan mahdolliset törmäykset
		*/
		struct CollisionCheck : public ComponentEvent{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			/** Alkuperäisen lähettäjän osoite
			*/
			const Theron::Address originator;
			/** Edellisestä ruudunpäivityksestä kulunut aika millisekunteina
			*/
			float elapsedMS;
			CollisionCheck(uint64_t id, stage_common::Collider& coll, Theron::Address originator, float elapsedMS, int senderComponent, int receiverComponent)
				: ComponentEvent(id, senderComponent, receiverComponent), coll(coll), originator(originator), elapsedMS(elapsedMS){}
		};

		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt lähettäjään
		*/
		struct CollisionDetected : public ComponentEvent{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			/** Lähettäjän nopeus
			*/
			glm::vec3 otherVelocity;
			/** Lähettäjän massa
			*/
			float otherMass;
			CollisionDetected(uint64_t id, stage_common::Collider& coll, glm::vec3 otherVelocity, float otherMass, int senderComponent, int receiverComponent)
				: ComponentEvent(id, senderComponent, receiverComponent), coll(coll), otherVelocity(otherVelocity), otherMass(otherMass){}
		};

		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt liikkumattomaan peliolioon
		*/
		struct StaticCollision : public ComponentEvent{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			StaticCollision(uint64_t id, stage_common::Collider& coll, int senderComponent, int receiverComponent) : 
				ComponentEvent(id, senderComponent, receiverComponent), coll(coll){}
		};

		/** Viesti, joka ilmoittaa törmäyksen käsittelyn onnistuneen
		*/
		struct FinishCollision : public ComponentEvent{
			/** Vastaanottajan nopeuteen tehtävä muutos
			*/
			glm::vec3 velocityAdjustment;
			FinishCollision(uint64_t id, glm::vec3 velAdj, int senderComponent, int receiverComponent) : 
				ComponentEvent(id, senderComponent, receiverComponent), velocityAdjustment(velAdj){}
		};

		/** Luo uuden fysiikkakomponentin pallotörmäyshahmolla
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param radius					Pallotörmäyshahmon säde
		@param initialV					Fysiikkaolion liikesuunta ja nopeus simulaation alussa
		@param mass						Fysiikkaolion massa
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID,
			float radius, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){

			collider = new stage_common::SphereCollider(radius, glm::vec3());
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		/** Luo uuden fysiikkakomponentin AABB-törmäyshahmolla
		@param fw						Komponenttia hallinnoiva Theron::Framework
		@param owner					Komponentin omistavan peliolion osoite
		@param transform				Isäntäolion sijaintia ylläpitävän komponentin osoite
		@param size						Törmäyshahmon koko
		@param initialV					Fysiikkaolion liikesuunta ja nopeus simulaation alussa
		@param mass						Fysiikkaolion massa
		@param collisionEventChannel	Fysiikkaolioiden törmäystarkistusviestien käyttämä tapahtumakanava
		*/
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID,
			glm::vec3 size, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){

			collider = new stage_common::AABBCollider(size, glm::vec3());
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		virtual void initialize(GameObject* owner){
			Component::initialize(owner);
			Send(EventChannel<CollisionCheck>::RegisterRecipient(
				Destination(owner->GetAddress(), PHYSICSCOMPONENT_ID)), collisionEventChannel.GetAddress());
			transform = (Transform*)owner->getComponent(TRANSFORM_ID);
			collider->center = transform->getPosition();
			oldPos = collider->center;
			
			//Rekisteröidään viestinkäsittelijät
			RegisterHandler <PhysicsComponent, CollisionCheck, &PhysicsComponent::collisionCheck>();
			RegisterHandler <PhysicsComponent, CollisionDetected, &PhysicsComponent::collisionDetected>();
			RegisterHandler <PhysicsComponent, StaticCollision, &PhysicsComponent::staticCollision>();
			RegisterHandler <PhysicsComponent, FinishCollision, &PhysicsComponent::finishCollision>();
		}				

		/** Tuhoaa fysiikkakomponentin
		*/
		~PhysicsComponent(){
			delete collider;
			delete tempCollider;
		}

		/** Hakee fysiikkakomponentin komponenttitunnuksen
		@returns	Komponentin tunnus
		*/
		virtual int id(){ return PHYSICSCOMPONENT_ID; }
		virtual std::string name(){ return std::string("Simple Physics"); }
	private:
		/** Viite fysiikkamoottorin viestien käyttämään viestikanavaan
		*/
		EventChannel<CollisionCheck>& collisionEventChannel;

		/** Fysiikkaolion törmäyshahmo
		*/
		stage_common::Collider* collider;

		/** Kopio fysiikkaolion törmäyshahmosta.
		Käytetään törmäystentunnistuksessa varmistamaan, että törmäyshahmon muokkaaminen ei aiheuta rinnakkaisuusongelmia
		*/
		stage_common::Collider* tempCollider;

		/** Isäntäolion sijaintia ylläpitävän olion osoite
		*/
		Transform* transform;

		/** Fysiikkaolion liikesuunta ja nopeus
		*/
		glm::vec3 velocity;

		/** Fysiikkaolion sijainti ruudunpäivityksen alussa
		*/
		glm::vec3 oldPos;

		/** Fysiikkaolion massa
		*/
		float mass;

		/** Onko fysiikkaolion tila päivitetty nykyisen ruudunpäivityksen aikana
		*/
		bool updatedThisFrame = false;

		/** Niiden fysiikkaolioiden joukko, joihin on törmätty tämän ruudunpäivityksen aikana.
		Käytetään estämään saman törmäyksen käsitteleminen kahdesti, jos kumpikin fysiikkaolio
		käsittelee sitä samanaikaisesti rinnakkain.
		*/
		std::unordered_set<int> collidedThisFrame;		

		//--Kontekstiyksikkö alkaa--

		/** Päivitetään tila uutta ruudunpäivitystä vastaavaksi
		@param up	Päivityspyyntö
		@param from	Pyynnön lähettäjä
		*/
		void update(float elapsedMS, uint64_t id){
			//Päivitetään sijainti vain jos sitä ei vielä ole tehty törmäyskäsittelyn seurauksena
			if (!updatedThisFrame) updatePosition(elapsedMS);

			//Luodaan konteksti törmäystarkistusviesteille
			uint64_t newid = tracker.getNextID();
			EventContext& context = tracker.addContext(id, newid, Destination(owner->GetAddress(), INVALID_COMPONENT_ID), 0);
			context.finalize = [this, &context](){
				finishUpdate(context.getOriginalID());
			};
			context.error = [this, &context](){
				this->abortPhase(context.getOriginalID());
			};

			//Lähetetään törmäystarkistusviestit kaikille tapahtumakanavaan rekisteröityneille
			const std::list<Destination>& recipients = collisionEventChannel.getRecipients();
			
			bool sent = false;
			for (std::list<Destination>::const_iterator i = recipients.cbegin(); i != recipients.cend(); i++){
				if (!((*i).address == owner->GetAddress() && (*i).component == PHYSICSCOMPONENT_ID)){
					CollisionCheck newmsg(newid, *tempCollider, owner->GetAddress(), elapsedMS, PHYSICSCOMPONENT_ID, (*i).component);
					tracker.trackedSend<CollisionCheck>(id, newmsg, (*i).address, Destination(owner->GetAddress(), INVALID_COMPONENT_ID));
					sent = true;
				}
			}
			//Jos ei lähetettäviä viestejä, suljetaan tarpeeton konteksti
			if (!sent){
				context.finalize();
				tracker.remove(id);
				//Suoritus jatkuu metodissa finishUpdate
			}
		}

		/** Suorittaa loppuun tilanpäivityksen päivittämällä muuttuneen sijainnin sijaintikomponenttiin
		@param msg	Isäntäolion nykyisen sijainnin ilmoittava viesti
		@param from	Viestin lähettäjä
		*/
		void finishUpdate(uint64_t id){
			//Lasketaan nykyisen ruudunpäivityksen aikana liikuttu matka
			glm::vec3 translation = collider->center - oldPos;
			//Päivitetään törmäyshahmon sijainti
			collider->center = oldPos = transform->getPosition() + translation;
			//Päivitetään sijaintikomponentti Translate-viestillä, jotta muiden komponenttien mahdollisesti tekemiä
			//muutoksia ei peruta
			transform->translate(translation);
			finishPhase(id);
		}

		//--Kontekstiyksikkö päättyy--

		//--Kontekstiyksikkö alkaa--

		/**Tarkistaa, onko tämä fysiikkolio törmännyt toiseen
		@param msg	Toisen fysiikkaolion törmäyshahmon sisältävä viesti
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const CollisionCheck& msg, Theron::Address from){
			//Päivitetään sijainti, jos sitä ei vielä ole päivitetty Update-viestin seurauksena
			if (!updatedThisFrame) updatePosition(msg.elapsedMS);
			//Ei tehdä mitään, jos toinen olio käsittelee jo samaa törmäystä rinnakkain tai oliot eivät ole törmänneet
			if (collidedThisFrame.count(msg.originator.AsInteger()) || !tempCollider->checkCollision(msg.coll)){
				Send(AllDone(msg.id, PHYSICSCOMPONENT_ID, msg.senderComponent), from);
			}
			else {
				//Merkitään, että tämä törmäys on jo käsittelyssä, jos toisen olion osoite on omaa suurempi
				//Tällä varmistetaan, että törmäys käsitellään tasan kerran
				if (owner->GetAddress().AsInteger() < msg.originator.AsInteger()) collidedThisFrame.insert(msg.originator.AsInteger());
				//Luodaan uusi konteksti törmäyksen käsittelyä varten
				uint64_t id = createActorContext(msg.id, Destination(from, msg.senderComponent));
				Send(CollisionDetected(id, *tempCollider, velocity, mass, PHYSICSCOMPONENT_ID, msg.senderComponent), msg.originator);
				//Suoritus jatkuu metodissa FinishCollision
			}
		}

		/** Suorittaa loppuun törmäystapahtuman käsittelyn
		@param msg	Viesti, joka ilmoittaa törmäystapahtuman käsittelyn päättyneen
					ja sisältäää törmäyksen aiheuttaman muutoksen tämän kappaleen nopeuteen
		@param from	Viestin lähettäjä
		*/
		void finishCollision(const FinishCollision& msg, Theron::Address from){
			if (!tracker.contains(msg.id)){
				LOGERR("Warning: Unknown collision finish event received from " + std::to_string(from.AsInteger()));
				return;
			}
			velocity += msg.velocityAdjustment;
			//Vastausviesti törmäystapahtuman aiheuttajalle lähetetään automaattisesti kontekstin sulkeutuessa
			tracker.decrement(msg.id);
		}

		//--Kontekstiyksikkö päättyy--

		/** Käsittelee havaitun törmäyksen kahden fysiikkaolion välillä
		@param msg	Törmäyksestä kertova viesti
		@param from	Toinen törmännyt fysiikkaolio
		*/
		void collisionDetected(const CollisionDetected& msg, Theron::Address from){
			//Ei tehdä mitään, jos toinen olio käsittelee jo samaa törmäystä rinnakkain
			if (collidedThisFrame.count(from.AsInteger())) Send(AllDone(msg.id, PHYSICSCOMPONENT_ID, msg.senderComponent), from);
			else {
				//Merkitään, että tämä törmäys on jo käsittelyssä, jos toisen olion osoite on omaa suurempi
				//Tällä varmistetaan, että törmäys käsitellään tasan kerran
				if (owner->GetAddress().AsInteger() < from.AsInteger()) collidedThisFrame.insert(from.AsInteger());
				glm::vec3 otherNewV = msg.otherVelocity;
				//Lasketaan törmäyksestä aiheutuvat nopeuden muutokset
				stage_common::Collisions::collisionVelocityChange(velocity, mass, otherNewV, msg.otherMass);
				//Siirretään tämän olion törmäyshahmoa, kunnes oliot eivät enää törmää
				stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
				//Tiedotetaan toiselle oliolle sen nopeuteen tulleesta muutoksesta
				Send(FinishCollision(msg.id, otherNewV - msg.otherVelocity, PHYSICSCOMPONENT_ID, msg.senderComponent), from);
			}
		}

		/** Käsittelee havaitun törmäyksen fysiikkaolion ja staattisen törmäyshahmon välillä
		@param msg	Törmäyksestä kertova viesti
		@param from	Staattinen törmäyshahmokomponentti, johon törmättiin
		*/
		void staticCollision(const StaticCollision& msg, Theron::Address from){
			//Lasketaan törmäyksestä aiheutuvat nopeuden muutokset
			velocity = stage_common::Collisions::reflect(velocity, msg.coll.getCollisionNormal(*tempCollider, velocity));
			//Siirretään tämän olion törmäyshahmoa, kunnes oliot eivät enää törmää
			stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
			Send(AllDone(msg.id, PHYSICSCOMPONENT_ID, msg.senderComponent), from);
		}
		
		/** Siirtää törmäyshahmoa fysiikkaolion nykyisen nopeuden mukaisesti
		@param elapsedMS	Edellisestä ruudunpäivityksestä kulunut aika millisekunteina
		*/
		void updatePosition(float elapsedMS){
			collider->center = oldPos + (velocity * elapsedMS);
			updatedThisFrame = true;
			delete tempCollider;
			//Luodaan virtuaalikopiometodilla törmäyshahmosta väliaikainen kopio, jonka tilaa voidaan turvallisesti lukea
			//muista säikeistä
			tempCollider = collider->copy();
		}

		/** Suorittaa pelisilmukan piirtovaiheen laskennan
		@param rend	Piirtopyyntöviesti
		@param from	Viestin lähettäjä
		*/
		void render(uint64_t id){
			updatedThisFrame = false;
			collidedThisFrame.clear();
			finishPhase(id);
		}
	};
}

#endif
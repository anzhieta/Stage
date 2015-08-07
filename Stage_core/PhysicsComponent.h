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
		struct CollisionCheck : public Event{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			/** Alkuperäisen lähettäjän osoite
			*/
			const Theron::Address originator;
			/** Edellisestä ruudunpäivityksestä kulunut aika millisekunteina
			*/
			float elapsedMS;
			CollisionCheck(uint64_t id, stage_common::Collider& coll, Theron::Address originator, float elapsedMS)
				: Event(id), coll(coll), originator(originator), elapsedMS(elapsedMS){}
		};

		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt lähettäjään
		*/
		struct CollisionDetected : public Event{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			/** Lähettäjän nopeus
			*/
			glm::vec3 otherVelocity;
			/** Lähettäjän massa
			*/
			float otherMass;
			CollisionDetected(uint64_t id, stage_common::Collider& coll, glm::vec3 otherVelocity, float otherMass)
				: Event(id), coll(coll), otherVelocity(otherVelocity), otherMass(otherMass){}
		};

		/** Viesti, joka ilmoittaa, että vastaanottaja on törmännyt liikkumattomaan peliolioon
		*/
		struct StaticCollision : public Event{
			/** Lähettäjän törmäyshahmo
			*/
			const stage_common::Collider& coll;
			StaticCollision(uint64_t id, stage_common::Collider& coll) : Event(id), coll(coll){}
		};

		/** Viesti, joka ilmoittaa törmäyksen käsittelyn onnistuneen
		*/
		struct FinishCollision : public Event{
			/** Vastaanottajan nopeuteen tehtävä muutos
			*/
			glm::vec3 velocityAdjustment;
			FinishCollision(uint64_t id, glm::vec3 velAdj) : Event(id), velocityAdjustment(velAdj){}
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
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform, 
			float radius, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), transform(transform), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){
			//Rekisteröidään viestinkäsittelijä olion alustuksen lopettavalle metodille
			RegisterHandler(this, &PhysicsComponent::finishSphereSetup);
			//Konstruktoreille yhteiset alustukset
			uint64_t id = setup();
			//Tallennetaan törmäyshahmon säde kontekstimuuttujaan 0
			tracker.setVariable<float>(id, 0, radius);
			//Pyydetään sijaintioliolta nykyinen sijainti
			Send(Transform::GetPosition(id), transform);
			//Suoritus jatkuu metodissa finishSphereSetup
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
		PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform, 
			glm::vec3 size, glm::vec3 initialV, float mass, EventChannel<CollisionCheck>& collisionEventChannel) :
			Component(fw, owner), transform(transform), velocity(initialV), mass(mass), collisionEventChannel(collisionEventChannel){
			//Rekisteröidään viestinkäsittelijä olion alustuksen lopettavalle metodille
			RegisterHandler(this, &PhysicsComponent::finishAABBSetup);
			//Konstruktoreille yhteiset alustukset
			uint64_t id = setup();
			//Tallennetaan törmäyshahmon koko kontekstimuuttujaan 0
			tracker.setVariable<glm::vec3>(id, 0, size);
			//Pyydetään sijaintioliolta nykyinen sijainti
			Send(Transform::GetPosition(id), transform);
			//Suoritus jatkuu metodissa finishAABBSetup
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
		Theron::Address transform;

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

		/** Onko komponentin alustus suoritettu loppuun
		*/
		bool init = false;

		/** Niiden fysiikkaolioiden joukko, joihin on törmätty tämän ruudunpäivityksen aikana.
		Käytetään estämään saman törmäyksen käsitteleminen kahdesti, jos kumpikin fysiikkaolio
		käsittelee sitä samanaikaisesti rinnakkain.
		*/
		std::unordered_set<int> collidedThisFrame;

		/** Molemmille konstruktoreille yhteiset alustukset suorittava metodi
		@returns	Konteksti-ID, jota käytetään alustusviestien tunnistamiseen
		*/
		uint64_t setup(){
			//Rekisteröidään viestinkäsittelijät
			RegisterHandler(this, &PhysicsComponent::collisionCheck);
			RegisterHandler(this, &PhysicsComponent::collisionDetected);
			RegisterHandler(this, &PhysicsComponent::staticCollision);
			RegisterHandler(this, &PhysicsComponent::finishCollision);
			//Luodaan uusi viestikonteksti alustuksen loppuunsuorittamista varten
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(0, id, Theron::Address::Null());
			context.finalize = [](){};
			context.error = context.finalize;
			//Rekisteröidään fysiikkaolio tapahtumakanavan viestien vastaanottajaksi
			Send(EventChannel<CollisionCheck>::RegisterRecipient(this->GetAddress()), collisionEventChannel.GetAddress());
			return id;
		}

		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi pallo
		@param msg		Fysiikkaolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &PhysicsComponent::finishSphereSetup);
			RegisterHandler(this, &PhysicsComponent::finishUpdate);
			//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla säteellä ja viestin sisältämällä sijainnilla
			collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
			oldPos = msg.position;
			init = true;
			tracker.decrement(msg.id);
		}

		/** Suoritetaan komponentin alustus loppuun ja asetetaan törmäyshahmoksi AABB (Axis-Aligned Bounding Box)
		@param msg		Fysiikkaolion sijainnin sisältävä viesti
		@param sender	Viestin lähettäjä
		*/
		void finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
			DeregisterHandler(this, &PhysicsComponent::finishAABBSetup);
			RegisterHandler(this, &PhysicsComponent::finishUpdate);
			//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla koolla ja viestin sisältämällä sijainnilla
			collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
			oldPos = msg.position;
			init = true;
			tracker.decrement(msg.id);
		}

		//--Kontekstiyksikkö alkaa--

		/** Päivitetään tila uutta ruudunpäivitystä vastaavaksi
		@param up	Päivityspyyntö
		@param from	Pyynnön lähettäjä
		*/
		void update(const Update &up, Theron::Address from){
			if (!init){
				//Ei tehdä mitään, ellei tilaa ole vielä alustettu loppuun
				Send(AllDone(up.id), from);
				return;
			}
			//Päivitetään sijainti vain jos sitä ei vielä ole tehty törmäyskäsittelyn seurauksena
			if (!updatedThisFrame) updatePosition(up.elapsedMS);
			//Luodaan konteksti törmäystarkistusviesteille
			uint64_t id = tracker.getNextID();
			EventContext& context = tracker.addContext(up.id, id, from, 0);
			context.finalize = [this, &context](){
				//Kun kaikki törmäyksen on tarkistettu, luodaan uusi konteksti sijainnin päivittämiselle
				uint64_t id = tracker.getNextID();
				tracker.addContext(context.getOriginalID(), id, context.getOriginalSender());
				//Haetaan isäntäolion sijainti uudestaan, koska muut komponentit ovat ehkä tehneet siihen muutoksia
				Send(Transform::GetPosition(id), transform);
				//Suoritus jatkuu metodissa finishUpdate
			};
			//Lähetetään törmäystarkistusviestit kaikille tapahtumakanavaan rekisteröityneille
			CollisionCheck newmsg(id, *tempCollider, this->GetAddress(), up.elapsedMS);
			const std::list<Theron::Address>& recipients = collisionEventChannel.getRecipients();
			bool sent = false;
			for (std::list<Theron::Address>::const_iterator i = recipients.cbegin(); i != recipients.cend(); i++){
				if ((*i) != this->GetAddress()){
					tracker.trackedSend<CollisionCheck>(up.id, newmsg, *i, from);
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
		void finishUpdate(const Transform::Position& msg, Theron::Address from){
			//Lasketaan nykyisen ruudunpäivityksen aikana liikuttu matka
			glm::vec3 translation = collider->center - oldPos;
			//Päivitetään törmäyshahmon sijainti
			collider->center = oldPos = msg.position + translation;
			//Päivitetään sijaintikomponentti Translate-viestillä, jotta muiden komponenttien mahdollisesti tekemiä
			//muutoksia ei peruta
			Send(Transform::Translate(msg.id, translation), transform);
			//Sijaintikomponentti vastaa AllDone-viestillä, joka sulkee nykyisen kontekstin ja
			//lähettää automaattisesti AllDone-viestin isäntäoliolle
		}

		//--Kontekstiyksikkö päättyy--

		//--Kontekstiyksikkö alkaa--

		/**Tarkistaa, onko tämä fysiikkolio törmännyt toiseen
		@param msg	Toisen fysiikkaolion törmäyshahmon sisältävä viesti
		@param from	Viestin lähettäjä
		*/
		void collisionCheck(const CollisionCheck& msg, Theron::Address from){
			if (!init){
				//Ei tehdä mitään, jos komponenttia ei ole alustettu loppuun
				Send(AllDone(msg.id), from);
				return;
			}
			//Päivitetään sijainti, jos sitä ei vielä ole päivitetty Update-viestin seurauksena
			if (!updatedThisFrame) updatePosition(msg.elapsedMS);
			//Ei tehdä mitään, jos toinen olio käsittelee jo samaa törmäystä rinnakkain tai oliot eivät ole törmänneet
			if (collidedThisFrame.count(msg.originator.AsInteger()) || !tempCollider->checkCollision(msg.coll)) Send(AllDone(msg.id), from);
			else {
				//Merkitään, että tämä törmäys on jo käsittelyssä, jos toisen olion osoite on omaa suurempi
				//Tällä varmistetaan, että törmäys käsitellään tasan kerran
				if (this->GetAddress().AsInteger() < msg.originator.AsInteger()) collidedThisFrame.insert(msg.originator.AsInteger());
				//Luodaan uusi konteksti törmäyksen käsittelyä varten
				uint64_t id = tracker.getNextID();
				tracker.addContext(msg.id, id, from);
				Send(CollisionDetected(id, *tempCollider, velocity, mass), msg.originator);
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
			if (collidedThisFrame.count(from.AsInteger())) Send(AllDone(msg.id), from);
			else {
				//Merkitään, että tämä törmäys on jo käsittelyssä, jos toisen olion osoite on omaa suurempi
				//Tällä varmistetaan, että törmäys käsitellään tasan kerran
				if (this->GetAddress().AsInteger() < from.AsInteger()) collidedThisFrame.insert(from.AsInteger());
				glm::vec3 otherNewV = msg.otherVelocity;
				//Lasketaan törmäyksestä aiheutuvat nopeuden muutokset
				stage_common::Collisions::collisionVelocityChange(velocity, mass, otherNewV, msg.otherMass);
				//Siirretään tämän olion törmäyshahmoa, kunnes oliot eivät enää törmää
				stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
				//Tiedotetaan toiselle oliolle sen nopeuteen tulleesta muutoksesta
				Send(FinishCollision(msg.id, otherNewV - msg.otherVelocity), from);
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
			Send(AllDone(msg.id), from);
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
		void render(const Render &rend, Theron::Address from){
			updatedThisFrame = false;
			collidedThisFrame.clear();
			Send(AllDone(rend.id), from);
		}
	};
}

#endif
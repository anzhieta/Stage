#include "stdafx.h"
#include "PhysicsComponent.h"

using namespace stage;

PhysicsComponent::PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform,
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
PhysicsComponent::PhysicsComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform,
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
PhysicsComponent::~PhysicsComponent(){
	delete collider;
	delete tempCollider;
}

uint64_t PhysicsComponent::setup(){
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
void PhysicsComponent::finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
	DeregisterHandler(this, &PhysicsComponent::finishSphereSetup);
	RegisterHandler(this, &PhysicsComponent::finishUpdate);
	//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla säteellä ja viestin sisältämällä sijainnilla
	collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
	oldPos = msg.position;
	init = true;
	tracker.decrement(msg.id);
}
void PhysicsComponent::finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
	DeregisterHandler(this, &PhysicsComponent::finishAABBSetup);
	RegisterHandler(this, &PhysicsComponent::finishUpdate);
	//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla koolla ja viestin sisältämällä sijainnilla
	collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
	oldPos = msg.position;
	init = true;
	tracker.decrement(msg.id);
}

//--Kontekstiyksikkö alkaa--
void PhysicsComponent::update(const Update &up, Theron::Address from){
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
void PhysicsComponent::finishUpdate(const Transform::Position& msg, Theron::Address from){
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
void PhysicsComponent::collisionCheck(const CollisionCheck& msg, Theron::Address from){
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
void PhysicsComponent::finishCollision(const FinishCollision& msg, Theron::Address from){
	if (!tracker.contains(msg.id)){
		LOGERR("Warning: Unknown collision finish event received from " + std::to_string(from.AsInteger()));
		return;
	}
	velocity += msg.velocityAdjustment;
	//Vastausviesti törmäystapahtuman aiheuttajalle lähetetään automaattisesti kontekstin sulkeutuessa
	tracker.decrement(msg.id);
}
//--Kontekstiyksikkö päättyy--

void PhysicsComponent::collisionDetected(const CollisionDetected& msg, Theron::Address from){
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
void PhysicsComponent::staticCollision(const StaticCollision& msg, Theron::Address from){
	//Lasketaan törmäyksestä aiheutuvat nopeuden muutokset
	velocity = stage_common::Collisions::reflect(velocity, msg.coll.getCollisionNormal(*tempCollider, velocity));
	//Siirretään tämän olion törmäyshahmoa, kunnes oliot eivät enää törmää
	stage_common::Collisions::backOff(*collider, -1.0f * velocity, msg.coll);
	Send(AllDone(msg.id), from);
}
void PhysicsComponent::updatePosition(float elapsedMS){
	collider->center = oldPos + (velocity * elapsedMS);
	updatedThisFrame = true;
	delete tempCollider;
	//Luodaan virtuaalikopiometodilla törmäyshahmosta väliaikainen kopio, jonka tilaa voidaan turvallisesti lukea
	//muista säikeistä
	tempCollider = collider->copy();
}
void PhysicsComponent::render(const Render &rend, Theron::Address from){
	updatedThisFrame = false;
	collidedThisFrame.clear();
	Send(AllDone(rend.id), from);
}
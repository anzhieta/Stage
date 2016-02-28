#include "stdafx.h"
#include "StaticGeometryComponent.h"

using namespace stage;

StaticGeometryComponent::StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, float radius, Theron::Address transform,
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
StaticGeometryComponent::StaticGeometryComponent(Theron::Framework& fw, Theron::Address owner, glm::vec3 size, Theron::Address transform,
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
StaticGeometryComponent::~StaticGeometryComponent(){
	delete collider;
}

//--Kontekstiyksikkö alkaa--
void StaticGeometryComponent::update(const Update &up, Theron::Address from){
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
void StaticGeometryComponent::finishUpdate(const Transform::Position& msg, Theron::Address from){
	//Päivitetään törmäyshahmon sijainti
	collider->center = msg.position;
	//Poistetaan konteksti, jolloin vastausviesti Update-viestiin lähtee automaattisesti
	tracker.decrement(msg.id);
}
//--Kontekstiyksikkö päättyy--

void StaticGeometryComponent::collisionCheck(const PhysicsComponent::CollisionCheck& msg, Theron::Address from){
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
void StaticGeometryComponent::finishSphereSetup(const Transform::Position& msg, Theron::Address sender){
	DeregisterHandler(this, &StaticGeometryComponent::finishSphereSetup);
	RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
	//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla säteellä ja viestin sisältämällä sijainnilla
	collider = new stage_common::SphereCollider(tracker.getVariable<float>(msg.id, 0), msg.position);
	init = true;
	tracker.decrement(msg.id);
}
void StaticGeometryComponent::finishAABBSetup(const Transform::Position& msg, Theron::Address sender){
	DeregisterHandler(this, &StaticGeometryComponent::finishAABBSetup);
	RegisterHandler(this, &StaticGeometryComponent::finishUpdate);
	//Luodaan törmäyshahmo kontekstimuuttujasta haettavalla koolla ja viestin sisältämällä sijainnilla
	collider = new stage_common::AABBCollider(tracker.getVariable<glm::vec3>(msg.id, 0), msg.position);
	init = true;
	tracker.decrement(msg.id);
}
uint64_t StaticGeometryComponent::setup(){
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
#include "stdafx.h"

#ifndef CAMERACONTROLCOMPONENT_H
#define CAMERACONTROLCOMPONENT_H

#include <Component.h>
#include <Transform.h>
#include <Input.h>
#include <GLFW\glfw3.h>
#include <SceneManager.h>

#define CAMERACONTROLCOMPONENT_ID 6
#define CAMERASPEED 0.025f

namespace stage{
	/** Kameran hallintaan tarkoitettu komponentti, joka mahdollistaa isäntäolionsa liikuttamisen näppäimistöllä
	Käsittelee viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	*/
	class CameraControlComponent : public Component{
	public:
		/** Luo uuden kameranhallintakomponentin
		@param fw			Komponenttia hallinnoiva Theron::Framework
		@param owner		Komponentin isäntäolion Theron-osoite
		@param transform	Isäntäolion sijaintia ylläpitävän komponentin Theron-osoite
		*/
		CameraControlComponent(Theron::Framework& fw, Theron::Address owner, Destination notifyDest, uint64_t notifyID)
			: Component(fw, owner){
			
			stage_common::Input& in = stage_common::Input::getSingleton();
			//Rekisteröidään ne näppäimet, joiden tila halutaan lukea
			//Voidaan tehdä säieturvallisesti, koska Input-olion näppäinlistaan
			//haetaan arvot vasta ylläpitovaiheessa
			in.registerKey(GLFW_KEY_W);
			in.registerKey(GLFW_KEY_S);
			in.registerKey(GLFW_KEY_A);
			in.registerKey(GLFW_KEY_D);
			in.registerKey(GLFW_KEY_R);
			in.registerKey(GLFW_KEY_F);
			in.registerKey(GLFW_KEY_ESCAPE);
			registerSelf(fw, owner, notifyDest, notifyID);
		}

		/** Hakee kameranhallintakomponentin komponenttitunnuksen
		@returns	Komponentin tyyppitunnus
		*/
		virtual int id(){ return CAMERACONTROLCOMPONENT_ID; }

		virtual std::string name(){ return "Camera controls"; }

		virtual void initialize(GameObject* owner){
			Component::initialize(owner);
			transform = (Transform*)owner->getComponent(TRANSFORM_ID);
		}

	private:
		/** Isäntäolion sijaintia ylläpitävän komponentin Theron-osoite
		*/
		Transform* transform;

		/** Päivittää komponentin tilan
		@param msg		Päivityspyyntö
		@param sender	Pyynnön lähettäjä
		*/
		virtual void update(float elapsedMS, uint64_t id){
			stage_common::Input& in = stage_common::Input::getSingleton();
			//Isäntäolion liikettä kuvaava vektori
			glm::vec3 movement;
			//muutetaan liikevektoria pohjassa olevien näppäinten perusteella
			//Voidaan tehdä säieturvallisesti, koska Input-olion näppäinlistan
			//arvoja muutetaan vain ylläpitovaiheessa
			if (in.getKeyDown(GLFW_KEY_W)) movement.z += CAMERASPEED * elapsedMS;
			if (in.getKeyDown(GLFW_KEY_S)) movement.z -= CAMERASPEED  * elapsedMS;
			if (in.getKeyDown(GLFW_KEY_A)) movement.x += CAMERASPEED  * elapsedMS;
			if (in.getKeyDown(GLFW_KEY_D)) movement.x -= CAMERASPEED  * elapsedMS;
			if (in.getKeyDown(GLFW_KEY_F)) movement.y += CAMERASPEED  * elapsedMS;
			if (in.getKeyDown(GLFW_KEY_R)) movement.y -= CAMERASPEED  * elapsedMS;
			//Lopetetaan suoritus, jos Esc pohjassa
			if (in.getKeyDown(GLFW_KEY_ESCAPE)) Send(SceneManager::Abort(), SceneManager::getGlobalManager());
			transform->translate(movement);
			finishPhase(id);
		}
	};
}

#endif
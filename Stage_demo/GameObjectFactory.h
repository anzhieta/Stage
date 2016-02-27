#ifndef GAMEOBJECTFACTORY_H
#define GAMEOBJECTFACTORY_H

#include "stdafx.h"
#include "Plane.h"
#include "Sphere.h"
#include "Waiter.h"
#include <Scene.h>
#include <glm\glm.hpp>
#include <PhysicsComponent.h>
#include <StaticGeometryComponent.h>
#include <ModelComponent.h>
#include <SimpleShader.h>

namespace stage{
	/** Tehdasluokka, joka valmistaa pelimoottorin demo-ohjelman käyttämiä peliolioita*/
	class GameObjectFactory{
	public:
		/** Rakentaa satunnaiseen paikkaan pallon, joka lähtee liikkumaan satunnaiseen suuntaan
		@param fw						Pelimoottorin käyttämä Theron::Framework
		@param scene					Pelialue, johon pallo luodaan
		@param maxCoordinates			Pallon maksimietäisyys pelimaailman origosta
		@param collisionEventChannel	Fysiikkaolioiden käyttämä tapahtumakanava
		@returns						Luodun peliolion Theron-osoite
		*/
		Theron::Address constructRandomSphere(Theron::Framework& fw, Theron::Address scene, glm::vec3 maxCoordinates, 
			EventChannel<PhysicsComponent::CollisionCheck>& collisionEventChannel, int waitLimit){
			//Placeholder-muuttujat vastaanotettaville viesteille
			Scene::NewObject obj(0, Theron::Address::Null());
			Theron::Address temp;
			//Pyydetään pelialuetta luomaan uusi peliolio
			fw.Send(Scene::CreateObject(0), rec.GetAddress(), scene);
			//Odotetaan olion valmistumista
			rec.Wait();
			catcher.Pop(obj, temp);
			//Arvotaan pallolle sijainti
			glm::vec3 translation(randomFloat(-maxCoordinates.x, maxCoordinates.x),
				randomFloat(-maxCoordinates.y, maxCoordinates.y),
				randomFloat(-maxCoordinates.z, maxCoordinates.z));
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
			//Luodaan pallolle sijaintikomponentti
			Transform* tf = new Transform(fw, obj.object, transform);
			//Liitetään palloon 3D-malli
			ModelComponent* mod = new ModelComponent(fw, &(getSingleton().mod_sphere), obj.object);
			//Arvotaan pallolle nopeus
			glm::vec3 velocity(randomFloat(-0.01f, 0.01f), randomFloat(-0.01f, 0.01f), randomFloat(-0.01f, 0.01f));
			//Luodaan pallolle fysiikkakomponentti
			PhysicsComponent* pc = new PhysicsComponent(fw, obj.object, tf->GetAddress(), 1.0f, velocity, 1.0f, collisionEventChannel);
			if (waitLimit > 0) Waiter* w = new Waiter(fw, obj.object, waitLimit);
			return obj.object;
		}
		/** Rakentaa haluttuun pelimaailman sijaintiin seinän
		@param fw						Pelimaailmaa hallinnoiva Theron::Framework
		@param scene					Sen pelialueen osoite, johon seinä rakennetaan
		@param transform				Seinän sijaintia kuvaava 4x4-matriisi
		@param size						Seinän koko
		@param collisionEventChannel	Fysiikkaolioiden käyttämä tapahtumakanava
		@returns						Luodun peliolion Theron-osoite
		*/
		Theron::Address constructWall(Theron::Framework& fw, Theron::Address scene, glm::mat4& transform, glm::vec3 size,
			Theron::Address collisionEventChannel){
			//Placeholder-muuttujat vastaanotettaville viesteille
			Scene::NewObject obj(0, Theron::Address::Null());
			Theron::Address temp;
			//Pyydetään pelialuetta luomaan uusi peliolio
			fw.Send(Scene::CreateObject(0), rec.GetAddress(), scene);
			//Odotetaan olion valmistumista
			rec.Wait();
			catcher.Pop(obj, temp);
			//Luodaan seinälle sijaintikomponentti
			Transform* tf = new Transform(fw, obj.object, transform);
			//Liitetään seinään 3D-malli
			ModelComponent* mod = new ModelComponent(fw, &(getSingleton().mod_plane), obj.object);
			//Liitetään seinään törmäyshahmo
			StaticGeometryComponent* sgc = new StaticGeometryComponent(fw, obj.object, size, tf->GetAddress(), collisionEventChannel);
			return obj.object;
		}		
		/** Hakee viitteen globaaliin GameObjectFactory-instanssiin
		@returns	Viite GameObjectFactory-singletoniin
		*/
		static GameObjectFactory& getSingleton(){
			static GameObjectFactory gof;
			return gof;
		}
	private:
		/** Demon 3D-mallien käyttämä sävytinohjelma*/
		stage_common::SimpleShader ss;
		/** Pallon 3D-malli*/
		stage_common::Model mod_sphere;
		/** Seinän 3D-malli*/
		stage_common::Model mod_plane;
		/** Olio, jota tehdasolio käyttää vastaanottamaan viestejä aktoreilta*/
		Theron::Receiver rec;
		/** Olio, jota tehdasolio käyttää käsittelemään aktoreilta vastaanotetut viestit*/
		Theron::Catcher<Scene::NewObject> catcher;		
		/** Apufunktio, joka arpoo satunnaisen liukuluvun kahden luvun väliltä
		@param start	Arvottavan luvun alaraja
		@param end		Arvottavan luvun yläraja
		@returns		Staunnainen luku ala- ja ylärajan väliltä
		*/
		static float randomFloat(float start, float end){
			float random = ((float)rand()) / (float)RAND_MAX;
			return start + (end - start) * random;
		}
		/** Luo pelioliotehtaan*/
		GameObjectFactory() : mod_sphere(generate_sphere_vertices(), generate_sphere_colors(), &ss),
			mod_plane(generate_plane_vertices(), generate_plane_colors(), &ss){
			rec.RegisterHandler(&catcher, &Theron::Catcher<Scene::NewObject>::Push);
		}
	};
}
#endif
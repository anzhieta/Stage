#ifndef GAMEOBJECTFACTORY_H
#define GAMEOBJECTFACTORY_H

#include "stdafx.h"
#include "Plane.h"
#include "Sphere.h"
#include <Scene.h>
#include <glm\glm.hpp>
#include <PhysicsComponent.h>
#include <StaticGeometryComponent.h>
#include <ModelComponent.h>
#include <SimpleShader.h>

namespace stage{
	class GameObjectFactory{
	public:
		Theron::Address constructRandomSphere(Theron::Framework& fw, Theron::Address scene, glm::vec3 maxCoordinates, 
			EventChannel<PhysicsComponent::CollisionCheck>& collisionEventChannel){

			Scene::NewObject obj(0, Theron::Address::Null());
			Theron::Address temp;

			fw.Send(Scene::CreateObject(0), rec.GetAddress(), scene);
			rec.Wait();
			catcher.Pop(obj, temp);

			glm::vec3 translation(randomFloat(-maxCoordinates.x, maxCoordinates.x),
				randomFloat(-maxCoordinates.y, maxCoordinates.y),
				randomFloat(-maxCoordinates.z, maxCoordinates.z));
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);

			Transform* tf = new Transform(fw, obj.object, transform);
			ModelComponent* mod = new ModelComponent(fw, &(getSingleton().mod_sphere), obj.object);
			glm::vec3 velocity(randomFloat(-0.01f, 0.01f), randomFloat(-0.01f, 0.01f), randomFloat(-0.01f, 0.01f));
			PhysicsComponent* pc = new PhysicsComponent(fw, obj.object, tf->GetAddress(), 1.0f, velocity, 1.0f, collisionEventChannel);
			return obj.object;
		}
		Theron::Address constructWall(Theron::Framework& fw, Theron::Address scene, glm::mat4& transform, glm::vec3 size,
			Theron::Address collisionEventChannel){
			
			Scene::NewObject obj(0, Theron::Address::Null());
			Theron::Address temp;

			fw.Send(Scene::CreateObject(0), rec.GetAddress(), scene);
			rec.Wait();
			catcher.Pop(obj, temp);

			Transform* tf = new Transform(fw, obj.object, transform);
			ModelComponent* mod = new ModelComponent(fw, &(getSingleton().mod_plane), obj.object);
			StaticGeometryComponent* sgc = new StaticGeometryComponent(fw, obj.object, size, tf->GetAddress(), collisionEventChannel);
			return obj.object;
		}
		
		static GameObjectFactory& getSingleton(){
			static GameObjectFactory gof;
			return gof;
		}
	private:
		stage_common::SimpleShader ss;
		stage_common::Model mod_sphere;
		stage_common::Model mod_plane;
		Theron::Receiver rec;
		Theron::Receiver adRec;
		Theron::Catcher<Scene::NewObject> catcher;
		Theron::Catcher<AllDone> adCatcher;
		
		static float randomFloat(float start, float end){
			float random = ((float)rand()) / (float)RAND_MAX;
			return start + (end - start) * random;
		}

		GameObjectFactory() : mod_sphere(generate_sphere_vertices(), generate_sphere_colors(), &ss),
			mod_plane(generate_plane_vertices(), generate_plane_colors(), &ss){

			rec.RegisterHandler(&catcher, &Theron::Catcher<Scene::NewObject>::Push);
			adRec.RegisterHandler(&adCatcher, &Theron::Catcher<AllDone>::Push);
		}
	};
}

#endif
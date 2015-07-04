
#include "stdafx.h"
#include <Theron\Theron.h>
#include <string>
#include <iostream>
#include <Gameloop.h>
#include <GameObject.h>
#include <Component.h>
#include <CoreEvents.h>
#include <Scene.h>
#include <functional>
#include <Transform.h>
#include <SimpleShader.h>
#include <ModelComponent.h>
#include <CameraComponent.h>
#include <EventChannel.h>
#include <PhysicsComponent.h>
#include <StaticGeometryComponent.h>
#include "Plane.h"
#include "Sphere.h"
#include "CameraControlComponent.h"
#include "GameObjectFactory.h"
#include <fstream>

using namespace stage;


int _tmain(int argc, _TCHAR* argv[])
{
	char c;
	int SCALE = 10;
	int SPHERES = 5;
	std::string configfile;
	std::ifstream configStream("config.ini", std::ios::in);

	if (configStream.is_open())
	{
		std::string line = "";
		std::string start, end;
		int delimiterPos;
		while (getline(configStream, line)){
			delimiterPos = line.find("=");
			if (delimiterPos == std::string::npos){
				std::cerr << "Invalid configuration parameter " << start << std::endl;
				continue;
			}
			start = line.substr(0, delimiterPos);
			end = line.substr(delimiterPos + 1);
			if (start == "SCALE"){
				try{
					SCALE = std::stoi(end);
					if (SCALE < 5) SCALE = 5;
				}
				catch (...){
					std::cerr << "Error parsing configuration parameter SCALE" << std::endl;
					continue;
				}
			}
			else if (start == "SPHERES"){
				try{
					SPHERES = std::stoi(end);
					if (SPHERES < 1) SPHERES = 1;
				}
				catch (...){
					std::cerr << "Error parsing configuration parameter SCALE" << std::endl;
					continue;
				}
			}
			else std::cerr << "Unknown configuration parameter " << start << std::endl;
		}
		configStream.close();
	}
	else std::cerr << "Warning: config.ini not found, falling back to default parameters" << std::endl;

	stage::Gameloop loop(std::string("Stage engine demo"), 640, 480);
	Theron::Framework& fw = loop.getFramework();
	Theron::Address sc = loop.createScene();

	loop.setActiveScene(0);

	EventChannel<PhysicsComponent::CollisionCheck> collChannel(fw);
	loop.getEventChannelManager().addChannel(collChannel.GetAddress());

	Theron::Receiver rec;
	Theron::Receiver adRec;
	Theron::Catcher<Scene::NewObject> catcher;
	Theron::Catcher<AllDone> adCatcher;
	rec.RegisterHandler(&catcher, &Theron::Catcher<Scene::NewObject>::Push);
	adRec.RegisterHandler(&adCatcher, &Theron::Catcher<AllDone>::Push);


	Scene::NewObject camobject(0, Theron::Address::Null());
	Theron::Address temp;

	fw.Send(Scene::CreateObject(0), rec.GetAddress(), sc);
	rec.Wait();
	catcher.Pop(camobject, temp);

	Transform* tr1 = new Transform(fw, camobject.object);
	fw.Send(Transform::SetMatrix(0, glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -SCALE * 2))), adRec.GetAddress(), tr1->GetAddress());

	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, SCALE * 10.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, -SCALE * 2),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	CameraComponent* cam = new CameraComponent(fw, camobject.object);
	fw.Send(CameraComponent::SetViewMatrix(0, View), adRec.GetAddress(), cam->GetAddress());
	fw.Send(CameraComponent::SetProjectionMatrix(0, Projection), adRec.GetAddress(), cam->GetAddress());
	loop.setActiveCamera(cam->getRawCamera());

	CameraControlComponent* camcc = new CameraControlComponent(fw, camobject.object, tr1->GetAddress());

	GameObjectFactory& factory = GameObjectFactory::getSingleton();

	glm::mat4 bottompos;
	bottompos = glm::scale(bottompos, glm::vec3(SCALE, 1, SCALE));
	bottompos = glm::translate(bottompos, glm::vec3(0, -SCALE, 0));
	factory.constructWall(fw, sc, bottompos, glm::vec3(SCALE, 0, SCALE), collChannel.GetAddress());

	glm::mat4 toppos;
	toppos = glm::rotate(toppos, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	toppos = glm::translate(toppos, glm::vec3(0, -SCALE, 0));
	toppos = glm::scale(toppos, glm::vec3(SCALE, 1, SCALE));
	factory.constructWall(fw, sc, toppos, glm::vec3(SCALE, 0, SCALE), collChannel.GetAddress());

	glm::mat4 leftpos;
	leftpos = glm::rotate(leftpos, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	leftpos = glm::translate(leftpos, glm::vec3(0, -SCALE, 0));
	leftpos = glm::scale(leftpos, glm::vec3(SCALE, 1, SCALE));
	factory.constructWall(fw, sc, leftpos, glm::vec3(0, SCALE, SCALE), collChannel.GetAddress());

	glm::mat4 rightpos;
	rightpos = glm::rotate(rightpos, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	rightpos = glm::translate(rightpos, glm::vec3(0, -SCALE, 0));
	rightpos = glm::scale(rightpos, glm::vec3(SCALE, 1, SCALE));
	factory.constructWall(fw, sc, rightpos, glm::vec3(0, SCALE, SCALE), collChannel.GetAddress());

	glm::mat4 backpos;
	backpos = glm::rotate(backpos, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	backpos = glm::translate(backpos, glm::vec3(0, -SCALE, 0));
	backpos = glm::scale(backpos, glm::vec3(SCALE, 1, SCALE));
	factory.constructWall(fw, sc, backpos, glm::vec3(SCALE, SCALE, 0), collChannel.GetAddress());

	glm::mat4 frontpos;
	frontpos = glm::rotate(frontpos, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	frontpos = glm::translate(frontpos, glm::vec3(0, -SCALE, 0));
	frontpos = glm::scale(frontpos, glm::vec3(SCALE, 1, SCALE));
	factory.constructWall(fw, sc, frontpos, glm::vec3(SCALE, SCALE, 0), collChannel.GetAddress());

	for (int i = 0; i < SPHERES; i++){
		factory.constructRandomSphere(fw, sc, glm::vec3(SCALE - 1, SCALE - 1, SCALE - 1), collChannel);
	}



	loop.start();

	std::cin >> c;
	return 0;
}


// Stage_test.cpp : Defines the entry point for the console application.
//

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

using namespace stage;

static std::vector<glm::vec3> vertices = {
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f), // triangle 2 : begin
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f), // triangle 2 : end
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, -1.0f, 1.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, -1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(-1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, -1.0f, 1.0f)
};
static std::vector<glm::vec3> colors = {
	glm::vec3(0.583f, 0.771f, 0.014f),
	glm::vec3(0.609f, 0.115f, 0.436f),
	glm::vec3(0.327f, 0.483f, 0.844f),
	glm::vec3(0.822f, 0.569f, 0.201f),
	glm::vec3(0.435f, 0.602f, 0.223f),
	glm::vec3(0.310f, 0.747f, 0.185f),
	glm::vec3(0.597f, 0.770f, 0.761f),
	glm::vec3(0.559f, 0.436f, 0.730f),
	glm::vec3(0.359f, 0.583f, 0.152f),
	glm::vec3(0.483f, 0.596f, 0.789f),
	glm::vec3(0.559f, 0.861f, 0.639f),
	glm::vec3(0.195f, 0.548f, 0.859f),
	glm::vec3(0.014f, 0.184f, 0.576f),
	glm::vec3(0.771f, 0.328f, 0.970f),
	glm::vec3(0.406f, 0.615f, 0.116f),
	glm::vec3(0.676f, 0.977f, 0.133f),
	glm::vec3(0.971f, 0.572f, 0.833f),
	glm::vec3(0.140f, 0.616f, 0.489f),
	glm::vec3(0.997f, 0.513f, 0.064f),
	glm::vec3(0.945f, 0.719f, 0.592f),
	glm::vec3(0.543f, 0.021f, 0.978f),
	glm::vec3(0.279f, 0.317f, 0.505f),
	glm::vec3(0.167f, 0.620f, 0.077f),
	glm::vec3(0.347f, 0.857f, 0.137f),
	glm::vec3(0.055f, 0.953f, 0.042f),
	glm::vec3(0.714f, 0.505f, 0.345f),
	glm::vec3(0.783f, 0.290f, 0.734f),
	glm::vec3(0.722f, 0.645f, 0.174f),
	glm::vec3(0.302f, 0.455f, 0.848f),
	glm::vec3(0.225f, 0.587f, 0.040f),
	glm::vec3(0.517f, 0.713f, 0.338f),
	glm::vec3(0.053f, 0.959f, 0.120f),
	glm::vec3(0.393f, 0.621f, 0.362f),
	glm::vec3(0.673f, 0.211f, 0.457f),
	glm::vec3(0.820f, 0.883f, 0.371f),
	glm::vec3(0.982f, 0.099f, 0.879f)
};


class Testprinter : public Component {
public:
	Testprinter(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner), owner(owner){
	}
protected:
	void update(const Update &up, Theron::Address from){
		std::cout << "object " << owner.AsInteger() << " updating after " << up.elapsedMS << " ms." << std::endl;
		finish(up, from);
	}
	void render(const Render &rend, Theron::Address from){
		std::cout << "object " << owner.AsInteger() << " rendering." << std::endl;
		finish(rend, from);
	}
	void finish(Event ev, Theron::Address from){
		Send(AllDone(ev.id), from);
	}
	virtual int id(){
		return 99;
	}
private:
	Theron::Address owner;
};

class Vibrate : public Component {
public:
	Vibrate(Theron::Framework& fw, Theron::Address owner) : Component(fw, owner){
		RegisterHandler(this, &Vibrate::initialize);
		Send(GameObject::GetComponent(TRANSFORM_ID, this->GetAddress()), owner);
	}
	
	virtual int id(){
		return 99;
	}
private:
	void update(const Update& msg, Theron::Address sender){
		if (!init) Send(AllDone(msg.id), sender);
		float rand1 = (float)(std::rand() % 100) / 1000 - 0.05;
		float rand2 = (float)(std::rand() % 100) / 1000 - 0.05;
		float rand3 = (float)(std::rand() % 100) / 1000 - 0.05;
		uint64_t id = tracker.getNextID();
		tracker.addContext(msg.id, id, sender);
		Send(Transform::Translate(id, glm::vec3(rand1, rand2, rand3)), transform);
	}
	void initialize(const Component::ComponentID &msg, Theron::Address sender){
		if (msg.id != TRANSFORM_ID) return;
		DeregisterHandler(this, &Vibrate::initialize);
		transform = sender;
		init = true;
	}

	bool init = false;
	Theron::Address transform;
};

int _tmain(int argc, _TCHAR* argv[])
{
	char c;

	stage::Gameloop loop(std::string("test"), 640, 480);
	Theron::Framework& fw = loop.getFramework();
	Theron::Address sc = loop.createScene();
	
	loop.setActiveScene(0);

	Theron::Receiver rec;
	Theron::Receiver adRec;
	Theron::Catcher<Scene::NewObject> catcher;
	Theron::Catcher<AllDone> adCatcher;
	rec.RegisterHandler(&catcher, &Theron::Catcher<Scene::NewObject>::Push);
	adRec.RegisterHandler(&adCatcher, &Theron::Catcher<AllDone>::Push);

	std::cout << "setup1\n";
	//std::cin >> c;

	Scene::NewObject obj1;
	Scene::NewObject obj2;
	Scene::NewObject obj3;
	Theron::Address whatev;

	fw.Send(Scene::CreateObject(rec.GetAddress()), rec.GetAddress(), sc);
	rec.Wait();
	catcher.Pop(obj1, whatev);
	fw.Send(Scene::CreateObject(rec.GetAddress()), rec.GetAddress(), sc);
	rec.Wait();
	catcher.Pop(obj2, whatev);
	fw.Send(Scene::CreateObject(rec.GetAddress()), rec.GetAddress(), sc);
	rec.Wait();
	catcher.Pop(obj3, whatev);

	std::cout << "setup2\n";
	//std::cin >> c;

	Testprinter* print1 = new Testprinter(fw, obj1.object);
	Testprinter* print2 = new Testprinter(fw, obj2.object);
	Testprinter* print3 = new Testprinter(fw, obj3.object);
	Transform* tr1 = new Transform(fw, obj1.object);
	Transform* tr2 = new Transform(fw, obj2.object);
	Transform* tr3 = new Transform(fw, obj3.object);
	fw.Send(Transform::SetMatrix(0, glm::mat4(1.0f)), adRec.GetAddress(), tr1->GetAddress());
	fw.Send(Transform::Translate(0, glm::vec3(2, 0, -5)), adRec.GetAddress(), tr2->GetAddress());
	fw.Send(Transform::Translate(0, glm::vec3(-1, 0, -5)), adRec.GetAddress(), tr3->GetAddress());


	std::cout << "setup3\n";
	//std::cin >> c;

	stage_common::SimpleShader ss;
	stage_common::Model mod(vertices, colors, &ss);

	std::cout << "setup4\n";
	//std::cin >> c;

	ModelComponent* mod2 = new ModelComponent(fw, &mod, obj2.object);
	ModelComponent* mod3 = new ModelComponent(fw, &mod, obj3.object);

	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 5), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	CameraComponent* cam = new CameraComponent(fw, obj1.object);
	fw.Send(CameraComponent::SetViewMatrix(0, View), adRec.GetAddress(), cam->GetAddress());
	fw.Send(CameraComponent::SetProjectionMatrix(0, Projection), adRec.GetAddress(), cam->GetAddress());
	loop.setActiveCamera(cam->getRawCamera());

	Vibrate* v2 = new Vibrate(fw, obj2.object);
	Vibrate* v3 = new Vibrate(fw, obj3.object);

	std::cout << "setup5\n";
	loop.start();
	
	std::cin >> c;
	return 0;
}


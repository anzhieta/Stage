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
	class CameraControlComponent : public Component{
	public:
		CameraControlComponent(Theron::Framework& fw, Theron::Address owner, Theron::Address transform)
			: Component(fw, owner), transform(transform){
			
			stage_common::Input& in = stage_common::Input::getSingleton();
			in.registerKey(GLFW_KEY_W);
			in.registerKey(GLFW_KEY_S);
			in.registerKey(GLFW_KEY_A);
			in.registerKey(GLFW_KEY_D);
			in.registerKey(GLFW_KEY_R);
			in.registerKey(GLFW_KEY_F);
			in.registerKey(GLFW_KEY_ESCAPE);
		}

		virtual int id(){ return CAMERACONTROLCOMPONENT_ID; }

	private:
		Theron::Address transform;

		void update(const Update& msg, Theron::Address sender){
			stage_common::Input& in = stage_common::Input::getSingleton();

			glm::vec3 movement;

			if (in.getKeyDown(GLFW_KEY_W)) movement.z += CAMERASPEED * msg.elapsedMS;
			if (in.getKeyDown(GLFW_KEY_S)) movement.z -= CAMERASPEED  * msg.elapsedMS;
			if (in.getKeyDown(GLFW_KEY_A)) movement.x += CAMERASPEED  * msg.elapsedMS;
			if (in.getKeyDown(GLFW_KEY_D)) movement.x -= CAMERASPEED  * msg.elapsedMS;
			if (in.getKeyDown(GLFW_KEY_F)) movement.y += CAMERASPEED  * msg.elapsedMS;
			if (in.getKeyDown(GLFW_KEY_R)) movement.y -= CAMERASPEED  * msg.elapsedMS;

			if (in.getKeyDown(GLFW_KEY_ESCAPE)) Send(SceneManager::Abort(), SceneManager::getGlobalManager());

			uint64_t id = tracker.getNextID();
			tracker.addContext(msg.id, id, sender);
			Send(Transform::Translate(id, movement), transform);
		}
	};
}

#endif
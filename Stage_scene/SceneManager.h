#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "stdafx.h"
#include <vector>

#include "Scene.h"

namespace stage{
	/** Abstrakti luokkka, joka tarjoaa aktorirajapinnan pääsäikeessä suoritettavaan pelisilmukkaan
	Ottaa vastaan viestit:
	SceneManager::Abort (ei lähetä vastausta)
	SceneManager::SetActiveScene (vastaa AllDone)
	SceneManager::CreateScene (vastaa SceneManager::NewScene)
	*/
	class SceneManager{
	public:
		/** Viesti, joka ilmoittaa, että pelin suoritus on lopetettava
		*/
		struct Abort{};

		/** Viesti, jolla pyydetään pelimoottoria vaihtamaan aktiivista pelialuetta
		*/
		struct SetActiveScene : ComponentEvent{
			/** Uuden pelialueen tunnusnumero
			*/
			unsigned int scene;
			SetActiveScene(uint64_t id, unsigned int scene, int senderComponent) : 
				ComponentEvent(id, senderComponent, INVALID_COMPONENT_ID), scene(scene){}
		};


		/** Viesti, jolla pyydetään pelimoottoria luomaan uusi pelialue
		*/
		struct CreateScene : ComponentEvent{
			CreateScene(uint64_t id, int senderComponent) : ComponentEvent(id, senderComponent, INVALID_COMPONENT_ID){}
		};

		/** Vastaus pelialueen luontipyyntöön - ilmoittaa uuden pelialueen tunnusnumeron ja osoitteen
		*/
		struct NewScene : ComponentEvent{
			/** Uuden pelialueen tunnusnumero
			*/
			unsigned int scene;
			/** Uuden pelialueen Theron-osoite
			*/
			Theron::Address scAddress;
			NewScene(uint64_t id, unsigned int scene, Theron::Address scAddress, int receiverComponent) : 
				ComponentEvent(id, INVALID_COMPONENT_ID, receiverComponent), scene(scene), scAddress(scAddress){}
		};

		/** Luo uuden pelialueiden hallintaolion
		*/
		SceneManager();

		/** Tuhoaa pelialueiden hallintaolion
		Toteutus:
		for (std::vector<Scene*>::iterator i = scenes.begin(); i != scenes.end(); i++){
			delete *i;
		}
		asetettava aliluokkaan, koska aliluokka vastaa Theron::Frameworkin hallinnoimisesta
		*/
		~SceneManager();

		/** Hakee globaalin pelialueiden hallintasingletonin Theron-osoitteen
		@returns	Pelialueiden hallintaolion osoite
		*/
		static Theron::Address getGlobalManager(){ return globalManager; }

		/** Asettaa aktiivisen pelialueen
		@param scene	pelialueen tunnusnumero
		*/
		virtual bool setActiveScene(unsigned int scene);

		//TODO: fix
		//virtual void setActiveCamera(Theron::Address cam);

		/** Luo uuden pelialueen
		@returns	Uuden pelialueen Theron-osoite
		*/
		virtual Theron::Address createScene();

		/** Hakee viitteen hallintaolion käyttämään Theron::Framework-olioon
		*/
		virtual Theron::Framework& getFramework() = 0;

	protected:
		/** Pelialuelista
		*/
		std::vector<Scene*> scenes;

		/** Globaalin pelialueiden hallintasingletonin Theron-osoite
		*/
		static Theron::Address globalManager;

		/** Aktiivisen pelialueen Theron-osoite
		*/
		Theron::Address activeScene;

		/** Olio, joka vastaanottaa hallintaoliolle lähetetyt viestit
		*/
		Theron::Receiver receiver;

		/** Olio, joka käsittelee hallintaolion vastaanottamat AllDone-viestit
		*/
		Theron::Catcher<AllDone> doneCatcher;

		/** Olio, joka käsittelee hallintaolion vastaanottamat Abort-viestit
		*/
		Theron::Catcher<Abort> abortCatcher;

		/** Olio, joka käsittelee hallintaolion vastaanottamat SetActiveScene-viestit
		*/
		Theron::Catcher<SetActiveScene> setSceneCatcher;

		/** Olio, joka käsittelee hallintaolion vastaanottamat CreateScene-viestit
		*/
		Theron::Catcher<CreateScene> createSceneCatcher;

	};

}

#endif
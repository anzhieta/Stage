#ifndef CAMERACOMPONENT_H
#define CAMERACOMPONENT_H

/**Kamerakomponentin komponenttitunnus*/
#define CAMERA_ID 2

#include "stdafx.h"
#include <Component.h>
#include <Camera.h>
#include <Transform.h>
#include <CoreEvents.h>
#include <iostream>
#include "GraphicsControlActor.h"

namespace stage{
	/** Peliolioon liitettävä kamerakomponentti. Toimii wrapperina stage_common::Camera:lle.
	Kun peliolioon liitetään tämä komponentti, pelimaailma voidaan piirtää sen kuvakulmasta.
	Ottaa vastaan viestit:
	Update (vastaa AllDone)
	Render (vastaa AllDone)
	CameraComponent::SetViewMatrix (vastaa AllDone)
	CamaraComponent::SetProjectionMatrix (vastaa AllDone)
	*/
	class CameraComponent : public Component{
		friend class GameLoop;
	public:
		//---Viestit---

		/** Viesti, joka pyytää kamerakomponenttia asettamaan uuden näkymämatriisin 
		(Huom. normaalisti kamera hakee omistajaolioltaan näkymämatriisin joka Render-viestin yhteydessä)*/
		struct SetViewMatrix : public Event{
			/** Uusi näkymämatriisi*/
			glm::mat4& view;
			SetViewMatrix(uint64_t id, glm::mat4& view) : Event(id), view(view){}
		};
		/** Viesti, joka pyytää kamerakomponenttia asettamaan uuden projektiomatriisin */
		struct SetProjectionMatrix : public Event{
			/** Uusi projektiomatriisi*/
			glm::mat4& projection;
			SetProjectionMatrix(uint64_t id, glm::mat4& projection) : Event(id), projection(projection){}
		};

		//---Metodit---

		/** Luo uuden kamerakomponentin. Katso oikea käyttö yliluokasta.
		@param fw		Theron::Framework, jonka alaisuudessa tämä komponentti toimii
		@param owner	Sen peliolion osoite, joka omistaa tämän komponentin
		*/
		CameraComponent(Theron::Framework& fw, Theron::Address owner);		
		/** Hakee osoittimen tämän komponentin kameraolioon
		HUOM: ei säieturvallinen, älä käytä paluuarvoa pelimoottorin ollessa käynnissä
		@returns	Osoitin kameraolioon
		*/
		stage_common::Camera* getRawCamera(){ return &cam; }
		/** Hakee olion komponenttitunnuksen
		@returns	Tämän komponentin tunnus
		*/
		virtual int id(){ return CAMERA_ID; }
	private:
		/** Onko komponentti käynnistetty, eli voiko se suorittaa update- ja render-kutsuja	*/
		bool init = false;
		/** Komponentin kameraolio*/
		stage_common::Camera cam;
		/** Omistajaolion sijaintia ylläpitävän olion osoite*/
		Theron::Address transform;

		//---Metodit---

		/** Suorittaa loppuun komponentin käynnistyksen
		@param msg		Sijaintiolion komponenttitunnuksen sisältävä viesti
		@param sender	Sijaintiolion osoite
		*/
		void initialize(const GameObject::ComponentFound &msg, Theron::Address sender);
		/** Suorittaa tarvittavan laskennan ruudun piirtoa varten
		@param msg		Renderöintipyyntö
		@param sender	Lähettäjän osoite
		*/
		virtual void render(const Render& msg, Theron::Address sender);
		/** Suorittaa loppuun tarvittavan laskennan ruudun piirtoa varten
		@param msg		Kameran uusi näkymämatriisi
		@param sender	Lähettäjän osoite
		*/
		void completeRender(const Transform::Matrix& msg, Theron::Address sender);
		/** Asettaa kameralle uuden näkymämatriisin
		@param msg		Kameran uusi näkymämatriisi
		@param sender	Lähettäjän osoite
		*/
		void setViewMatrix(const SetViewMatrix& msg, Theron::Address sender);
		/** Asettaa kameralle uuden projektiomatriisin
		@param msg		Kameran uusi projektiomatriisi
		@param sender	Lähettäjän osoite
		*/
		void setProjectionMatrix(const SetProjectionMatrix& msg, Theron::Address sender);
	};
}
#endif
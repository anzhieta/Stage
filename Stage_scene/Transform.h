#pragma once

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define TRANSFORM_ID 1

#include "stdafx.h"
#include "Component.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <CoreEvents.h>

namespace stage{
	/** Peliolion sijaintia, suuntaa ja kokoa ylläpitävä koodrinaattikomponenttiolio
	Ottaa vastaan viestit:
	Update (palauttaa AllDone)
	Render (palauttaa AllDone)
	Transform::GetMatrix (palauttaa Transform::Matrix)
	Transform::SetMatrix (palauttaa AllDone)
	Transform::Translate (palauttaa AllDone)
	*/
	class Transform : public Component {
	public:

		/** Viesti, joka ilmoittaa Transform-olion tilan vastauksena GetMatrix-viestiin
		*/
		struct Matrix : public ComponentEvent {
			/** 4x4-matriisi, joka kertoo peliolion sijainnin 3D-maailmassa
			*/
			glm::mat4& matrix;
			//Matrix(glm::mat4& mt, Theron::Address originator, uint32_t msgID) : matrix(mt), Event(originator, msgID){}
			Matrix(uint64_t msgID, glm::mat4& mt, int receiverComponent) : matrix(mt), ComponentEvent(msgID, TRANSFORM_ID, receiverComponent){}
		};

		/** Viesti, joka ilmoittaa Transform-olion sijainnin vastauksena GetMatrix-viestiin
		*/
		struct Position : public ComponentEvent {
			/** 4x4-matriisi, joka kertoo peliolion sijainnin 3D-maailmassa
			*/
			glm::vec3 position;
			//Position(glm::vec3& pos, Theron::Address originator, uint32_t msgID) : position(pos), Event(originator, msgID){}
			Position(uint64_t msgID, glm::vec3 pos, int receiverComponent) : position(pos), ComponentEvent(msgID, TRANSFORM_ID, receiverComponent){}
		};

		/** Viesti, jolla pyydetään Transform-komponenttia lähettämään tiedon nykyisestä tilastaan
		*/
		struct GetMatrix : public ComponentEvent {
			//GetMatrix(Theron::Address originator, uint32_t msgID) : Event(originator, msgID){}
			GetMatrix(uint64_t msgID, int senderComponent) : ComponentEvent(msgID, senderComponent, TRANSFORM_ID){}
		};

		/** Viesti, jolla pyydetään Transform-komponenttia lähettämään nykyisen sijaintinsa
		*/
		struct GetPosition : public ComponentEvent {
			//GetPosition(Theron::Address originator, uint32_t msgID) : Event(originator, msgID){}
			GetPosition(uint64_t msgID, int senderComponent) : ComponentEvent(msgID, senderComponent, TRANSFORM_ID){}
		};

		/** Viesti, jolla pyydetään Transform-komponenttia asettamaan itselleen uusi tila
		*/
		struct SetMatrix : public ComponentEvent {
			/** 4x4-matriisi, joka kertoo peliolion uuden sijainnin 3D-maailmassa
			*/
			glm::mat4& matrix;
			//SetMatrix(Theron::Address originator, uint32_t msgID, glm::mat4& mt) : matrix(mt), Event(originator, msgID){}
			SetMatrix(uint64_t msgID, glm::mat4& mt, int senderComponent) : matrix(mt), ComponentEvent(msgID, senderComponent, TRANSFORM_ID){}
		};

		/** Viesti, jolla pyydetään Transform-komponenttia siirtämään pelioliotaan tiettyyn suuntaan
		*/
		struct Translate : public ComponentEvent {
			/** Vektori, joka kertoo miten paljon ja mihin suuntaan oliota siirretään
			*/
			glm::vec3 vector;
			//Translate(Theron::Address originator, uint32_t msgID, glm::vec3 vec) : vector(vec), Event(originator, msgID){}
			Translate(uint64_t msgID, glm::vec3 vec, int senderComponent) : vector(vec), ComponentEvent(msgID, senderComponent, TRANSFORM_ID){}
		};

		/** Luo uuden koordinaattikomponentin. Katso oikea käyttö yliluokasta.
		@param fw		Komponenttia ylläpitävä Theron::Framework
		@param owner	Komponentin omistava peliolio
		@param tr		Komponentin alkutilaa kuvaava 4x4-matriisi
		*/
		Transform(Theron::Framework& fw, Theron::Address owner, glm::mat4& tr = glm::mat4());
		
		/** Palauttaa koordinaattikomponentin tunnusluvun
		*/
		virtual int id(){ return TRANSFORM_ID; }
		virtual std::string name(){ return std::string("Transform"); }

		glm::mat4& getMatrix(){ return transform; }
		glm::vec3 getPosition(){ return glm::vec3(transform[3]); };

	protected:
		virtual void initialize(GameObject* owner);
	private:
		
		/** Komponentin omistavan peliolion sijaintia 3D-pelimaailmassa kuvaava 4x4-matriisi
		*/
		glm::mat4 transform;			

		/** Hakee tämän komponentin sisäistä tilaa kuvaavan matriisin
		@param msg		Tilan hakupyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void getMatrix(const GetMatrix& msg, Theron::Address sender);

		/** Hakee tämän komponentin sijaintia pelimaailmassa kuvaavan vektorin
		@param msg		Sijainnin hakupyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void getPosition(const GetPosition& msg, Theron::Address sender);

		/** Asettaa tämän komponentin sisäistä tilaa kuvaavan matriisin
		@param msg		Tilan muutospyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void setMatrix(const SetMatrix& msg, Theron::Address sender);

		/** Siirtää tämän komponentin kuvaamia koordinaatteja haluttuun suuntaan
		@param msg		Siirtopyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void translate(const Translate& msg, Theron::Address sender);
	};
}

#endif
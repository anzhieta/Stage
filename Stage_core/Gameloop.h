#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "stdafx.h"
#include <Theron\Framework.h>
#include <SceneManager.h>
#include "GraphicsControlActor.h"
#include <LogActor.h>
#include <EventChannelManager.h>

namespace stage {

	/** Olio, joka pyörittää pelimoottorin pelisilmukkaa ja hoitaa pelimoottorin yleiseen ylläpitoon kuuluvia asioita.
	*/
	class Gameloop : public SceneManager{
	public:

		/** Luo pelisilmukan ja avaa uuden OpenGL-ikkunan.
		@param windowName	Ikkunan nimi
		@param xres			Ikkunan vaakaresoluutio
		@param yres			Ikkunan pystyresoluutio
		*/
		Gameloop(std::string& windowName, int xres, int yres, uint32_t threadcount);

		/** Tuhoaa pelisilmukan
		*/
		~Gameloop();

		/** Palauttaa pelisilmukan aikaskaalan (kuinka nopeasti simulaatiota suoritetaan)
		@returns	Aikaskaala liukulukuna
		*/
		float getTimescale();

		/** Asettaa pelisilmukan aikaskaalan (kuinka nopeasti simulaatiota suoritetaan)
		@param ts	Aikaskaala liukulukuna
		*/
		void setTimescale(float ts);

		/** Palauttaa viitteen pelimaailman olioita hallinnoivaan Theron::Framework-olioon
		*/
		Theron::Framework& getFramework(){ return fw; }

		/** Asettaa pelin pääkameran, jonka näkökulmasta pelimaailmaa kuvataan
		@param cam	Viite kameraolioon
		*/
		void setActiveCamera(stage_common::Camera* cam);

		/** Käynnistää pelisilmukan suorituksen, joka päättyy peliohjelman sulkeutuessa
		*/
		void start();

		/** Hakee viitteen tapahtumakanavia hallinnoivaan aktoriin
		@returns	Viite tapahtumakanavien hallinta-aktoriin
		*/
		EventChannelManager& getEventChannelManager(){ return eventChannelManager; }
		
	private:
		/** Pelin olioita ja niiden välisiä viestejä hallinnoiva olio
		*/
		Theron::Framework fw;

		/** Grafiikkamoottorin toiminnasta vastaava aktori
		*/
		GraphicsControlActor* gc;

		/** Lokista vastaava aktori
		*/
		LogActor* logger;

		/** Kamera, jonka kuvakulmasta pelimaailma piirretään
		*/
		stage_common::Camera* activeCam;

		/** Tapahtumakanavia huoltava aktori
		*/
		EventChannelManager eventChannelManager;


		/** Pelisilmukan aikaskaala (kuinka nopeasti simulaatiota suoritetaan)
		*/
		float timescale = 1;

		/** Pysäytetäänkö pelisilmukan suoritus tämän kierroksen jälkeen
		*/
		bool abort = false;

		/** Seuraavan lähetettävän viestin tunnus
		*/
		uint32_t msgid = 0;


		/** Pelisilmukkametodi: kutsuu toistuvasti peliolioiden update- ja render-metodeja.
		*/
		void loop();

		/** Pysäyttää pelisilmukan suorituksen
		*/
		void stop();

		/** Viimeistelee pelisilmukan pysäyttämisen ja jättää pelisilmukan tilaan, jossa se voidaan turvallisesti tuhota.
		*/
		void shutdown();
	};
}

#endif
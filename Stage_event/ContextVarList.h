#ifndef CONTEXTVARLIST_H
#define CONTEXTVARLIST_H

#include "stdafx.h"
#include <boost\any.hpp>

namespace stage{
	/** Linkitetyn listan alkio, joka pitää sisällään tapahtumakontekstiin liittyvän muuttujan
	*/
	struct ContextVar{
		/** Muuttujan arvo
		*/
		boost::any content;

		/** Muuttujalistan seuraava alkio
		*/
		ContextVar* next = nullptr;

		/** Luo uuden lista-alkion
		*/
		ContextVar(boost::any content) : content(content){}

		/** Tuhoaa lista-alkion sekä rekursiivisesti kaikki sitä seuraavat alkiot
		*/
		~ContextVar(){
			if (next != nullptr) delete next;
		}

		/** Hakee rekursiivisesti muuttujalistasta yksittäisen muuttujan
		@param depth	Haettavan muuttujan järjestysluku tästä alkiosta eteenpäin (0 palauttaa tämän alkion muuttujan)
		@returns		Haettavan muuttujan arvo
		*/
		boost::any get(int depth){
			if (depth < 1) return content;
			return next->get(depth - 1);
		}

		/** Asettaa rekursiivisesti muuttujalistaan yksittäisen muuttujan arvon
		@param depth	Asetettavan muuttujan järjestysluku tästä alkiosta eteenpäin (0 asettaa tämän alkion muuttujan)
		@param content	Asetettavan muuttujan arvo
		*/
		void set(int depth, boost::any content){
			if (depth < 1) this->content = content;
			else {
				if (next == nullptr){
					boost::any a;
					next = new ContextVar(a);
				}
				next->set(depth - 1, content);
			}
		}
	};
}

#endif
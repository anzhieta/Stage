#ifndef LOGACTOR_H
#define LOGACTOR_H

#include "stdafx.h"
#include <Logger.h>
#include <iostream>

/**Makro, jolla kirjoitetaan lokiin normaali lokiviesti*/
#define LOGMSG(MSG) Send(LogActor::LogMessage(MSG), LogActor::getGlobalLogger());
/**Makro, jolla kirjoitetaan lokiin normaali virheilmoitus*/
#define LOGERR(MSG) Send(LogActor::LogError(MSG), LogActor::getGlobalLogger());

namespace stage{
	/** Aktori, joka tarjoaa pelimoottorille lokipalvelun.
	Kapseloi lokipalvelun aktorin sisään, jotta lokiin voidaan kirjoittaa säieturvallisesti
	Ensimmäinen luotu lokiaktori toimii globaalina singletonina, jota muut aktorit oletuksena hyödyntävät.
	Ottaa vastaan viestit:
	LogActor::LogMessage
	LogActor::LogError
	LogActor::Terminate (palauttaa LogActor::Terminate)
	*/
	class LogActor : public Theron::Actor {
	public:
		//---Viestit---

		/** Viesti, jolla pyydetään lokiaktoria kirjoittamaan merkintä lokiin*/
		struct LogMessage{
			/** Lokiin kirjoitettava viesti*/
			std::string message;
			LogMessage(std::string msg) : message(msg){}
		};
		/** Viesti, jolla pyydetään lokiaktoria kirjoittamaan merkintä virhelokiin*/
		struct LogError{
			/** Lokiin kirjoitettava viesti*/
			std::string message;
			LogError(std::string msg) : message(msg){}
		};
		/** Viesti, jolla pyydetään lokia valmistautumaan järjestelmän sammuttamiseen.
		Kun kaikki nykyiset viestit on saatu käsiteltyä, lokiaktori ei enää ota vastaan viestejä ja palautaa takaisin
		saman Terminate-viestin, joka ilmaisee, että aktori voidaan tuhota turvallisesti.*/
		struct Terminate{};

		//---Metodit---

		/** Luo uuden lokiaktorin. Jos muita lokiaktoreita ei ole luotu, luotavan aktorin
		Theron-osoitteen saa selville getGlobalLogger-funktiolla.
		@param fw			Lokiaktoria hallinnoiva Theron::Framework
		@param standard		ostream, johon kirjoitetaan normaalit lokiviestit (oletusarvo cout)
		@param error		ostream, johon kirjoitetaan virheilmoitukset (oletusarvo cerr)
		*/
		LogActor(Theron::Framework& fw, std::ostream& standard = std::cout, std::ostream& error = std::cerr);
		/** Palauttaa globaalin lokiaktorin osoitteen.
		Globaali lokiaktori toimii oletuslokina, johon kirjoitetaan kaikki lokiviestit ellei toisin määritellä
		@returns	Globaalin lokiaktorin Theron-osoite
		*/
		static Theron::Address getGlobalLogger(){ return globalLogger; }
	private:
		/** Globaalin lokiaktorin Theron-osoite*/
		static Theron::Address globalLogger;
		/** Tätä aktoria sammumaan pyytäneen aktorin Theron-osoite*/
		Theron::Address terminator;
		/** Lokiolio, joka kirjoittaa vastaanotettavat viestit lokiin*/
		stage_common::Logger logger;

		/** Kirjoittaa merkinnän lokiin
		@param msg		LogMessage-tietue, joka sisältää kirjoitettavan viestin
		@param sender	Lokimerkintää pyytävän aktorin osoite
		*/
		void logMessage(const LogMessage& msg, Theron::Address sender);
		/** Kirjoittaa virheilmoituksen lokiin
		@param msg		LogMessage-tietue, joka sisältää kirjoitettavan viestin
		@param sender	Lokimerkintää pyytävän aktorin osoite
		*/
		void logError(const LogError& msg, Theron::Address sender);
		/** Asettaa lokiaktorin tilaan, jossa se sammuu, kun se on saanut kaikki lokimerkintänsä kirjoitettua lokiin
		@param msg		Sammutuspyyntö
		@param sender	Sammutusta pyytävän aktorin osoite
		*/
		void terminate(const Terminate& msg, Theron::Address sender);
		/** Tarkistaa, onko vielä kirjoitettavia viestejä - jos ei, sammuttaa lokiaktorin
		*/
		void tryShutdown();
	};
}
#endif
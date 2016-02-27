#ifndef WAITER_H
#define WAITER_H

#define WAITER_ID 7

#include "stdafx.h"
#include <Component.h>

namespace stage{
	/**Pelioliokomponentti, joka simuloi raskasta laskentaa etsimällä tietyn määrän alkulukuja jokaisen ruudunpäivityksen aikana*/
	class Waiter : public Component{
	public:
		/**Luo uuden Waiter-komponentin.
		@param fw		Komponenttiaktoria hallinnoiva aktorijärjestelmä
		@param owner	Komponentin omistava peliolio
		@param limit	Se luku, johon asti alkulukuja etsitään joka ruudunpäivityksen aikana
		*/
		Waiter(Theron::Framework& fw, Theron::Address owner, int limit) :
			Component(fw, owner), limit(limit){}
		/**Suorittaa päivitysvaiheen laskennan
		@param up	Päivityspyyntöviesti
		@param from	Pyynnön lähettäjä
		*/
		virtual void update(const Update &up, Theron::Address from){
			for (int i = 3; i <= limit; i++){
				for (int j = 2; j < i; j++){
					if (i % j == 0) break;
				}
			}
			Send(AllDone(up.id), from);
		}
		/**Hakee Waiter-komponentin komponenttitunnuksen
		@returns	Tämän komponentin komponenttitunnus
		*/
		virtual int id(){ return WAITER_ID; }
	private:
		/**Se luku, johon asti alkulukuja etsitään jokaisen ruudunpäivityksen aikana*/
		int limit;
	};
}
#endif
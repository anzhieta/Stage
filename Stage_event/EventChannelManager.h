#ifndef EVENTCHANNELMANAGER_H
#define EVENTCHANNELMANAGER_H

#include "stdafx.h"
#include "ContextTracker.h"
#include "CoreEvents.h"

namespace stage{
	/** Luokka, joka pitää kirjaa tapahtumakanavista ja huoltaa niitä*/
	class EventChannelManager : public Theron::Actor{
	public:
		//---Viestit---

		/** Viesti, jolla pyydetään tapahtumakanavia suorittamaan huollon eli lisäämään ja
		poistamaan lisäystä tai poistoa odottavat aktorit vastaanottajalistastaan*/
		struct ChannelMaintenance: public Event{
			ChannelMaintenance(uint64_t id): Event(id){}
		};

		//---Metodit---

		/** Luo uuden EventChannelManager-aktorin
		@param fw	Aktoria hallinnoiva Theron::Framework
		*/
		EventChannelManager(Theron::Framework& fw): Theron::Actor(fw), tracker(fw, this->GetAddress()){
			RegisterHandler(this, &EventChannelManager::channelMaintenance);
			RegisterHandler(this, &EventChannelManager::allDone);
		}
		/** Lisää kanavan huollettavien kanavien listaan
		@param channel	Lisättävä kanava
		*/
		void addChannel(Theron::Address channel){
			channels.push_back(channel);
		}
	private:
		/** Lista hallinnoitavista kanavista*/
		std::list<Theron::Address> channels;
		/** Tapahtumakonteksteista kirjaa pitävä olio*/
		ContextTracker tracker;

		/** Pyytää kaikkia hallinnoimiaan kanavia suorittamaan huollon
		@param msg		Huoltopyyntö
		@param sender	Pyynnön lähettäjä
		*/
		void channelMaintenance(const ChannelMaintenance& msg, Theron::Address sender){
			if (channels.size() == 0) Send(AllDone(msg.id), sender);
			ChannelMaintenance newmsg(tracker.getNextID());
			for (std::list<Theron::Address>::const_iterator it = channels.begin(); it != channels.end(); it++){
				tracker.trackedSend(msg.id, newmsg, *it, sender);
			}
		}
		/** Käsittelee kanavilta saapuvat huollon päättymisestä ilmoittavat viestit
		@param msg		Huollon päättymisestä kertova viesti
		@param sender	Viestin lähettäjä
		*/
		void allDone(const AllDone& msg, Theron::Address sender){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
	};
}
#endif
#pragma once
#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include "stdafx.h"
#include <list>
#include "CoreEvents.h"
#include <iostream>
#include "LogActor.h"
#include "EventChannelManager.h"

namespace stage {

	/** Luokka, joka vastaa tapahtumaviestien lähettämisetä eteenpäin niistä kiinnostuneille aktoreille
	Ottaa vastaan viestit:
	<Tapahtumakanavan tyyppiparametri> (palauttaa AllDone)
	EventChannel::RegisterRecipient
	EventChannel::DeregisterRecipient
	EventChannelManager::ChannelMaintenance (palauttaa AllDone)
	*/
	template <class MessageType>
	class EventChannel : public Theron::Actor{
	public:
		/** Viesti, jolla lisätään uusi vastaaottaja kanavan vastaanottajalistaan
		*/
		struct RegisterRecipient {
			RegisterRecipient(Destination rec) : recipient(rec){}
			/** Rekisteröitävän aktorin Theron-osoite
			*/
			Destination recipient;
		};
		/** Viesti, jolla poistetaan vastaanottaja vastaanottajalistasta
		*/
		struct DeregisterRecipient {
			DeregisterRecipient(Destination rec) : recipient(rec){}
			/** Poistettavan aktorin Theron-osoite
			*/
			Destination recipient;
		};

		/** Luo uuden tapahtumakanavan
		@param fw	Tapahtumakanavaa hallinnoiva Theron::Framework
		*/
		EventChannel(Theron::Framework& fw) : Theron::Actor(fw), recipients(), tracker(fw, Destination(this->GetAddress(), INVALID_COMPONENT_ID)){
			RegisterHandler(this, &EventChannel<MessageType>::forward);
			RegisterHandler(this, &EventChannel<MessageType>::registerRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::deregisterRecipient);
			RegisterHandler(this, &EventChannel<MessageType>::allDone);
			RegisterHandler(this, &EventChannel<MessageType>::error);
			RegisterHandler(this, &EventChannel<MessageType>::channelMaintenance);
		}

		/** Hakee listan kanavaa kuuntelemaan rekisteröityneistä aktoreista
		Säieturvallinen pelisilmukan päivitys- ja piirtovaiheissa
		@returns	Kanavaa kuuntelemaan rekisteröityneet aktorit
		*/
		const std::list<Destination>& getRecipients(){
			return recipients;
		}
		
	private:
		/** Lista aktoreista, jotka ovat rekisteröityneet kuuntelemaan kanavaa
		*/
		std::list<Destination> recipients;
		/** Lista aktoreista, jotka lisätään kuuntelijalistaan ruudunpäivityksen lopussa
		*/
		std::list<Destination> pendingAdd;
		/** Lista aktoreista, jotka poistetaan kuuntelijalistasta ruudunpäivityksen lopussa
		*/
		std::list<Destination> pendingRemove;
		/** Tapahtumakanavan konteksteista kirjaa pitävä olio
		*/
		ContextTracker tracker;

		/** Lähettää viestin eteenpäin kaikille tapahtumakanavaan rekisteröityneille aktoreille
		alkuperäistä lähettäjää lukuun ottamatta
		@param msg	Viesti, joka jaetaan kaikille kuuntelijoille
		@param from	Viestin lähettäjä
		*/
		void forward(const MessageType &msg, const Theron::Address from){
			int sent = 0;
			for (std::list<Destination>::const_iterator it = recipients.begin(); it != recipients.end(); it++){
				if (!((*it).address == from && (*it).component == msg.senderComponent)){
					tracker.trackedSend(msg.id, msg, (*it).address, Destination(from, msg.senderComponent));
					sent++;
				}
			}
			if (sent == 0){
				//Jos ei kuuntelijoita, kaikki valmista
				Send(AllDone(msg.id, INVALID_COMPONENT_ID, msg.senderComponent), from);
			}
		}
		/** Liittää uuden kuuntelijan kuuntelijalistaan ruudunpäivityksen lopussa
		@param msg	Rekisteröintipyyntö
		@param from	Pyynnön lähettäjä
		*/
		void registerRecipient(const RegisterRecipient& msg, const Theron::Address from){
			pendingAdd.push_back(msg.recipient);
		}
		/** Poistaa kuuntelijan kuuntelijalistasta ruudunpäivityksen lopussa
		@param msg	Poistopyyntö
		@param from	Pyynnön lähettäjä
		*/
		void deregisterRecipient(const DeregisterRecipient& msg, const Theron::Address from){
			pendingRemove.push_back(msg.recipient);
		}
		/** Lisää ja poistaa kuuntelijalistasta kaikki lisäysta tai poistoa odottavat aktorit
		@param msg	Kanavanhuoltopyyntö
		@param from	Pyynnön lähettäjä
		*/
		void channelMaintenance(const EventChannelManager::ChannelMaintenance& msg, const Theron::Address from){
			for (std::list<Destination>::const_iterator it = pendingAdd.begin(); it != pendingAdd.end(); it++){
				recipients.push_back(*it);
			}
			pendingAdd.clear();
			for (std::list<Destination>::const_iterator it1 = pendingRemove.begin(); it1 != pendingRemove.end(); it1++){
				for (std::list<Destination>::const_iterator it2 = recipients.begin(); it2 != recipients.end(); it2++){
					if (*it1 == *it2) recipients.erase(it2);
				}
			}
			pendingRemove.clear();
			Send(AllDone(msg.id, INVALID_COMPONENT_ID, INVALID_COMPONENT_ID), from);
		}

		/**Käsittelee eteenpäin lähetettyjen viestien käsittelyn päättymisestä ilmoittavat viestit
		@param msg	Valmistumisesta ilmoittava viesti
		@param from	Viestin lähettäjä
		*/
		void allDone(const AllDone& msg, const Theron::Address from){
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}

		/**Käsittelee eteenpäin lähetettyjen viestien käsittelyssä tapahtuneesta virheestä ilmoittavat viestit
		@param msg	Virheestä ilmoittava viesti
		@param from	Viestin lähettäjä
		*/
		void error(const Error &msg, Theron::Address from){
			LOGERR(std::string("Warning: component ") + from.AsString() + " reported error during processing");
			if (tracker.contains(msg.id)) tracker.decrement(msg.id);
		}
	};
}

#endif
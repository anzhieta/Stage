#ifndef LOGACTOR_H
#define LOGACTOR_H

#include "stdafx.h"
#include <Logger.h>
#include <iostream>

namespace stage{
	class LogActor : public Theron::Actor {
	public:
		struct LogMessage{
			std::string message;
			LogMessage(std::string msg) : message(msg){}
		};
		struct LogError{
			std::string message;
			LogError(std::string msg) : message(msg){}
		};
		struct Terminate{};
		LogActor(Theron::Framework& fw, std::ostream& standard = std::cout, std::ostream& error = std::cerr) : Theron::Actor(fw), logger(standard, error){
			if (globalLogger == Theron::Address::Null()) globalLogger = this->GetAddress();
			terminator = Theron::Address::Null();
			RegisterHandler(this, &LogActor::logMessage);
			RegisterHandler(this, &LogActor::logError);
			RegisterHandler(this, &LogActor::terminate);
		}

		static Theron::Address getGlobalLogger(){ return globalLogger; }
	private:
		static Theron::Address globalLogger;
		Theron::Address terminator;
		stage_common::Logger logger;

		void logMessage(const LogMessage& msg, Theron::Address sender){
			logger.Log(msg.message);
			if (terminator != Theron::Address::Null()){
				tryShutdown();
			}
		}
		void logError(const LogError& msg, Theron::Address sender){
			logger.LogError(msg.message);
			if (terminator != Theron::Address::Null()){
				tryShutdown();
			}
		}
		void terminate(const Terminate& msg, Theron::Address sender){
			logger.Log("Logger ordered to shut down");
			this->terminator = sender;
			tryShutdown();
		}
		void tryShutdown(){
			if (this->GetNumQueuedMessages() <= 1){
				logger.Log("Logger shutting down");
				DeregisterHandler(this, &LogActor::logMessage);
				DeregisterHandler(this, &LogActor::logError);
				DeregisterHandler(this, &LogActor::terminate);
				Send(Terminate(), terminator);
			}
		}
	};
}

#endif
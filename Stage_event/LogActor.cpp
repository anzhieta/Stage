#include "stdafx.h"
#include "LogActor.h"

using namespace stage;

/** Globaalin lokiosoitteen alkuarvo*/
Theron::Address LogActor::globalLogger = Theron::Address::Null();

LogActor::LogActor(Theron::Framework& fw, std::ostream& standard, std::ostream& error) : Theron::Actor(fw), logger(standard, error){
	if (globalLogger == Theron::Address::Null()) globalLogger = this->GetAddress();
	terminator = Theron::Address::Null();
	RegisterHandler(this, &LogActor::logMessage);
	RegisterHandler(this, &LogActor::logError);
	RegisterHandler(this, &LogActor::terminate);
}
void LogActor::logMessage(const LogMessage& msg, Theron::Address sender){
	logger.Log(msg.message);
	if (terminator != Theron::Address::Null()){
		tryShutdown();
	}
}
void LogActor::logError(const LogError& msg, Theron::Address sender){
	logger.LogError(msg.message);
	if (terminator != Theron::Address::Null()){
		tryShutdown();
	}
}
void LogActor::terminate(const Terminate& msg, Theron::Address sender){
	logger.Log("Logger ordered to shut down");
	this->terminator = sender;
	tryShutdown();
}
void LogActor::tryShutdown(){
	if (this->GetNumQueuedMessages() <= 1){
		logger.Log("Logger shutting down");
		DeregisterHandler(this, &LogActor::logMessage);
		DeregisterHandler(this, &LogActor::logError);
		DeregisterHandler(this, &LogActor::terminate);
		Send(Terminate(), terminator);
	}
}
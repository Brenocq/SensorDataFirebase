///////////////////////////////////////////////////////////
//  FirebaseSensorRtDbSend.h
//  Implementation of the Class FirebaseSensorRtDbSend
//  Created on:      17-jan-2019 13:10:18
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////
#if !defined(FirebaseSensorRtDbSend_H)
#define FirebaseSensorRtDbSend_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



class FirebaseSensorRtDbSend
{
public:
	FirebaseSensorRtDbSend();//_codeCommunication deve ser >=1

	void addSensor(String name);
	void updateValue(String name, float value);
	
	void run(int hour, int minute, int second, int dayOfWeek, int day, int month, int year);//RUN library (always after send all data)
private:
	void sendHour(int hour, int minute, int second, int dayOfWeek, int day, int month, int year);//send hour to Node
	void sendValue(String name, float value);//send sensor values to Node

	String names[30];//name of each sensor
	float values[30][30];//30 sensores - each sensor receive up to 30 values (30 minutes)
	bool checkCycles[48];//check each cycle
};
#endif // !defined(FirebaseSensorRtDbSend_H)

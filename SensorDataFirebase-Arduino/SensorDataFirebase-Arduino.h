///////////////////////////////////////////////////////////
//  SensorDataFirebase-Arduino.h
//  Implementation of the Class SensorDataFirebaseArduino
//  Created on:      17-jan-2019 13:10:18
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////

//This class was made to send data from sensors connected in one Arduino to NodeMCU or Raspberry using UART
//Check more info about how to use this library here: https://github.com/Brenocq/SensorDataFirebase

#if !defined(SensorDataFirebaseArduino_H)
#define SensorDataFirebaseArduino_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class SensorDataFirebaseArduino
{
public:
	SensorDataFirebaseArduino();

	void begin();

	void addSensor(String name);//to add sensors to send data to the other device
	void updateValue(String name, float value);//update data from one sensor (can update up to 30 time in 30 minutes)

	void run(int hour, int minute, int second, int dayOfWeek, int day, int month, int year);//RUN the library (use after update each sensor)
private:
	void sendHour(int hour, int minute, int second, int dayOfWeek, int day, int month, int year);//send hour to the other device
	void sendValue(String name, float value);//send sensor values to the other device

	String names[30];//	to store the name of each sensor
	float values[30][30];//	maximum of 30 sensors - each sensor receive up to 30 values (30 minutes)
	bool checkCycles[48];//	to check each cycle
};
#endif // !defined(SensorDataFirebaseArduino_H)

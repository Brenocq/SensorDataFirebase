///////////////////////////////////////////////////////////
//  SensorDataFirebase-NodeMCU.h
//  Implementation of the Class SensorDataFirebaseNodeMCU
//  Created on:      17-jan-2019 13:10:18
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////
#if !defined(SensorDataFirebaseNodeMCU_H)
#define SensorDataFirebaseNodeMCU_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class SensorDataFirebaseNodeMCU
{
public:
	SensorDataFirebaseNodeMCU();

	void begin();

	void addSensor(String name, String address);

	void run();
private:
	void updateFirebaseToday(int index, float value );//add value to Day
	void updateFirebaseYesterday( int index);//pass values from Today to Yesterday and delete Today
	void updateFirebaseMonth(int index);//pass values from Today to Month
	void updateFirebaseYear( int index);//pass values from Month to Year

	String names[30];//name of each sensor
	String addresses[30];//address of each sensor

	float valuesWeek[7];//Store data to update at the end of the week
	float valuesToday[30][48];//

	int hour, minute, second, dayOfWeek, day, month, year;
};
#endif // !defined(SensorDataFirebaseNodeMCU_H)

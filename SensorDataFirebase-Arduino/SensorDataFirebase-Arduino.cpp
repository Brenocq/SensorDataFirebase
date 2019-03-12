///////////////////////////////////////////////////////////
//  FirebaseSensorRtDbSend.cpp
//  Implementation of the Class FirebaseSensorRtDbSend
//  Created on:      17-jan-2019 13:10:18
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////

#include "SensorDataFirebase-Arduino.h"

#define SerialToSend Serial1 //To use (TX0 define as Serial) (TX1 define as Serial1) (TX2 define as Serial2) (TX3 define as Serial3)
#define isSerial0    false	//set as true if SerialToSend = Serial (TX0). Otherwise set as false

FirebaseSensorRtDbSend::FirebaseSensorRtDbSend()
{
	for (int i = 0; i < 30; i++){
		names[i]=" ";
	}
	for (int i = 0; i < 48; i++){
		checkCycles[i] == false;
	}
	for (int i = 0; i < 30; i++){
		for (int j = 0; j < 30; j++){
			values[i][j] = -1;
		}
	}
}

void FirebaseSensorRtDbSend::begin(){
	Serial.begin(9600);

	if (!isSerial0)
		SerialToSend.begin(115200);

}

void FirebaseSensorRtDbSend::addSensor(String name){
	for (int i = 0; i < 30; i++){//seach for empty index
		if (names[i] == " "){
			names[i] = name;
			i = 30;
		}
	}
}

void FirebaseSensorRtDbSend::updateValue(String name, float value){
	int indexSensor = 0;
	for (int i = 0; i < 30; i++){
		if (names[i] == name){
			indexSensor = i;
			i = 30;
		}
	}

	for (int i = 0; i < 30; i++){
		if (values[indexSensor][i] == -1){
			values[indexSensor][i] = value;
			i = 30;
		}
	}
	if (!isSerial0){
		Serial.print("update("); Serial.print(name); Serial.print("): "); Serial.println(value);
	}
	
}

void FirebaseSensorRtDbSend::run(int hour, int minute, int second, int dayOfWeek, int day, int month, int year){

	int actualCycle = (minute + hour * 60) / 30;// how int work: 15/30=0  29/30=0   30/30=1 

	//--- Send Hour ---//
	sendHour(hour, minute, second, dayOfWeek, day, month, year);
	//--- Check cycle ---//
	if (checkCycles[actualCycle]==false){//first time cycle 30minutes (just run one cicle once)
		checkCycles[actualCycle] = true;

		//--- Clean Previous Cycle ---//
		int lastCycle = actualCycle - 1;
		lastCycle < 0 ? lastCycle += 48 : lastCycle;
		checkCycles[lastCycle] = false;
		//--- Send sensor values ---//
		for (int i = 0; i < 30; i++){//for each sensor
			if (names[i] != " "){
				float sum=0;//sum of all values from that sensor in 30 minutes
				float numbersSum=0;//value sum
				for (int j = 0; j < 30; j++){//for each value
					if (values[i][j] != -1){
						sum += values[i][j];
						numbersSum++;
					}
				}
				float mean = sum / numbersSum;//calculate mean
				sendValue(names[i], mean);//send value to Node
			}
			else{ i = 30; }
		}
		//reset sensor values after send values
		for (int i = 0; i < 30; i++){
			for (int j = 0; j < 30; j++){
				values[i][j] = -1;
			}
		}	

		//show message
		if (!isSerial0){
			Serial.print("actualCycle: "); Serial.print(actualCycle); Serial.print(" - hour: ");
			Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.println(second); Serial.println(" ");
		}

	}
}
//Send values to Node
void FirebaseSensorRtDbSend::sendValue(String name, float value){
	//float to 4 bytes (send)
	union u_tag {
		byte b[4];
		float fval;
	} u;
	u.fval = value;
	//locate the sensor that the string refer to
	int indexSensor=0;
	int codeValue=0;
	for (int i = 0; i < 30; i++){
		if (names[i] == name){
			indexSensor = i;
			codeValue = i+1;
			i = 30;
		}
	}
	//set the confirmation byte
	int confirmation = codeValue + u.b[0] + u.b[1] + u.b[2] + u.b[3];
	while (confirmation>255)confirmation -= 255;
	//send bytes
	SerialToSend.write(254);//initial byte
	SerialToSend.write(253);//initial byte
	SerialToSend.write(codeValue);//code
	SerialToSend.write(codeValue);//code
	SerialToSend.write(u.b[0]);
	SerialToSend.write(u.b[1]);
	SerialToSend.write(u.b[2]);
	SerialToSend.write(u.b[3]);
	SerialToSend.write(confirmation);

	//show message
	if (!isSerial0){
		Serial.print("Send("); Serial.print(name); Serial.print(") - bytes:");
		Serial.print(u.b[0]); Serial.print(" "); Serial.print(u.b[1]); Serial.print(" "); Serial.print(u.b[2]); Serial.print(" "); Serial.print(u.b[3]);
		Serial.print(" - value:");
		Serial.println(u.fval);
	}
}
//Send hour to Node
void FirebaseSensorRtDbSend::sendHour(int hour, int minute, int second, int dayOfWeek, int day, int month, int year){
	//set the confirmation byte
	int confirmation = hour + minute + second + dayOfWeek + day + month + (year - 2018);
	while (confirmation>255)confirmation -= 255;
	//send bytes
	SerialToSend.write(254);//initial byte
	SerialToSend.write(253);//codeValue to time
	SerialToSend.write(0);//codeValue to time
	SerialToSend.write(0);//codeValue to time
	SerialToSend.write(hour);//code
	SerialToSend.write(minute);//code
	SerialToSend.write(second);
	SerialToSend.write(dayOfWeek);
	SerialToSend.write(day);
	SerialToSend.write(month);
	SerialToSend.write(year - 2018);
	SerialToSend.write(confirmation);
	//show message
	if (!isSerial0){
	Serial.print("Send(time) - bytes:");
	Serial.print(hour); Serial.print(" "); Serial.print(minute); Serial.print(" "); Serial.print(second); Serial.print(" "); Serial.print(dayOfWeek); Serial.print(" ");
	Serial.print(day); Serial.print(" "); Serial.print(month); Serial.print(" "); Serial.print(year - 2018); Serial.print(" "); Serial.print(confirmation);
	Serial.print(" - dados: "); Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second); 
	Serial.print(" (DoW:"); Serial.print(dayOfWeek); Serial.print(") "); 
	Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.println(year);
	}
	
}


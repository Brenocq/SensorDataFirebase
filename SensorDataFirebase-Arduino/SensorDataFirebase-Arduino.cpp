///////////////////////////////////////////////////////////
//  SensorDataFirebase-Arduino.cpp
//  Implementation of the Class SensorDataFirebaseArduino
//  Created on:      17-jan-2019 13:10:18
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////

#include "SensorDataFirebase-Arduino.h"

//Setting Serial port to send data
#define SerialToSend Serial1 //To use (TX0 define as Serial) (TX1 define as Serial1) (TX2 define as Serial2) (TX3 define as Serial3)
#define isSerial0    false	//if TX0/Serial set as true. Otherwise set as false

SensorDataFirebaseArduino::SensorDataFirebaseArduino()
{
	//Initiate variables with default values
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

void SensorDataFirebaseArduino::begin(){
	Serial.begin(9600);

	if (!isSerial0)//If is not using Serial0(TX0) to send bytes
		SerialToSend.begin(9600);//Set communication to baudrate 9600

}

void SensorDataFirebaseArduino::addSensor(String name){
	//---------- Add a new sensor to send data to the other device ----------//

	for (int i = 1; i < 30; i++){//seach for empty index to add the new sensor name (the name index is used to send bytes)
		if (names[i] == " "){
			names[i] = name;
			i = 30;
		}
	}
}

void SensorDataFirebaseArduino::updateValue(String name, float value){
	//---------- Add the sensor value to the vector (the mean at the end of the cycle will be sent to the other device) ----------//
	int indexSensor = 0;
	for (int i = 0; i < 30; i++){//Find the sensor number (name index) - this value is sent to the other device in the beginning
		if (names[i] == name){
			indexSensor = i;
			i = 30;
		}
	}

	for (int i = 0; i < 30; i++){//Add the value to the end of the array (The mean of the values will be sent to the firebase each 30 minutes)(Up to 30 values)
		if (values[indexSensor][i] == -1){
			values[indexSensor][i] = value;
			i = 30;
		}
	}

	if (!isSerial0){//Print only when not using Serial0/T0 to send bytes to the other device
		Serial.print("updating("); Serial.print(name); Serial.print("): "); Serial.println(value);
	}

}

void SensorDataFirebaseArduino::run(int hour, int minute, int second, int day, int month, int year){
	//---------- Send bytes when necessary to the other decive ----------//
	//Bytes about the time are sent to the other device

	int actualCycle = (minute + hour * 60) / 30;//Calculate the actual cycle

	//Send Hour
	sendHour(hour, minute, second, dayOfWeek, day, month, year);


	//Check cycle and executes if is a new cycle (each cycle has 30 minutes)(each cycle run just once)
	if (checkCycles[actualCycle]==false){
		checkCycles[actualCycle] = true;

		//Clean Previous Cycle
		int lastCycle = actualCycle - 1;
		lastCycle < 0 ? lastCycle += 48 : lastCycle;
		checkCycles[lastCycle] = false;
		//Send sensor values
		for (int i = 1; i < 30; i++){//for each sensor
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
				Serial.println(mean);
				sendValue(names[i], mean);//send value to the other device
			}
			else{ i = 30; }
		}

		//reset sensor values after send values
		for (int i = 0; i < 30; i++){
			for (int j = 0; j < 30; j++){
				values[i][j] = -1;
			}
		}

		if (!isSerial0){//Print only when not using Serial0/T0 to send bytes to the other device
			Serial.print("actualCycle: "); Serial.print(actualCycle); Serial.print(" - hour: ");
			Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.println(second); Serial.println(" ");
		}

	}
}

void SensorDataFirebaseArduino::sendValue(String name, float value){
	//---------- Send sensor values to the other device ----------//

	//Convert from float to 4 bytes
	union u_tag {
		byte b[4];
		float fval;
	} u;
	u.fval = value;

	//Locate the sensor that the String name refer to
	int indexSensor=0;
	int codeValue=0;
	for (int i = 0; i < 30; i++){
		if (names[i] == name){
			indexSensor = i;
			codeValue = i;
			i = 30;
		}
	}

	//Set the confirmation byte
	int confirmation = codeValue + u.b[0] + u.b[1] + u.b[2] + u.b[3];
	while (confirmation>255)confirmation -= 255;

	//Send bytes
	SerialToSend.write(254);//initial byte
	SerialToSend.write(253);//initial byte
	SerialToSend.write(codeValue);//code
	SerialToSend.write(codeValue);//code
	SerialToSend.write(u.b[0]);
	SerialToSend.write(u.b[1]);
	SerialToSend.write(u.b[2]);
	SerialToSend.write(u.b[3]);
	SerialToSend.write(confirmation);

	if (!isSerial0){//Print only when not using Serial0/T0 to send bytes to the other device
		Serial.print("Sending("); Serial.print(name); Serial.print(") - bytes: ");
		Serial.print(u.b[0]); Serial.print(" "); Serial.print(u.b[1]); Serial.print(" "); Serial.print(u.b[2]); Serial.print(" "); Serial.print(u.b[3]);
		Serial.print(" - value: ");
		Serial.println(u.fval);
	}
}


void SensorDataFirebaseArduino::sendHour(int hour, int minute, int second, int dayOfWeek, int day, int month, int year){
		if(hour<24 && minute<60 && second<60){
		//---------- Send time to the other device ----------//

		//Set the confirmation byte
		int confirmation = hour + minute + second + dayOfWeek + day + month + (year - 2018);
		while (confirmation>255)confirmation -= 255;

		//Send bytes
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

		if (!isSerial0){//Print only when not using Serial0/T0 to send bytes to the other device
		Serial.print("Sending(time) - bytes: ");
		Serial.print(hour); Serial.print(" "); Serial.print(minute); Serial.print(" "); Serial.print(second); Serial.print(" "); Serial.print(dayOfWeek); Serial.print(" ");
		Serial.print(day); Serial.print(" "); Serial.print(month); Serial.print(" "); Serial.print(year - 2018); Serial.print(" "); Serial.print(confirmation);
		Serial.print(" - data: "); Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
		Serial.print(" (DoW:"); Serial.print(dayOfWeek); Serial.print(") ");
		Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.println(year);
		}
	}
}

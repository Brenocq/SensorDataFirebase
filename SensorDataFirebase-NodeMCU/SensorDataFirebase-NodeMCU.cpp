///////////////////////////////////////////////////////////
//  SensorDataFirebase-NodeMCU.cpp
//  Implementation of the Class SensorDataFirebaseNodeMCU
//  Created on:      17-jan-2019 18:24:56
//  Original author: Breno Queiroz
///////////////////////////////////////////////////////////

#include "SensorDataFirebaseNodeMCU.h"
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <time.h>

//Setting Serial port to send data
#define SerialToSend Serial1 //To use (TX0 define as Serial) (TX1 define as Serial1) (TX2 define as Serial2) (TX3 define as Serial3)
#define isSerial0    false	//if TX0/Serial set as true. Otherwise set as false

//edit info below
#define FIREBASE_HOST "firebase link"
#define FIREBASE_AUTH "authCode"
#define WIFI_SSID "Name"
#define WIFI_PASSWORD "WifiPassword"

#define PRINTALL false

SensorDataFirebaseNodeMCU::SensorDataFirebaseNodeMCU()
{
	for (int i = 0; i < 30; i++){
		names[i] = " ";
	}

	for (int i = 0; i < 30; i++){
		addresses[i] = " ";
	}
	for (int i = 0; i < 7; i++){
		valuesWeek[i] = -1;
	}
	for (int i = 0; i < 30; i++){
		for (int j = 0; j < 48; j++){
			valuesToday[i][j] = -1;
		}
	}
}

void SensorDataFirebaseNodeMCU::begin(){
	// connect to wifi.
	Serial.begin(57600);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("connecting");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println();
	Serial.print("connected: ");
	Serial.println(WiFi.localIP());

	Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void SensorDataFirebaseNodeMCU::addSensor(String name, String address){
	for (int i = 0; i < 30; i++){//seach for empty index
		if (names[i] == " "){
			names[i] = name;
			addresses[i] = address;
			i = 30;
		}
	}
}

void SensorDataFirebaseNodeMCU::run(){
	if (PRINTALL)
		Serial.println("//--------------------------- RUN RECEIVE ---------------------------//");
	if (Serial.available() >= 1) {//Is there data on the serial port?
		int byte1 = Serial.read();
		if (byte1==254){//first byte equal 254
			int byte2 = Serial.read();
			if (byte2 == 253){//second byte equal 253
				//read bytes
				int	code1 = Serial.read();
				int	code2 = Serial.read();

				if (code1 == code2){//check codes
					//---------- TIME ----------//
					if (code1 == 0){
						int _hour, _minute, _second, _dayOfWeek, _day, _month, _year;
						//receive bytes
						_hour = Serial.read();
						_minute = Serial.read();
						_second = Serial.read();
						_dayOfWeek = Serial.read();
						_day = Serial.read();
						_month = Serial.read();
						_year = Serial.read() + 2018;

						int	confirmation = Serial.read();//sum of all numbers (I will assume that its OK now)(need to change later)(talk with Breno Queiroz)

						//!!!!NO PROTECTION!!!!
						hour = _hour;
						minute = _minute;
						second = _second;
						dayOfWeek = _dayOfWeek;
						day = _day;
						month = _month;
						year = _year;

						//----- Zeller's rule to calculate dayOfWeek -----// info: https://www.dreamincode.net/forums/topic/401335-program-to-determine-day-of-week-given-any-date/
						//f = k + [(13*m-1)/5] + D + [D/4] + [C/4] - 2*C
						float k = day;
						float m = month + 10; //"March is 1, April is 2, and so on to February, which is 12"
						m > 12 ? m -=12 : m;
						float D = year - 2000;//vai dar bug em 2100
						float C = int(year / 100);

						dayOfWeek = k + ((13 * m - 1) / 5) + D + (D / 4) + (C / 4) - 2 * C;
						dayOfWeek >= 7 ? dayOfWeek = dayOfWeek % 7 : dayOfWeek;

						//----- Print -----//
						if (minute % 30 == 0){
							Serial.print("//-------------------------CYCLE "); Serial.print(hour); Serial.print("h"); Serial.print(minute); Serial.println("-------------------------//");
							Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second); Serial.print(" ("); Serial.print(dayOfWeek); Serial.print(") ");
							Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.println(year);
						}

						//----- Update time -----//
						String dataBaseAddress = "Time/CurrentTime";

						String hourString = hour < 10 ? "0" + String(hour) : String(hour);
						String minuteString = minute < 10 ? "0" + String(minute) : String(minute);
						String secondString = second < 10 ? "0" + String(second) : String(second);

						String currentTime = hourString + ":" + minuteString + ":" + secondString;
						if (PRINTALL){
							Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(currentTime);
						}
						//set value firebase
						Firebase.setString(dataBaseAddress, currentTime);

						//----- Update date -----//
						dataBaseAddress = "Time/CurrentDate";

						String dayString = day < 10 ? "0" + String(day) : String(day);
						String monthString = month < 10 ? "0" + String(month) : String(month);

						String currentDate = dayString + "/" + monthString + "/" + String(year);
						if (PRINTALL){
							Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(currentDate);
						}
						//set value firebase
						Firebase.setString(dataBaseAddress, currentDate);


					}
					//---------- VALUES ----------//
					else{

						//4 bytes to float(send)
						union u_tag {
							byte b[4];
							float fval;
						} u;

						//receive bytes
						u.b[0] = Serial.read();
						u.b[1] = Serial.read();
						u.b[2] = Serial.read();
						u.b[3] = Serial.read();
						int	confirmation = Serial.read();//sum of all numbers

						//set the sumReceived byte
						int sumReceived = code1 + u.b[0] + u.b[1] + u.b[2] + u.b[3];
						while (sumReceived>255)sumReceived -= 255;

						//------ Values OK? ------//
						if (sumReceived == confirmation && code1 == code2){//only continues if there is no corrupted byte
							Serial.print("//----------SENSOR "); Serial.print(names[code1 - 1]); Serial.println("----------//");
							Serial.print("Value received:"); Serial.println(u.fval);

							int index = code1 - 1;//set the sensor index (sensor 1 -> index 0)
							float value = u.fval;

							if (hour == 0 && minute < 10){
								updateFirebaseYesterday(index);//				UPDATE YESTERDAY
							}

							updateFirebaseToday(index, value);//				UPDATE DAY

							if (hour == 23 && minute >= 30 && minute < 40){
								updateFirebaseMonth(index);//					UPDATE MONTH
								if (dayOfWeek == 0){//Sunday
									updateFirebaseYear(index);//				UPDATE YEAR
								}
							}

						}
					}
				}
			}
		}

	}
}

//**********************************************************************************************************************TODAY
//**********************************************************************************************************************TODAY
void SensorDataFirebaseNodeMCU::updateFirebaseToday(int index, float value ){
	Serial.println("//-----TODAY-----//");
	int indexHour = (hour * 2) + (minute / 30);

	//each index refer to one hour
	String hours[48] = { "00h00", "00h30", "01h00", "01h30", "02h00", "02h30", "03h00", "03h30", "04h00", "04h30",
						"05h00", "05h30", "06h00", "06h30", "07h00", "07h30", "08h00", "08h30", "09h00", "09h30",
						"10h00", "10h30", "11h00", "11h30", "12h00", "12h30", "13h00", "13h30", "14h00", "14h30",
						"15h00", "15h30", "16h00", "16h30", "17h00", "17h30", "18h00", "18h30", "19h00", "19h30",
						"20h00", "20h30", "21h00", "21h30", "22h00", "22h30", "23h00", "23h30"};
	//create complete address to firebase
	String dataBaseAddress = addresses[index] + "Today/" + hours[indexHour];
	//print address screen
	Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(value);
	//set value firebase
	Firebase.setFloat(dataBaseAddress, value);

	//check possible errors and try
	if (Firebase.failed()) {
		Serial.print("		Failed to set value");
		delay(100);
		Firebase.setFloat(dataBaseAddress, value);

		Serial.println("");
	}
}
//**********************************************************************************************************************YESTERDAY
//**********************************************************************************************************************YESTERDAY
void SensorDataFirebaseNodeMCU::updateFirebaseYesterday(int index){
Serial.println("//-----YESTERDAY-----//");
	//----- get values Today -----//
	//each index refer to one hour
	String hours[48] = { "00h00", "00h30", "01h00", "01h30", "02h00", "02h30", "03h00", "03h30", "04h00", "04h30",
		"05h00", "05h30", "06h00", "06h30", "07h00", "07h30", "08h00", "08h30", "09h00", "09h30",
		"10h00", "10h30", "11h00", "11h30", "12h00", "12h30", "13h00", "13h30", "14h00", "14h30",
		"15h00", "15h30", "16h00", "16h30", "17h00", "17h30", "18h00", "18h30", "19h00", "19h30",
		"20h00", "20h30", "21h00", "21h30", "22h00", "22h30", "23h00", "23h30" };

	for (int i = 0; i < 48; i++){
		//get all values from Today (files hours[i] inside folder)
		valuesToday[index][i] = -1;//@TODO delete this line
		valuesToday[index][i] = Firebase.getFloat(addresses[index] + "Today/" + hours[i]);
		//print address screen
		Serial.print("GET:"); Serial.print(addresses[index] + "Today/" + hours[i]); Serial.print(" : "); Serial.println(valuesToday[index][i]);
	}
	//----- print values valuesToday -----//
	/*Serial.println("Print (valuesToday[index][i]):	");
	for (int i = 0; i < 30; i++){
		Serial.print("["); Serial.print(i); Serial.print("]");
		for (int j = 0; j < 48; j++){
			Serial.print("("); Serial.print(valuesToday[i][j]); Serial.print(")");
			if (j % 10 == 0){//to organize the printing
				Serial.print("|");
			}
		}
		Serial.println("");
	}*/

	//----- update values Yesterday -----//
	Serial.print("DELETE:"); Serial.println(addresses[index] + "Yesterday");
	//remove value firebase
	Firebase.remove(addresses[index] + "Yesterday");

	for (int i = 0; i < 48; i++){
		//create complete address to firebase
		String dataBaseAddress = addresses[index] + "Yesterday/" + hours[i];
		//
		if (valuesToday[index][i] != -1 && valuesToday[index][i] != 0){//(ignore all -1 and 0 values)
			//Month has already updated valuesToday[index][i], so it's ok to use it
			//print address screen
			Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(valuesToday[index][i]);
			//set value firebase
			Firebase.setFloat(dataBaseAddress, valuesToday[index][i]);
			//check possible errors and try
			if (Firebase.failed()) {
				Serial.print("		Failed to set value");
				delay(100);
				Firebase.setFloat(dataBaseAddress, valuesToday[index][i]);

				Serial.println("");
			}
		}
	}
	for (int i = 0; i < 48; i++){//set valuesToday[index][i] as default
		valuesToday[index][i] = -1;
	}
	//----- delete values Today -----//
	//create complete address to firebase
	String dataBaseAddress = addresses[index] + "Today";
	//print address screen
	Serial.print("DELETE:"); Serial.println(dataBaseAddress);
	//remove value firebase
	Firebase.remove(dataBaseAddress);
}
//**********************************************************************************************************************MONTH
//**********************************************************************************************************************MONTH
void SensorDataFirebaseNodeMCU::updateFirebaseMonth(int index){
	Serial.println("//-----MONTH-----//");
	//----- delete month if day 1 -----//
	if (day == 1)
	{
		//create complete address to firebase
		String dataBaseAddress = addresses[index] + "Month";
		//print address screen
		Serial.print("DELETE:"); Serial.println(dataBaseAddress);
		//remove value firebase
		Firebase.remove(dataBaseAddress);
	}
	//----- get values Today -----//
	//each index refer to one hour
	String hours[48] = { "00h00", "00h30", "01h00", "01h30", "02h00", "02h30", "03h00", "03h30", "04h00", "04h30",
		"05h00", "05h30", "06h00", "06h30", "07h00", "07h30", "08h00", "08h30", "09h00", "09h30",
		"10h00", "10h30", "11h00", "11h30", "12h00", "12h30", "13h00", "13h30", "14h00", "14h30",
		"15h00", "15h30", "16h00", "16h30", "17h00", "17h30", "18h00", "18h30", "19h00", "19h30",
		"20h00", "20h30", "21h00", "21h30", "22h00", "22h30", "23h00", "23h30" };

	for (int i = 0; i < 48; i++){
		//get all values from Today (files hours[i] inside folder)
		valuesToday[index][i] = Firebase.getFloat(addresses[index] + "Today/" + hours[i]);
		//print address screen
		Serial.print("GET:"); Serial.print(addresses[index] + "Today/" + hours[i]); Serial.print(" : "); Serial.println(valuesToday[index][i]);
		//check possible errors and try
		if (Firebase.failed()) {
			Serial.print("		Failed to get value");
			delay(100);
			Firebase.getFloat(addresses[index] + "Today/" + hours[i]);

			Serial.println("");
		}
	}

	//----- print values valuesToday -----//
	/*Serial.println("Print (valuesToday[index][i]):	");
	for (int i = 0; i < 30; i++){
		Serial.print("["); Serial.print(i); Serial.print("]");
		for (int j = 0; j < 48; j++){
			Serial.print("("); Serial.print(valuesToday[i][j]); Serial.print(")");
			if (j % 10 == 0){//to organize the printing
				Serial.print("|");
			}
		}
		Serial.println("");
	}*/

	//----- calculate mean Today -----// (ignore all -1 values)
	Serial.print("Calculating mean... [");
	float mean = 0;
	float totalSum = 0;
	for (int i = 0; i < 48; i++){
		if (valuesToday[index][i] != -1 && valuesToday[index][i] != 0){
			mean += valuesToday[index][i];
			Serial.print(valuesToday[index][i]); Serial.print("("); Serial.print(hours[i]); Serial.print(")"); Serial.print("+");
			totalSum++;
		}
	}
	for (int i = 0; i < 48; i++){
		valuesToday[index][i] = -1;
	}
	Serial.print("] --> ("); Serial.print(mean); Serial.print("/"); Serial.print(totalSum);
	mean /= totalSum;
	Serial.print(")="); Serial.println(mean);
	//----- update mean Month -----//
	//create complete address to firebase
	String dataBaseAddress = addresses[index] + "Month/" + day;
	//print address screen
	Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(mean);
	//set value firebase
	Firebase.setFloat(dataBaseAddress, mean);

	//check possible errors and try
	if (Firebase.failed()) {
		Serial.print("		Failed to set value");
		delay(100);
		Firebase.setFloat(dataBaseAddress, mean);

		Serial.println("");
	}
	//----- add value vector year -----//
	Serial.println("//-----MONTH (WEEK)-----//");
	dataBaseAddress = addresses[index] + "Week/" + dayOfWeek;
	Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(mean);
	//set value firebase
	Firebase.setFloat(dataBaseAddress, mean);
}
//**********************************************************************************************************************YEAR
//**********************************************************************************************************************YEAR
void SensorDataFirebaseNodeMCU::updateFirebaseYear(int index){
	Serial.println("//-----YEAR-----//");
	//----- calculate week -----//
	String weekNumberS;

	// Reference: https://en.wikipedia.org/wiki/ISO_8601
	int WW;
	int YYYY = year;

	int Monday = day - (day + 6) % 7;                          // Monday this week: may be negative down to 1-6 = -5;
	int MondayYear = 1 + (Monday + 6) % 7;                            // First Monday of the year
	int Monday01 = (MondayYear > 4) ? MondayYear - 7 : MondayYear;    // Monday of week 1: should lie between -2 and 4 inclusive
	WW = 1 + (Monday - Monday01) / 7;                                 // Nominal week ... but see below

	// In ISO-8601 there is no week 0 ... it will be week 52 or 53 of the previous year
	if (WW == 0)
	{
		YYYY--;
		WW = 52;

		bool isLeap = false;

		if (year % 4 == 0)
		{
			if (year % 100 == 0 && year % 400 != 0) isLeap = false;
			else                                      isLeap = true;
		}

		if (MondayYear == 3 || MondayYear == 4 || (isLeap && MondayYear == 2)) WW = 53;
	}

	bool isLeap=false;

	if (year % 4 == 0)
	{
		if (year % 100 == 0 && year % 400 != 0) isLeap= false;
		else                                      isLeap= true;
	}

	// Similar issues at the end of the calendar year
	if (WW == 53)
	{
		int daysInYear = isLeap ? 366 : 365;
		if (daysInYear - Monday < 3)
		{
			YYYY++;
			WW = 1;
		}
	}
	//week number OK
	weekNumberS = WW;

	//----- get week values -----//
	for (int i = 0; i < 7; i++){
		//get all values from Today (files hours[i] inside folder)
		valuesWeek[index] = Firebase.getFloat(addresses[index] + "Week/" + i);
		//print address screen
		Serial.print("GET:"); Serial.print(addresses[index] + "Week/" + i); Serial.print(" : "); Serial.println(valuesWeek[index]);
		//check possible errors and try
		if (Firebase.failed()) {
			Serial.print("		Failed to get value -> ");
			delay(100);
			valuesWeek[index] = Firebase.getFloat(addresses[index] + "Week/" + i);
			Serial.print("GETagain:"); Firebase.getFloat(addresses[index] + "Week/" + i); Serial.print(" : "); Serial.println(valuesWeek[index]);

			Serial.println("");
		}
	}
	//----- calculate mean Week -----// ignore all -1 (default)
	Serial.print("Calculating mean... [");
	float mean = 0;
	float totalSum=0;
	for (int i = 0; i < 7; i++){
		if (valuesWeek[i] != -1 && valuesWeek[i] != 0){
			mean += valuesWeek[i];
			//print [value(dayOfWeek)+...]			0=sunday
			Serial.print(valuesWeek[i]); Serial.print("("); Serial.print(i); Serial.print(")"); Serial.print("+");
			totalSum++;
		}
	}
	Serial.print("] --> ("); Serial.print(mean); Serial.print("/"); Serial.print(totalSum);
	mean /= totalSum;
	Serial.print(")="); Serial.println(mean);
	//reset valuesMonth[index][i]
	for (int i = 0; i < 7; i++){
		valuesWeek[i] = -1;
	}
	//----- update mean Year -----//
	//EX: 2019-month1-week2
	String yearCode = String(year) + "-month" + String(month) + "-week" + String(weekNumberS);
	//create complete address to firebase
	String dataBaseAddress = addresses[index] + "Years/" + yearCode;
	//print address screen
	Serial.print("SET:"); Serial.print(dataBaseAddress); Serial.print(" = "); Serial.println(mean);
	//set value firebase
	Firebase.setFloat(dataBaseAddress, mean);

	//check possible errors and try
	if (Firebase.failed()) {
		Serial.print("		Failed to set value");
		delay(100);
		Firebase.setFloat(dataBaseAddress, mean);

		Serial.println("");
	}

	//----- delete values Week -----//
	//create complete address to firebase
	dataBaseAddress = addresses[index] + "Week";
	//print address screen
	Serial.print("DELETE:"); Serial.println(dataBaseAddress);
	//remove value firebase
	Firebase.remove(dataBaseAddress);
}

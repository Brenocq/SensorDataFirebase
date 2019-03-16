# Introduction
SensorDataFirebase is a group of libraries that were created to help to send data from sensors to the Firebase RealTimeDatabase using some Arduino and one NodeMCU or Raspberry PI. The data sent to the Firebase is formated to help users to plot graph using the data.

Because the Raspberry and the NodeMCU work with 3.3V, sometimes can be difficult to use analog sensors (Raspberry problem) or sensors that send data with 5V (Raspberry and NodeMCU problem). The use of communication with some 5V Arduino solves this problem (the library do not work yet using only the Raspberry or NodeMCU).

The libraries allow the user to add as much sensor as (s)he wants. All the data colected about these sensors will be sent to the Firebase.
Be warned: because sometimes it is slow to read data from the firebase using NodeMCU and Raspberry, the use of a lot of sensors can result in bugs.

# How the data is stored in Firebase?
First, is necessary to have created the "Real TimeDatabase" in your Firebase project. The data is stored only in this type of database.
In the database is created two folders: *"Sensors"* and *"Time"*. In the first folder is created a subfolder for each sensor. In the second folder is stored data about the current time (*"CurrentTime"*) and current date (*"CurrentDate"*).

The data from sensors are organized in 5 ways in each subfolder: Today, Yesterday, Week, Month, and Year.

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Sensors.PNG">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Time.PNG">
</p>

### Today
In this folder is stored all the data collected today by that sensor. The data in this the folder "Today" is stored every 30 minutes. For each cycle a new variable will be created on the folder "Today" to store the average of all readings performed by that sensor (only data sent to the Arduino libray).

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Today.PNG">
</p>

### Yesterday
Every 00h00 all data stored in Today is copied to the subfolder *"Yesterday"*. (The data in "Today" is deleted after copied)

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Yesterday.PNG">
</p>

### Week
Every 00h00 a variable is created in the subfolder *"Week"*. This variable stores the mean of all data in *"Today"*.
The variable name can be: <br/>
*"1"* to Monday | *"2"* to Tuesday | *"3"* to Wednesday | *"4"* to Thursday | *"5"* to Friday | *"6"* to Saturday | *"7"* to Sunday

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Week.PNG">
</p>

### Month
Every 00h00 a variable is created in the subfolder *"Month"*. This variable stores the mean of all data in *"Today"*.
The variable name is the number of the day.

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Month.PNG">
</p>

### Year
Every Monday 00h01 a variable is created in the subfolder *"Year"*. This variable stores the mean of all data in *"Week"*. The folder *"Week"* is deleted after this.

<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Firebase-Year.PNG">
</p>

# Hardware to use the libraries
These libraries can work in two ways: the first one consists of using some **Arduino** and one **NodeMCU**; the second one consists of using some **Arduino** and one **Raspberry**.<br/>
In both systems the Arduino is connected to sensors and send the data to the other device. When the other device receive the bytes, it add the data to the Firebase Database and organize all the data.

### Arduino
To the library works the Arduino must be connected to some module that returns the time (I am using the **DS1307 I2C RTC Module**).<br/>
This information about the time is used to send all data in the right time to the other device. Information about the time is sent every minute and data about the sensors are sent every 30 minutes.

### Arduino <-> NodeMCU
The communication between the Arduino and the NodeMCU is made through Serial. This communication occurs using 9600 or baudrate and Serial1 in both by default. (these values can be changed in the *.cpp* file).<br/>
The Node TX can be connected directily on the Arduino RX, but the voltage from the Arduino TX to the Node RX **must** be reduced. I am using one 1k resistor and one 2k2 resistor to lower the signal. 
 
<p align="center">
  <img src="https://github.com/Brenocq/SensorDataFirebase/blob/master/Images/Arduino-Node-connection-hq.png" width="700" height="360">
</p>

Do not forget to use some clock device.

### Arduino <-> Raspberry
The communication between the Arduino and the Raspberry is made through Serial too. This communication occurs by default using 9600 of baudrate, Serial1 for the Arduino and ttyAMA0 for the Raspberry. These values can be changed in the *.cpp* file for the Arduino and in the *SensorFirebasePi.py* for the Raspberry.

# Using the Arduino library
Copy the _SensorDataFirebase-Arduino_ folder to your _Documents/Arduino/Library_ folder.<br/>
After creating the object from the class SensorDataFirebaseArduino there are 3 main commands.<br/>
`addSensor(String name)` -  This command is used to each sensors, the exactly same name must be used on the other commands.<br/>
`updateValue(String name, float value)` - This command is used to update each sensor data. A maximum of 30 numbers can be sent for the library on a period of 30 minutes.<br/>
`run(int hour, int minute, int second, int day, int month, int year)` - This command should be runned each minute. Data about the time will be sent to the other device each time and sensors data will be sent every 30 minutes.


```c++
/*ARDUINO CODE*/
#include "SensorDataFirebase-Arduino.h"
#include "Clock.h" // Use some library for your RTC clock device

SensorDataFirebaseArduino sensorData;

void setup() {
  sensorData.begin();
  sensorData.addSensor("temperature");//(String name, int codeCommunication)
}

int lastMinute;

void loop() {
    int hour,minute,second;

   rtclock.updateTime();
   minute = rtclock.getMinute();
   
  if (minute != lastMinute) {
    lastminute      = minute;
    int sensorValue = analogRead(A0);
    
    sensorData.updateValue("temperatura", sensorValue);//add new data to the temperature sensor
    
    sensorData.run( rtclock.getHour(), minute, rtclock.getSecond(), rtclock.getDayOfWeek(),
                    rtclock.getDay(), rtclock.getMonth(), rtclock.getYear());
  }
  delay(5);
}
```

To change the Serial port that will communicate with the other device go to _SensorDataFirebase-Arduino.cpp_ and change the lines 11 and 12.

```c++
#define SerialToSend Serial1
#define isSerial0    false
```

# Using the NodeMCU library
Copy the _SensorDataFirebase-NodeMCU folder to your _Documents/Arduino/Library_ folder.
After creating the object from the class SensorDataFirebaseNodeMCU there are 2 main commands.<br/>
`addSensor(String name, String address)` -  This command is used to each sensors. The sequence you add the sensors must be the same on both files. Set the address as the address file in your Firebase where you want the data about this sensor to be stored.<br/>
`run()` - This command can be runned as much as you can. Every time this runs the data on the Serial port is checked and sent to the Firebase RealTimeDatabase.<br/>

To change the Serial port that will communicate with the other device go to _SensorDataFirebase-NodeMCU.cpp_ and change the lines 14 and 15.
```c++
#define SerialToSend Serial1
#define isSerial0    false
```
You must change the lines 18-21 to connect the NodeMCU to your wifi and set your Firebase project.
```c++
#define FIREBASE_HOST "firebase link"
#define FIREBASE_AUTH "authCode"
#define WIFI_SSID "Name"
#define WIFI_PASSWORD "WifiPassword"
```
Also, you should to open your Firebase database to allow the NodeMCU to write and read data. Go to your Firebase project on the Firebase website and access your RealtimeDatabase. Go to the "rules" window and change read and write to true. Will look like this:
```c++
{
  /* Visit https://firebase.google.com/docs/database/security to learn more about security rules. */
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

Nice, now you should be able to use the library properly. The example code is:

```c++
/*NodeMCU CODE*/
#include "SensorDataFirebase-NodeMCU.h"

SensorDataFirebaseNodeMCU sensorData;

void setup() {
  sensorData.begin();
  sensorData.addSensor("temperatura","Sensors/Temperature/");
}

void loop() {
  sensorData.run();
  delay(5);
}
```

# Using the Raspberry library

Any doubt, error, or sugestion, please let me know.

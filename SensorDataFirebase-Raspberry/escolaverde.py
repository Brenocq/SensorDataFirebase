from firebase import firebase
from Libraries.SensorFirebasePi import SensorFirebasePi
import time

sensorFirebase = SensorFirebasePi('https://escolaverde-febrace.firebaseio.com/')

sensorFirebase.addSensor('temperature','Sensors/Temperature')
sensorFirebase.addSensor('humidity','Sensors/Humidity')
sensorFirebase.sensorInfo('temperature')
sensorFirebase.sensorInfo('humidity')

while True:
	sensorFirebase.run()
	time.sleep(1) 

'''
sendFirebase.addSensor('temperature','Sensors/Temperature')

sendFirebase.printSensor('temperature')

firebase = firebase.FirebaseApplication('https://escolaverde-febrace.firebaseio.com/')
result = firebase.put('Sensors/Humidity','00h30',22)
getData = firebase.get('/Sensors/Humidity/00h00',None);
print(result)
print(getData)'''

from firebase import firebase
from Libraries.SensorFirebasePi import SensorFirebasePi
import time

sensorFirebase = SensorFirebasePi('https://escolaverde-portinari.firebaseio.com/')

sensorFirebase.addSensor('temperature','Sensors/Temperature/')
sensorFirebase.addSensor('humidity','Sensors/Humidity/')
sensorFirebase.sensorInfo('temperature')
sensorFirebase.sensorInfo('humidity')

while True:
	sensorFirebase.run()
	time.sleep(1)

import RPi.GPIO as GPIO
import serial
import struct

ser = serial.Serial("/dev/ttyAMA0",115200,timeout=1)

class SensorFirebasePi:
    
    sensors    = {}
    sensorsName = ["None"]
    sensorsNum = 0
    hour = 0
    minute = 0
    second = 0
    dayOfWeek = 0
    day = 0
    month = 0
    year = 0

    def __init__(self,linkFirebase):
        self.link = linkFirebase
 
    def addSensor(self,name, address):
        self.sensorsNum += 1;
        self.sensors[name]= {'address':address, 'number': self.sensorsNum}
        self.sensorsName.append(name)

    def sensorInfo(self,name):
        print(name + ' info: ')
        if(self.sensors.get(name) != None):
            print('\tAddress: %s ' %(self.sensors[name]['address']))
            print('\tNumber: %d ' %(self.sensors[name]['number']))
            print("\n")
        else:
            print("\t" + name + " sensor do not exist")
            print("\n")

    def run(self):
        try:    
            init1 = ord(ser.read())
            init2 = ord(ser.read()) 
            code1 = ord(ser.read())
            code2 = ord(ser.read())
            if(code1==0 and code2==0):
                #Receiving data about the time
                self.hour = ord(ser.read())
                self.minute = ord(ser.read())
                self.second = ord(ser.read())
                self.dayOfWeek = ord(ser.read())
                self.day = ord(ser.read())
                self.month = ord(ser.read())
                self.year = ord(ser.read())+2018
                sumBytes = ord(ser.read())
                print("Receiving time: %d:%d:%d (DoW:%d) %d/%d/%d c:%d" % \
                    (self.hour,self.minute,self.second,self.dayOfWeek,self.day,self.month,self.year,sumBytes))
            else:
                #Receiving some sensor data
                b1 = ord(ser.read())
                b2 = ord(ser.read())
                b3 = ord(ser.read())
                b4 = ord(ser.read())
                sumBytes = ord(ser.read())
                
                x = [hex(b1),hex(b2),hex(b3),hex(b4)]
                y = int.from_bytes(x, byteorder='little', signed=False) #interpret bytes as an unsigned little-endian integer (so far so good)
                z = float(y) #attempt to cast as float reinterprets integer value rather than its byte values
                print z
                '''
                #data  = [b1, b2, b3, b4]
                data  = [51,51,215,65]
                b = ''.join(chr(i) for i in data)
                print struct.unpack('>f', b)
                '''
                #value = struct.unpack('f', b"%s"%(byteString))
                print("Receiving %s: (%d %d %d %d) c:%d" % (self.sensorsName[code1],b1,b2,b3,b4,sumBytes))  
                #print(value)
        except:
            pass

    def updateFirebaseToday(self,name):
        print('update today')

    def updateFirebaseYesterday(self,name):
        print('update today')

    def updateFirebaseMonth(self,name):
        print('update today')

    def updateFirebaseYear(self,name):
        print('update today')

    def dayOfWeekZeller(self, day, month, year):
        k = day
        m = month + 10 #"March is 1, April is 2, and so on to February, which is 12"
        m > 12 ? m -=12 : m;
        D = year - 2000;#vai dar bug em 2100 
        C = int(year / 100)
        dayOfWeek = k + ((13 * m - 1) / 5) + D + (D / 4) + (C / 4) - 2 * C
        return dayOfWeek >= 7 ? dayOfWeek = dayOfWeek % 7 : dayOfWeek

'''
    _sensorNum = 0
    def __init__(self,linkFirebase):
        self.link = linkFirebase
    
#   def run(self):

    def printSensor(self,name):
        print("Name: ",name)
        print("Number: ",self.__dict__[name]._number)
        print("Address: ",self.__dict__[name].address)'''

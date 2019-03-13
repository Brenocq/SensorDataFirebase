import RPi.GPIO as GPIO
from firebase import firebase
import serial
import struct

ser = serial.Serial("/dev/ttyAMA0",9600,timeout=1)

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
    firebase = 0

    def __init__(self,linkFirebase):
        self.link = linkFirebase
        self.firebase = firebase.FirebaseApplication(self.link)

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

                self.dayOfWeekZeller()#Update the dayOfWeek value using info about day,month and year

                print("Receiving(time) - data: %d:%d:%d (DoW:%d) %d/%d/%d" % \
                    (self.hour,self.minute,self.second,self.dayOfWeek,self.day,self.month,self.year))

                #----- Print Cycle -----#
                if(self.minute % 30 == 0):
                    print("\n#--------------- CYCLE %dh%d ---------------#" % (self.hour,self.minute))
                #----- Update Time Firebase -----#
                hourStr      = ''
                minuteStr    = ''
                secondStr    = ''

                if self.hour < 10:
                    hourStr = '0' + str(self.hour)
                else:
                    hourStr = str(self.hour)

                if self.minute < 10:
                    minuteStr = '0' + str(self.minute)
                else:
                    minuteStr = str(self.minute)

                if self.second < 10:
                    secondStr = '0' + str(self.second)
                else:
                    secondStr = str(self.second)

                currentTimeStr = hourStr + ":" + minuteStr + ":" + secondStr

                print("SET firebase: Time/CurrentTime = %s" % (currentTimeStr))
                self.firebase.put('Time','CurrentTime',currentTimeStr)

                #----- Update Date Firebase -----#
                dayStr      = ''
                monthStr    = ''

                if self.day < 10:
                    dayStr = '0' + str(self.day)
                else:
                    dayStr = str(self.day)

                if self.month < 10:
                    monthStr = '0' + str(self.month)
                else:
                    monthStr = str(self.month)

                currentDateStr = dayStr + "/" + monthStr + "/" +str(self.year)

                print("SET firebase: Time/CurrentDate = %s" % (currentDateStr))
                self.firebase.put('Time','CurrentDate',currentDateStr)
            elif(code1 == code2):
                #Receiving some sensor data
                b1 = ord(ser.read())
                b2 = ord(ser.read())
                b3 = ord(ser.read())
                b4 = ord(ser.read())
                sumBytes = ord(ser.read())

                byte1=format(struct.unpack('!I', struct.pack('!i', b1))[0], '08b')
                byte2=format(struct.unpack('!I', struct.pack('!i', b2))[0], '08b')
                byte3=format(struct.unpack('!I', struct.pack('!i', b3))[0], '08b')
                byte4=format(struct.unpack('!I', struct.pack('!i', b4))[0], '08b')

                floatData=struct.unpack('!f',struct.pack('!I', int(byte4+byte3+byte2+byte1, 2)))[0]

                print("Receiving(%s) - bytes: %d %d %d %d - value: %f" % (self.sensorsName[code1],b1,b2,b3,b4,floatData))

                #Checking sum received (confirmation byte)
                sumReceived = code1 + b1 + b2 + b3 + b4
                while (sumReceived>255):
                    sumReceived -= 255

                if(sumReceived == sumBytes):
                    print("#----- Sensor %s -----#" % (self.sensorsName[code1]))
                    if (self.hour == 0 and self.minute < 10):
                        self.updateFirebaseYesterday(self.sensorsName[code1])#				UPDATE YESTERDAY

                    self.updateFirebaseToday(self.sensorsName[code1], floatData)#		UPDATE DAY

                    if (self.hour == 23 and self.minute >= 30 and self.minute < 40):
                        self.updateFirebaseMonth(self.sensorsName[code1])#					UPDATE MONTH
                        if (self.dayOfWeek == 0):#Sunday
                            self.updateFirebaseYear(self.sensorsName[code1])#				UPDATE YEAR
                else:
                    print("ERROR: wrong confirmation byte (%d != %d)" % (sumReceived,sumBytes))
            else:
                print("ERROR: code1 != code2 (%d != %d)" % (code1,code2))

        except:
            pass
#**********************************************************************************************************************TODAY
    def updateFirebaseToday(self,name,value):
        print('#TODAY#')
        indexHour = (self.hour * 2) + (self.minute / 30)
        #each index refer to one hour
        hoursStr= [  "00h00", "00h30", "01h00", "01h30", "02h00", "02h30", "03h00", "03h30", "04h00", "04h30",\
        			 "05h00", "05h30", "06h00", "06h30", "07h00", "07h30", "08h00", "08h30", "09h00", "09h30",\
        			 "10h00", "10h30", "11h00", "11h30", "12h00", "12h30", "13h00", "13h30", "14h00", "14h30",\
        			 "15h00", "15h30", "16h00", "16h30", "17h00", "17h30", "18h00", "18h30", "19h00", "19h30",\
        			 "20h00", "20h30", "21h00", "21h30", "22h00", "22h30", "23h00", "23h30"]
        #create complete address to firebase
        dataBaseAddress = self.sensors[name]['address'] + "Today/"
        #print address screen
        print("SET firebase: %s = %f" % (dataBaseAddress,value))
        #set value firebase
        self.firebase.put(self.sensors[name]['address'] + "Today/",hoursStr[indexHour], value)
        print('#END#')
#**********************************************************************************************************************YESTERDAY
    def updateFirebaseYesterday(self,name):
        print('#YESTERDAY#')
        #----- get values Today -----#
        #each index refer to one hour
        hoursStr = [ "00h00", "00h30", "01h00", "01h30", "02h00", "02h30", "03h00", "03h30", "04h00", "04h30",\
        		"05h00", "05h30", "06h00", "06h30", "07h00", "07h30", "08h00", "08h30", "09h00", "09h30",\
        		"10h00", "10h30", "11h00", "11h30", "12h00", "12h30", "13h00", "13h30", "14h00", "14h30",\
        		"15h00", "15h30", "16h00", "16h30", "17h00", "17h30", "18h00", "18h30", "19h00", "19h30",\
        		"20h00", "20h30", "21h00", "21h30", "22h00", "22h30", "23h00", "23h30" ]

        valuesToday = []

        for i in range(48):
        	#get all values from Today (files hours[i] inside folder)
            dataBaseAddress = self.sensors[name]['address'] + "Today/" + hoursStr[i]
            valuesToday.append(self.firebase.get(dataBaseAddress,None))
            #print valuesToday[i]
            #valuesToday.append(self.firebase.get(dataBaseAddress,None))
        	#print address screen
            if valuesToday[i] != None:
                print("GET firebase: %s : %f" % (dataBaseAddress,valuesToday[i]))
            else:
                print("GET firebase: %s : None" % (dataBaseAddress))

        #----- update values Yesterday -----#
        dataBaseAddressYest = self.sensors[name]['address'] + "Yesterday/"
        print("DELETE firebase: %s" % (dataBaseAddressYest))
        self.firebase.delete(dataBaseAddressYest, None)

        for i in range(48):
            if valuesToday[i] != None:
                print(i)
                print("SET firebase: %s = %f" % (dataBaseAddressYest+hoursStr[i],valuesToday[i]))
                self.firebase.put(dataBaseAddressYest,hoursStr[i], valuesToday[i])

        #----- delete values Today -----#
        #create complete address to firebase
        dataBaseAddressToday = self.sensors[name]['address'] + "Today"
        #print address screen
        print("DELETE firebase: %s" % (dataBaseAddressToday))
        #remove value firebase
        self.firebase.delete(dataBaseAddressToday, None)

        print('#END#')
#**********************************************************************************************************************MONTH
    def updateFirebaseMonth(self,name):
        #TODO finish updateFirebaseMonth
        print('update Month')

#**********************************************************************************************************************YEAR
    def updateFirebaseYear(self,name):
        #TODO finish updateFirebaseMonth
        print('update Year')

    def dayOfWeekZeller(self):
        k = self.day
        m = self.month + 10 #"March is 1, April is 2, and so on to February, which is 12"
        if m > 12:
            m -=12
        D = self.year - 2000;#vai dar bug em 2100
        C = int(self.year / 100)
        self.dayOfWeek = k + ((13 * m - 1) / 5) + D + (D / 4) + (C / 4) - 2 * C
        if self.dayOfWeek >= 7:
            self.dayOfWeek = self.dayOfWeek % 7

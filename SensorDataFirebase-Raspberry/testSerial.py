import RPi.GPIO as GPIO
import serial
ser = serial.Serial("/dev/ttyAMA0",115200,timeout=1)

while(True):
    try:    
        init1 = ord(ser.read())
        init2 = ord(ser.read()) 
        code1 = ord(ser.read())
        code2 = ord(ser.read())
        if(code1==0 and code2==0):
            hour = ord(ser.read())
            minute = ord(ser.read())
            second = ord(ser.read())
            dayOfWeek = ord(ser.read())
            day = ord(ser.read())
            month = ord(ser.read())
            year = ord(ser.read())+2018
            sumBytes = ord(ser.read())
            print "%d %d %d %d %d %d %d %d" % (hour,minute,second,dayOfWeek,day,month,year,sumBytes)

        else:
            b1 = ord(ser.read())
            b2 = ord(ser.read())
            b3 = ord(ser.read())
            b4 = ord(ser.read())
            sumBytes = ord(ser.read())
            print("%d %d %d %d %d" % (b1,b2,b3,b4,sumBytes))   
    except:
        pass

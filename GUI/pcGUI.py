# pcGUI.py
import PySimpleGUI as sg
import math
import os.path
import time
from random import randint
from datetime import datetime as dt

from meter import Meter
from meter import Clock
from meter import Tick
from meter import Pointer

from rylr import rylr
from rylr import Log
from rylr import Packet

import serial.tools.list_ports
ports = serial.tools.list_ports.comports()

#For checking GUI without connecting LoRa
#True --> going to connect LoRa
#False --> only want to test GUI
portConnect = False

#Selects color Theme / Layout of Window
sg.theme("Light Blue")

#Baud Rate
br = 115200

GRAPH_SIZE = (400,200)
GRAPH_STEP_SIZE = 5

#Element Keys
keys = ['-CODE-',
        '-LAT-','-LONG-','-SPEED-',
        '-iBAT-','-iM1-','-iM2-','-iSOLAR-',
        '-V12-','-V24-','-V36-','-VM1-','-VM2-','-vSOLAR-',
        '-TEMP1-','-TEMP2-','-RPMS-',]

otherKeys = ['-B1BAR-','-B2BAR-','-B3BAR-',
             '-M1BAR-','-M2BAR-',
             '-GRAPH-','-CODE_MEANING-',]

csvTitle = "timestamp,message code,lat,long,Speed,Battery Current,Motor 1 Current,Motor 2 current,Solar current,V12,V24,V36,Vm1,Vm2,Vsolar,Temp1,Temp2,rpm"
rcvTitle = "TS,MC,Lat,Long,Speed,iBat,iM1,iM2,iSolar,V12,V24,V36,VM1,VM2,VSolar,Temp1,Temp2,RPM"

#Battery Layout
bat1 = [[sg.Text(text = 'Battery V12', font=('Arial Bold', 10),size=10)],
        [sg.ProgressBar(200, orientation='h', size=(5.5, 15),  key='-B1BAR-')],
        [sg.Text(text = '', font=('Arial', 10), key = '-V12-')]
        ]

bat2 = [[sg.Text(text = 'Battery V24', font=('Arial Bold', 10),size=10)],
        [sg.ProgressBar(200, orientation='h', size=(5.5, 15),  key='-B2BAR-')],
        [sg.Text(text = '', font=('Arial', 10), key = '-V24-')]
        ]

bat3 = [[sg.Text(text = 'Battery V36', font=('Arial Bold', 10),size=10)],
        [sg.ProgressBar(200, orientation='h', size=(5.5, 15),  key='-B3BAR-')],
        [sg.Text(text = '', font=('Arial', 10), key = '-V36-')]
        ]

batCur = [sg.Text(text = 'Current: ', font=('Arial', 10)),
          sg.Text(text = '', font=('Arial', 10), key = '-iBAT-')
          ]

graph = [[sg.Graph(GRAPH_SIZE, (0,0), GRAPH_SIZE, key='-GRAPH-', background_color='lightblue')]]

bat_layout = [[sg.Column(bat1), sg.Column(bat2), sg.Column(bat3)], batCur, [sg.Column(graph)]]

batteryFrame = sg.Frame('Batteries',bat_layout, font=('Arial Bold', 20))

#----------------------------------------------------------------------------------------------------#
#Motor Layout

#Motor 1
motor1 = [  [sg.Text(text = 'Motor 1', font=('Arial Bold', 10),size=10)],
            [sg.Text(text = 'Voltage: ', font=('Arial Bold', 10),size=10), 
             sg.Text(text = '', font=('Arial', 10), key = '-VM1-')],
            [sg.Text(text = 'Current: ', font=('Arial Bold', 10),size=10), 
             sg.Text(text = '', font=('Arial', 10), key = '-iM1-')],
            [sg.Text(text = 'Temperature', font=('Arial', 10))],
            [sg.ProgressBar(200, orientation='h', size=(18, 15),  key='-M1BAR-')],
            [sg.Text(text = '', font=('Arial', 10),size=10, justification= "center", key = '-TEMP1-')]
         ]

#Motor 2
motor2 = [  [sg.Text(text = 'Motor 2', font=('Arial Bold', 10),size=10)],
            [sg.Text(text = 'Voltage: ', font=('Arial Bold', 10),size=10), 
             sg.Text(text = '', font=('Arial', 10), key = '-VM2-')],
            [sg.Text(text = 'Current: ', font=('Arial Bold', 10),size=10), 
             sg.Text(text = '', font=('Arial', 10), key = '-iM2-')],
            [sg.Text(text = 'Temperature', font=('Arial', 10))],
            [sg.ProgressBar(200, orientation='h', size=(18, 15),  key='-M2BAR-')],
            [sg.Text(text = '', font=('Arial', 10),size=10, justification= "center", key = '-TEMP2-')]
         ]

#Tachometer
rpm = [sg.Text(text = 'Rotations (RPM): ', font=('Arial Bold', 15),size=15),
       sg.Text(text = '', font=('Arial', 10), size = 10, key= '-RPMS-')]

meter_rpm = [[sg.Graph((300, 130), (-150, -20), (150, 110), key='-RPMMeter-')]]

motor_layout = [[sg.Column(motor1), sg.Column(motor2)], rpm, [sg.Column(meter_rpm, justification='r')]]

motorFrame = sg.Frame('Motors', motor_layout, font=('Arial Bold', 20))

#----------------------------------------------------------------------------------------------------#
#Solar Panels
solar_layout = [[sg.Text(text = 'Voltage: ', font=('Arial', 10)),
                sg.Text(text = '', font=('Arial', 10), key = '-vSOLAR-')],
                [sg.Text(text = 'Current: ', font=('Arial', 10)),
                 sg.Text(text = '', font=('Arial', 10), key = '-iSOLAR-', size=(5, 1))],
                [sg.Text(text='')]
                ]
solarFrame = sg.Frame('Solar', solar_layout, font=('Arial Bold', 20))

#----------------------------------------------------------------------------------------------------#
#GPS
meter_gps = [[sg.Graph((300, 130), (-150, -20), (150, 110), key='-GPSMeter-')]]

gps_txt = [[sg.Text(text = 'Latitude:', font=('Arial', 10)), 
               sg.Text(text = '', font=('Arial', 10), key = '-LAT-')],
              [sg.Text(text = 'Longitude:', font=('Arial', 10)), 
               sg.Text(text = '', font=('Arial', 10), key = '-LONG-',size=(12,1))],  
              [sg.Text(text = 'Speed:', font=('Arial', 10)),
               sg.Text(text = '', font=('Arial', 10), key = '-SPEED-')]]

gps_layout = [[sg.Column(gps_txt), sg.Column(meter_gps)]]

gpsFrame = sg.Frame('GPS', gps_layout, font=('Arial Bold', 20))

#----------------------------------------------------------------------------------------------------#
#Message
message_layout = [[sg.Text(text = '', font=('Arial', 10), key='-CODE-')],
               [sg.Text(text = '', font=('Arial', 10), key='-CODE_MEANING-', size = (13, 2))]]

codeFrame = sg.Frame('Message', message_layout, font=('Arial Bold', 20), element_justification='center', right_click_menu=keys)

#----------------------------------------------------------------------------------------------------#
#Counter
counterButtons = [[sg.Text('0', font=('Arial Bold', 20), key='-NUM-')],[sg.Button('-', size=(2,1)), sg.Button('Clear'), sg.Button('+', size=(2,1))]]
counterFrame = sg.Frame('Counter',counterButtons, font=('Arial Bold', 20), element_justification="c")

#----------------------------------------------------------------------------------------------------#
#Checks Before getting to GUI
if portConnect:
    #Check for COM port
    if len(ports) == 0:
        sg.popup_auto_close("No Port Found")
        exit()
    elif len(ports) == 1:
        for port, desc, hwid in sorted(ports):
                portDesc = ("{}: {}".format(port, desc))
        sg.popup_auto_close("Port found\n" + portDesc)
        selectedPort = portDesc[0:4]
    else:
        multiport = [sg.Text(text='Select a port:', font=('Arial Bold', 15),size=15)]
        selectPort = []
        thePorts = []
        i = 0
        
        for port, desc, hwid in sorted(ports):
            portDesc = ("{}".format(port))
            thePorts.append(portDesc)
            selectPort.append(sg.Radio(portDesc, "p", key=str(i)))
            i = i + 1
        portButton = [sg.Button("OK"), sg.Button("Exit")]
        portLayout = [multiport, selectPort, portButton]
        portWin = sg.Window('Port Selection', portLayout)
        
        while True:
            event, values = portWin.read()
            print(event, values)
            
            if event in (None, 'Exit'):
                break
            if event == 'OK':
                selected = False
                for x in values:
                    if values[str(x)] == True:
                        selectedPort = thePorts[int(x)]
                        selected = True
                
                if(not selected):
                   sg.popup_auto_close("No port selected")

                break  
        portWin.close()

     #LoRa Check
    lora = rylr(br, port = selectedPort)
    if(lora == None):
        sg.popup_auto_close("No connection with LoRa")
        exit();
    else:
        paramName = ["Address: ", "Network ID: ", "IPR: ", "Paramaters: "]
        param = [lora.address, lora.networkID, lora.ipr, lora.parameter]
        param_str = ''
        for x in range(4):
            param_str = param_str + paramName[x] + param[x] + "\n"
        sg.popup_auto_close(param_str, auto_close_duration=20)

    lora_layout = [[sg.Text(text='Last Updated: ' + lora.timestamp, key="-TIME-")],
                [sg.Text(text='')],
                [sg.Text(text=paramName[0] + param[0])],
                [sg.Text(text=paramName[1] + param[1])],
                [sg.Text(text=paramName[2] + param[2])],
                [sg.Text(text=paramName[3] + param[3])]]
else:
    ts = dt.now().strftime("%Y-%m-%d %H:%M:%S.%f")
    lora_layout = [[sg.Text(text='Last Updated: ' + ts[:-3], key="-TIME-")],
                [sg.Text(text='')],
                [sg.Text(text='Address: 5')],
                [sg.Text(text='Network ID: 5')],
                [sg.Text(text='IPR: 115200')],
                [sg.Text(text='Parameters: 12,7,1,4')]]
#----------------------------------------------------------------------------------------------------#

loraFrame = sg.Frame('LoRa', lora_layout, font=('Arial Bold', 20))   
rcvFrame = sg.Frame('RCV',[[sg.Multiline('',autoscroll=True,size=(95,8), key='-RCV-', justification='l')]], font=('Arial Bold', 20))

#----------------------------------------------------------------------------------------------------#

lora_row = [loraFrame, rcvFrame]

col1 = [[batteryFrame], [codeFrame, solarFrame, counterFrame]]

col2 = [[gpsFrame], [motorFrame]]

topRow = [[sg.Column(col1, vertical_alignment="top"),sg.Column(col2)]]

layout = [lora_row,[topRow]]

window = sg.Window('Solar Boat Land', layout, resizable = True,finalize=True)

#----------------------------------------------------------------------------------------------------#
#Tachometer Meter

drawRPM = window.find_element('-RPMMeter-')

clockRPM = Clock(drawRPM, start_angle=0, stop_angle=180, fill_color='black',
              line_width=5, style='arc')
minor_tickRPM = Tick(drawRPM, start_angle=30, stop_angle=150, line_width=2)
major_tickRPM = Tick(drawRPM, start_angle=30, stop_angle=150, line_width=5,
                 start_radius=80, step=30)
pointerRPM = Pointer(drawRPM, angle=30, inner_radius=10, outer_radius=75,
                  pointer_color='red',outer_color="red", line_width=5)
meterRPM = Meter(drawRPM, clock=clockRPM, minor_tick=minor_tickRPM, major_tick=major_tickRPM,
              pointer=pointerRPM)
new_angleRPM = randint(0, 180)
meterRPM.change(degree=new_angleRPM)

#----------------------------------------------------------------------------------------------------#
#Tachometer Meter

drawGPS = window.find_element('-GPSMeter-')

clockGPS = Clock(drawGPS, start_angle=0, stop_angle=180, fill_color='black',
              line_width=5, style='arc')
minor_tickGPS = Tick(drawGPS, start_angle=30, stop_angle=150, line_width=2)
major_tickGPS = Tick(drawGPS, start_angle=30, stop_angle=150, line_width=5,
                  start_radius=80, step=30)
pointerGPS = Pointer(drawGPS, angle=30, inner_radius=10, outer_radius=75,
                  pointer_color='red',outer_color="red", line_width=5)
meterGPS = Meter(drawGPS, clock=clockGPS, minor_tick=minor_tickGPS, major_tick=major_tickGPS,
              pointer=pointerGPS)
new_angleGPS = randint(0, 180)
meterGPS.change(degree=new_angleGPS)

#----------------------------------------------------------------------------------------------------#
#Names of Files
timestamp = dt.now().strftime("%Y-%m-%d")
rcv_name = timestamp + "_RCV.txt"
data_name = timestamp + "_Data.txt"

pathData = './' + data_name
checkDataFile = os.path.exists(pathData)

keyNum = len(keys)

#----------------------------------------------------------------------------------------------------#
count = 0
testData = "0,40.546906,73.0733222,12.3,200.1,400.1,400.1,40.1,12.1,24.2,36.3,16.1,16.1,17.5,20.5,30.5,300"
testPacket = Packet(data=testData)

with Log(rcv_name) as rcvfile, Log(data_name) as datafile:
    if not checkDataFile:
        datafile.logging(csvTitle)
    
    delay = x = lastx = lasty = 0
    while True:                             # Event Loop
        event, values = window.read(timeout=0)
        if event in (None, sg.WIN_CLOSED):
            break
        
    #For Meters    
        if meterRPM.change():
            new_angle = randint(30, 150)
            meterRPM.change(degree=new_angle)

        if meterGPS.change():
            new_angle = randint(30, 150)
            meterGPS.change(degree=new_angle)
    
    #For Lap Counter    
        if event == '+':
            count = count + 1
            window['-NUM-'].update(count)

        if event == '-':
            count = count - 1
            window['-NUM-'].update(count)

        if event == 'Clear':
            count = 0
            window['-NUM-'].update()
    #For Connected LoRa
        if portConnect:
            if(lora.receive()):
                rcvfile.logging(lora.rcv)
                datafile.logging(lora.timestamp + "," + str(lora._packet))        
                for i in range(keyNum):
                    window[keys[i]].update(lora._packet.dataSplit[i])
                    window['-TIME-'].update("Last Updated:" + lora.timestamp)
                window['-RCV-'].update(lora.timestamp+'\n', append=True)
                window['-RCV-'].update(rcvTitle+'\n', append=True)
                window['-RCV-'].update(str(lora._packet)+'\n\n', append=True)
    #For GUI Testing without LoRa
        else:
            message = "0,40.546906,73.0733222,12.3,200.1,400.1,400.1,40.1,12.1,24.2,36.3,16.1,16.1,17.5,20.5,30.5,300"
            ts = dt.now().strftime("%Y-%m-%d %H:%M:%S.%f")
            for i in range(keyNum):
                    window[keys[i]].update(testPacket.dataSplit[i])
                    window['-TIME-'].update("Last Updated:" + ts)
            window['-RCV-'].update(ts[:-3]+'\n', append=True)
            window['-RCV-'].update(rcvTitle+'\n', append=True)
            window['-RCV-'].update(message+'\n\n', append=True)

        step_size, delay = 5, 30 #values['-STEP-SIZE-'], values['-DELAY-']
        y = randint(0,GRAPH_SIZE[1])        # get random point for graph
        if x < GRAPH_SIZE[0]:               # if still drawing initial width of graph
            window['-GRAPH-'].DrawLine((lastx, lasty), (x, y), width=1)
        else:                               # finished drawing full graph width so move each time to make room
            window['-GRAPH-'].Move(-step_size, 0)
            window['-GRAPH-'].DrawLine((lastx, lasty), (x, y), width=1)
            x -= step_size
        lastx, lasty = x, y
        x += step_size
    window.close()
exit()
#import PySimpleGUI as sg
import signal
import serial
import sys
import time

from datetime import datetime as dt

import serial.tools.list_ports
ports = serial.tools.list_ports.comports()

#CTRL + C --> exit while loop

# Compatable UART Baud Rates
#uartBR = [300, 1200, 4800, 9600, 19200, 28800, 38400, 57600, 115200]
br = 115200

run = True

def signal_handler(signal, frame):
    global run
    print("exiting")
    run = False

#----------Data Packet Class----------#
class Packet:
    def __init__(self, data, addr=0, rssi=0, snr=0):
        self.data = data
        self.addr = addr
        self.rssi = rssi
        self.snr = snr
        
        self.dataSplit = self.data.split(",")
        if(len(self.dataSplit) == 17):
            self.messageCode = self.dataSplit[0]
            self.lat = self.dataSplit[1]
            self.long = self.dataSplit[2]
            self.speed = self.dataSplit[3]
            self.iBat = self.dataSplit[4]
            self.iM1 = self.dataSplit[5]
            self.iM2 = self.dataSplit[6]
            self.iSolar = self.dataSplit[7]
            self.v12 = self.dataSplit[8]
            self.v24 = self.dataSplit[9]
            self.v36 = self.dataSplit[10]
            self.vm1 = self.dataSplit[11]
            self.vm2 = self.dataSplit[12]
            self.vSolar = self.dataSplit[13]
            self.temp1 = self.dataSplit[14]
            self.temp2 = self.dataSplit[15]
            self.rpm = self.dataSplit[16]

    def __str__(self):
        return self.data

#----------File Log Class----------#
class Log:
    def __init__(self,filename):
        self.filename=filename
        self.fp=None    
    
    def logging(self,text):
        self.fp.write(text+'\n')
    
    def __enter__(self):
        self.fp=open(self.filename,"a+")
        return self    
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.fp.close()

#----------LoRa RYLR896 Class----------#
#Doesn't encompass all the possible commands
#Does enough needed

class rylr:
    def __new__(cls, baudrate, port):
        s = serial.Serial(port,baudrate, timeout=0.5)
        at = "AT\r\n"
        s.write(at.encode())
        message = s.read(3).decode('UTF-8')
        if (message != "+OK"):
            s.close()
            time.sleep(1)
            return None
        
        s.close()
        time.sleep(1)
        return super().__new__(cls)
    
    def __init__(self,baudrate,port="COM4"):
        self.ser = serial.Serial(port,baudrate=baudrate,timeout=0.5)
        self.serPort = port 
        self.message = None
        self._packet = None
        self._sent = None
        self.rcv = None
        self.hasRCV = False
        self.address = self.getAddress()
        self.ipr = self.getIPR()
        self.networkID = self.getNetworkID()
        self.parameter = self.getParameters()
        x = self.parameter.split(',')
        self.spreading_factor = x[0]
        self.bandwidth = x[1]
        self.coding_rate = x[2]
        self.programmed_preamble = x[3]

        self.timestamp = self.getTimestamp()
             
    def AT_command(self,cmd,time_sleep=1):
        # check AT command format
        cmd = cmd if cmd[-2:]=="\r\n" else cmd+"\r\n"
        self.ser.write(cmd.encode())
        time.sleep(time_sleep)
        self.message = self.ser.readline().decode('UTF-8')
        if("=" in cmd) :
            print(f"{cmd.split('=')[0][3:]} > {self.message}")
        elif (cmd == "AT\r\n") :
            print("AT > " + self.message)
        elif (not "?" in cmd) :
            print(f"{cmd.replace('/r/n', '')[0][3:]} > {self.message}")
        else:
            print(self.message)
    
    def send(self,message,address=5):
        length = len(str(message))
        self.AT_command(f"AT+SEND={address},{length},{message}\r\n")
    
    def receive(self):
        msg = self.ser.readline()
        time.sleep(1)
        if b'+RCV' in msg:
            self.rcv = msg[:-2].decode("UTF-8")
            self.timestamp = self.getTimestamp()
            print(self.rcv)
            msg = msg[5:].decode("UTF-8")
            addr, n, x = msg.split(',', 2)
            n = int(n)
            data = x[:n]
            x = x[n+1:]
            rssi, snr = x.split(',')
            self._packet = Packet(data, int(addr), int(rssi), int(snr))
            return True

    def test(self):
        self.AT_command("AT")

    def close(self):
        self.ser.close()
#get parameters
    def getAddress(self):
        self.AT_command("AT+ADDRESS?",time_sleep=0.2)
        return self.message.split('=')[1][:-2]
    
    def getIPR(self):
        self.AT_command("AT+IPR?",time_sleep=0.2)
        return self.message.split('=')[1][:-2]
    
    def getNetworkID(self):
        self.AT_command("AT+NETWORKID?",time_sleep=0.2)
        return self.message.split('=')[1][:-2]
    
    def getParameters(self):
        self.AT_command("AT+PARAMETER?",time_sleep=0.2)
        return self.message.split('=')[1][:-2]
    
    def getTimestamp(self):
        ts = dt.now().strftime("%Y-%m-%d %H:%M:%S.%f")
        return ts[:-3]

#Set parameters
    def setAddress(self, param):
        self.AT_command("AT+ADDRESS=" + str(param),time_sleep=0.2)
        if (self.message == "+OK\r\n") :
            self.address = param
            return True
        else :
            print("Could not set ADDRESS")
            print(self.message)
            return False
    
    def setIPR(self, param):
        self.AT_command("AT+IPR=" + str(param),time_sleep=0.2)
        if (self.message == "+OK\r\n") :
            self.ipr = param
            self.ser.close()
            self.ser = serial.Serial(self.serPort,param,timeout=0.5)
            return True
        else :
            print("Could not set IPR")
            print(self.message)
            return False
        
    def setNetworkID(self, param):
        self.AT_command("AT+NETWORKID=" + str(param),time_sleep=0.2)
        if (self.message == "+OK\r\n") :
            self.address = param
            return True
        else :
            print("Could not set NETWORK ID")
            print(self.message)
            return False
        
#String
    def addressSTR(self):
        return "Address: " + self.address
    
    def networkIDSTR(self):
        return "Network ID: " + self.networkID
    
    def iprSTR(self):
        return "IPR: " + self.ipr
    

    
    
    
    
        
        
if __name__ == "__main__" :
    
    #Get Port name
    if(len(ports) == 0):
        print("No ports available")
        exit()
    else:
        for port, desc, hwid in sorted(ports):
            print("{}: {}".format(port, desc))
        enteredPort = input("Enter port name: ")
    
    signal.signal(signal.SIGINT, signal_handler)
    
    timestamp = dt.now().strftime("%Y-%m-%d")
    rcv_name = timestamp + "_RCV.txt"
    data_name = timestamp + "_Data.txt"

    lora = rylr(br, port = enteredPort)
    if(lora == None):
        print("No connection with LoRa")
        exit();

    print(lora.timestamp + "\n")

    with Log(rcv_name) as rcvfile, Log(data_name) as datafile:
        while run:
            if(lora.receive()):
                print(lora.timestamp)
                print(lora.rcv)
                print(lora._packet)
                rcvfile.logging(lora.timestamp)
                rcvfile.logging(lora.rcv)
                datafile.logging(str(lora._packet))

# Error Codes
# +ERR=1 -- There is not “enter” or 0x0D 0x0A in the end of the AT Command
# +ERR=2 -- The head of AT command is not “AT” string
# +ERR=3 -- There is not “=” symbol in the AT command
# +ERR=4 -- Unknow command
# +ERR=10 -- TX is over times
# +ERR=11 -- RX is over times
# +ERR=12 -- CRC error
# +ERR=13 -- TX data more than 240bytes
# +ERR=15 --Unknown error

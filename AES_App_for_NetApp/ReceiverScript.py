#!/usr/bin/env python3

import socket
from cdp import *
#from cdp.ciholas_serial_number import *
import datetime
import time
import struct
import argparse
import os
import hashlib
import codecs
import binascii

parser = argparse.ArgumentParser(description = 'Pass in encryption file, send cdp encryption packets, receive corrected encryption packets')
parser.add_argument('-filename', type = str, default = 'EncrFileOutput.txt', help='Path to encryption file')

parserK = argparse.ArgumentParser(description = 'Pass in key file')
parserK.add_argument('-filename', type = str, default = 'Transmission/key.dat', help='Path to encryption file')


CONFIG_PORT = 7671
INTERNAL_PORT = 7694
#interface = '10.99.127.70' 
interface = '10.99.127.70' 
group = "239.255.76.94"

#Addresses of anchors
ANCHOR_0081_ADDRESS = ('10.99.129.105', 49153)
ANCHOR_02FE_ADDRESS = ('10.99.129.180', 49153)
ANCHOR_0249_ADDRESS = ('10.99.129.145', 49153)
ANCHOR_01E3_ADDRESS = ('10.99.130.195', 49153)

#How many bits to shift nt64
SHIFT_AMOUNT = 16

#Encryption packets under this length will be padded with trailing spaces (packets larger are left alone)
MIN_PACKET_SIZE = 20

#Convert uint bytes to ints, then to string
def convertBuffer(buffer):    
    intBuffer = []
    #uintFromByte = struct.unpack('>H', buffer)
    for item in buffer:        
        intFromUInt = item if item <128 else item - 2**8        
        intBuffer.append(intFromUInt)
    
    string = ' '.join(str(e) for e in intBuffer)
    
    return string

def convertKeyToHex(key):
    string = ''
    for item in key:
        string += str(hex(int(item)).lstrip("0x"))
        string += ' '
    
    return string

def writeToFile(stringToWrite, fileName):
    fileDes = open(fileName, 'w+')    
    
    fileDes.write(stringToWrite)    
    
    fileDes.close()
    
    return

#setup aggregation data read socket
data_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
data_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 32)
data_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
data_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
data_socket.bind(('', INTERNAL_PORT))
#data_socket.bind(('', CONFIG_PORT))
data_socket.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF, socket.inet_aton(interface))
data_socket.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(group) + socket.inet_aton(interface)) 

accepted_serials = [0x01040081, 0x010402FE, 0x01040249, 0x010401E3]

print('Listening for packets on ', INTERNAL_PORT)

tickCount = 0
awaiting_reply = False
sentTime = None
lastSentPacket = None
receiver_addr = None
payload_strings = []
received_packets = []

ANCHORS = [ ANCHOR_0081_ADDRESS, ANCHOR_02FE_ADDRESS, ANCHOR_0249_ADDRESS, ANCHOR_01E3_ADDRESS ]

ACNHOR_RECV = ANCHORS[3]

#variables for the cryptosystem
TICKS_PER_SECOND =  975000 * 65536
#TICKS_PER_SECOND =  97500 * 65536

#OFFSET = 975000 * 65536

#Speed of light in m/s
C = 300000000
#Distance of anchors from receivers in meter

#distances from the anchor
dA = 4.961
dB = 4.812
dC = 4.826

last_sent = None

#Distance of anchor to receiver
TdistA = (dA // C) * TICKS_PER_SECOND
TdistB = (dB // C) * TICKS_PER_SECOND
TdistC = (dC // C) * TICKS_PER_SECOND

anchorDistances = [TdistA, TdistB, TdistC]

#Frame offset
TbtwnOffset = .0025 * TICKS_PER_SECOND

#1/255 of space of 2m sphere
Tslot = 0.00000000667 * TICKS_PER_SECOND

#Distance of last anchor to receiver
Tdistlast = 0
#Time of last transmission
Ttxlast = 0
#Time to transmit
Tx = 0
#time of last reception
TrxLast = 0
#current anor selected
currentAnchor = 1

#First one is dummy packet
sha = []#[0xFF, 0x5E,0x88,0x48,0x98,0xDA,0x28,0x04,0x71,0x51,0xD0,0xE5,0xC6,0x29,0x27,0x73,0x60,0x3D,0x0D,0x6A,0xAB,0xBD,0xD6,0xEF,0x72,0x1D,0x15,0x42,0xD8,0x49,0x29,0xFF,0x01]
shaIndex = 0

#print("SHA LEN: ", len(sha))

#Received Key bytes
#key = [0 for i in range(0,33)]
key = []

#print("KEY LEN", len(key))

print(' ')

packetIdx = 0

#current time
curentTime = 0

class EncryptionPacket(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Acknowledge List Response Data Item Definition"""

    type = 0x807F
    definition = [DIUInt64Attr('network_time'),
                  DIVariableLengthBytesAttr('data_payload'),]

class CorrectedEncryptionPacket(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Acknowledge List Response Data Item Definition"""

    type = 0x8080
    definition = [DIUInt64Attr('network_time'),
                  DIVariableLengthBytesAttr('data_payload'),]

class EncryptionPacketBuffer:
    anchorNumber = None
    nt64_send = None
    byte = None

    def __init__(self, anchor, time, byte):
        self.anchorNumber = anchor
        self.nt64_send = time
        self.byte = byte

def ReadKeyFile(fileName):
    script_dir = os.path.dirname(__file__) #<-- absolute dir the script is in
    abs_file_path = os.path.join(script_dir, fileName)

    file = open(abs_file_path)

    BYTES_IN_LINE = 32    

    data = []

    data.append(0xFF)

    while True:       
        line = file.readline()
        #Stop at EOF
        if(line == ''):
            #Add last packet before leaving                        
            break
        #Read data
        else:
            line = line.rstrip()

            #print(line)
            #List of ints
            lineData = list(line.split(' '))    
            #print(lineData)

            for item in lineData:                
                data.append(int(item,16))

            #print (data)
            
    file.close()
     
    return data


#Read encryption file, return list of (nt, packet) tuples
def ReadEncryptionFile(fileName):
    script_dir = os.path.dirname(__file__) #<-- absolute dir the script is in
    abs_file_path = os.path.join(script_dir, fileName)

    file = open(abs_file_path)

    packet = []
    data = []
    lineIdx = 0


    BYTES_IN_LINE = 32
    LINES_IN_PACKET = 800 / BYTES_IN_LINE

    while True:
        lineIdx += 1
        line = file.readline()
        #Stop at EOF
        if(line == ''):
            #Add last packet before leaving
            data.append( packet )
            packet = []
            break
        #Read data
        else:
            line = line.rstrip()
            #List of ints
            lineData = list(map(int,line.split(' ')))

            #Convert int8 to uint8
            for idx,num in enumerate(lineData):
                if(num < 0):
                    lineData[idx] = num + 2**8

            packet += (lineData)
            buffer = EncryptionPacketBuffer(int(lineData[0]), int(lineData[1]), lineData[2])
        #Every 800 lines is 1 packet length
        if(lineIdx % LINES_IN_PACKET == 0):
            data.append( packet )
            packet = []


    file.close()

    return data
    
def SendEncryptionBuffer(timeToSend, bytesToSend, anchorToSend):
    global ANCHORS, currentTime
    
    timeToSend = round(timeToSend)

    #Create data 
    dataItem = EncryptionPacket()
    dataItem.network_time = timeToSend
    #dataItem.data_payload = byteToSend.ljust(MIN_PACKET_SIZE, ' ')
    #print(('Bytes to send \t: '), bytesToSend)    
    dataItem.data_payload = bytes(bytesToSend)

    #Create packet, add header, data payload
    cdp_packet = CDP()
    #This serial number doesn't seem to matter?
    cdp_packet.serial_number = CiholasSerialNumber(0x010403B4)
    cdp_packet.add_data_item(dataItem) 
    packet_data = cdp_packet.encode()

    #print(datetime.datetime.now().time(), ' Sending encr packet @ nt: ', 
    #    timeToSend, ' currTime', currentTime, '\ndiff time', timeToSend - currentTime)

    #send packet
    data_socket.sendto(packet_data, ANCHORS[anchorToSend])


args = parser.parse_args()

#Read file
packet_list = ReadEncryptionFile(args.filename)

data_buffer = []

argsK = parserK.parse_args()

#read the key
sha = ReadKeyFile(argsK.filename)

#for item in sha:
#   print(item,  end =" ")
#print('\n') 

try:
    while True:#shaIndex < len(sha) + 1: # or awaiting_reply:
        try:
            #Read packets until we get a reply from the receiving anchor
            data, addr = data_socket.recvfrom(65536)
            new_rx_packet = CDP(data) #decode the data into a cdp packet and decode data items into their appropriate types\            

            #print( 'Serial: ', new_rx_packet.serial_number, '\t Addr: ', addr )

            for data_item in new_rx_packet.data_items_by_type[0x802D]:
                if(not awaiting_reply and shaIndex < len(sha)):
                    if(Ttxlast == 0):
                            Ttxlast = data_item.nt64 + TICKS_PER_SECOND
                            Trxlast = data_item.nt64 + TICKS_PER_SECOND
                            #Ttxlast = data_item.nt64
                            #TrxLast = data_item.nt64
                    else:
                        slot = sha[shaIndex]                        
                        shaIndex +=1 

                        #print(slot)

                        #Send packet, then wait for reply
                        #TICKS_PER_SECOND is a one frame offset
                        time_to_send = Ttxlast + Tdistlast + TbtwnOffset - anchorDistances[currentAnchor] + ((slot + 0.5) * Tslot) + TICKS_PER_SECOND
                    
                        #print('Current nt: ', data_item.nt64, 'ShaIndex: ', shaIndex, 'Scheduled to send: ', int(time_to_send)

                        currentTime = data_item.nt64 

                        #packetsToSend = [shaIndex] + packet_list[packetIdx] if shaIndex > 5 else GARBAGE_DATA
                        packetsToSend = [shaIndex] + packet_list[packetIdx]
                        
                        SendEncryptionBuffer( time_to_send, packetsToSend, currentAnchor )
                        awaiting_reply = True

                        Ttxlast = time_to_send

                        Tdistlast = anchorDistances[currentAnchor]

                        #nchor += 1
                        if(currentAnchor > 2):
                            currentAnchor = 0

                        keyByte = slot

                        packetIdx += 1

                        if (shaIndex == 33):
                            shaIndex = 0 
                            Ttxlast = 0                    
                
            #if(addr == ANCHORS[3]):
            if(True):
                if(addr == ANCHOR_01E3_ADDRESS):
                    #print( data_item.nt64)
                    for data_item in new_rx_packet.data_items_by_type[0x8080]:
                        print('Serial:', new_rx_packet.serial_number, ' Received a packet 0x8080', ' nt: ', data_item.network_time)#, 'shaIdx: ', data_item.data_payload[0])
                        #print ('payload: ', data_item.data_payload.strip() )
                        #print ('time received: ', hex(data_item.network_time) )
                        #print('nt: \t\t', data_item.network_time, ' trxLast: ', TrxLast, ' tbtwenoffset: ', TbtwnOffset, 'tslot: ', Tslot)
                        #Calculate keyByte
                        keyByte = (data_item.network_time - TrxLast - TbtwnOffset - TICKS_PER_SECOND ) / Tslot 

                        #Record shaindex
                        shaIndexR = data_item.data_payload[0]
                        #Remove shaindex from data payload
                        #del data_item.data_payload[0]

                        
                        if(keyByte < 256 and shaIndexR > 0): # and key[shaIndexR-1] == 0):
                            #key[shaIndexR-1] = round(keyByte)
                            key.append(int(keyByte))

                        #Append payload to buffer (disregard first byte: shaindex)
                        data_buffer += data_item.data_payload[1:]

                        #Update TrxLast for next recv
                        TrxLast = data_item.network_time

                        print('Key byte = ', hex(int(keyByte)), 'Index: ', shaIndexR,'\n')#, '\tSha indexR: ', shaIndexR, '\n\n\n')

                        awaiting_reply = False                        

                        
                        if (shaIndexR == 33):
                            print('THE KEY IS:')
                            for item in key:
                                print(hex(int(item)).lstrip("0x"),  end =" ")
                            print('\n')                            
                            convertedBuf = convertBuffer(data_buffer)
                            writeToFile(convertedBuf, "Output.txt")
                            convertedKey = convertKeyToHex(key)
                            writeToFile(convertedKey, "key.dat")

                            key.clear()

        except (KeyboardInterrupt, SystemExit):
            data_socket.close()

    print('Done with loop')

    convertedBuf = convertBuffer(data_buffer)
    writeToFile(convertedBuf, "OutputBuffer.txt")
    convertedKey = convertKeyToHex(key)
    writeToFile(convertedKey, "key.dat")

except Exception as e:
    print (e)    

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

parser = argparse.ArgumentParser(description = 'Pass in encryption file, send cdp encryption packets, receive corrected encryption packets')
parser.add_argument('-filename', type = str, default = 'EncrFileOutput.txt', help='Path to encryption file')

CONFIG_PORT = 7671
INTERNAL_PORT = 7694
interface = '12.12.0.1' 
group = "239.255.76.94"

#Addresses of anchors
ANCHOR_03B4_ADDRESS = ('12.12.0.4', 49153)
ANCHOR_0288_ADDRESS = ('12.12.0.18', 49153)
ANCHOR_056A_ADDRESS = ('12.12.0.17', 49153)
ANCHOR_01E3_ADDRESS = ('12.12.0.7', 49153)

#How many bits to shift nt64
SHIFT_AMOUNT = 16

#Encryption packets under this length will be padded with trailing spaces (packets larger are left alone)
MIN_PACKET_SIZE = 20

#setup aggregation data read socket
data_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
data_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 32)
data_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
data_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
data_socket.bind(('', INTERNAL_PORT))
#data_socket.bind(('', CONFIG_PORT))
data_socket.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF, socket.inet_aton(interface))
data_socket.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(group) + socket.inet_aton(interface)) 

accepted_serials = [0x010403B4, 0x01040288, 0x0104056A, 0x010401E3]

print('Listening for packets on ', INTERNAL_PORT)

tickCount = 0
awaiting_reply = False
sentTime = None
lastSentPacket = None
receiver_addr = None
payload_strings = []
received_packets = []

ANCHORS = [ ANCHOR_03B4_ADDRESS, ANCHOR_0288_ADDRESS, ANCHOR_056A_ADDRESS, ANCHOR_01E3_ADDRESS ]

ACNHOR_RECV = ANCHORS[3]

#variables for the cryptosystem

TICKS_PER_SECOND =  975000 * 65536
OFFSET = 97500 * 65536 
#Speed of light in m/s
C = 300000000
#Distance of anchors from receivers in meter
ANCHOR_DISTANCE1 = 1.441 #3b4
ANCHOR_DISTANCE2 = 1.452 #288
ANCHOR_DISTANCE3 = 1.626 #564


#distances from the anchor
dA = ANCHOR_DISTANCE1
dB = ANCHOR_DISTANCE2
dC = ANCHOR_DISTANCE3

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
sha = [0xFF, 0x5E,0x88,0x48,0x98,0xDA,0x28,0x04,0x71,0x51,0xD0,0xE5,0xC6,0x29,0x27,0x73,0x60,0x3D,0x0D,0x6A,0xAB,0xBD,0xD6,0xEF,0x72,0x1D,0x15,0x42,0xD8,0x49,0x29,0xFF,0x01]
shaIndex = 0

#Received Key bytes
key = [0 for i in range(0,32)]

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
            #buffer = EncryptionPacketBuffer(int(lineData[0]), int(lineData[1]), lineData[2])
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
    print('Bytes to send \t: ')#, bytesToSend)
    dataItem.data_payload = bytes(bytesToSend)

    #Create packet, add header, data payload
    cdp_packet = CDP()
    #This serial number doesn't seem to matter?
    cdp_packet.serial_number = CiholasSerialNumber(0x010403B4)
    cdp_packet.add_data_item(dataItem) 
    packet_data = cdp_packet.encode()

    print(datetime.datetime.now().time(), ' Sending encr packet @ nt: ', 
        timeToSend, ' currTime', currentTime, '\ndiff time', timeToSend - currentTime)

    #send packet
    data_socket.sendto(packet_data, ANCHORS[anchorToSend])


args = parser.parse_args()

#Read file
packet_list = ReadEncryptionFile(args.filename)


try:
    while shaIndex < len(sha)+1: # or awaiting_reply:
        try:
            #Read packets until we get a reply from the receiving anchor
            data, addr = data_socket.recvfrom(65536)
            new_rx_packet = CDP(data) #decode the data into a cdp packet and decode data items into their appropriate types\            

            #print( 'Serial: ', new_rx_packet.serial_number, '\t Addr: ', addr )

            for data_item in new_rx_packet.data_items_by_type[0x802D]:
                if(not awaiting_reply):
                    if(Ttxlast == 0):
                            #Ttxlast = data_item.nt64 + TICKS_PER_SECOND
                            Ttxlast = data_item.nt64
                            TrxLast = data_item.nt64
                    else:
                        slot = sha[shaIndex]
                        shaIndex +=1 

                        #Send packet, then wait for reply
                        #TICKS_PER_SECOND is a one frame offset
                        time_to_send = Ttxlast + Tdistlast + TbtwnOffset - anchorDistances[currentAnchor] + ((slot + 0.5) * Tslot) + OFFSET
                    
                        print('\nCurrent nt: \t\t', data_item.nt64, 'ShaIndex: ', shaIndex)

                        currentTime = data_item.nt64 

                        #packetsToSend = [shaIndex] + packet_list[packetIdx] if shaIndex > 5 else GARBAGE_DATA
                        packetsToSend = [shaIndex] + packet_list[packetIdx]

                        SendEncryptionBuffer( time_to_send, packetsToSend, currentAnchor )
                        awaiting_reply = True

                        Ttxlast = time_to_send

                        Tdistlast = anchorDistances[currentAnchor]

                        currentAnchor += 1
                        if(currentAnchor > 2):
                            currentAnchor = 0

                        keyByte = slot

                        packetIdx += 1                       
                    
                
            #if(addr == ANCHORS[3]):
            if(True):
                #for data_item in new_rx_packet.data_items_by_type[0x802D]:
                    #print( data_item.nt64)
                for data_item in new_rx_packet.data_items_by_type[0x8080]:
                    print('got 0x8080 from serial:', new_rx_packet.serial_number, ' nt: \t\t', data_item.network_time)
                    #print ('payload: ', data_item.data_payload.strip() )
                    #print ('time received: ', hex(data_item.network_time) )
                    
                    #Calculate keyByte
                    keyByte = (data_item.network_time - TrxLast - TbtwnOffset - OFFSET) / Tslot

                    #Record shaindex
                    shaIndexR = data_item.data_payload[0]
                    #Remove shaindex from data payload
                    #del data_item.data_payload[0]
                    
                    if(keyByte < 256 and key[shaIndexR] == 0):
                        key[shaIndexR] = round(keyByte)

                    #Append payload to buffer (disregard first byte: shaindex)
                    #data_buffer += data_item.data_payload[1:]

                    #Update TrxLast for next recv
                    TrxLast = data_item.network_time

                    print('Key byte = ', int(keyByte), '\tSha indexR: ', shaIndexR, '\n\n\n')

                    awaiting_reply = False

        except (KeyboardInterrupt, SystemExit):
            data_socket.close()

    print('Done with loop')

except Exception as e:
    print (e)    
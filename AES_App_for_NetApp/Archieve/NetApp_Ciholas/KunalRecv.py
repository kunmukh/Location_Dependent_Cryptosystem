
#!/usr/bin/env python3
import socket
from cdp import *
from cdp.ciholas_serial_number import *
import datetime
import time
import struct
import argparse
import os
import hashlib

parser = argparse.ArgumentParser(description = 'Pass in encryption file, send cdp encryption packets, receive corrected encryption packets')
parser.add_argument('-filename', type = str, default = 'AES_App_for_NetApp/client/transmission.dat', help='Path to encryption file')

CONFIG_PORT = 7671
INTERNAL_PORT = 7694
interface = '12.12.0.1' 
group = "239.255.76.94"

#10Mhz 
REPEAT_RATE = 97500
#Ensures we're well clear of command window to start sending our own cdp packets
SCHEDULE_TIME_OFFSET = 10000
#Skip ahead a frame or two to make sure there's enough time to schedule our packet
CADENCE_OFFSET = 1
#CADENCE_OFFSET = 2

TICKS_PER_SECOND =  97500 * 65536
#TICKS_PER_SECOND =  975000 * 65536
#Speed of light in m/s
C = 300000000
#Distance of anchors from receivers in meter
ANCHOR_DISTANCE = 1

#How many nts to receive before attempting to send an encryption packet (how often to send)
#Not being used
TICK_COUNT_BEFORE_SENDING = 2
#TICK_COUNT_BEFORE_SENDING = 15

#Addresses of anchors
ANCHOR_03B4_ADDRESS = ('12.12.0.4', 49153)
ANCHOR_05BC_ADDRESS = ('12.12.0.5', 49153)
ANCHOR_03CA_ADDRESS = ('12.12.0.6', 49153)
ANCHOR_01E3_ADDRESS = ('12.12.0.7', 49153)

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
        string += str(hex(item))

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

accepted_serials = [0x01040288, 0x010405C9, 0x010405AE, 0x01040225, 0x010405BB, 0x01040409]

print('Listening for packets on ', INTERNAL_PORT)

tickCount = 0
awaiting_reply = False
sentTime = None
lastSentPacket = None
receiver_addr = None
payload_strings = []
received_packets = []

#               0288,                   0409,                       05BB,                   05AE
#ANCHORS = [ ('10.99.129.182', 49153), ('10.99.130.158', 49153), ('10.99.130.151', 49153), ('10.99.130.136', 49153) ]
ANCHORS = [ ANCHOR_03B4_ADDRESS, ANCHOR_05BC_ADDRESS, ANCHOR_03CA_ADDRESS, ANCHOR_01E3_ADDRESS ]

ACNHOR_RECV = ANCHORS[3]

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

args = parser.parse_args()


dA = ANCHOR_DISTANCE
dB = ANCHOR_DISTANCE
dC = ANCHOR_DISTANCE


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
#Tslot = 0.00000001668 * TICKS_PER_SECOND

#Distance of last anchor to receiver
Tdistlast = 0
#Time of last transmission
Ttxlast = 0
#Time to transmit
Tx = 0

TrxLast = 0

currentAnchor = 0

#Received Key bytes
key = [0 for i in range(0,32)]

data_buffer = []

shaIndex = 0

try:
    while True:
        try:
            #Read packets until we get a reply from the receiving anchor
            data, addr = data_socket.recvfrom(4096)
            new_rx_packet = CDP(data) #decode the data into a cdp packet and decode data items into their appropriate types\            

            #print( 'Serial: ', new_rx_packet.serial_number, '\t Addr: ', addr )

                
            if(addr == ANCHORS[3]):
                for data_item in new_rx_packet.data_items_by_type[0x8080]:
                    print('got 0x8080 from serial:', new_rx_packet.serial_number, ' nt: \t\t', hex(data_item.network_time))
                    #print ('payload: ', data_item.data_payload.strip() )
                    #print ('time received: ', hex(data_item.network_time) )
                    
                    #Calculate keyByte
                    keyByte = (data_item.network_time - TrxLast - TbtwnOffset - TICKS_PER_SECOND) / Tslot

                    #Record shaindex
                    shaIndex = data_item.data_payload[0]
                    #Remove shaindex from data payload
                    #del data_item.data_payload[0]

                    print(shaIndex)
                    if(keyByte < 256 and key[shaIndex] == 0):
                        key[shaIndex] = round(keyByte)

                    #Append payload to buffer (disregard first byte: shaindex)
                    #data_buffer += data_item.data_payload[1:]

                    #Update TrxLast for next recv
                    TrxLast = data_item.network_time

                    print('Key byte = ', hex(int(keyByte)), '\tSha index: ', shaIndex)

                    awaiting_reply = False

        except (KeyboardInterrupt, SystemExit):
            data_socket.close()

    print('Done with loop')

    convertedBuf = convertBuffer(data_buffer)
    writeToFile(convertedBuf, "Output.txt")
    convertedKey = convertKeyToHex(key)
    writeToFile(convertedKey, "Key.txt")

except Exception as e:
    print (e)    

    


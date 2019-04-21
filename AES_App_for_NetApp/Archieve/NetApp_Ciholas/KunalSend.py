
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

#10Mhz 
REPEAT_RATE = 97500
#Ensures we're well clear of command window to start sending our own cdp packets
SCHEDULE_TIME_OFFSET = 10000
#Skip ahead a frame or two to make sure there's enough time to schedule our packet
CADENCE_OFFSET = 1
#CADENCE_OFFSET = 2

#Uncomminging first one is quicker transmission, but key comes out noisy. Second one makes transmisison very slow, but improves accuracy
TICKS_PER_SECOND =  97500 * 65536
#TICKS_PER_SECOND =  975000 * 65536


#Speed of light in m/s
C = 300000000
#Distance of anchors from receivers in meter
ANCHOR_DISTANCE = 1

#How many nts to receive before attempting to send an encryption packet (how often to send)
#Not being used
#TICK_COUNT_BEFORE_SENDING = 2
#TICK_COUNT_BEFORE_SENDING = 15
TICK_COUNT_BEFORE_SENDING = 150

#Addresses of anchors
ANCHOR_03B4_ADDRESS = ('12.12.0.4', 49153)
ANCHOR_05BC_ADDRESS = ('12.12.0.5', 49153)
ANCHOR_03CA_ADDRESS = ('12.12.0.6', 49153)
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

accepted_serials = [0x01040288, 0x010405C9, 0x010405AE, 0x01040225, 0x010405BB, 0x01040409]

print('Sending packets on ', INTERNAL_PORT)

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

class EncryptionPacketBuffer:
    anchorNumber = None
    nt64_send = None
    byte = None

    def __init__(self, anchor, time, byte):
        self.anchorNumber = anchor
        self.nt64_send = time
        self.byte = byte

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
    global ANCHORS
    
    timeToSend = round(timeToSend)

    #Create data 
    dataItem = EncryptionPacket()
    dataItem.network_time = timeToSend
    #dataItem.data_payload = byteToSend.ljust(MIN_PACKET_SIZE, ' ')
    dataItem.data_payload = bytes(bytesToSend)

    #Create packet, add header, data payload
    cdp_packet = CDP()
    #This serial number doesn't seem to matter?
    cdp_packet.serial_number = CiholasSerialNumber(0x01040225)
    cdp_packet.add_data_item(dataItem) 
    packet_data = cdp_packet.encode()

    print(datetime.datetime.now().time(), ' - Sending encrypion packet with nt: \t', hex(timeToSend))

    #send packet
    data_socket.sendto(packet_data, ANCHORS[anchorToSend])




args = parser.parse_args()

#Read file
packet_list = ReadEncryptionFile(args.filename)

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

#First one is dummy packet
sha = [0x88, 0x88, 0x88, 0x88, 0x88, 0x5E,0x88,0x48,0x98,0xDA,0x28,0x04,0x71,0x51,0xD0,0xE5,0xC6,0x29,0x27,0x73,0x60,0x3D,0x0D,0x6A,0xAB,0xBD,0xD6,0xEF,0x72,0x1D,0x15,0x42,0xD8,0x49,0x29,0xFF,0x01]
shaIndex = 0

counter = TICK_COUNT_BEFORE_SENDING

packetIdx = 0

GARBAGE_DATA = [1,2,3,4,5,6,7,8,9,10]

try:
    while(packetIdx < len(packet_list)):
        while shaIndex < len(sha): # or awaiting_reply:
            try:
                #Read packets until we get a reply from the receiving anchor
                data, addr = data_socket.recvfrom(65536)
                new_rx_packet = CDP(data) #decode the data into a cdp packet and decode data items into their appropriate types\            

                #print( 'Serial: ', new_rx_packet.serial_number, '\t Addr: ', addr )

                for data_item in new_rx_packet.data_items_by_type[0x802D]:
                    #if(not awaiting_reply):
                    if(counter > 0):
                        if(Ttxlast == 0):
                            startTime = data_item.nt64 // TICKS_PER_SECOND
                            startTime = startTime * TICKS_PER_SECOND

                            Ttxlast = data_item.nt64 + TICKS_PER_SECOND
                        else:
                            slot = sha[shaIndex]
                            shaIndex +=1 

                            #Send packet, then wait for reply
                            #TICKS_PER_SECOND is a one frame offset
                            time_to_send = Ttxlast + Tdistlast + TbtwnOffset - anchorDistances[currentAnchor] + ((slot + 0.5) * Tslot) + TICKS_PER_SECOND

                        
                            print('\nCurrent nt: \t\t\t\t\t\t', hex(data_item.nt64), 'ShaIndex: ', shaIndex-5)

                            packetsToSend = [shaIndex] + packet_list[packetIdx] if shaIndex > 5 else GARBAGE_DATA

                            SendEncryptionBuffer( time_to_send, packetsToSend, currentAnchor )
                            awaiting_reply = True

                            Ttxlast = time_to_send

                            Tdistlast = anchorDistances[currentAnchor]

                            currentAnchor += 1
                            if(currentAnchor > 2):
                                currentAnchor = 0

                            keyByte = slot

                            counter -= 1

                            packetIdx += 1
                    else:
                        counter = TICK_COUNT_BEFORE_SENDING

                            
                        
                    
                # if(addr == ANCHORS[3]):
                #     for data_item in new_rx_packet.data_items_by_type[0x8080]:
                #         print('got 0x8080 from serial:', new_rx_packet.serial_number, ' nt: \t\t', hex(data_item.network_time))
                #         print ('payload: ', data_item.data_payload.strip() )
                #         #print ('time received: ', hex(data_item.network_time) )
                        
                #         #Calculate keyByte
                #         keyByte = (data_item.network_time - TrxLast - TbtwnOffset - TICKS_PER_SECOND) / Tslot

                #         #Update TrxLast for next recv
                #         TrxLast = data_item.network_time

                #         print('Key byte = ', hex(int(keyByte)), '\tSha index: ', shaIndex)

                #         awaiting_reply = False

            except (KeyboardInterrupt, SystemExit):
                data_socket.close()

        shaIndex = 5
        print('Done with sha loop')

    print('Sent all packets')

except Exception as e:
    print (e)    



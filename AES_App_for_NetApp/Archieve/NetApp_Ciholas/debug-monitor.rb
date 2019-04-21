#!/usr/bin/env ruby
require 'bindata'
require 'socket'
require 'ipaddr'
require_relative 'packet'

def serial_to_str(serial)
  serial_str = ""
  serial_str += sprintf("%02x", (serial >> 24) & 0xff)
  serial_str += sprintf("%02x", (serial >> 16) & 0xff)
  serial_str += sprintf("%04x", (serial) & 0xffff)
  serial_str
end

unless ARGV.size.between?(1,2)
  puts "Usage: #{$0} BIND_IP_ADDR [SERIAL]"
  exit -1
end

BIND_ADDR      = ARGV[0]
MULTICAST_ADDR = '239.255.11.11'
PORT           = 1111

FILTER = ARGV.size == 2 ? ARGV[1].to_i(16) : nil

socket = UDPSocket.new
membership = IPAddr.new(MULTICAST_ADDR).hton + IPAddr.new(BIND_ADDR).hton
socket.setsockopt(:IPPROTO_IP, :IP_ADD_MEMBERSHIP, membership)
socket.setsockopt(:SOL_SOCKET, :SO_REUSEADDR, 1)
socket.bind(Socket::INADDR_ANY, PORT)

begin
  while true do
    message, info = socket.recvfrom(512)

    cdp_packet = CdpPacket.read(message)

    if cdp_packet.header.mark == CDP_HEADER__MARK and cdp_packet.header.str[0..6] == CDP_HEADER__STRING[0..6]

      cdp_packet.data_items.each do |data_set|
        if FILTER.nil? or FILTER == cdp_packet.header.serial
          case data_set.identifier
          when 0x00FD
            if data_set.data =~ /^file: /
              File.open('output.txt','a').puts data_set.data.sub(/^file: /,'').chomp
            else
              puts sprintf("%s #{Time.now.to_s} - %s", serial_to_str(cdp_packet.header.serial), data_set.data).chomp
            end
          end
        end
      end
    end
  end
rescue SystemExit, Interrupt
  puts "\nGot CTRL-C"
  exit 0
end

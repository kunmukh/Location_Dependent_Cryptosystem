#!/usr/bin/env ruby
# ALL RIGHTS RESERVED - Copyright 2016 - Ciholas, Inc.
# Any use of this code without explicit written permission is prohibited
require 'bindata'

CDP_HEADER__MARK=0x3230434c
CDP_HEADER__STRING="CDP0002"

class CdpDataItem < BinData::Record
  endian :little

  uint16 :identifier
  uint16 :len
  string :data, :read_length => :len
end

class CdpPacket < BinData::Record
  endian :little

  buffer :header, :length => 20 do
    uint32 :mark
    uint32 :sequence
    string :str, :length => 8
    uint32 :serial
  end
  array  :data_items, :type => CdpDataItem, :read_until => :eof

  def IsValid
    if self.header.mark == CDP_HEADER__MARK and self.header.str[0..6] == CDP_HEADER__STRING[0..6]
      return true
    end

    return false
  end
  
  def SetupHeader
    self.header.mark     = CDP_HEADER__MARK
    self.header.sequence = 0
    self.header.str      = CDP_HEADER__STRING
    self.header.serial   = 0xFE000000
  end

  def AddItem(item_id, item_bytes)
    data_item = CdpDataItem.new
    data_item.identifier = item_id
    data_item.len = item_bytes.length
    data_item.data.read(item_bytes)

    self.data_items.push(data_item)
  end
  
end

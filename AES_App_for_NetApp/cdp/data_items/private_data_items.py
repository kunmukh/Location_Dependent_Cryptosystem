# pylint: disable=trailing-whitespace, too-few-public-methods

from cdp.cdp import *

# Protected Data Items

class UserDefinedCommandV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol User Defined Command Data Item Definition"""

    type = 0x000B
    definition = [DISerialNumberAttr('serial_number'),  # Recipient's serial number
                  DIUInt8Attr('command_type'),  # 1B - Command demuxing byte
                  DIVariableLengthBytesAttr('payload')]  # User defined data


class PinStateReport(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Pin State Report Data Item Definition"""

    type = 0x000C
    definition = [DIUInt32Attr('gnt'),  # 4B - global network time
                  DIUInt8Attr('pin_bank'),  # 3b - unused/5b - reported pin bank
                  DIUInt8Attr('pin_mask')]  # 1B - pin mask representing pin status

    def get_reported_pin_bank(self):
        """Returns number representing which pin bank is being reported"""
        return self.pin_bank & 0x1F


class PinConfig:
    """Pin Config Class Definition"""

    definition = [DIUInt8Attr('data')]

    def __init__(self, data=0):
        self.data = data

    def get_poll_interval(self):
        return self.data >> 5

    def get_change_report_flag(self):
        return (self.data & 0b00011000) >> 3

    def get_pin_configuration(self):
        return self.data & 0x07

    def __repr__(self):
        return self.data

    def __eq__(self, other):
        if isinstance(other, PinConfig):
            return self.data == other.data
        elif isinstance(other, int):
            return self.data == other
        return False

    def __str__(self):
        return str(self.data)


class PinConfigurationReport(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Pin Configuration Report Data Item Definition"""

    type = 0x000D
    definition = [DIUInt32Attr('gnt'),  # 4B - global network time
                  DIUInt8Attr('pin_bank'),  # 3b - unused/5b - reported pin bank
                  DIUInt8Attr('pin_mask'),  # 1B - pin mask representing pin status
                  DIListAttr('pin_configurations', PinConfig)]

    def get_reported_pin_bank(self):
        """Returns number representing which pin bank is being reported"""
        return self.pin_bank & 0x1F

    def add_pin_configurations(self, data):
        self.pin_configurations.append(PinConfig(data))


class PinConfiguration(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Pin Configuration Data Item Definition"""

    type = 0x000E
    definition = [DISerialNumberAttr('serial_number'),  # Serial number of device being configured
                  DIUInt8Attr('pin_bank'),  # 1b-send report flag/2b-unused/5b-reported pin bank
                  DIUInt8Attr('pin_mask'),  # 1B - pin mask representing pin status
                  DIListAttr('config_options', PinConfig)]

    def get_send_report_flag(self):
        """Indicates if a report telling configurations of pins should be sent"""
        return (self.pin_bank >> 7) != 0

    def get_reported_pin_bank(self):
        """Returns number defining which 8-pin Pin Bank to configure"""
        return self.pin_bank & 0x1F

    def add_pin_configurations(self, data):
        self.config_options.append(PinConfig(data))


class UserDataFronthaulV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol User Data Fronthaul Data Item Definition"""

    type = 0x010C
    definition = [DISerialNumberAttr('serial_number'),  # Recipient's serial number
                  DIVariableLengthBytesAttr('payload')]  # User defined data


class NavigationModePosition(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Navigation Mode Position Data Item Definition"""

    type = 0x0120
    definition = [DISerialNumberAttr('serial_number'),  # Serial number of the device
                  DIUInt32Attr('position_x'),  # 4B - x-coordinate position of the device
                  DIUInt32Attr('position_y'),  # 4B - y-coordinate position of the device
                  DIUInt32Attr('position_z')]  # 4B - z-coordinate position of the device


class DatabaseChunk(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Database Chunk Data Item Definition"""

    type = 0x0121
    definition = [DIUInt16Attr('number_of_chunks'),  # 2B - chunks needed to transmit complete db
                  DIUInt16Attr('chunk_id'),  # 2B - ID of this chunk
                  DIFixedLengthStrAttr('server_name', 256),  # name of database file
                  DIVariableLengthBytesAttr('database_chunk')]


class RFSchedule(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol RF Schedule Data Item Definition"""

    type = 0x0122
    definition = [DISerialNumberAttr('serial_number'),  # Transmitter's serial number
                  DIUInt32Attr('packet_type'),  # 4B - type of transmitted packet
                  DIUInt32Attr('start_time'),  # 4B - start time of transmission
                  DIUInt32Attr('repeat_rate')]  # 4B - amount of time between each transmission


# Private Data Items

class TickV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Tick Data Item Definition"""

    type = 0x8000
    definition = [DIUInt8Attr('sequence'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('source_id'),
                  DIUInt32Attr('gnt'),
                  DIUInt8Attr('gnt_quality'),
                  DIUInt32Attr('announce_offset'),
                  DIUInt8Attr('announce_prob'),
                  DIUInt8Attr('cmd_countdown'),
                  DIUInt8Attr('cmd_count'),
                  DIUInt32Attr('sync_period'),
                  DIUInt64Attr('tx_timestamp')]


class TimedRxV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Timed Reception Data Item Definition"""

    type = 0x8001
    definition = [DIUInt8Attr('sequence'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('source_id'),
                  DIUInt8Attr('rx_packet_type'),
                  DIUInt64Attr('tx_timestamp'),
                  DIUInt64Attr('rx_timestamp'),
                  DISignalStrengthAttr('signal_strength')]


class PingV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ping Data Item Definition"""

    type = 0x8002
    definition = [DIUInt8Attr('sequence'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('source_id'),
                  DIUInt8Attr('flags'),
                  DIUInt64Attr('rx_timestamp'),
                  DISignalStrengthAttr('signal_strength'),
                  DIVariableLengthBytesAttr('payload')]


class DiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Discovery Data Item Definition"""

    type = 0x8003
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt64Attr('rx_timestamp'),
                  DISignalStrengthAttr('signal_strength')]


class SSPV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol SSP  Data Item Definition"""

    type = 0x8004
    definition = [DIUInt8Attr('response_type')]


class SmoothingFactorV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Smoothing Factor Data Item Definition"""

    type = 0x8005
    definition = [DIUInt16Attr('src_address'),
                  DIUInt16Attr('dest_address'),
                  DIUInt32Attr('smoothing_factor')]


class ConfigurationDumpV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Configuration Dump Data Item Definition"""

    type = 0x8006
    definition = [DIVariableLengthBytesAttr('binary_config_data')]


class PositionStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Position Stats Data Item Definition"""

    type = 0x8007
    definition = [DIInt32Attr('x0'),
                  DIInt32Attr('y0'),
                  DIInt32Attr('z0'),
                  DIUInt16Attr('iterations')]


class AnchorResiduals(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Anchor Residuals Data Item Definition"""

    type = 0x8008
    definition = [DISerialNumberAttr('serial_number'),  # 4B - serial number of the anchor
                  DIUInt64Attr('rx_timestamp'),  # 8B - time beacon was received at the anchor
                  DIUInt32Attr('weight'),  # 4B - weight of the node
                  DIInt32Attr('adjustment_magnitude')]  # 4B - offset in millimeters


class Image:
    """Image Class Definition"""

    definition = [DIUInt8Attr('type'),  # 1B - type of the image
                  DIFixedLengthStrAttr('version', 32), # 32B - version string of the image
                  DIFixedLengthBytesAttr('sha1', 20)]   # 20B - IVSHA1 of the image

    def __init__(self, type=0, version=0, sha1=0):
        self.type = type
        self.version = version
        self.sha1 = sha1

    def __str__(self):
        return "{}, {}, {}".format(self.type, self.version, self.sha1.hex())


class ImageDiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Image Discovery Data Item Definition"""

    type = 0x8009
    definition = [DIFixedLengthStrAttr('manufacturer', 64),  # 64B - string of the manufacturer
                  DIFixedLengthStrAttr('product', 32),  # 32B - string of the product
                  DIUInt8Attr('running_image_type'),  # 1B - type of the current running image
                  DIListAttr('image_information', Image)]

    def add_image(self, type=0, version=0, sha1=0):
        self.image_information.append(Image(type, version, sha1))


class ImageNotificationV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Image Notification Data Item Definition"""

    type = 0x800A
    definition = [DIUInt8Attr('image_type'),  # 1B - type of the image
                  DIFixedLengthStrAttr('image_version', 32),  # 32B - version string of the image
                  DIFixedLengthBytesAttr('image_sha1', 20),  # 20B - IVSHA1 of the image
                  DIUInt32Attr('image_size'),  # 4B - bytes in the image
                  DIUInt16Attr('sector_size'),  # 2B - bytes in each sector that will be transmitted
                  DISerialNumberAttr('image_serial'),  # 4B - serial to which image is mapped
                  DIFixedLengthBytesAttr('hardware_match', 20)]  #20B - manufacturer IVSHA1


class ImageRequestV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Image Request Data Item Definition"""

    type = 0x800B
    definition = [DISerialNumberAttr('image_serial'),  # 4B - serial to which image is mapped
                  DIUInt32Attr('sha1_end'),  # 4B - last 4 bytes of the IVSHA1 image
                  DIUInt16Attr('sector_number')]  # 2B - requested sector


class ImageSectorV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Image Sector Data Item Definition"""

    type = 0x800C
    definition = [DISerialNumberAttr('image_serial'),  # 4B - serial to which image is mapped
                  DIUInt32Attr('sha1_end'),  # 4B - last 4 bytes of the IVSHA1 image
                  DIUInt16Attr('sector_number'),  # 2B - requested sector
                  DIVariableLengthBytesAttr('sector')]  # sector to be loaded to the device


class IPDiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol IP Discovery Data Item Definition"""

    type = 0x800D
    definition = [DIUInt32Attr('transaction_id'),  # 4B - random number
                  DIFixedLengthBytesAttr('mac_address', 6),  # 6B - MAC Address of the CUWB Device
                  DIUInt32Attr('ip_address'),  # 4B - IP address of the device
                  DIUInt32Attr('subnet_mask'),  # 4B - subnet mask of the device
                  DIUInt32Attr('default_gateway'),  # 4B - Default gateway of the LAN
                  DIUInt16Attr('vlan_id'),  # 2B - VLAN ID
                  DIUInt8Attr('config_type'),  # 1B - configuration type
                  DIUInt32Attr('listen_ip')]  # 4B - IP address to listen on for IP Assignment


class IPAssignmentV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol IP Assignment Data Item Definition"""

    type = 0x800E
    definition = [DIUInt32Attr('transaction_id'),  # 4B - matching Transaction ID from IP Discovery
                  DIFixedLengthBytesAttr('mac_address', 6),  # 6B - MAC Address of the CUWB Device
                  DIUInt32Attr('ip_address'),  # 4B - IP address to assign the CUWB device
                  DIUInt32Attr('subnet_mask'),  # 4B - subnet mask to assign the CUWB device
                  DIUInt32Attr('default_gateway'),  # 4B - Default gateway to assign the CUWB device
                  DIUInt16Attr('vlan_id'),  # 2B - VLAN ID to assign the CUWB device
                  DIUInt8Attr('config_type')]  # 1B - configuration type


class WiredDiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Wired Descovery Data Item Definition"""

    type = 0x800F
    definition = [DIUInt8Attr('hardware_version'),  # 3b - unused/5b - hardware version number
                  DIUInt32Attr('server_instance'),
                  DIUInt64Attr('capabilities'),
                  DIVariableLengthBytesAttr('payload')]

    def get_hardware_version(self):
        """Returns the version sense lookup index"""
        return self.hardware_version & 0x1F


class TimedRxV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Timed Reception Data Item Definition"""

    type = 0x8010
    definition = [DIUInt64Attr('tx_dt40'),
                  DIUInt64Attr('tx_nt64'),
                  DIUInt64Attr('rx_dt40'),
                  DIUInt64Attr('rx_nt64'),
                  DISerialNumberAttr('source_serial_number'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('tx_nt_quality'),
                  DIUInt8Attr('rx_nt_quality'),
                  DIUInt8Attr('rx_packet_type')]


class TimedRxV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Timed Reception Data Item Definition"""

    type = 0x8011
    definition = [DIUInt64Attr('tx_dt40'),
                  DIUInt64Attr('tx_nt64'),
                  DIUInt64Attr('rx_dt40'),
                  DIUInt64Attr('rx_nt64'),
                  DISerialNumberAttr('source_serial_number'),
                  DICondensedSignalStrengthAttr('condensed_signal_strength'),
                  DIUInt8Attr('tx_nt_quality'),
                  DIUInt8Attr('rx_nt_quality'),
                  DIUInt8Attr('rx_packet_type')]


# TODO: This data item consists of a list of commands, not one
class ExecuteDeviceCommandV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Execute Device Command Data Item Definition"""

    type = 0x8012
    definition = [DIUInt32Attr('destination_group'),  # 4B - group that should execute this command
                  DIUInt16Attr('command_type_length'),  # 6b - command type/10b - command length
                  DIVariableLengthBytesAttr('command_data')]  # Data specific to the current command

    def get_command_type(self):
        """Returns the type of the command"""
        return self.command_type_length >> 10

    def get_command_length(self):
        """Returns the length of the command"""
        return self.command_type_length & 0x03FF


class PingV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ping Data Item Definition"""

    type = 0x8013
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('sequence'),
                  DIUInt8Attr('beacon_type'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt64Attr('dt40'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIVariableLengthBytesAttr('payload')]


class PingV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ping Data Item Definition"""

    type = 0x8014
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('sequence'),
                  DIUInt8Attr('packet_type'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt64Attr('dt40'),
                  DIUInt64Attr('nt64'),
                  DICondensedSignalStrengthAttr('condensed_signal_strength'),
                  DIVariableLengthBytesAttr('payload')]


class TickV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Tick Data Item Definition"""

    type = 0x8015
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('dt40'),
                  DIUInt8Attr('nt_quality')]


class PortInformation:
    """Port Information Class Definition"""

    definition = [DIUInt16Attr('voltage'),  # 2B - port's voltage (mV)
                  DIUInt16Attr('current')]  # 2B - port's current (mA)

    def __init__(self, voltage=0, current=0):
        self.voltage = voltage
        self.current = current

    def __str__(self):
        return ", ".join(str(getattr(self, attr.name)) for attr in self.definition)


class PowerReport(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Power Report Data Item Definition"""

    type = 0x8016
    definition = [DIUInt8Attr('port_mask'),  # 1B - mask specifying number of available power ports
                  DIUInt8Attr('output_enable_mask'),  #1B - ports currently outputting power mask
                  DIListAttr('port_information', PortInformation)]

    def add_port_information(self, voltage=0, current=0):
        self.port_information.append(PortInformation(voltage, current))


class SyncStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Sync Stats Data Item Definition"""

    type = 0x8017
    definition = [DIUInt64Attr('tx_dt64'),
                  DIDoubleAttr('error'),
                  DIDoubleAttr('clock_adjustment')]


class DomainInformation():
    """Domain Information: Domain Information Class Definition """

    def __init__(self):
        self.nt64 = 0  # Network time used in last update
        self.domaint = 0  # Domain time used in last update
        self.quality = 0  # Error between network and domain time
        self.slope = 0.0  # Slope used to covert domain time into network time
        self.intercept = 0  # Network time when the domain time was 0

    def __str__(self):
        return "{}, {}, {}, {}, {}".format(self.nt64,
                                           self.domaint,
                                           self.quality,
                                           self.slope,
                                           self.intercept)


class DIDomainInfoAttr(DataItemAttribute):
    """Data Item Attribute: CDP Data Item Domain Information Attribute Class Definition"""

    def __init__(self, name):
        super().__init__(name, 'QQqfq', 36, DomainInformation())

    def _decode(self, data):
        domain_info = DomainInformation()
        domain_info.nt64, \
            domain_info.domaint, \
            domain_info.quality, \
            domain_info.slope, \
            domain_info.intercept = struct.unpack("<" + self.format, data[:self.size])
        return (domain_info, self.size)

    def _encode(self, domain_info):
        return struct.pack("<" + self.format,
                           domain_info.nt64,
                           domain_info.domaint,
                           domain_info.quality,
                           domain_info.slope,
                           domain_info.intercept)


class ClockDomain(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock Domain Data Item Definition"""

    type = 0x8018
    definition = [DIDomainInfoAttr('proc_domain'),
                  DIDomainInfoAttr('deca_domain')]


class DecawaveStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Decawave Stats Data Item Definition"""

    type = 0x8019
    definition = [DIUInt32Attr('phr_error'),
                  DIUInt32Attr('rsd_error'),
                  DIUInt32Attr('fcs_good'),
                  DIUInt32Attr('fcs_error'),
                  DIUInt32Attr('rx_filter_rejection'),
                  DIUInt32Attr('rx_overrun_error'),
                  DIUInt32Attr('sfd_timeout'),
                  DIUInt32Attr('preamble_timeout'),
                  DIUInt32Attr('rx_wait_timeout'),
                  DIUInt32Attr('tx_sent'),
                  DIUInt32Attr('half_period_warning'),
                  DIUInt32Attr('tx_power_up_warning')]


class UWBDiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Discovery Data Item Definition"""

    type = 0x801A
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt32Attr('server_instance'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt8Attr('hardware_version'),  # 3b-unused/5b-hardware version
                  DIVariableLengthBytesAttr('payload')]

    def get_hardware_version(self):
        """Returns the version sense lookup index"""
        return self.hardware_version & 0x1F


class NetworkTraffic(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Network Traffic Data Item Definition"""

    type = 0x801B
    definition = [DIUInt32Attr('server_ip'),  # 4B - destination IP address
                  DIUInt16Attr('server_port')]  # 2B - destination UDP port


class NetworkLockV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Network Lock Data Item Definition"""

    type = 0x801C
    definition = [DIUInt64Attr('dt40'),  # 8B - last mapped Decawave Time
                  DIUInt64Attr('nt64'),  # 8B - last mapped Network Time
                  DIFloatAttr('adjustment')]


class ClockNTDTV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock NT and DT Data Item Definition"""

    type = 0x801D
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('dt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIDoubleAttr('ntdt_slope'),
                  DIDoubleAttr('dtnt_slope'),
                  DIInt64Attr('ntdt_error'),
                  DIInt64Attr('dtnt_error')]


class ClockPTDT(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock PT and DT Data Item Definition"""

    type = 0x801E
    definition = [DIUInt64Attr('pt64'),
                  DIUInt64Attr('dt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIDoubleAttr('ptdt_slope'),
                  DIDoubleAttr('dtpt_slope'),
                  DIInt64Attr('ptdt_error'),
                  DIInt64Attr('dtpt_error')]


class ClockNTPTV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock NT and PT Data Item Definition"""

    type = 0x801F
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('pt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIDoubleAttr('ntpt_slope'),
                  DIDoubleAttr('ptnt_slope'),
                  DIInt64Attr('ntpt_error'),
                  DIInt64Attr('ptnt_error')]


class UWBNetworkStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Network Stats Data Item Definition"""

    type = 0x8020
    definition = [DIUInt32Attr('irq_handler'),
                  DIUInt32Attr('irq_spurious_event'),
                  DIUInt32Attr('irq_read_buffer_error'),
                  DIUInt32Attr('tx_total'),
                  DIUInt32Attr('tx_time_error'),
                  DIUInt32Attr('tx_quality_error'),
                  DIUInt32Attr('tx_callback'),
                  DIUInt32Attr('tx_task_busy_error'),
                  DIUInt32Attr('tx_task_launch_error'),
                  DIUInt32Attr('tx_task'),
                  DIUInt32Attr('tx_app_callback'),
                  DIUInt32Attr('rx_callback'),
                  DIUInt32Attr('rx_task_busy_error'),
                  DIUInt32Attr('rx_task_launch_error'),
                  DIUInt32Attr('rx_task'),
                  DIUInt32Attr('rx_app_callback')]


class SystemOwnershipRequest(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol System Ownership Request Data Item Definition"""

    type = 0x8021
    definition = [DIUInt8Attr('version')]  # 1B - version of the request


class UserDataConfiguration(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol User Data Configuration Data Item Definition"""

    type = 0x8022
    definition = [DIUInt8Attr('size'),
                  DIUInt8Attr('headroom'),
                  DIUInt8Attr('frequency')]


class SystemOwnershipReply(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol System Ownership Reply Data Item Definition"""

    type = 0x8023
    definition = [DIUInt8Attr('rf_channel'),
                  DIUInt8Attr('rf_prf'),
                  DIUInt8Attr('preamble_code'),
                  DIUInt16Attr('tag_count'),
                  DISerialNumberListAttr('serial_numbers')]

    def add_serial_number(self, serial_number):
        self.serial_numbers.append(CiholasSerialNumber(serial_number))


class InactiveSerialNumbers(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Inactive Serial Numbers Data Item Definition"""

    type = 0x8024
    definition = [DISerialNumberListAttr('serial_numbers')]

    def add_serial_number(self, serial_number):
        self.serial_numbers.append(CiholasSerialNumber(serial_number))


class TimedRxV4(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Timed Reception Data Item Definition"""

    type = 0x8025
    definition = [DIUInt64Attr('tx_dt40'),
                  DIUInt64Attr('tx_nt64'),
                  DIUInt64Attr('rx_dt40'),
                  DIUInt64Attr('rx_nt64'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt8Attr('source_interface_id'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('tx_nt_quality'),
                  DIUInt8Attr('rx_nt_quality'),
                  DIUInt8Attr('rx_packet_type')]


class PingV4(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ping Data Item Definition"""

    type = 0x8026
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('sequence'),
                  DIUInt8Attr('beacon_type'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt64Attr('dt40'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIVariableLengthBytesAttr('payload')]


class UWBDiscoveryV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Discovery Data Item Definition"""

    type = 0x8027
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt8Attr('hardware_version'),  # 2b-unused/1b-OTA upgrade/5b-hardware version
                  DIUInt32Attr('recovery_sha1'),
                  DIUInt32Attr('bootloader_sha1'),
                  DIUInt32Attr('firmware_sha1'),
                  DIVariableLengthBytesAttr('payload')]

    def get_ota_upgrade(self):
        """Indicates if the device supports over the air upgrading"""
        return (self.hardware_version & 0b00100000) != 0

    def get_hardware_version(self):
        """Returns the version sense lookup index"""
        return self.hardware_version & 0x1F


class UWBDiscoveryV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Discovery Data Item Definition"""

    type = 0x8028
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt8Attr('hardware_version'),  # 2b-unused/1b-OTA upgrade/5b-hardware version
                  DIUInt32Attr('recovery_sha1'),
                  DIUInt32Attr('bootloader_sha1'),
                  DIUInt32Attr('firmware_sha1'),
                  DIVariableLengthBytesAttr('payload')]

    def get_ota_upgrade(self):
        """Indicates if the device supports over the air upgrading"""
        return (self.hardware_version & 0b00100000) != 0

    def get_hardware_version(self):
        """Returns the version sense lookup index"""
        return self.hardware_version & 0x1F


class UWBNetworkCommand:
    """UWB Network Command Class Definition"""

    def __init__(self, destination_group=0, type=0, length=0, data=0):
        self.destination_group = CiholasSerialNumber(destination_group)
        self.type = type
        self.length = length
        self.data = data

    def __str__(self):
        return "{}, 0x{:02X}, {}, {}".format(self.destination_group,
                                             self.type,
                                             self.length,
                                             self.data.hex())


class ExecuteDeviceCommandV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Execute Device Command Data Item Definition"""

    type = 0x8029
    definition = [DIListAttr('commands', UWBNetworkCommand)] # List of UWB Network Commands

    def _decode(self):
        self.commands = []
        while self.di_data:
            grp, typ, lng = struct.unpack("<IBH", self.di_data[:7])
            cmd_data = self.di_data[7:7+lng]
            self.commands.append(UWBNetworkCommand(grp, typ, lng, cmd_data))
            self.di_data = self.di_data[7+lng:]
        self.di_data = None

    def _encode(self):
        data = b''
        for cmd in self.commands:
            data += struct.pack("<IBH{:d}s".format(cmd.length), cmd.destination_group.as_int,
                                cmd.type, cmd.length, cmd.data)
        self.di_size = len(data)
        return struct.pack("<HH", self.type, self.di_size) + data

    def add_uwb_network_command(self, destination_group=0, type=0, length=0, data=0):
        self.commands.append(UWBNetworkCommand(destination_group, type, length, data))


class TickV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Tick Data Item Definition"""

    type = 0x802A
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('dt40'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt8Attr('interface_id')]


class DBChanged(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Debug Transmission Data Item Definition"""

    type = 0x802B
    definition = []  # This data item has no content


class TimedRxV5(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Timed Reception Data Item Definition"""

    type = 0x802C
    definition = [DIUInt64Attr('tx_nt64'),
                  DIUInt64Attr('rx_dt64'),
                  DIUInt64Attr('rx_nt64'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt8Attr('source_interface_id'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('tx_nt_quality'),
                  DIUInt8Attr('rx_nt_quality'),
                  DIUInt8Attr('rx_packet_type')]


class TickV4(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Tick Data Item Definition"""

    type = 0x802D
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('dt64'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt8Attr('interface_id')]


class NetworkLockV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Network Lock Data Item Definition"""

    type = 0x802E
    definition = [DIUInt64Attr('dt64'),  # 8B - last mapped Decawave Time
                  DIUInt64Attr('nt64'),  # 8B - last mapped Network Time
                  DIFloatAttr('adjustment'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('nt_quality')]


class PingV5(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ping Data Item Definition"""

    type = 0x802F
    definition = [DISerialNumberAttr('source_serial_number'),
                  DIUInt16Attr('sequence'),
                  DIUInt8Attr('beacon_type'),
                  DIUInt8Attr('nt_quality'),
                  DIUInt64Attr('dt64'),
                  DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIVariableLengthBytesAttr('payload')]


class SendDeviceCommand(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Send Device Command Data Item Definition"""

    type = 0x8030
    definition = [DISerialNumberAttr('serial_number'),
                  DIVariableLengthBytesAttr('debug_data')]


class AnchorResidualsV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Anchor Residuals Data Item Definition"""

    type = 0x8031
    definition = [DISerialNumberAttr('serial_number'),  # 4B - serial number of the anchor
                  DIUInt8Attr('interface_id'),  # 1B - interface identifier of the anchor
                  DIUInt64Attr('rx_timestamp'),  # 8B - time beacon was received at the anchor
                  DIUInt32Attr('weight'),  # 4B - weight of the node
                  DIInt32Attr('adjustment_magnitude')]  # 4B - offset in millimeters


class ClockNTDTV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock NT and DT Data Item Definition"""

    type = 0x8032
    definition = [DIUInt8Attr('interface_id'),
                  DIUInt64Attr('nt64'),
                  DIUInt64Attr('dt64'),
                  DIDoubleAttr('ntdt_slope'),
                  DIDoubleAttr('dtnt_slope'),
                  DIInt64Attr('ntdt_error'),
                  DIInt64Attr('dtnt_error')]


class ClockNTPTV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock NT and PT Data Item Definition"""

    type = 0x8033
    definition = [DIUInt64Attr('nt64'),
                  DIUInt64Attr('pt64'),
                  DIDoubleAttr('ntpt_slope'),
                  DIDoubleAttr('ptnt_slope'),
                  DIInt64Attr('ntpt_error'),
                  DIInt64Attr('ptnt_error')]


class ClockSeeder(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock Seeder Data Item Definition"""

    type = 0x8034
    definition = [DIUInt64Attr('nt64'),  # 8B - NT used during last clock seeder update
                  DIUInt8Attr('interface_id'),  # 1B - interface associates with the infomation
                  DISerialNumberAttr('serial_number')]  # 4B - serial number of the seeding anchor


class ClockDT(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Clock DT Data Item Definition"""

    type = 0x8035
    definition = [DIUInt8Attr('interface_id'),
                  DIUInt64Attr('dt64_adjusted'),
                  DIUInt64Attr('dt64_raw'),
                  DISignalStrengthAttr('signal_strength')]


class ContentDiscoveryV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Content Discovery Data Item Definition"""

    type = 0x8036
    definition = [DIFixedLengthBytesAttr('sha256', 32)]  # 32B - SHA256 of the content


class ContentNotificationV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Content Notification Data Item Definition"""

    type = 0x8037
    definition = [DIFixedLengthBytesAttr('sha256', 32),  # 32B - SHA256 of the content
                  DIUInt64Attr('content_size'),  # 8B - bytes in the content
                  DIUInt16Attr('sector_size')]  # 2B - bytes in every sector except the last


class ContentRequestV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Content Request Data Item Definition"""

    type = 0x8038
    definition = [DIUInt64Attr('sha256_end'),
                  DIUInt32Attr('sector_number')]


class ContentSectorV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Content Sector Data Item Definition"""

    type = 0x8039
    definition = [DIUInt64Attr('sha256_end'),
                  DIUInt32Attr('sector_number'),
                  DIVariableLengthBytesAttr('sector')]


class Ack(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ack Data Item Definition"""

    type = 0x803A
    definition = [DISerialNumberAttr('source_serial_number')]


class NetworkLockV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Network Lock Data Item Definition"""

    type = 0x803B
    definition = [DIUInt64Attr('intercept'),
                  DIFloatAttr('adjustment'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('nt_quality')]


class BridgeCommand(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Bridge Command Data Item Definition"""

    type = 0x803C
    definition = [DIUInt8Attr('type'),
                  DIUInt16Attr('size'),
                  DIVariableLengthBytesAttr('data')]


class LEDStates:
    """LED States Class Definition """

    definition = [DIUInt16Attr('start_time_offset'),
                  DIUInt16Attr('led_duration'),
                  DIUInt16Attr('led_period'),
                  DIUInt8Attr('red'),
                  DIUInt8Attr('green'),
                  DIUInt8Attr('blue')]

    def __init__(self, start_time_offset=0, led_duration=0, led_period=0, red=0, green=0, blue=0):
        self.start_time_offset = start_time_offset
        self.led_duration = led_duration
        self.led_period = led_period
        self.red = red
        self.green = green
        self.blue = blue

    def __str__(self):
        return ", ".join(str(getattr(self, attr.name)) for attr in self.definition)


class SetDiagnosticLED(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Set Diagnostic LED Data Item Definition"""

    type = 0x803D
    definition = [DIUInt32Attr('destination_group'),
                  DIListAttr('led_states', LEDStates)]

    def add_led_states(self, start_time_offset=0, led_duration=0,
                       led_period=0, red=0, green=0, blue=0):
        self.led_states.append(LEDStates(start_time_offset, led_duration,
                                         led_period, red, green, blue))


class CDPBandwidthTestCommand(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol CDP Bandwidth Test Command Data Item Definition"""

    type = 0x803E
    definition = [DIUInt16Attr('packet_length'),
                  DIUInt16Attr('packet_interval'),
                  DIUInt16Attr('test_duration')]


class CDPBandwidthTestPacket(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol CDP Bandwidth Test Packet Data Item Definition"""

    type = 0x803F
    definition = [DIUInt16Attr('sequence'),
                  DIVariableLengthBytesAttr('data')]


class UWBDiscoveryV4(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Discovery Data Item Definition"""

    type = 0x8040
    definition = [DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('nt_quality'),
                  DIVariableLengthBytesAttr('announce_packet')]


class DevicesReady(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Devices Ready Data Item Definition"""

    type = 0x8041
    definition = [DIUInt32Attr('server_instance')]  # 4B - instance ID of the server


class DeviceData(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Device Data Data Item Definition"""

    type = 0x8042
    definition = [DIUInt64Attr('nt64'),
                  DISignalStrengthAttr('signal_strength'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('nt_quality'),
                  DIVariableLengthBytesAttr('device_data')]


class EthernetTimedRx(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ethernet Timed Reception Data Item Definition"""

    type = 0x8043
    definition = [DIUInt16Attr('sequence'),
                  DIUInt64Attr('rx_dt64'),
                  DIUInt64Attr('rx_nt64'),
                  DIUInt64Attr('rx_pt64'),
                  DISerialNumberAttr('source_serial_number'),
                  DIUInt8Attr('interface_id'),
                  DIUInt8Attr('tx_nt_quality'),
                  DIUInt8Attr('rx_nt_quality')]


class EthernetTick(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Ethernet Tick Data Item Definition"""

    type = 0x8044
    definition = [DIUInt64Attr('tx_nt64'),
                  DIUInt64Attr('tx_pt64'),
                  DIUInt8Attr('nt_quality')]


class ServerReady(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Server Ready Data Item Definition"""

    type = 0x8045
    definition = [DIUInt32Attr('server_instance')]  # 4B - instance ID of the server


class SyncStatsV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Sync Stats Data Item Definition"""

    type = 0x8046
    definition = [DIUInt64Attr('tx_dt64'),
                  DIDoubleAttr('tx_avg_error'),
                  DIDoubleAttr('tx_std_dev_error'),
                  DIDoubleAttr('tx_error_count'),
                  DIDoubleAttr('rx_avg_error'),
                  DIDoubleAttr('rx_std_dev_error'),
                  DIDoubleAttr('rx_error_count'),
                  DIUInt64Attr('tx_intercept_nt'),
                  DIUInt64Attr('tx_intercept_dt'),
                  DIUInt64Attr('rx_intercept_nt'),
                  DIUInt64Attr('rx_intercept_dt'),
                  DIDoubleAttr('clock_adjustment'),
                  DIUInt16Attr('tx_locked_count'),
                  DIUInt16Attr('tx_locking_count'),
                  DIUInt16Attr('rx_locked_count'),
                  DIUInt16Attr('rx_locking_count')]


class ContentSectorV2(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Content Sector Data Item Definition"""

    type = 0x8047
    definition = [DIFixedLengthBytesAttr('sha256', 32),
                  DIFixedLengthStrAttr('version_string', 32),
                  DIUInt64Attr('content_size'),
                  DIUInt32Attr('sector_size'),
                  DIUInt32Attr('sector_count'),
                  DIUInt32Attr('sector_index'),
                  DIVariableLengthBytesAttr('data')]


class ProcessorStats(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Processor Stats Data Item Definition"""

    type = 0x8048
    definition = [DIUInt32Attr('processor_uptime'),
                  DIUInt32Attr('memory_available'),
                  DIUInt8Attr('reset_type'),
                  DIVariableLengthBytesAttr('abort_info')]


class UWBStackStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Stats Data Item Definition"""

    type = 0x8049
    definition = [DIUInt32Attr('sync_attempts'),  # 4B - attempts to sync multiple NIs
                  DIUInt32Attr('sync_failures')]  # 4B - failures while trying to sync multiple NIs


class UWBStackNetworkInterfaceStatsV1(CDPDataItem):
    """
    CDP Data Item: Ciholas Data Protocol
    UWB Stack Network Interface Stats Data Item Definition
    """

    type = 0x804A
    definition = [DIUInt32Attr('identifier'),
                  DIUInt32Attr('tx_event_callback'),
                  DIUInt32Attr('rx_event_callback'),
                  DIUInt32Attr('schedule_tx_success'),
                  DIUInt32Attr('schedule_tx_failure'),
                  DIUInt32Attr('schedule_rx_success'),
                  DIUInt32Attr('schedule_rx_failure'),
                  DIUInt32Attr('schedule_config_success'),
                  DIUInt32Attr('schedule_config_failure')]


class UWBStackNetworkLayerStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Network Layer Stats Data Item Definition"""

    type = 0x804B
    definition = [DIUInt32Attr('network_create'),
                  DIUInt32Attr('network_add_itf_success'),
                  DIUInt32Attr('network_add_itf_failure'),
                  DIUInt32Attr('network_repeated_event_missed'),
                  DIUInt32Attr('network_one_time_event_missed'),
                  DIUInt32Attr('schedule_task_success'),
                  DIUInt32Attr('schedule_task_failure'),
                  DIUInt32Attr('remove_event'),
                  DIUInt32Attr('schedule_event_success'),
                  DIUInt32Attr('schedule_event_failure'),
                  DIUInt32Attr('destroy_event')]


class UWBStackLinkManagerStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Link Manager Stats Data Item Definition"""

    type = 0x804C
    definition = [DIUInt32Attr('identifier'),
                  DIUInt32Attr('add_transmit_immediate'),
                  DIUInt32Attr('add_transmit_in_future'),
                  DIUInt32Attr('add_transmit_in_past'),
                  DIUInt32Attr('add_transmit_success'),
                  DIUInt32Attr('add_transmit_failure'),
                  DIUInt32Attr('add_receive_immediate'),
                  DIUInt32Attr('add_receive_in_future'),
                  DIUInt32Attr('add_receive_already_started'),
                  DIUInt32Attr('add_receive_in_past'),
                  DIUInt32Attr('add_receive_success'),
                  DIUInt32Attr('add_receive_failure'),
                  DIUInt32Attr('add_config_immediate'),
                  DIUInt32Attr('add_config_in_future'),
                  DIUInt32Attr('add_config_success'),
                  DIUInt32Attr('add_config_failure'),
                  DIUInt32Attr('remove_event_success'),
                  DIUInt32Attr('remove_event_failure'),
                  DIUInt32Attr('reset'),
                  DIUInt32Attr('task_config'),
                  DIUInt32Attr('task_stop_receive'),
                  DIUInt32Attr('task_start_receive'),
                  DIUInt32Attr('task_transmit'),
                  DIUInt32Attr('task_wake'),
                  DIUInt32Attr('handler_wake_success'),
                  DIUInt32Attr('handler_tx_failure'),
                  DIUInt32Attr('handler_tx_success'),
                  DIUInt32Attr('handler_tx_start'),
                  DIUInt32Attr('handler_tx_capture'),
                  DIUInt32Attr('handler_rx_failure'),
                  DIUInt32Attr('handler_rx_success'),
                  DIUInt32Attr('handler_rx_start'),
                  DIUInt32Attr('lde_done'),
                  DIUInt32Attr('tx_callback'),
                  DIUInt32Attr('rx_callback'),
                  DIUInt32Attr('rx_valid_data'),
                  DIUInt32Attr('task_irq_process'),
                  DIUInt32Attr('rx_timeout')]


class UWBStackLinkLayerStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Link Layer Stats Data Item Definition"""

    type = 0x804D
    definition = [DIUInt32Attr('uwb_event_manager_create')]


class UWBStackPhysicalLayerStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Physical Layer Stats Data Item Definition"""

    type = 0x804E
    definition = [DIUInt32Attr('uwb_device_create_success'),
                  DIUInt32Attr('uwb_device_create_error'),
                  DIUInt32Attr('uwb_device_destroy'),
                  DIUInt32Attr('uwb_device_state_create_error'),
                  DIUInt32Attr('uwb_device_config_create_error'),
                  DIUInt32Attr('dw1000_create_failure')]


class UWBStackDeviceStatsV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol UWB Stack Device Stats Data Item Definition"""

    type = 0x804F
    definition = [DIUInt32Attr('identifier'),
                  DIUInt64Attr('bytes_received'),
                  DIUInt64Attr('bytes_transmitted'),
                  DIUInt32Attr('configure'),
                  DIUInt32Attr('irq_handler'),
                  DIUInt32Attr('irq_handler_loop'),
                  DIUInt32Attr('cplock'),
                  DIUInt32Attr('irq_handled'),
                  DIUInt32Attr('txfrb'),
                  DIUInt32Attr('txprs'),
                  DIUInt32Attr('txphs'),
                  DIUInt32Attr('txfrs'),
                  DIUInt32Attr('txberr'),
                  DIUInt32Attr('hpdwarn'),
                  DIUInt32Attr('rxprd'),
                  DIUInt32Attr('rxsfdto'),
                  DIUInt32Attr('rxsfdd'),
                  DIUInt32Attr('rxphe'),
                  DIUInt32Attr('rxphd'),
                  DIUInt32Attr('rxrfsl'),
                  DIUInt32Attr('rxrfto'),
                  DIUInt32Attr('rxdfr'),
                  DIUInt32Attr('rxfcg'),
                  DIUInt32Attr('rxfce'),
                  DIUInt32Attr('lde_not_done'),
                  DIUInt32Attr('receiver_reset'),
                  DIUInt32Attr('dcs_passed'),
                  DIUInt32Attr('dcs_failed'),
                  DIUInt32Attr('go_to_idle'),
                  DIUInt32Attr('go_to_sleep'),
                  DIUInt32Attr('tx_now'),
                  DIUInt32Attr('tx_at'),
                  DIUInt32Attr('tx_error_sync_clock_reset'),
                  DIUInt32Attr('go_to_rx'),
                  DIUInt32Attr('tx_data_too_big')]


class ParticleUpdater(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Particle Updater Data Item Definition"""

    type = 0x8050
    definition = [DIUInt8Attr('flags')]  # 1b-content/1b-firmware/1b-almanac/5b-unused

    def get_content_flag(self):
        return (self.flags & 0b10000000) != 0

    def get_firmware_flag(self):
        return (self.flags & 0b01000000) != 0

    def get_almanac_flag(self):
        return (self.flags & 0b00100000) != 0


class DebugTraffic(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Debug Traffic Data Item Definition"""

    type = 0x8051
    definition = [DIUInt32Attr('server_ip'),  # 4B - destination IP address
                  DIUInt16Attr('server_port')]  # 2B - destination UDP port


#TODO - Definition is missing in documentation. Currently treated as an array of bytes.
class UWBSnifferPacket(CDPDataItem):
    type = 0x8052
    definition = [DIVariableLengthBytesAttr('data')]


class SyncStatsV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Sync Stats Data Item Definition"""

    type = 0x8053
    definition = [DIUInt64Attr('tx_dt64'),
                  DIDoubleAttr('tx_avg_error'),
                  DIDoubleAttr('tx_std_dev_error'),
                  DIDoubleAttr('tx_error_count'),
                  DIDoubleAttr('rx_avg_error'),
                  DIDoubleAttr('rx_std_dev_error'),
                  DIDoubleAttr('rx_error_count'),
                  DIUInt64Attr('tx_intercept_nt'),
                  DIUInt64Attr('tx_intercept_dt'),
                  DIUInt64Attr('rx_intercept_nt'),
                  DIUInt64Attr('rx_intercept_dt'),
                  DIDoubleAttr('clock_adjustment'),
                  DIUInt16Attr('tx_locked_count'),
                  DIUInt16Attr('tx_locking_count'),
                  DIUInt16Attr('rx_locked_count'),
                  DIUInt16Attr('rx_locking_count'),
                  DIUInt64Attr('tx_rogue_node_fail_count'),
                  DIUInt64Attr('tx_twr_threshold_fail_count'),
                  DIUInt64Attr('tx_invalid_range_count'),
                  DIUInt64Attr('rx_rogue_node_fail_count'),
                  DIUInt64Attr('rx_twr_threshold_fail_count'),
                  DIUInt64Attr('rx_invalid_range_count')]


class AnchorResidualsV3(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Anchor Residuals Data Item Definition"""

    type = 0x8055
    definition = [DISerialNumberAttr('serial_number'),
                  DIUInt8Attr('interface_id'),
                  DIUInt64Attr('rx_timestamp'),
                  DIUInt32Attr('weight'),
                  DIInt32Attr('adjustment_magnitude'),
                  DIUInt16Attr('sequence'),
                  DIInt16Attr('first_path'),
                  DIInt16Attr('total_path')]


class SystemOwnershipReleaseUnconfigured(CDPDataItem):
    """
    CDP Data Item: Ciholas Data Protocol
    System Ownership Release Unconfigured Data Item Definition
    """

    type = 0x8056
    definition = [DIUInt32Attr('server_instance'),
                  DISerialNumberListAttr('serial_numbers')]

    def add_serial_number(self, serial_number):
        self.serial_numbers.append(CiholasSerialNumber(serial_number))


class RFAgilityRead(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol RF Agility Read Data Item Definition"""

    type = 0x8058
    definition = [DISerialNumberAttr('serial_number')]  # SN of device to read RF settings from


class BucketSettings:
    """Bucket Settings Class Definition """

    definition = [DIUInt8Attr('channel'),
                  DIUInt8Attr('prf'),
                  DIUInt8Attr('preamble_code'),
                  DIUInt16Attr('preamble_length'),
                  DIUInt16Attr('bit_rate'),
                  DIUInt16Attr('power_mode'),
                  DIUInt8Attr('phr_power'),
                  DIUInt8Attr('data_power'),
                  DIUInt8Attr('on_time'),
                  DIUInt8Attr('initial_off_time'),
                  DIUInt8Attr('backoff'),
                  DIUInt8Attr('max_off_time')]

    def __init__(self, channel=0, rpf=0, preamble_code=0, preamble_length=0,
                 bit_rate=0, power_mode=0, phr_power=0, data_power=0,
                 on_time=0, initial_off_time=0, backoff=0, max_off_time=0):
        self.channel = channel
        self.rpf = rpf
        self.preamble_code = preamble_code
        self.preamble_length = preamble_length
        self.bit_rate = bit_rate
        self.power_mode = power_mode
        self.phr_power = phr_power
        self.data_power = data_power
        self.on_time = on_time
        self.initial_off_time = initial_off_time
        self.backoff = backoff
        self.max_off_time = max_off_time

    def __str__(self):
        return ", ".join(str(getattr(self, attr.name)) for attr in self.definition)


class RFAgilityWrite(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol RF Agility Write Data Item Definition"""

    type = 0x8059
    definition = [DISerialNumberAttr('serial_number_mask'),  # SN mask of recipient device(s)
                  DIListAttr('bucket_settings', BucketSettings)]  # List of all bucket settings

    def add_bucket_settings(self, channel=0, rpf=0, preamble_code=0, preamble_length=0,
                            bit_rate=0, power_mode=0, phr_power=0, data_power=0,
                            on_time=0, initial_off_time=0, backoff=0, max_off_time=0):
        self.bucket_settings.append(BucketSettings(channel, rpf, preamble_code, preamble_length,
                                                   bit_rate, power_mode, phr_power, data_power,
                                                   on_time, initial_off_time, backoff, max_off_time))


class CacheRequest(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Cache Request Data Item Definition"""

    type = 0x805A
    definition = [DIUInt32Attr('transaction_id'),  # 4B - random number to identify transacion
                  DIUInt16Attr('type')]  # 2B - CDP data item type


class AcknowledgeListResponse(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Acknowledge List Response Data Item Definition"""

    type = 0x805B
    definition = [DIUInt16Attr('num_acknowledged'),
                  DIUInt16Attr('num_not_acknowledged'),
                  DISerialNumberListAttr('acknowledged_serials'),
                  DISerialNumberListAttr('not_acknowledged_serials'),
                  DIFixedLengthStrAttr('ack_string', 32)]

    def _decode(self):
        for attr in self.definition:
            # The length of the data for 'acknowledged_serials' and 'not_acknoledged_serials'
            # depends on the value of 'num_acknoledged' and 'num_not_acknowledged'
            if attr.name == 'acknowledged_serials':
                value, size = attr._decode(self.di_data[:self.num_acknowledged*4])
            elif attr.name == 'not_acknowledged_serials':
                value, size = attr._decode(self.di_data[:self.num_not_acknowledged*4])
            else:
                value, size = attr._decode(self.di_data)
            self.di_data = self.di_data[size:]
            setattr(self, attr.name, value)
        self.di_data = None

    def _encode(self):
        self.num_acknowledged = len(self.acknowledged_serials)
        self.num_not_acknowledged = len(self.not_acknowledged_serials)
        return super()._encode()

    def add_acknowledged_serial(self, serial_number):
        self.acknowledged_serials.append(CiholasSerialNumber(serial_number))
        self.num_acknowledged += 1

    def add_not_acknowledged_serial(self, serial_number):
        self.not_acknowledged_serials.append(CiholasSerialNumber(serial_number))
        self.num_not_acknowledged += 1


class CacheReadResult(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Cache Read Result Data Item Definition"""

    type = 0x805C
    definition = [DIUInt32Attr('transaction_id'),  # 4B - random number to identify transacion
                  DIVariableLengthStrAttr('error_string')]  # Details in case of error


class NavigationModeDataBackhaulCommand(CDPDataItem):
    """
    CDP Data Item: Ciholas Data Protocol
    Navigation Mode Data Backhaul Command Data Item Definition
    """

    type = 0x805D
    definition = [DISerialNumberAttr('serial_number'),  # Recipient's serial number
                  DIUInt8Attr('enable')]  # 1B - disabled/enabled navigation mode data backhaul

class NetAppTimeModelStats(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol Net App Time Model Data Item Definition"""

    type = 0x805F
    definition = [DIInt64Attr('calculated_nt'), # NT Calculated by the netapp time model
                  DIInt64Attr('reported_nt'),   # NT Reported by the device
                  DIDoubleAttr('slope'),        # The slope being used in the netapp time model
                  DIUInt8Attr('nt_was_used')]   # A boolean indicating if this nt was actually used

class MultiRangeRx:
    """Multi Range Rx Class Definition """

    definition = [DISerialNumberAttr('rx_serial_number'),
                  DIUInt8Attr('rx_interface_identifier'),
                  DIUInt8Attr('sequence_number'),
                  DIUInt64Attr('dt_rx'),
                  DIUInt16Attr('quality'),
                  DIUInt8Attr('first_path'),
                  DIUInt8Attr('total_path')]

    def __init__(self, rx_serial_number=0, rx_interface_identifier=0, sequence_number=0, dt_rx=0, quality=0, first_path=0, total_path=0):
        self.rx_serial_number = CiholasSerialNumber(rx_serial_number)
        self.rx_interface_identifier = rx_interface_identifier
        self.sequence_number = sequence_number
        self.dt_rx = dt_rx
        self.quality = quality
        self.first_path = first_path
        self.total_path = total_path

    def __str__(self):
        return ", ".join(str(getattr(self, attr.name)) for attr in self.definition)

class MrTransmissionV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol MR Transmission V1 Data Item Definition"""

    type = 0x8060
    definition = [DISerialNumberAttr('serial_number'),    # The serial number of the device.
                  DIUInt8Attr('interface_identifier'),    # The interface identifier of the device.
                  DIUInt16Attr('antenna_delay'),          # The antenna delay of the device.
                  DIUInt8Attr('sequence_number'),         # The sequence number of this MR packet.
                  DIUInt64Attr('dt_tx'),                  # The dt time that this packet was transmitted at.
                  DIUInt8Attr('number_of_mr_packets'),    # The number of MR packets received from other MR devices.
                  DIListAttr('mr_rx_list', MultiRangeRx), # A list of multi range rx data from other MR devices.
                  DIVariableLengthStrAttr("payload")]

    def _decode(self):
        for attr in self.definition:
            if attr.name == 'mr_rx_list':
                value, size = attr._decode(self.di_data[:self.number_of_mr_packets*18])
            else:
                value, size = attr._decode(self.di_data)
            self.di_data = self.di_data[size:]
            setattr(self, attr.name, value)
        self.di_data = None

    def _encode(self):
        self.number_of_mr_packets = len(self.mr_rx_list)
        return super()._encode()

class MrReceptionV1(CDPDataItem):
    """CDP Data Item: Ciholas Data Protocol MR Reception V1 Data Item Definition"""

    type = 0x8061
    definition = [DISignalStrengthAttr('signal_strength'),# The signal strength of the reception of this packet.
                  DIUInt64Attr('rx_nt'),                  # The network time the mr transmission was received.
                  DISerialNumberAttr('serial_number'),    # The serial number of the device.
                  DIUInt8Attr('interface_identifier'),    # The interface identifier of the device.
                  DIUInt16Attr('antenna_delay'),          # The antenna delay of the device.
                  DIUInt8Attr('sequence_number'),         # The sequence number of this MR packet.
                  DIUInt64Attr('dt_tx'),                  # The dt time that this packet was transmitted at.
                  DIUInt8Attr('number_of_mr_packets'),    # The number of MR packets received from other MR devices.
                  DIListAttr('mr_rx_list', MultiRangeRx), # A list of multi range rx data from other MR devices.
                  DIVariableLengthStrAttr('payload'),]

    def _decode(self):
        for attr in self.definition:
            if attr.name == 'mr_rx_list':
                value, size = attr._decode(self.di_data[:self.number_of_mr_packets*18])
            else:
                value, size = attr._decode(self.di_data)
            self.di_data = self.di_data[size:]
            setattr(self, attr.name, value)
        self.di_data = None

    def _encode(self):
        self.number_of_mr_packets = len(self.mr_rx_list)
        return super()._encode()

class UwbTestRx(CDPDataItem):
    type = 0x8070
    definition = [DIUInt16Attr('length'),
                  DIVariableLengthStrAttr('data'),]

#!/usr/bin/env python

import posix
import fcntl
import struct
import ctypes
import array
from time import sleep

class spi_ioc_transfer(ctypes.Structure):  
    _fields_ = [
        ("tx_buf", ctypes.c_uint64),
        ("rx_buf", ctypes.c_uint64),
        ("len", ctypes.c_uint32),
        ("speed_hz", ctypes.c_uint32),
        ("delay_usecs", ctypes.c_uint16),
        ("bits_per_word", ctypes.c_uint8),
        ("cs_change", ctypes.c_uint8),
        ("pad", ctypes.c_uint32)]

def write_to_ioctl(writeData):
    data = array.array('B', writeData).tostring()
    length = len(data)
    transmit_buffer = ctypes.create_string_buffer(data)
    transmitAddress=ctypes.addressof(transmit_buffer)
    # create the spi transfer struct
    transfer = spi_ioc_transfer(
        tx_buf = transmitAddress,
        len = length,
        speed_hz = 500000,
        bits_per_word = 8
        );
    fcntl.ioctl(fd, 1075866368, transfer)

def calcOutput(val):
  
    spi_max_speed = 5 * 100000
    v_ref = 2200
    Bits = 2**16
    Resolution = (v_ref* 1000/ Bits)
    D_val16 = (val * 1000) / Resolution
    byte0 = 0x30 | (D_val16 & 0xF000) >> 12
    byte1 = (D_val16 & 0x0FF0) >> 4
    byte2 = (D_val16 & 0x000F) << 4
    return([byte0,byte1,byte2])

if __name__ == "__main__":
    DEBUG = True        
    SPIDEV = '/dev/spidev'
    spi_device = "%s%d.%d" % (SPIDEV, 0, 1)
    fd = posix.open(spi_device, posix.O_RDWR)
    
    write_to_ioctl([0x30, 0x0, 0x0])
    sleep(1)
    write_dac = calcOutput(2000)
    write_to_ioctl(write_dac)	
    print "done"
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import

import os
import sys
import posix
import spidev
import fcntl
import struct
import ctypes
import array
import os.path
from time import sleep


if __name__ == "__main__" and __package__ is None:

    parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.sys.path.insert(1, parent_dir)


    mod = __import__('dacPython')
    sys.modules["dacPython"] = mod
    # or just import it:
    # import my_package

    __package__='dacPython'

    from .submodule1.linux_spi_spidev  import spi_ioc_transfer, SPI_IOC_MESSAGE

    DEBUG = True
    spi_max_speed = 5 * 100000
    v_ref = 2186
    Bits = 8
    dac_bits = 16
    Resolution = 2**16
    val_mv = 2000
    
    CE = 0
    
    #spi = spidev.SpiDev()
    #spi.open(0, 2)
    #spi.max_speed_hz = spi_max_speed
	
    SPIDEV = '/dev/spidev'
    spi_device = "%s%d.%d" % (SPIDEV, 0, 1)
    print('spi_device:',spi_device);
    fd = posix.open(spi_device, posix.O_RDWR)
 
    print ("val : mv ", val_mv)
	
	#2000 
    writeData=[0x3e, 0xa3, 0x70];
	
	#
    #writeData=[0x30, 0x0, 0x0];
	
    data = array.array('B', writeData).tostring()
    length = len(data)
    transmit_buffer = ctypes.create_string_buffer(data)
    transmitAddress=ctypes.addressof(transmit_buffer)
	
	# create the spi transfer struct
    transfer = spi_ioc_transfer(
        tx_buf = transmitAddress,
        rx_buf = 0,
        cs_change = 0,
        len = length,
        delay_usecs = 0,
        speed_hz = 500000,
        bits_per_word = 8
        );

    fcntl.ioctl(fd, SPI_IOC_MESSAGE(1), transfer)
	
    print "done"
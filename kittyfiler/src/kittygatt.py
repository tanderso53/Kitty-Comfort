#! /usr/bin/python3

import pygatt

adapter = pygatt.GATTToolBackend()

def print_data(handle, value):
    """
    Print data to stdout
    handle -- characteristic read handle
    value -- bytearray from peripheral
    """
    print("Received data %d" % value)

try:
    adapter.start()
    device = adapter.connect('0c:32:43:93:84:sd')
    device.subscribe("19B10000-E8F2-537E-4C6C-D104768A1215",
                     callback=print_data)

finally:
    adapter.stop()

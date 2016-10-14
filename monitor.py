#!/usr/bin/env python
# File: monitor.py
# Date: 6 June 2016
# Author: David Randolph
# Purpose: Monitor MIDI and Dactylize Arduino serial output concurrently and
#          print their microsecond-timestamped messages to two separate files.
#          These files will be saved in the ./output/raw directory as
#          <epochtime>_finger <epochtime>_midi. A call to dactylizer.pl on
#          exit generates appropriately named .midi and .abcdf files in
#          the ./output/cooked directory.
#          
import mido
import serial
import thread
import signal
import sys
import os
import subprocess
from time import time
from time import sleep

mido.set_backend('mido.backends.rtmidi')

RAW_OUTPUT_DIR = './output/raw/'
DACTYLIZER_CMD = './dactylizer.pl'
SERIAL_PORT = '/dev/cu.usbmodem1461'
BAUD_RATE = 115200
start_time = time()
file_time = int(start_time)
print file_time
finger_file_path = RAW_OUTPUT_DIR + str(file_time) + '_finger'
midi_file_path = RAW_OUTPUT_DIR + str(file_time) + '_midi'

if not os.path.exists(RAW_OUTPUT_DIR):
    os.makedirs(RAW_OUTPUT_DIR)

finger_file = open(finger_file_path, 'w')
midi_file = open(midi_file_path, 'w')

def get_usec_timestamp():
    offset_usec = time() - start_time
    return offset_usec

def cleanup_and_exit(signal, frame):
    print("Closing monitor session. . . .")
    midi_file.close()
    finger_file.close()
    # Call dactylizer to generate .midi and .abcdf output.
    subprocess.call([DACTYLIZER_CMD, str(file_time)])
    sys.exit(0)

def twiddle_thumbs():
    count = 0
    while count < 5:
        sleep(5)
        count += 1
        print "%s: %s" % ("Twiddling", get_usec_timestamp())
    
def monitor_arduino():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    while True:
        msg = ser.readline().rstrip() + " When:" + str(get_usec_timestamp())
        print msg
        msg += "\n"
        finger_file.write(str(msg)) 

def monitor_midi():
    inny = mido.open_input()
    while True:
        msg = inny.receive()
        msg.time = get_usec_timestamp()
        msg = str(msg)
        print msg
        msg += "\n" 
        midi_file.write(str(msg))

signal.signal(signal.SIGINT, cleanup_and_exit)
# signal.pause()

try:
    print("Monitoring Arduino and MIDI USB input. . . .")
    thread.start_new_thread(monitor_arduino, ())
    thread.start_new_thread(monitor_midi, ())
except Exception, e:
    print("ERROR: Could not fork thread. " + str(e))

while True:
    pass

#!/usr/local/bin/python
# File: monitor.py
# Date: 6 June 2016
# Author: David Randolph
# Purpose: Monitor MIDI and Dactylize Arduino serial output concurrently and
#          print their microsecond-timestamped messages to two separate files.
#          These files will be saved in the ./output directory as
#          <epochtime>_finger <epochtime>_midi.
#          
import mido
import serial
import thread
import signal
import sys
import os
from time import time
from time import sleep

OUTPUT_DIR = './output/'
SERIAL_PORT = '/dev/cu.usbmodem1421'
BAUD_RATE = 115200
start_time = time()
file_time = int(start_time)
print file_time
finger_file_name = OUTPUT_DIR + str(file_time) + '_finger'
midi_file_name = OUTPUT_DIR + str(file_time) + '_midi'

if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

finger_file = open(finger_file_name, 'w')
midi_file = open(midi_file_name, 'w')

def get_usec_timestamp():
    offset_usec = time() - start_time
    return offset_usec

def cleanup_and_exit(signal, frame):
    print("Closing monitor session. . . .")
    midi_file.close()
    finger_file.close()
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
        msg = ser.readline().rstrip() + " When: " + str(get_usec_timestamp())
        msg += "\n"
        finger_file.write(str(msg)) 
        print msg

def monitor_midi():
    inny = mido.open_input()
    while True:
        msg = inny.receive()
        msg.time = get_usec_timestamp()
        midi_file.write(str(msg))
        print msg

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

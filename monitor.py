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
import serial.tools.list_ports
from multiprocessing import Process
import signal
import sys
import os
import re
import subprocess
from time import time
from time import sleep

serial_ports = serial.tools.list_ports.comports()
modem_pat = re.compile('^/dev/cu.usbmodem')
for port in serial_ports:
    if modem_pat.match(port.device):
        serial_port = port.device
        break

mido.set_backend('mido.backends.rtmidi')

RAW_OUTPUT_DIR = './output/raw/'
DACTYLIZER_CMD = './dactylizer.pl'
# SERIAL_PORT = '/dev/cu.usbmodem1411'
BAUD_RATE = 115200
start_time = time()
file_time = int(start_time)
print file_time
finger_file_path = RAW_OUTPUT_DIR + str(file_time) + '_finger'
midi_file_path = RAW_OUTPUT_DIR + str(file_time) + '_midi'
twiddle_file_path = RAW_OUTPUT_DIR + str(file_time) + '_twiddle'
pick_file_path = RAW_OUTPUT_DIR + str(file_time) + '_pick'

if not os.path.exists(RAW_OUTPUT_DIR):
    os.makedirs(RAW_OUTPUT_DIR)

def get_usec_timestamp():
    offset_usec = time() - start_time
    return offset_usec

def monitor_arduino():
    finger_file = open(finger_file_path, 'w')
    ser = serial.Serial(serial_port, BAUD_RATE)
    try:
        while True:
            msg = ser.readline().rstrip() + "W" + str(get_usec_timestamp())
            print(msg)
            msg += "\n"
            finger_file.write(str(msg)) 
    except KeyboardInterrupt:
        print("INTerrupted Arduino monitoring.\n");
    except:
        print("UNKNOWN ERROR in Arduino monitoring: {1}".format(sys.exc_info()[0]))
    finally:
        print("Cleaning up Arduino monitor.")
        finger_file.close()

def monitor_midi():
    midi_file = open(midi_file_path, 'w')
    inny = mido.open_input()
    try:
        while True:
            when = get_usec_timestamp()
            msg = inny.receive()
            msg = str(msg)
            msg += ' when={0}\n'.format(when)
            print msg,
            midi_file.write(str(msg))
    except KeyboardInterrupt:
        print("INTerrupted MIDI monitoring.\n");
    except:
        print("UNKNOWN ERROR in MIDI monitoring: {1}".format(sys.exc_info()[0]))
    finally:
        print("Cleaning up MIDI monitor.")
        midi_file.close()

try:
    print("Monitoring Arduino and MIDI USB input. . . .")
    arduino_proc = Process(target=monitor_arduino)
    midi_proc = Process(target=monitor_midi)
    arduino_proc.start()
    midi_proc.start()
    arduino_proc.join()
    midi_proc.join()     
except KeyboardInterrupt:
    midi_proc.terminate();
    midi_proc.join();
    arduino_proc.terminate();
    arduino_proc.join();
except Exception, e:
    print("ERROR: Could not fork process. " + str(e))

# Call dactylizer to generate .midi and .abcdf output.
# subprocess.call([DACTYLIZER_CMD, str(file_time)])
sys.exit(0)

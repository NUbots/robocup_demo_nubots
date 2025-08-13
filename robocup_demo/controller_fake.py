#!/usr/bin/env python3
import uinput
import time

# Define buttons & axes your F710 uses
device = uinput.Device([
    uinput.BTN_A,  # Button A
    uinput.BTN_B,  # Button B
    uinput.BTN_X,
    uinput.BTN_Y,
    uinput.ABS_X + (-32768, 32767, 0, 0),  # Left stick X
    uinput.ABS_Y + (-32768, 32767, 0, 0),  # Left stick Y
])

time.sleep(1)  # Wait for device to register

# Example: move left stick fully right
device.emit(uinput.ABS_X, 32767)  
time.sleep(0.5)
device.emit(uinput.ABS_X, 0)  # Center stick

# Example: press & release Button A
device.emit_click(uinput.BTN_A)

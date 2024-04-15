import serial
import uinput
import time
ser = serial.Serial('/dev/rfcomm0', 115200)

device = uinput.Device([
    uinput.BTN_PIN_SW,
    uinput.BTN_PIN_1,
    uinput.BTN_PIN_2,
    uinput.BTN_PIN_3,
    uinput.BTN_PIN_ENTER,
    uinput.BTN_PIN_UP,
    uinput.BTN_PIN_RIGHT,
    uinput.BTN_PIN_DOWN,
    uinput.BTN_PIN_LEFT,
    uinput.X_PIN,
    uinput.Y_PIN,
])

def parse_data(mouse_data):
    x = mouse_data[0]
    y = mouse_data[1]
    sw = mouse_data[2]
    print(f"x: {x},y: {y}, sw: {sw}")
    return x, y, sw

def move_mouse(x, y, sw):
    if x == 0:
        device.emit(uinput.KEY_X)
    elif y == 0:
        device.emit(uinput.KEY_Y)
    elif sw == 0:
        device.emit(uinput.KEY_SW)

try:
    # sync package
    while True:
        print('Waiting for sync package...')
        while True:
            mouse_data = ser.read(3)
            x, y, sw = parse_data(mouse_data)
            move_mouse(x, y, sw)


except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
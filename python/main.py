import serial
import uinput
import time
ser = serial.Serial('/dev/rfcomm0', 115200)

ax = [uinput.X_PIN, uinput.Y_PIN]
btns = [   uinput.KEY_A,
    uinput.KEY_B,
    uinput.KEY_C,
    uinput.KEY_D,
    uinput.KEY_E,
    uinput.KEY_F,
    uinput.KEY_G,
    uinput.KEY_H,
    uinput.KEY_I,
    uinput.KEY_J,
    uinput.KEY_K,
    uinput.KEY_L,
    uinput.KEY_M,
    uinput.KEY_N,
    uinput.KEY_O,
    uinput.KEY_Q,
    uinput.KEY_R,
    uinput.KEY_S,
    uinput.KEY_T,
    uinput.KEY_U,
    uinput.KEY_V,
    uinput.KEY_W,
    uinput.KEY_X,
    uinput.KEY_Y,
    uinput.KEY_Z,
    uinput.KEY_COMMA,
    uinput.KEY_DOT, 
    uinput.KEY_SLASH,]

device = uinput.Device(btns + ax)
btn_qntt = 27

def parse_data(data):
    axis = data[0]  # 0 for X, 1 for Y
    value = int.from_bytes(data[1:3], byteorder='little', signed=True)
    return axis, value

def emulate_controller(axis, value):
    if axis <btn_qntt:
        device.emit(btns[axis], value)
    else:
        device.emit(ax[axis - btn_qntt], value)

try:
    # sync package
    while True:
        print('Waiting for sync package...')
        while True:
            data = ser.read(3)
            axis, val= parse_data(data)
            emulate_controller(axis, val)

except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
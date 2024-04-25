import serial
import uinput
import time
#ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser = serial.Serial('/dev/rfcomm0', 115200, timeout=10)
ser.flushInput()

device = uinput.Device([
   uinput.KEY_A,
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
    uinput.KEY_SLASH,
    uinput.BTN_LEFT,
    uinput.REL_X,
    uinput.REL_Y,
])

def parse_data(data):
    axis = data[0]  # 0 for X, 1 for Y
    value = int.from_bytes(data[1:3], byteorder='big', signed=True)
    print(f"Received data: {data}")
    print(f"axis: {axis}, value: {value}")
    return axis, value


def move_mouse(axis, value):
    if axis == 0:    # X-axis
        device.emit(uinput.REL_X, value)
    elif axis == 1:  # Y-axis
        device.emit(uinput.REL_Y, value)


try:
    # sync package
    while True:
        
        print('Waiting for sync package...')
        while True:
            data = ser.read(1)
            print(data)
            if data == b'\xff':
                break

        data = ser.read(4)

        if len(data) < 4:
            continue
            
        print(data)
        if data[0] == 0:

            print(ascii(data))
            button = data[1]
            print(button)

            if chr(button) == 'A':
                print("a chegou")
                device.emit_click(uinput.KEY_A)
            if  chr(button)== 'B':
                print("b chegou")
                device.emit_click(uinput.KEY_B)
            if chr(button) == 'C':
                print("c chegou")
                device.emit_click(uinput.KEY_C)
            if chr(button) == 'D':
                print("d chegou")
                device.emit_click(uinput.KEY_D)
            if chr(button) == 'E':
                print("e chegou")
                device.emit_click(uinput.KEY_E)
            if chr(button) == 'F':
                print("f chegou")
                device.emit_click(uinput.KEY_F)
            if chr(button) == 'G':
                print("g chegou")
                device.emit_click(uinput.KEY_G)
            if chr(button) == 'H':
                print("h chegou")
                device.emit_click(uinput.KEY_H)
            if chr(button) == 'I':
                print("i chegou")
                device.emit_click(uinput.KEY_I)
            if chr(button) == 'J':
                print("j chegou")
                device.emit_click(uinput.KEY_J)
            if chr(button) == 'K':
                print("k chegou")
                device.emit_click(uinput.KEY_K)
            if chr(button) == 'L':
                print("l chegou")
                device.emit_click(uinput.KEY_L)
            if chr(button) == 'M':
                print("m chegou")
                device.emit_click(uinput.KEY_M)
            if chr(button) == 'N':
                print("n chegou")
                device.emit_click(uinput.KEY_N)
            if chr(button) == 'O':
                print("o chegou")
                device.emit_click(uinput.KEY_O)
            if chr(button) == 'Q':
                print("q chegou")
                device.emit_click(uinput.KEY_Q)
            if chr(button) == 'R':
                print("r chegou")
                device.emit_click(uinput.KEY_R)
            if chr(button) == 'S':
                print("s chegou")
                device.emit_click(uinput.KEY_S)
            if chr(button) == 'T':
                print("t chegou")
                device.emit_click(uinput.KEY_T)
            if chr(button) == 'U':
                print("u chegou")
                device.emit_click(uinput.KEY_U)
            if chr(button) == 'V':
                print("v chegou")
                device.emit_click(uinput.KEY_V)
            if chr(button) == 'W':
                print("w chegou")
                device.emit_click(uinput.KEY_W)
            if chr(button) == 'X':
                print("x chegou")
                device.emit_click(uinput.KEY_X)
            if chr(button) == 'Y':
                print("y chegou")
                device.emit_click(uinput.KEY_Y)
            if chr(button) == 'Z':
                print("z chegou")
                device.emit_click(uinput.KEY_Z)
            if chr(button) == ',':
                print(", chegou")
                device.emit_click(uinput.KEY_COMMA)
            if chr(button) == '.':
                print(". chegou")
                device.emit_click(uinput.KEY_DOT)
            if chr(button) == '/':
                print("/ chegou")
                device.emit_click(uinput.KEY_SLASH)
            if chr(button) == '|':
                print("left click")
                device.emit_click(uinput.BTN_LEFT)
            
        if data[0] == 1:
            #breakpoint()
            axis, valor = parse_data(data[1:4])     
            move_mouse(axis, valor)






    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
import serial
import uinput
import time
ser = serial.Serial('/dev/rfcomm0', 115200)

device = uinput.Device([
    uinput.BTN_PIN_SW,
    uinput.X_PIN,
    uinput.Y_PIN,
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

])

try:
    # sync package
    while True:
        print('Waiting for sync package...')
        while True:
            data = ser.read(2)
            print(ascii(data))
            axis = data[0]
            print(chr(axis))
            val = data[1]
            if chr(axis) == 'A|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_A, val)
            if chr(axis) == 'B|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_B, val)
            if chr(axis) == 'B|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_B, val)
            if chr(axis) == 'C|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_C, val)
            if chr(axis) == 'D|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_D, val)
            if chr(axis) == 'E|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_E, val)
            if chr(axis) == 'F|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_F, val)
            if chr(axis) == 'G|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_G, val)
            if chr(axis) == 'H|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_H, val)
            if chr(axis) == 'I|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_I, val)
            if chr(axis) == 'J|':
                print("a chegou")
                #device.emit_click(uinput.KEY_A)
                device.emit(uinput.KEY_J, val)
            if chr(axis) == 'K':
                print("K chegou")
                #device.emit_click(uinput.KEY_K)
                device.emit(uinput.KEY_K, val)
            if chr(axis) == 'L':
                print("L chegou")
                #device.emit_click(uinput.KEY_L)
                device.emit(uinput.KEY_L, val)
            if chr(axis) == 'M':
                print("M chegou")
                #device.emit_click(uinput.KEY_M)
                device.emit(uinput.KEY_M, val)
            if chr(axis) == 'N':
                print("N chegou")
                #device.emit_click(uinput.KEY_N)
                device.emit(uinput.KEY_N, val)
            if chr(axis) == 'O':
                print("O chegou")
                #device.emit_click(uinput.KEY_O)
                device.emit(uinput.KEY_O, val)
            if chr(axis) == 'Q':
                print("Q chegou")
                #device.emit_click(uinput.KEY_Q)
                device.emit(uinput.KEY_Q, val)
            if chr(axis) == 'R':
                print("R chegou")
                #device.emit_click(uinput.KEY_R)
                device.emit(uinput.KEY_R, val)
            if chr(axis) == 'T':
                print("T chegou")
                #device.emit_click(uinput.KEY_T)
                device.emit(uinput.KEY_T, val)
            if chr(axis) == 'U':
                print("U chegou")
                #device.emit_click(uinput.KEY_U)
                device.emit(uinput.KEY_U, val)
            if chr(axis) == 'V':
                print("V chegou")
                #device.emit_click(uinput.KEY_V)
                device.emit(uinput.KEY_V, val)
            if chr(axis) == 'W':
                print("W chegou")
                #device.emit_click(uinput.KEY_W)
                device.emit(uinput.KEY_W, val)
            if chr(axis) == 'X':
                print("X chegou")
                #device.emit_click(uinput.KEY_X)
                device.emit(uinput.KEY_X, val)
            if chr(axis) == 'Y':
                print("Y chegou")
                #device.emit_click(uinput.KEY_Y)
                device.emit(uinput.KEY_Y, val)
            if chr(axis) == 'Z':
                print("Z chegou")
                #device.emit_click(uinput.KEY_Z)
                device.emit(uinput.KEY_Z, val)
            if chr(axis) == 'S':
                print("S chegou")
                #device.emit_click(uinput.KEY_S)
                device.emit(uinput.KEY_S, val)

except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
from pymata4 import pymata4

board = pymata4.Pymata4()

input_pin_no = 3

board.set_pin_mode_digital_input(input_pin_no)

while True:

    try:
        pin_state = board.digital_read(input_pin_no)
        if pin_state[0] == 1:
            print("Button Pressed")
    except KeyboardInterrupt:
        break

board.shutdown()

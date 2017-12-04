from digitalio import DigitalInOut, Direction, Pull
import board
import time
  
button1 = DigitalInOut(board.BUTTON_A)
button1.direction = Direction.INPUT
button1.pull = Pull.DOWN

button2 = DigitalInOut(board.BUTTON_B)
button2.direction = Direction.INPUT
button2.pull = Pull.DOWN
  
while True:
    if button2.value:
        print("r")
    elif button1.value:
        print("p")
    else:
        print("s")
  
    time.sleep(0.01)
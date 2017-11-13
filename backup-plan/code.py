import touchio
import board
import time
  
touch1 = touchio.TouchIn(board.A1)
touch2 = touchio.TouchIn(board.A2)
  
while True:
    if touch1.value:
        print("r")
    elif touch2.value:
        print("p")
    else:
        print("s")
  
    time.sleep(0.01)
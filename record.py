import board
import audioio
import audiobusio
import digitalio
import time
import array
import math

buf = bytearray(16000)

print(3)
time.sleep(1)
print(2)
time.sleep(1)
print(1)
time.sleep(1)
#print("recording", time.monotonic())
print("recording")
#trigger = digitalio.DigitalInOut(board.A1)
#trigger.switch_to_output(value = True)
with audiobusio.PDMIn(board.MICROPHONE_CLOCK, board.MICROPHONE_DATA) as mic:
    mic.record(buf, len(buf))
#trigger.value = False
#print("done recording", time.monotonic())
print("done recording")

speaker_enable = digitalio.DigitalInOut(board.SPEAKER_ENABLE)
speaker_enable.switch_to_output(value=True)
time.sleep(1)

#trigger.value = True
#print("playback", time.monotonic())
print("playback")
with audioio.AudioOut(board.SPEAKER, buf) as speaker:
    speaker.frequency = 8000
    speaker.play()
    while speaker.playing:
        pass

#trigger.value = False
